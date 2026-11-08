//! UAX #42 Unicode Character Database in XML (Unicode 18.0.0, Revision 40).
//!
//! Distribution-format helpers: canonical format identifier and a
//! single-character XML record writer.

use crate::ucd;

/// Format identifier.
pub const FORMAT: &str = "UCDxml";

/// True when `cp` is representable in UCD XML.
pub fn is_representable(cp: u32) -> bool {
    cp <= 0x10FFFF && !(0xD800..=0xDFFF).contains(&cp)
}

/// Emit a `<char>` record for `cp`.
pub fn char_record(cp: u32) -> String {
    format!(
        "<char cp=\"{cp:04X}\" GC=\"{}\" Bidi_C=\"{}\" sc=\"{}\"/>\n",
        ucd::general_category(cp),
        ucd::bidi_class(cp),
        ucd::script_name(ucd::script(cp)).unwrap_or("Unknown"),
    )
}
