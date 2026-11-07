//! Explicit transcoding between independent encoding systems.
//!
//! Two systems are bridged in exactly one of these ways:
//!
//! * they are the same system (byte copy after validation);
//! * both declare Unicode profiles (route through Unicode scalar values);
//! * the caller supplies a bridge closure for non-Unicode value mapping.
//!
//! There is no implicit identity mapping between unrelated systems.

use crate::codec::Codec;
use crate::system::EncodingSystem;
use crate::text_string::TextString;
use crate::types::{CharValue, DecodeOutcome, Status, TranscodePolicy};

/// Explicit value bridge for systems that do not share a Unicode profile.
///
/// The closure maps one native source character value to one native destination
/// character value, returning [`Status::UnmappableChar`] when no mapping exists.
pub type BridgeFn<'a> = Box<
    dyn Fn(&dyn EncodingSystem, CharValue, &dyn EncodingSystem) -> Result<CharValue, Status> + 'a,
>;

/// Substitute-output callback for [`TranscodePolicy::Callback`].
///
/// The optional character is `None` when the source byte stream could not be
/// decoded at all.  Returning `None` aborts transcoding with
/// [`Status::UnmappableChar`].
pub type SubstitutionFn<'a> = Box<
    dyn Fn(&dyn EncodingSystem, &dyn EncodingSystem, Option<CharValue>) -> Option<Vec<u8>> + 'a,
>;

/// Transcoding configuration.
pub struct TranscodeConfig<'a> {
    pub policy: TranscodePolicy,
    /// `None` uses the destination system's replacement character.
    pub custom_replacement: Option<CharValue>,
    pub callback: Option<SubstitutionFn<'a>>,
    pub bridge: Option<BridgeFn<'a>>,
}

impl<'a> Default for TranscodeConfig<'a> {
    fn default() -> Self {
        Self {
            policy: TranscodePolicy::Strict,
            custom_replacement: None,
            callback: None,
            bridge: None,
        }
    }
}

impl<'a> TranscodeConfig<'a> {
    pub fn new() -> Self {
        Self::default()
    }
}

fn same_system(a: &dyn EncodingSystem, b: &dyn EncodingSystem) -> bool {
    core::ptr::eq(a, b) || a.id() == b.id()
}

/// Transcode one native character value into another system.
///
/// This performs value mapping only; it does not encode bytes.
pub fn transcode_char(
    src_system: &dyn EncodingSystem,
    src_ch: CharValue,
    dst_system: &dyn EncodingSystem,
    config: &TranscodeConfig<'_>,
) -> Result<CharValue, Status> {
    if same_system(src_system, dst_system) {
        return Ok(src_ch);
    }

    if src_system.has_unicode_profile() && dst_system.has_unicode_profile() {
        let src_profile = src_system.unicode_profile().ok_or(Status::Unsupported)?;
        let dst_profile = dst_system.unicode_profile().ok_or(Status::Unsupported)?;

        let scalar = src_profile
            .try_to_unicode(src_ch)
            .ok_or(Status::UnmappableChar)?;
        return dst_profile
            .try_from_unicode(scalar)
            .ok_or(Status::UnmappableChar);
    }

    if let Some(bridge) = &config.bridge {
        return bridge(src_system, src_ch, dst_system);
    }

    // No implicit identity mapping between unrelated systems.
    Err(Status::UnmappableChar)
}

fn encode_into(codec: &mut Codec<'_>, ch: CharValue, out: &mut Vec<u8>) -> Result<(), Status> {
    let mut buffer = vec![0u8; codec.system().max_bytes_per_char().max(8)];
    let written = codec.encode_char(ch, &mut buffer)?;
    out.extend_from_slice(&buffer[..written]);
    Ok(())
}

fn emit_substitute(
    config: &TranscodeConfig<'_>,
    src_system: &dyn EncodingSystem,
    dst_system: &dyn EncodingSystem,
    ch: Option<CharValue>,
    out: &mut Vec<u8>,
) -> Result<bool, Status> {
    if config.policy != TranscodePolicy::Callback {
        return Ok(false);
    }

    let Some(callback) = &config.callback else {
        return Ok(false);
    };

    match callback(src_system, dst_system, ch) {
        Some(bytes) => {
            out.extend_from_slice(&bytes);
            Ok(true)
        }
        None => Err(Status::UnmappableChar),
    }
}

