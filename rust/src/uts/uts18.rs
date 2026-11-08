//! UTS #18 Unicode Regular Expressions (Revision 25, RL1.1 level).
//!
//! Unicode-aware `\p{...}` / `\P{...}` property escapes over the generated
//! tables.  Full pattern compilation is out of scope.

/// Kind of property match.
pub mod kind {
    pub const NONE: i32 = 0;
    pub const GENERAL_CATEGORY: i32 = 1;
    pub const SCRIPT: i32 = 2;
    pub const BINARY: i32 = 3;
    pub const EAW: i32 = 4;
    pub const INVALID: i32 = 5;
}

/// Length of a `\p{...}` escape at `pat`, or 0.
pub fn property_escape_length(pat: &str) -> usize {
    if !(pat.starts_with(r"\p{") || pat.starts_with(r"\P{")) {
        return 0;
    }
    match pat.find('}') {
        Some(close) => close + 1,
        None => 0,
    }
}

/// Evaluate `cp` against the escape at `pat`.
pub fn property_match(pat: &str, cp: u32) -> i32 {
    let (neg, name_start) = if let Some(rest) = pat.strip_prefix(r"\p{") {
        (false, rest)
    } else if let Some(rest) = pat.strip_prefix(r"\P{") {
        (true, rest)
    } else {
        return kind::NONE;
    };
    let Some(close) = name_start.find('}') else {
        return kind::INVALID;
    };
    let name = &name_start[..close];
    if name.is_empty() {
        return kind::INVALID;
    }
    let matched = match_property(name, cp);
    match matched {
        kind::INVALID => kind::INVALID,
        kind::NONE if neg => kind::BINARY,
        kind::NONE => kind::NONE,
        _ if neg => kind::NONE,
        _ => matched,
    }
}

fn match_property(name: &str, cp: u32) -> i32 {
    use crate::generated::ucd_data as data;
    use crate::ucd;

    if matches!(name, "Alphabetic" | "Alpha") {
        return if ucd::has_property(cp, data::PROP_ALPHABETIC) { kind::BINARY } else { kind::NONE };
    }
    if name == "Uppercase" {
        return if ucd::has_property(cp, data::PROP_UPPERCASE) { kind::BINARY } else { kind::NONE };
    }
    if name == "Lowercase" {
        return if ucd::has_property(cp, data::PROP_LOWERCASE) { kind::BINARY } else { kind::NONE };
    }
    if name == "Emoji" {
        return if ucd::has_property(cp, data::PROP_EMOJI) { kind::BINARY } else { kind::NONE };
    }
    let gc = ucd::general_category(cp);
    let gc_short = match gc {
        ucd::gc::LU => "Lu",
        ucd::gc::LL => "Ll",
        _ => "",
    };
    if name == gc_short || name == "Uppercase_Letter" && gc == ucd::gc::LU
        || name == "Lowercase_Letter" && gc == ucd::gc::LL
    {
        return kind::GENERAL_CATEGORY;
    }
    if matches!(name, "Lu" | "Ll" | "Letter" | "Uppercase_Letter" | "Lowercase_Letter") {
        return kind::NONE;
    }
    if ucd::script_name(ucd::script(cp)) == Some(name) {
        return kind::SCRIPT;
    }
    for i in 0..ucd::script_count() {
        if ucd::script_name(i as u16) == Some(name) {
            return kind::NONE;
        }
    }
    let ea_name = match ucd::east_asian_width(cp) {
        ucd::eaw::W => "Wide",
        ucd::eaw::NA => "Narrow",
        ucd::eaw::N => "Neutral",
        ucd::eaw::A => "Ambiguous",
        ucd::eaw::H => "Halfwidth",
        ucd::eaw::F => "Fullwidth",
        _ => "",
    };
    if name == ea_name {
        return kind::EAW;
    }
    kind::INVALID
}

/// True when `pat` is a valid property escape.
pub fn is_valid_escape(pat: &str) -> bool {
    property_escape_length(pat) != 0
}
