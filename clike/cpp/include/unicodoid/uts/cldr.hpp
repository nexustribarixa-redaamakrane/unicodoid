#ifndef UNICODOID_CLDR_HPP
#define UNICODOID_CLDR_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace unicodoid {
namespace uts35 {

/**
 * @brief UTS #35 Unicode Locale Data Markup Language (CLDR 49).
 *
 * Aggregated C++ surface for the CLDR parts implemented by the SDK: core
 * locale identifiers, numbers, dates, collation, general display data,
 * keyboards, MessageFormat 2.0 helpers, person names and supplemental data.
 * These are the interoperability helpers the encoding and collation layers
 * consume; full CLDR XML resource bundles are external distribution data.
 */

// ---- Core ---------------------------------------------------------------

struct Locale {
    std::string language = "und";
    std::string script;
    std::string region;
    std::string variant;
};

/** Parse a BCP 47 tag. */
bool parse_locale(std::string_view tag, Locale& out);

/** True when @p tag is a well-formed BCP 47 (CLDR) language tag. */
bool is_valid_locale(std::string_view tag);

/** Default locale for @p language, or "root". */
std::string_view default_locale(std::string_view language);

/** CLDR version string. */
std::string_view version();

// ---- Numbers ------------------------------------------------------------

struct NumberSymbols {
    std::string group = ",";
    std::string decimal = ".";
    std::string minus = "-";
    std::string percent = "%";
};

/** Symbols for a region; root defaults when empty. */
NumberSymbols number_symbols(std::string_view region);

/** True when @p text uses only ASCII digits and at most one '.'. */
bool is_decimal(std::string_view text);

// ---- Dates --------------------------------------------------------------

struct DateFormat {
    std::string calendar;
    std::string pattern;
};

/** Calendar name for @p locale (defaults to gregory). */
std::string_view calendar(std::string_view locale);

/** First day of week (0=Sunday..6=Saturday). */
int first_weekday(std::string_view locale);

/** True when @p pattern uses only CLDR date-pattern characters. */
bool is_date_pattern(std::string_view pattern);

// ---- Collation ----------------------------------------------------------

struct Collation {
    int strength = 3;
    int alternate = 0;
};

/** Default collation settings for @p locale. */
Collation collation(std::string_view locale);

/** True when @p locale needs the shifted alternate. */
bool needs_shifted(std::string_view locale);

// ---- General ------------------------------------------------------------

/** Display-name sizes. */
enum class DisplaySize : int {
    Narrow = 0,
    Short = 1,
    Wide = 2
};

/** True when @p text is a wide English weekday name. */
bool is_weekday(std::string_view locale, std::string_view text);

/** Month count (12 for Gregorian CLDR). */
std::size_t month_count(std::string_view locale);

/** Day count in @p month (1..12); February honours @p leap. */
std::size_t day_count(std::string_view locale, int month, bool leap);

// ---- Keyboards ----------------------------------------------------------

/** True when @p id is a CLDR keyboard id. */
bool is_keyboard_id(std::string_view id);

/** True when the layout is transparent (QWERTY-family default). */
bool is_transparent(std::string_view id);

// ---- MessageFormat 2.0 --------------------------------------------------

/** Plural categories. */
enum class Plural : int {
    Other = 0,
    One = 1,
    Two = 2,
    Few = 3,
    Many = 4,
    Zero = 5
};

/** Plural category of @p n for @p locale (English + root rules). */
Plural plural(std::string_view locale, double n);

/** True when @p pattern has balanced braces. */
bool is_message_pattern_valid(std::string_view pattern);

// ---- Person names -------------------------------------------------------

/** Name-order vocabulary. */
enum class NameOrder : int {
    FamilyFirst = 0,
    GivenFirst = 1
};

/** Name order for @p locale. */
NameOrder name_order(std::string_view locale);

/** True when @p name looks like a personal name. */
bool is_person_name(std::string_view locale, std::string_view name);

// ---- Supplemental -------------------------------------------------------

/** True when @p zone is a CLDR zone ID (IANA-style). */
bool is_zone_id(std::string_view zone);

/** Day-period name ("AM"/"PM") for @p hour, or "at night" when invalid. */
std::string_view day_period(int hour);

/** Number of day periods (4). */
std::size_t day_period_count();

} // namespace uts35
} // namespace unicodoid

#endif /* UNICODOID_CLDR_HPP */
