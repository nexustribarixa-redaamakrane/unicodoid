//! ShiftModal-16, a deliberately non-Unicode modal example system.
//!
//! Base mode:
//!   0x00..0x7F  -> values 0x0000..0x007F, except the three control bytes below.
//!   0x1B b      -> value 0x0100 | b   (escape; consumes the following byte)
//!   0x0E        -> switch to shifted mode (emits no character)
//!   0x0F        -> switch to base mode    (emits no character)
//!
//! Shifted mode:
//!   0x0F        -> switch to base mode    (emits no character)
//!   0x1B b      -> value 0x0100 | b      (escape; mode is unchanged)
//!   b           -> value 0x4000 | b      (0x0E is ordinary data here)
//!
//! Reserved / unrepresentable native values are `0x000E`, `0x000F`, `0x001B`
//! in base mode and `0x400F`, `0x401B` in shifted mode.  The encoder reports
//! [`Status::UnmappableChar`] for them.
//!
//! The system intentionally has no Unicode profile: it exists to prove that
//! the codec, string, and transcoding layers do not assume a Unicode
//! relationship.

use crate::system::EncodingSystem;
use crate::types::{CharModel, CharValue, CompatibilityLevel, DecodeOutcome, Endianness, Status};

const ESC: u8 = 0x1B;
const SO: u8 = 0x0E;
const SI: u8 = 0x0F;

const BASE: u8 = 0;
const SHIFTED: u8 = 1;

fn mode_of(state: &[u8]) -> u8 {
    if state.is_empty() { BASE } else { state[0] }
}

fn set_mode(state: &mut [u8], mode: u8) {
    if let Some(slot) = state.first_mut() {
        *slot = mode;
    }
}

fn base_reserved(byte: u8) -> bool {
    byte == ESC || byte == SO || byte == SI
}

/// ShiftModal-16 encoding system.
#[derive(Debug, Default, Clone, Copy)]
pub struct ShiftModal16;

/// Process-wide ShiftModal-16 instance.
pub static SHIFTMODAL16: ShiftModal16 = ShiftModal16;

/// Borrow the process-wide ShiftModal-16 system.
pub fn system() -> &'static dyn EncodingSystem {
    &SHIFTMODAL16
}

impl EncodingSystem for ShiftModal16 {
    fn id(&self) -> &'static str {
        "org.unicodoid.shiftmodal16"
    }

    fn name(&self) -> &'static str {
        "ShiftModal-16"
    }

    fn version(&self) -> &'static str {
        "1.0.0"
    }

    fn author(&self) -> &'static str {
        "Unicodoid Reference Systems"
    }

    fn description(&self) -> &'static str {
        "Modal, stateful 16-bit example system with no Unicode profile"
    }

    fn code_unit_bits(&self) -> u32 {
        8
    }

    fn min_bytes_per_char(&self) -> usize {
        1
    }

    fn max_bytes_per_char(&self) -> usize {
        2
    }

    fn model(&self) -> CharModel {
        CharModel::ModalStateful
    }

    fn endianness(&self) -> Endianness {
        Endianness::None
    }

    fn compatibility_level(&self) -> CompatibilityLevel {
        CompatibilityLevel::None
    }

    fn max_char_value(&self) -> CharValue {
        0x40FF
    }

    fn replacement_char(&self) -> CharValue {
        0x003F
    }

    fn state_size(&self) -> usize {
        1
    }

    fn encode_char(
        &self,
        state: &mut [u8],
        ch: CharValue,
        destination: &mut [u8],
    ) -> Result<usize, Status> {
        let mut mode = mode_of(state);

        if ch <= 0x7F {
            let byte = ch as u8;
            if base_reserved(byte) {
                return Err(Status::UnmappableChar);
            }

            if mode == SHIFTED {
                if destination.len() < 2 {
                    return Err(Status::BufferTooSmall);
                }
                destination[0] = SI;
                destination[1] = byte;
                mode = BASE;
                set_mode(state, mode);
                Ok(2)
            } else {
                if destination.is_empty() {
                    return Err(Status::BufferTooSmall);
                }
                destination[0] = byte;
                set_mode(state, mode);
                Ok(1)
            }
        } else if (0x0100..=0x01FF).contains(&ch) {
            if destination.len() < 2 {
                return Err(Status::BufferTooSmall);
            }
            destination[0] = ESC;
            destination[1] = (ch & 0xFF) as u8;
            set_mode(state, mode);
            Ok(2)
        } else if (0x4000..=0x40FF).contains(&ch) {
            let byte = (ch & 0xFF) as u8;
            // In shifted mode SI and ESC are controls, not data.
            if byte == SI || byte == ESC {
                return Err(Status::UnmappableChar);
            }

            if mode == BASE {
                if destination.len() < 2 {
                    return Err(Status::BufferTooSmall);
                }
                destination[0] = SO;
                destination[1] = byte;
                mode = SHIFTED;
                set_mode(state, mode);
                Ok(2)
            } else {
                if destination.is_empty() {
                    return Err(Status::BufferTooSmall);
                }
                destination[0] = byte;
                set_mode(state, mode);
                Ok(1)
            }
        } else {
            Err(Status::UnmappableChar)
        }
    }

    fn decode_char(&self, state: &mut [u8], source: &[u8]) -> Result<DecodeOutcome, Status> {
        if source.is_empty() {
            return Err(Status::IncompleteSequence);
        }

        let mut mode = mode_of(state);
        let saved = mode;
        let mut consumed = 0usize;

        while consumed < source.len() {
            let byte = source[consumed];

            if byte == ESC {
                if consumed + 1 >= source.len() {
                    set_mode(state, saved);
                    return Err(Status::IncompleteSequence);
                }
                let ch = 0x0100u64 | u64::from(source[consumed + 1]);
                set_mode(state, mode);
                return Ok(DecodeOutcome::Char {
                    ch,
                    read: consumed + 2,
                });
            }

            if byte == SI {
                mode = BASE;
                consumed += 1;
                continue;
            }

            if mode == SHIFTED {
                let ch = 0x4000u64 | u64::from(byte);
                set_mode(state, mode);
                return Ok(DecodeOutcome::Char {
                    ch,
                    read: consumed + 1,
                });
            }

            if byte == SO {
                mode = SHIFTED;
                consumed += 1;
                continue;
            }

            set_mode(state, mode);
            return Ok(DecodeOutcome::Char {
                ch: u64::from(byte),
                read: consumed + 1,
            });
        }

        // Only shift controls were present: the state change is committed.
        set_mode(state, mode);
        Ok(DecodeOutcome::NoCharacter { read: consumed })
    }

    fn byte_count(&self, chars: &[CharValue]) -> Result<usize, Status> {
        let mut mode = BASE;
        let mut total = 0usize;

        for &ch in chars {
            if ch <= 0x7F {
                if base_reserved(ch as u8) {
                    return Err(Status::UnmappableChar);
                }
                if mode == SHIFTED {
                    total += 2;
                    mode = BASE;
                } else {
                    total += 1;
                }
            } else if (0x0100..=0x01FF).contains(&ch) {
                total += 2;
            } else if (0x4000..=0x40FF).contains(&ch) {
                let byte = (ch & 0xFF) as u8;
                if byte == SI || byte == ESC {
                    return Err(Status::UnmappableChar);
                }
                if mode == BASE {
                    total += 2;
                    mode = SHIFTED;
                } else {
                    total += 1;
                }
            } else {
                return Err(Status::UnmappableChar);
            }
        }

        Ok(total)
    }
}
