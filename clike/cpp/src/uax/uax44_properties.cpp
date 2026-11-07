// Native UAX #44 Unicode Character Database property access for the C++ API.

#include "unicodoid/uax/uax44_properties.hpp"

#include <cstddef>

namespace unicodoid {
namespace uax44 {
namespace {

std::string_view view(const char* text) {
    return text ? std::string_view(text) : std::string_view();
}

std::u32string mapping(
    std::size_t (*fn)(uint32_t, uint32_t*, std::size_t), uint32_t cp) {
    uint32_t buffer[8];
    const std::size_t n = fn(cp, buffer, 8);
    std::u32string out;
    out.reserve(n);
    for (std::size_t i = 0; i < n && i < 8; ++i) {
        out.push_back(static_cast<char32_t>(buffer[i]));
    }
    return out;
}

} // namespace

uint8_t general_category(uint32_t cp) { return unicodoid_ucd_general_category(cp); }
uint8_t canonical_combining_class(uint32_t cp) {
    return unicodoid_ucd_canonical_combining_class(cp);
}
uint8_t bidi_class(uint32_t cp) { return unicodoid_ucd_bidi_class(cp); }
uint8_t east_asian_width(uint32_t cp) { return unicodoid_ucd_east_asian_width(cp); }
uint8_t line_break(uint32_t cp) { return unicodoid_ucd_line_break(cp); }
uint8_t grapheme_break(uint32_t cp) { return unicodoid_ucd_grapheme_break(cp); }
uint8_t word_break(uint32_t cp) { return unicodoid_ucd_word_break(cp); }
uint8_t sentence_break(uint32_t cp) { return unicodoid_ucd_sentence_break(cp); }
uint8_t vertical_orientation(uint32_t cp) {
    return unicodoid_ucd_vertical_orientation(cp);
}
uint8_t joining_type(uint32_t cp) { return unicodoid_ucd_joining_type(cp); }
uint8_t indic_conjunct_break(uint32_t cp) {
    return unicodoid_ucd_indic_conjunct_break(cp);
}
uint16_t script(uint32_t cp) { return unicodoid_ucd_script(cp); }
uint16_t block(uint32_t cp) { return unicodoid_ucd_block(cp); }
uint8_t age(uint32_t cp) { return unicodoid_ucd_age(cp); }
bool has_binary_property(uint32_t cp, unicodoid_binary_property_t property) {
    return unicodoid_ucd_has_binary_property(cp, property);
}

std::string_view general_category_name(uint8_t value) {
    return view(unicodoid_ucd_general_category_name(value));
}
std::string_view bidi_class_name(uint8_t value) {
    return view(unicodoid_ucd_bidi_class_name(value));
}
std::string_view east_asian_width_name(uint8_t value) {
    return view(unicodoid_ucd_east_asian_width_name(value));
}
std::string_view line_break_name(uint8_t value) {
    return view(unicodoid_ucd_line_break_name(value));
}
std::string_view grapheme_break_name(uint8_t value) {
    return view(unicodoid_ucd_grapheme_break_name(value));
}
std::string_view word_break_name(uint8_t value) {
    return view(unicodoid_ucd_word_break_name(value));
}
std::string_view sentence_break_name(uint8_t value) {
    return view(unicodoid_ucd_sentence_break_name(value));
}
std::string_view vertical_orientation_name(uint8_t value) {
    return view(unicodoid_ucd_vertical_orientation_name(value));
}
std::string_view joining_type_name(uint8_t value) {
    return view(unicodoid_ucd_joining_type_name(value));
}
std::string_view indic_conjunct_break_name(uint8_t value) {
    return view(unicodoid_ucd_indic_conjunct_break_name(value));
}
std::string_view script_name(uint16_t value) {
    return view(unicodoid_ucd_script_name(value));
}
std::string_view block_name(uint16_t value) {
    return view(unicodoid_ucd_block_name(value));
}
std::string_view age_name(uint8_t value) {
    return view(unicodoid_ucd_age_name(value));
}

bool numeric_value(uint32_t cp, int64_t* numerator, int64_t* denominator) {
    return unicodoid_ucd_numeric_value(cp, numerator, denominator);
}

int32_t decimal_digit(uint32_t cp) {
    return unicodoid_ucd_decimal_digit(cp);
}

std::u32string to_uppercase(uint32_t cp) {
    return mapping(unicodoid_ucd_to_uppercase, cp);
}
std::u32string to_lowercase(uint32_t cp) {
    return mapping(unicodoid_ucd_to_lowercase, cp);
}
std::u32string to_titlecase(uint32_t cp) {
    return mapping(unicodoid_ucd_to_titlecase, cp);
}
std::u32string to_casefold(uint32_t cp) {
    return mapping(unicodoid_ucd_to_casefold, cp);
}

} // namespace uax44
} // namespace unicodoid
