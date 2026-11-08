//! UTR #23 The Unicode Character Property Model (Revision 15).

/// Number of supported binary properties.
pub const BINARY_PROPERTY_COUNT: usize = 41;

/// Property-kind classification.
pub fn property_kind(name: &str) -> u8 {
    match name {
        "General_Category" => 1,
        "Numeric_Type" => 2,
        "Unicode_1_Name" => 3,
        _ => 0,
    }
}
