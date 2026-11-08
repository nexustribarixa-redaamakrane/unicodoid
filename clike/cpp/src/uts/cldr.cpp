// UTS #35 CLDR helpers for the C++ API, aggregated into one translation unit.
//
// The behaviour matches the C reference implementation exactly; this file
// presents it through std::string and enum-class types.

#include "unicodoid/uts/cldr.hpp"
#include "unicodoid/uts/uts35_cldr/cldr_collation.h"
#include "unicodoid/uts/uts35_cldr/cldr_core.h"
#include "unicodoid/uts/uts35_cldr/cldr_dates.h"
#include "unicodoid/uts/uts35_cldr/cldr_general.h"
#include "unicodoid/uts/uts35_cldr/cldr_keyboards.h"
#include "unicodoid/uts/uts35_cldr/cldr_messageformat.h"
#include "unicodoid/uts/uts35_cldr/cldr_numbers.h"
#include "unicodoid/uts/uts35_cldr/cldr_person_names.h"
#include "unicodoid/uts/uts35_cldr/cldr_supplemental.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>

namespace unicodoid {
namespace uts35 {
namespace {

std::string owned(std::string_view value) { return std::string(value); }

} // namespace

bool parse_locale(std::string_view tag, Locale& out) {
    unicodoid_cldr_locale_t raw{};
    const std::string text = owned(tag);
    if (!unicodoid_cldr_parse_locale(text.c_str(), &raw)) return false;
    out.language = raw.language;
    out.script = raw.script;
    out.region = raw.region;
    out.variant = raw.variant;
    return true;
}

bool is_valid_locale(std::string_view tag) {
    const std::string text = owned(tag);
    return unicodoid_cldr_is_valid_locale(text.c_str());
}

std::string_view default_locale(std::string_view language) {
    const std::string text = owned(language);
    return unicodoid_cldr_default_locale(text.c_str());
}

std::string_view version() { return unicodoid_cldr_version(); }

NumberSymbols number_symbols(std::string_view region) {
    unicodoid_cldr_number_symbols_t raw{};
    const std::string text = owned(region);
    unicodoid_cldr_number_symbols(text.empty() ? nullptr : text.c_str(), &raw);
    return NumberSymbols{raw.group, raw.decimal, raw.minus, raw.percent};
}

bool is_decimal(std::string_view text) {
    const std::string owned_text = owned(text);
    return unicodoid_cldr_is_decimal(owned_text.c_str());
}

std::string_view calendar(std::string_view locale) {
    const std::string text = owned(locale);
    return unicodoid_cldr_calendar(text.c_str());
}

int first_weekday(std::string_view locale) {
    const std::string text = owned(locale);
    return unicodoid_cldr_first_weekday(text.c_str());
}

bool is_date_pattern(std::string_view pattern) {
    const std::string text = owned(pattern);
    return unicodoid_cldr_is_date_pattern(text.c_str());
}

Collation collation(std::string_view locale) {
    unicodoid_cldr_collation_t raw{};
    const std::string text = owned(locale);
    unicodoid_cldr_collation(text.c_str(), &raw);
    return Collation{raw.strength, raw.alternate};
}

bool needs_shifted(std::string_view locale) {
    const std::string text = owned(locale);
    return unicodoid_cldr_needs_shifted(text.c_str());
}

bool is_weekday(std::string_view locale, std::string_view text) {
    const std::string locale_text = owned(locale);
    const std::string name_text = owned(text);
    return unicodoid_cldr_is_weekday(locale_text.c_str(), name_text.c_str());
}

std::size_t month_count(std::string_view locale) {
    const std::string text = owned(locale);
    return unicodoid_cldr_month_count(text.c_str());
}

std::size_t day_count(std::string_view locale, int month, bool leap) {
    const std::string text = owned(locale);
    return unicodoid_cldr_day_count(text.c_str(), month, leap);
}

bool is_keyboard_id(std::string_view id) {
    const std::string text = owned(id);
    return unicodoid_cldr_is_keyboard_id(text.c_str());
}

bool is_transparent(std::string_view id) {
    const std::string text = owned(id);
    return unicodoid_cldr_is_transparent(text.c_str());
}

Plural plural(std::string_view locale, double n) {
    const std::string text = owned(locale);
    return static_cast<Plural>(unicodoid_cldr_plural(text.c_str(), n));
}

bool is_message_pattern_valid(std::string_view pattern) {
    const std::string text = owned(pattern);
    return unicodoid_cldr_is_message_pattern_valid(text.c_str());
}

NameOrder name_order(std::string_view locale) {
    const std::string text = owned(locale);
    return static_cast<NameOrder>(unicodoid_cldr_name_order(text.c_str()));
}

bool is_person_name(std::string_view locale, std::string_view name) {
    const std::string locale_text = owned(locale);
    const std::string name_text = owned(name);
    return unicodoid_cldr_is_person_name(locale_text.c_str(), name_text.c_str());
}

bool is_zone_id(std::string_view zone) {
    const std::string text = owned(zone);
    return unicodoid_cldr_is_zone_id(text.c_str());
}

std::string_view day_period(int hour) {
    return unicodoid_cldr_day_period(hour);
}

std::size_t day_period_count() { return unicodoid_cldr_day_period_count(); }

} // namespace uts35
} // namespace unicodoid
