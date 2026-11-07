//! Length-bounded byte string parameterized by an encoding system.

use crate::codec::Codec;
use crate::system::EncodingSystem;
use crate::types::{CharValue, Status};

/// A byte string that never depends on null terminators and may contain
/// embedded zero bytes.
///
/// A string keeps its own codec state so that appending characters to a modal
/// encoding produces one coherent stream.
#[derive(Clone)]
pub struct TextString<'a> {
    system: &'a dyn EncodingSystem,
    bytes: Vec<u8>,
    state: Vec<u8>,
}

impl<'a> TextString<'a> {
    pub fn new(system: &'a dyn EncodingSystem, initial_capacity: usize) -> Self {
        Self {
            system,
            bytes: Vec::with_capacity(initial_capacity),
            state: system.create_state(),
        }
    }

    /// Create an owned string by encoding a sequence of character values.
    pub fn from_chars(system: &'a dyn EncodingSystem, chars: &[CharValue]) -> Result<Self, Status> {
        let required = system.byte_count(chars)?;
        let mut string = Self::new(system, required.max(16));

        for &ch in chars {
            string.append_char(ch)?;
        }

        Ok(string)
    }

    /// Create an owned string from an already-encoded byte sequence.
    pub fn from_bytes(system: &'a dyn EncodingSystem, bytes: &[u8]) -> Result<Self, Status> {
        let mut state = system.create_state();
        system
            .validate(&mut state, bytes)
            .map_err(|error| error.status)?;

        let mut string = Self::new(system, bytes.len().max(16));
        string.bytes.extend_from_slice(bytes);
        // Keep the state reached by the validated byte stream so that a later
        // append continues the same modal stream.
        string.state = state;
        Ok(string)
    }

    pub fn system(&self) -> &'a dyn EncodingSystem {
        self.system
    }

    /// Raw bytes.
    pub fn bytes(&self) -> &[u8] {
        &self.bytes
    }

    /// Length in bytes.
    pub fn len(&self) -> usize {
        self.bytes.len()
    }

    pub fn is_empty(&self) -> bool {
        self.bytes.is_empty()
    }

    /// Append one character, updating the persistent codec state.
    pub fn append_char(&mut self, ch: CharValue) -> Result<(), Status> {
        let start = self.bytes.len();
        let reserve = self.system.max_bytes_per_char().max(1);
        self.bytes.resize(start + reserve, 0);

        match self
            .system
            .encode_char(&mut self.state, ch, &mut self.bytes[start..])
        {
            Ok(written) => {
                self.bytes.truncate(start + written);
                Ok(())
            }
            Err(status) => {
                self.bytes.truncate(start);
                Err(status)
            }
        }
    }

    /// Append already-validated bytes without interpreting them.
    ///
    /// The bytes are validated against a copy of the current state and the
    /// state is committed only on success.
    pub fn append_bytes(&mut self, bytes: &[u8]) -> Result<(), Status> {
        if bytes.is_empty() {
            return Ok(());
        }

        let mut trial = self.state.clone();
        self.system
            .validate(&mut trial, bytes)
            .map_err(|error| error.status)?;

        self.bytes.extend_from_slice(bytes);
        self.state = trial;
        Ok(())
    }

    /// Count characters by decoding from the initial state.
    pub fn char_len(&self) -> Result<usize, Status> {
        if self.bytes.is_empty() {
            return Ok(0);
        }

        let mut codec = Codec::new(self.system);
        codec.count_chars(&self.bytes).map_err(|error| error.status)
    }

    /// Retrieve a character by zero-based character index.
    pub fn char_at(&self, char_index: usize) -> Result<CharValue, Status> {
        let mut codec = Codec::new(self.system);
        let mut index = 0usize;
        let mut offset = 0usize;

        while offset < self.bytes.len() {
            match codec.decode_char(&self.bytes[offset..])? {
                crate::types::DecodeOutcome::Char { ch, read } => {
                    if read == 0 {
                        return Err(Status::Internal);
                    }
                    if index == char_index {
                        return Ok(ch);
                    }
                    offset += read;
                    index += 1;
                }
                crate::types::DecodeOutcome::NoCharacter { read } => {
                    if read == 0 {
                        return Err(Status::Internal);
                    }
                    offset += read;
                }
            }
        }

        Err(Status::NotFound)
    }

    /// Reset the string's persistent codec state to its initial value.
    pub fn reset_state(&mut self) {
        self.system.reset_state(&mut self.state);
    }

    /// Decode the entire string into character values.
    pub fn to_chars(&self) -> Result<Vec<CharValue>, Status> {
        let mut codec = Codec::new(self.system);
        codec.decode_all(&self.bytes)
    }

    pub fn as_bytes(&self) -> &[u8] {
        &self.bytes
    }

    pub fn into_bytes(self) -> Vec<u8> {
        self.bytes
    }
}

impl PartialEq for TextString<'_> {
    fn eq(&self, other: &Self) -> bool {
        self.bytes == other.bytes
    }
}

impl Eq for TextString<'_> {}

impl core::fmt::Debug for TextString<'_> {
    fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        f.debug_struct("TextString")
            .field("system", &self.system.id())
            .field("length", &self.bytes.len())
            .finish()
    }
}
