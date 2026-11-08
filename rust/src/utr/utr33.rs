//! UTR #33 Conformance Model (Revision 7).

/// Conformance level vocabulary.
pub fn level_name(level: u8) -> &'static str {
    match level {
        1 => "Well_Formed",
        2 => "Valid",
        3 => "Stable",
        _ => "Unspecified",
    }
}
