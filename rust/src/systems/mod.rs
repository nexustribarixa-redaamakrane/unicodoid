//! Built-in reference encoding systems.
//!
//! Two of the systems are deliberately independent of Unicode:
//! [`custom_conlang`] (a stateless 14-bit constructed script) and
//! [`shiftmodal16`] (a stateful modal example).  They demonstrate that the
//! codec, string, and transcoding layers do not assume a Unicode relationship.

pub mod ascii;
pub mod custom_conlang;
pub mod shiftmodal16;
pub mod utf16;
pub mod utf32;
pub mod utf8;