fn emit_replacement(
    config: &TranscodeConfig<'_>,
    dst_system: &dyn EncodingSystem,
    codec: &mut Codec<'_>,
    out: &mut Vec<u8>,
) -> Result<(), Status> {
    let replacement = config
        .custom_replacement
        .unwrap_or_else(|| dst_system.replacement_char());
    encode_into(codec, replacement, out)
}

fn handle_unmappable(
    config: &TranscodeConfig<'_>,
    src_system: &dyn EncodingSystem,
    dst_system: &dyn EncodingSystem,
    src_ch: Option<CharValue>,
    codec: &mut Codec<'_>,
    out: &mut Vec<u8>,
) -> Result<(), Status> {
    match config.policy {
        TranscodePolicy::Strict => Err(Status::UnmappableChar),
        TranscodePolicy::Skip => Ok(()),
        TranscodePolicy::Replace => emit_replacement(config, dst_system, codec, out),
        TranscodePolicy::Callback => {
            if emit_substitute(config, src_system, dst_system, src_ch, out)? {
                Ok(())
            } else {
                emit_replacement(config, dst_system, codec, out)
            }
        }
    }
}

/// Transcode a raw byte buffer into a newly allocated destination buffer.
pub fn transcode(
    src_system: &dyn EncodingSystem,
    input: &[u8],
    dst_system: &dyn EncodingSystem,
    config: &TranscodeConfig<'_>,
) -> Result<Vec<u8>, Status> {
    if input.is_empty() {
        return Ok(Vec::new());
    }

    // Same system: validate once, then copy the byte stream unchanged.
    if same_system(src_system, dst_system) {
        let mut check = Codec::new(src_system);
        check.validate(input).map_err(|error| error.status)?;
        return Ok(input.to_vec());
    }

    let mut src_codec = Codec::new(src_system);
    let mut dst_codec = Codec::new(dst_system);
    let mut out = Vec::new();
    let mut offset = 0usize;

    while offset < input.len() {
        match src_codec.decode_char(&input[offset..]) {
            Ok(DecodeOutcome::NoCharacter { read }) => {
                if read == 0 {
                    return Err(Status::Internal);
                }
                offset += read;
            }
            Ok(DecodeOutcome::Char { ch, read }) => {
                if read == 0 {
                    return Err(Status::Internal);
                }
                offset += read;

                match transcode_char(src_system, ch, dst_system, config) {
                    Ok(dst_ch) => encode_into(&mut dst_codec, dst_ch, &mut out)?,
                    Err(status) => {
                        // Strict mode preserves the exact mapping failure, as the
                        // C and C# SDKs do.
                        if config.policy == TranscodePolicy::Strict {
                            return Err(status);
                        }
                        handle_unmappable(
                            config,
                            src_system,
                            dst_system,
                            Some(ch),
                            &mut dst_codec,
                            &mut out,
                        )?;
                    }
                }
            }
            Err(status) => {
                if config.policy == TranscodePolicy::Strict {
                    return Err(status);
                }

                offset += 1;
                match config.policy {
                    TranscodePolicy::Skip => continue,
                    TranscodePolicy::Replace => {
                        emit_replacement(config, dst_system, &mut dst_codec, &mut out)?;
                    }
                    TranscodePolicy::Callback => {
                        if !emit_substitute(config, src_system, dst_system, None, &mut out)? {
                            emit_replacement(config, dst_system, &mut dst_codec, &mut out)?;
                        }
                    }
                    TranscodePolicy::Strict => unreachable!(),
                }
            }
        }
    }

    Ok(out)
}

/// Transcode a string into a newly allocated destination string.
pub fn transcode_string<'a>(
    src: &TextString<'_>,
    dst_system: &'a dyn EncodingSystem,
    config: &TranscodeConfig<'_>,
) -> Result<TextString<'a>, Status> {
    let bytes = transcode(src.system(), src.bytes(), dst_system, config)?;
    TextString::from_bytes(dst_system, &bytes)
}
