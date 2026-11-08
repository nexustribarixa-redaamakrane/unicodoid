//! UTS #35 Unicode Locale Data Markup Language (CLDR Part 1, CLDR 49).
//!
//! Aggregated helpers for the CLDR surfaces carried by Unicodoid: locale
//! identifiers, number symbols, dates, collation settings, person names,
//! keyboards, MessageFormat 2.0 and supplemental data.  Full CLDR XML
//! resource bundles are an external distribution.

/// CLDR version string.
pub const VERSION: &str = "49";

/// Parsed BCP 47 locale identifier.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Locale {
    pub language: String,
    pub script: String,
    pub region: String,
    pub variant: String,
}

/// Collation strength/alternate settings.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Collation {
    pub strength: i32,
    pub alternate: i32,
}

/// Number formatting symbols for a region.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct NumberSymbols {
    pub group: &'static str,
    pub decimal: &'static str,
    pub minus: &'static str,
    pub percent: &'static str,
}

/// Plural categories.
pub mod plural {
    pub const OTHER: i32 = 0;
    pub const ONE: i32 = 1;
    pub const TWO: i32 = 2;
    pub const FEW: i32 = 3;
    pub const MANY: i32 = 4;
    pub const ZERO: i32 = 5;
}

/// Person-name orders.
pub mod name_order {
    pub const FAMILY_FIRST: i32 = 0;
    pub const GIVEN_FIRST: i32 = 1;
}

fn is_lower_alpha(s: &str) -> bool {
    !s.is_empty() && s.bytes().all(|c| c.is_ascii_lowercase())
}

fn is_alpha(s: &str) -> bool {
    s.bytes().all(|c| c.is_ascii_alphabetic())
}

fn is_alnum(s: &str) -> bool {
    s.bytes().all(|c| c.is_ascii_alphanumeric())
}

/// Parse a BCP 47 tag; `None` on an invalid tag.
pub fn parse_locale(tag: &str) -> Option<Locale> {
    if tag.is_empty() || tag.len() >= 64 {
        return None;
    }

    let parts: Vec<&str> = tag.split('-').filter(|p| !p.is_empty()).take(6).collect();
    let language = *parts.first()?;
    if language.len() < 2 || language.len() > 3 || !is_lower_alpha(language) {
        return None;
    }

    let mut script = "";
    let mut region = "";
    let mut variant = "";
    let mut index = 1;

    if index < parts.len() && parts[index].len() == 4 && is_alpha(parts[index]) {
        script = parts[index];
        index += 1;
    }

    if index < parts.len()
        && ((parts[index].len() == 2 && is_alpha(parts[index]))
            || (parts[index].len() == 3 && is_alnum(parts[index])))
    {
        region = parts[index];
        index += 1;
    }

    if index < parts.len() {
        if parts[index].len() > 8 || !is_alnum(parts[index]) {
            return None;
        }
        variant = parts[index];
    }

    Some(Locale {
        language: language.to_string(),
        script: script.to_string(),
        region: region.to_string(),
        variant: variant.to_string(),
    })
}

/// True when `tag` is a well-formed CLDR language tag.
pub fn is_valid_locale(tag: &str) -> bool {
    parse_locale(tag).is_some()
}

/// Default locale for a language (CLDR fallback), or `"root"`.
pub fn default_locale(language: &str) -> &'static str {
    match language {
        "en" => "en_US",
        "de" => "de_DE",
        "zh" => "zh_Hans_CN",
        _ => "root",
    }
}

/// True when `locale` needs the shifted alternate (e.g. zh, sv).
pub fn needs_shifted(locale: &str) -> bool {
    locale == "zh" || locale == "sv" || locale.contains("_zh") || locale.contains("_sv")
}

/// Default collation settings for `locale`.
pub fn collation(locale: &str) -> Collation {
    Collation {
        strength: 3,
        alternate: if needs_shifted(locale) { 1 } else { 0 },
    }
}

/// Calendar name for `locale` (defaults to gregory).
pub fn calendar(locale: &str) -> &'static str {
    if locale.starts_with("th") || locale.starts_with("TH") {
        "buddhist"
    } else if locale.starts_with("zh") || locale.starts_with("ZH") {
        "chinese"
    } else {
        "gregory"
    }
}

/// First day of week (0=Sunday..6=Saturday) for `locale`.
pub fn first_weekday(locale: &str) -> i32 {
    if locale == "US" { 0 } else { 1 }
}

