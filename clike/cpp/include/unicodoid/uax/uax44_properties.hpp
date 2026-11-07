#ifndef UNICODOID_UAX44_PROPERTIES_HPP
#define UNICODOID_UAX44_PROPERTIES_HPP

#include "unicodoid/unicodoid_ucd.h"

#include <cstdint>
#include <string>
#include <string_view>

namespace unicodoid {
namespace uax44 {

/**
 * @brief UAX #44 Unicode Character Database property access.
 *
 * Thin, typed accessors over the generated Unicode 18.0.0 tables.  This module
 * performs no algorithm of its own; it is the shared data surface used by the
 * other specification modules.
 */

uint8_t general_category(uint32_t cp);
uint8_t canonical_combining_class(uint32_t cp);
uint8_t bidi_class(uint32_t cp);
uint8_t east_asian_width(uint32_t cp);
uint8_t line_break(uint32_t cp);
uint8_t grapheme_break(uint32_t cp);
uint8_t word_break(uint32_t cp);
uint8_t sentence_break(uint32_t cp);
uint8_t vertical_orientation(uint32_t cp);
uint8_t joining_type(uint32_t cp);
uint8_t indic_conjunct_break(uint32_t cp);
uint16_t script(uint32_t cp);
uint16_t block(uint32_t cp);
uint8_t age(uint32_t cp);
bool has_binary_property(uint32_t cp, unicodoid_binary_property_t property);

std::string_view general_category_name(uint8_t value);
std::string_view bidi_class_name(uint8_t value);
std::string_view east_asian_width_name(uint8_t value);
std::string_view line_break_name(uint8_t value);
std::string_view grapheme_break_name(uint8_t value);
std::string_view word_break_name(uint8_t value);
std::string_view sentence_break_name(uint8_t value);
std::string_view vertical_orientation_name(uint8_t value);
std::string_view joining_type_name(uint8_t value);
std::string_view indic_conjunct_break_name(uint8_t value);
std::string_view script_name(uint16_t value);
std::string_view block_name(uint16_t value);
std::string_view age_name(uint8_t value);

/** Numeric_Value; false for characters without a numeric value. */
bool numeric_value(uint32_t cp, int64_t* numerator, int64_t* denominator);
/** Decimal_Digit_Value, or -1. */
int32_t decimal_digit(uint32_t cp);

/** Full case mappings; the result is empty when the mapping is absent. */
std::u32string to_uppercase(uint32_t cp);
std::u32string to_lowercase(uint32_t cp);
std::u32string to_titlecase(uint32_t cp);
std::u32string to_casefold(uint32_t cp);

} // namespace uax44
} // namespace unicodoid

#endif /* UNICODOID_UAX44_PROPERTIES_HPP */
