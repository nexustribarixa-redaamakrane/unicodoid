//! Stateful stream facade over an encoding system.

use crate::system::EncodingSystem;
use crate::types::{CharValue, DecodeError, DecodeOutcome, Status};

/// A codec owns one instance of the system's per-stream state.
pub struct Codec<'a> {
    system: &'a dyn EncodingSystem,
    state: Vec<u8>,
}

impl<'a> Codec<'a> {
    pub fn new(system: &'a dyn EncodingSystem) -> Self {
        Self {
            system,
            state: system.create_state(),
        }
    }

    pub fn system(&self) -> &'a dyn EncodingSystem {
        self.system
    }

    /// Reset the codec to its initial state.
    pub fn reset(&mut self) {
        self.system.reset_state(&mut self.state);
    }

    pub fn encode_char(&mut self, ch: CharValue, destination: &mut [u8]) -> Result<usize, Status> {
        self.system.encode_char(&mut self.state, ch, destination)
    }

    pub fn decode_char(&mut self, source: &[u8]) -> Result<DecodeOutcome, Status> {
        self.system.decode_char(&mut self.state, source)
    }

    pub fn validate(&mut self, source: &[u8]) -> Result<(), DecodeError> {
        self.system.validate(&mut self.state, source)
    }

    pub fn count_chars(&mut self, source: &[u8]) -> Result<usize, DecodeError> {
        self.system.char_count(&mut self.state, source)
    }

    /// Encode a sequence as one stream.
    pub fn encode(&mut self, chars: &[CharValue], destination: &mut [u8]) -> Result<usize, Status> {
        let mut offset = 0usize;

        for &ch in chars {
            let written = self.encode_char(ch, &mut destination[offset..])?;
            offset += written;
        }

        Ok(offset)
    }

    /// Decode a complete byte stream into character values.
    ///
    /// A modal control-only fragment advances the input but contributes no
    /// character.
    pub fn decode(
        &mut self,
        source: &[u8],
        destination: &mut [CharValue],
    ) -> Result<usize, Status> {
        let mut offset = 0usize;
        let mut count = 0usize;

        while offset < source.len() {
            match self.decode_char(&source[offset..])? {
                DecodeOutcome::Char { ch, read } => {
                    if read == 0 {
                        return Err(Status::Internal);
                    }
                    if count >= destination.len() {
                        return Err(Status::BufferTooSmall);
                    }
                    destination[count] = ch;
                    count += 1;
                    offset += read;
                }
                DecodeOutcome::NoCharacter { read } => {
                    if read == 0 {
                        return Err(Status::Internal);
                    }
                    offset += read;
                }
            }
        }

        Ok(count)
    }

    /// Decode an entire byte stream into a newly allocated vector.
    pub fn decode_all(&mut self, source: &[u8]) -> Result<Vec<CharValue>, Status> {
        let mut result = Vec::new();
        let mut offset = 0usize;

        while offset < source.len() {
            match self.decode_char(&source[offset..])? {
                DecodeOutcome::Char { ch, read } => {
                    if read == 0 {
                        return Err(Status::Internal);
                    }
                    result.push(ch);
                    offset += read;
                }
                DecodeOutcome::NoCharacter { read } => {
                    if read == 0 {
                        return Err(Status::Internal);
                    }
                    offset += read;
                }
            }
        }

        Ok(result)
    }

    /// Encode an entire character sequence into a newly allocated vector.
    pub fn encode_all(&mut self, chars: &[CharValue]) -> Result<Vec<u8>, Status> {
        let mut result = Vec::new();
        let mut buffer = vec![0u8; self.system.max_bytes_per_char().max(16)];

        for &ch in chars {
            let written = self.encode_char(ch, &mut buffer)?;
            result.extend_from_slice(&buffer[..written]);
        }

        Ok(result)
    }
}