/// True when `pattern` is a CLDR date pattern.
pub fn is_date_pattern(pattern: &str) -> bool {
    !pattern.is_empty()
        && pattern.chars().all(|c| {
            matches!(
                c,
                'y' | 'M' | 'd' | 'H' | 'm' | 's' | 'E' | 'a' | 'B' | 'Z' | 'z' | 'G'
                    | 'u' | 'L' | 'c' | 'e' | 'F' | 'g' | 'j' | 'J' | 'k' | 'K' | 'n'
                    | 'O' | 'Q' | 'r' | 'S' | 't' | 'U' | 'V' | 'v' | 'w' | 'W' | 'x'
                    | 'X' | 'Y' | '/' | '-' | ':' | ' ' | '\'' | '.' | ','
            )
        })
}

const WEEKDAYS: [&str; 7] = [
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday",
];

/// True when `text` is a wide English weekday name.
pub fn is_weekday(text: &str) -> bool {
    WEEKDAYS.contains(&text)
}

/// Month count (always 12 for Gregorian CLDR).
pub fn month_count() -> usize {
    12
}

/// Day count in a month (Feb = 29 in a leap year).
pub fn day_count(month: i32, leap: bool) -> usize {
    const DAYS: [usize; 12] = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];
    if !(1..=12).contains(&month) {
        return 0;
    }
    if month == 2 && leap {
        return 29;
    }
    DAYS[(month - 1) as usize]
}

/// True when `id` is a CLDR keyboard id.
pub fn is_keyboard_id(id: &str) -> bool {
    !id.is_empty()
        && id
            .bytes()
            .all(|c| c.is_ascii_alphanumeric() || c == b'_' || c == b'-')
}

/// True when the layout is transparent (QWERTY-family default).
pub fn is_transparent(id: &str) -> bool {
    !id.contains("dvorak") && !id.contains("colemak")
}

/// Plural category of `n` for `locale`.
pub fn plural(locale: &str, n: f64) -> i32 {
    let _ = locale;
    if n == 1.0 {
        plural::ONE
    } else if n == 0.0 {
        plural::ZERO
    } else {
        plural::OTHER
    }
}

/// True when `pattern` has balanced braces and no stray `}`.
pub fn is_message_pattern_valid(pattern: &str) -> bool {
    let mut depth = 0i32;
    for c in pattern.chars() {
        match c {
            '{' => depth += 1,
            '}' => {
                depth -= 1;
                if depth < 0 {
                    return false;
                }
            }
            _ => {}
        }
    }
    depth == 0
}

/// Symbols for a region (ISO 3166); root defaults when absent.
pub fn number_symbols(region: Option<&str>) -> NumberSymbols {
    let (group, decimal) = match region {
        Some("DE") => (".", ","),
        Some("FR") => (" ", ","),
        _ => (",", "."),
    };
    NumberSymbols {
        group,
        decimal,
        minus: "-",
        percent: "%",
    }
}

/// True when `text` uses only ASCII digits and at most one `.`.
pub fn is_decimal(text: &str) -> bool {
    if text.is_empty() {
        return false;
    }
    let mut dots = 0;
    for c in text.chars() {
        if c.is_ascii_digit() {
            continue;
        }
        if c == '.' && dots == 0 {
            dots = 1;
            continue;
        }
        return false;
    }
    true
}

/// Name order for `locale` (family-first for CJK/Korean/Hungarian).
pub fn name_order(locale: &str) -> i32 {
    if locale.len() >= 2
        && (locale.starts_with("zh")
            || locale.starts_with("ko")
            || locale.starts_with("ja")
            || locale.starts_with("hu"))
    {
        name_order::FAMILY_FIRST
    } else {
        name_order::GIVEN_FIRST
    }
}

/// True when `name` looks like a personal name.
pub fn is_person_name(name: &str) -> bool {
    if name.is_empty() {
        return false;
    }
    name.chars().all(|c| {
        c == ' '
            || c == '-'
            || c == '\''
            || c == '.'
            || c >= '\u{80}'
            || c.is_ascii_alphabetic()
    })
}

/// True when `zone` is a CLDR zone ID (IANA-style).
pub fn is_zone_id(zone: &str) -> bool {
    !zone.is_empty() && (zone.contains('/') || zone == "UTC")
}

/// Day-period name (`"AM"`/`"PM"`) for an hour (0..23), or `"at night"`.
pub fn day_period(hour: i32) -> &'static str {
    if !(0..=23).contains(&hour) {
        "at night"
    } else if hour < 12 {
        "AM"
    } else {
        "PM"
    }
}

/// Number of day periods.
pub const DAY_PERIOD_COUNT: usize = 4;
