//! Encoding-system contract and shared validation loops.
//!
//! Operations are state-aware.  Stateless systems ignore the state slice, while
//! modal systems receive a per-stream buffer created by
//! [`EncodingSystem::create_state`].  The byte-slice representation mirrors the
//! C SDK and keeps state opaque to the codec layer.

use crate::types::{
    CharModel, CharValue, CompatibilityLevel, DecodeError, DecodeOutcome, Endianness, Status,
};
use crate::unicode::UnicodeProfile;

/// Contract implemented by all independent encoding systems.
pub trait EncodingSystem: Send + Sync {
    fn id(&self) -> &'static str;
    fn name(&self) -> &'static str;
    fn version(&self) -> &'static str;
    fn author(&self) -> &'static str;
    fn description(&self) -> &'static str;

    fn code_unit_bits(&self) -> u32;
    fn min_bytes_per_char(&self) -> usize;
    fn max_bytes_per_char(&self) -> usize;
    fn model(&self) -> CharModel;
    fn endianness(&self) -> Endianness;
    fn compatibility_level(&self) -> CompatibilityLevel;
    fn max_char_value(&self) -> CharValue;
    fn replacement_char(&self) -> CharValue;

    /// Optional Unicode profile; `None` means no Unicode relationship.
    fn unicode_profile(&self) -> Option<&'static UnicodeProfile> {
        None
    }

    /// True when the system declares and provides a usable Unicode profile.
    fn has_unicode_profile(&self) -> bool {
        self.unicode_profile().is_some() && self.compatibility_level() != CompatibilityLevel::None
    }

    /// Size in bytes of one codec state object; zero for stateless systems.
    fn state_size(&self) -> usize {
        0
    }

    /// True when the system requires per-stream codec state.
    fn is_stateful(&self) -> bool {
        self.state_size() > 0
    }

    /// Create a fresh zeroed state buffer.
    fn create_state(&self) -> Vec<u8> {
        vec![0; self.state_size()]
    }

    /// Reset a state buffer to its initial value.
    fn reset_state(&self, state: &mut [u8]) {
        state.fill(0);
    }

    /// Encode one character value, updating state.
    fn encode_char(
        &self,
        state: &mut [u8],
        ch: CharValue,
        destination: &mut [u8],
    ) -> Result<usize, Status>;

    /// Decode one character value, updating state.
    fn decode_char(&self, state: &mut [u8], source: &[u8]) -> Result<DecodeOutcome, Status>;

    /// Exact number of bytes needed to encode a sequence of characters.
    ///
    /// This is a pure sizing operation and must not mutate state.
    fn byte_count(&self, chars: &[CharValue]) -> Result<usize, Status>;

    /// Validate a complete byte buffer with the supplied state.
    fn validate(&self, state: &mut [u8], source: &[u8]) -> Result<(), DecodeError> {
        let mut offset = 0usize;

        while offset < source.len() {
            match self.decode_char(state, &source[offset..]) {
                Ok(DecodeOutcome::Char { read, .. }) => {
                    if read == 0 {
                        return Err(DecodeError::new(Status::Internal, offset));
                    }
                    offset += read;
                }
                Ok(DecodeOutcome::NoCharacter { read }) => {
                    if read == 0 {
                        return Err(DecodeError::new(Status::Internal, offset));
                    }
                    offset += read;
                }
                Err(status) => return Err(DecodeError::new(status, offset)),
            }
        }

        Ok(())
    }

    /// Count characters in a complete byte buffer with the supplied state.
    fn char_count(&self, state: &mut [u8], source: &[u8]) -> Result<usize, DecodeError> {
        let mut offset = 0usize;
        let mut count = 0usize;

        while offset < source.len() {
            match self.decode_char(state, &source[offset..]) {
                Ok(DecodeOutcome::Char { read, .. }) => {
                    if read == 0 {
                        return Err(DecodeError::new(Status::Internal, offset));
                    }
                    offset += read;
                    count += 1;
                }
                Ok(DecodeOutcome::NoCharacter { read }) => {
                    if read == 0 {
                        return Err(DecodeError::new(Status::Internal, offset));
                    }
                    offset += read;
                }
                Err(status) => return Err(DecodeError::new(status, offset)),
            }
        }

        Ok(count)
    }
}

/// Convert a native character value to a Unicode scalar through a system's
/// profile, or report that the system has no usable relationship with Unicode.
pub fn native_to_scalar(system: &dyn EncodingSystem, ch: CharValue) -> Result<u32, Status> {
    let profile = system.unicode_profile().ok_or(Status::Unsupported)?;
    if !system.has_unicode_profile() {
        return Err(Status::Unsupported);
    }
    profile.try_to_unicode(ch).ok_or(Status::UnmappableChar)
}

/// Convert a Unicode scalar to a native character value through a system's
/// profile, or report that the system has no usable relationship with Unicode.
pub fn scalar_to_native(system: &dyn EncodingSystem, scalar: u32) -> Result<CharValue, Status> {
    let profile = system.unicode_profile().ok_or(Status::Unsupported)?;
    if !system.has_unicode_profile() {
        return Err(Status::Unsupported);
    }
    profile
        .try_from_unicode(scalar)
        .ok_or(Status::UnmappableChar)
}
