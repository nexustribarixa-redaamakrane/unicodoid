#ifndef UNICODOID_CONVERTUTF_HPP
#define UNICODOID_CONVERTUTF_HPP

/* ---------------------------------------------------------------------
 * ConvertUTF.hpp -- Conversions between UTF-32, UTF-16, and UTF-8.
 *
 * Unicodoid native C++ port of the discontinued W3C/Unicode ConvertUTF
 * library, updated to the Unicode 18.0.0 data set.  Unlike a thin
 * wrapper over the C implementation, this header exposes a native
 * std::vector based API in namespace unicodoid::convertutf and the
 * implementation in ConvertUTF.cpp shares no code with the C port.
 *
 * The well-formedness rules enforce the Unicode 18.0.0 maximum code
 * point U+10FFFF.
 * --------------------------------------------------------------------- */

#include <cstdint>
#include <vector>

namespace unicodoid {
namespace convertutf {

/** Outcome codes returned by the conversion routines. */
enum class ConversionResult : int {
    ConversionOk = 0,
    SourceExhausted = 1,
    TargetExhausted = 2,
    SourceIllegal = 3
};

/** Strictness of a conversion. */
enum class ConversionFlags : int {
    StrictConversion = 0,
    LenientConversion = 1
};

/** The Unicode replacement character (U+FFFD). */
inline constexpr std::uint32_t kReplacementChar = 0x0000FFFD;
/** The largest code point of the Basic Multilingual Plane (U+FFFF). */
inline constexpr std::uint32_t kMaxBmp = 0x0000FFFF;
/** The largest encoded code point (U+10FFFF). */
inline constexpr std::uint32_t kMaxUtf16 = 0x0010FFFF;
/** The largest legal Unicode code point (U+10FFFF). */
inline constexpr std::uint32_t kMaxLegalUtf32 = 0x0010FFFF;
/** Start of the high surrogate range (U+D800). */
inline constexpr std::uint32_t kSurHighStart = 0x0000D800;
/** End of the high surrogate range (U+DBFF). */
inline constexpr std::uint32_t kSurHighEnd = 0x0000DBFF;
/** Start of the low surrogate range (U+DC00). */
inline constexpr std::uint32_t kSurLowStart = 0x0000DC00;
/** End of the low surrogate range (U+DFFF). */
inline constexpr std::uint32_t kSurLowEnd = 0x0000DFFF;
/** Shift used to split a code point into surrogate halves. */
inline constexpr int kHalfShift = 10;
/** Half-base offset for supplementary code points (U+10000). */
inline constexpr std::uint32_t kHalfBase = 0x0010000;
/** Mask holding the low part of a supplementary code point. */
inline constexpr std::uint32_t kHalfMask = 0x003FF;

/** Convert a complete UTF-8 buffer into UTF-16 code units. */
std::vector<std::uint16_t> utf8ToUtf16(
    const std::vector<std::uint8_t>& source,
    ConversionFlags flags = ConversionFlags::StrictConversion,
    ConversionResult* outResult = nullptr);

/** Convert a complete UTF-16 buffer into UTF-8 bytes. */
std::vector<std::uint8_t> utf16ToUtf8(
    const std::vector<std::uint16_t>& source,
    ConversionFlags flags = ConversionFlags::StrictConversion,
    ConversionResult* outResult = nullptr);

/** Convert a complete UTF-8 buffer into UTF-32 code points. */
std::vector<std::uint32_t> utf8ToUtf32(
    const std::vector<std::uint8_t>& source,
    ConversionFlags flags = ConversionFlags::StrictConversion,
    ConversionResult* outResult = nullptr);

/** Convert a complete UTF-32 buffer into UTF-8 bytes. */
std::vector<std::uint8_t> utf32ToUtf8(
    const std::vector<std::uint32_t>& source,
    ConversionFlags flags = ConversionFlags::StrictConversion,
    ConversionResult* outResult = nullptr);

/** Convert a complete UTF-16 buffer into UTF-32 code points. */
std::vector<std::uint32_t> utf16ToUtf32(
    const std::vector<std::uint16_t>& source,
    ConversionFlags flags = ConversionFlags::StrictConversion,
    ConversionResult* outResult = nullptr);

/** Convert a complete UTF-32 buffer into UTF-16 code units. */
std::vector<std::uint16_t> utf32ToUtf16(
    const std::vector<std::uint32_t>& source,
    ConversionFlags flags = ConversionFlags::StrictConversion,
    ConversionResult* outResult = nullptr);

/**
 * Returns whether the UTF-8 sequence starting at @p offset within
 * @p source is legal.
 */
bool isLegalUtf8(const std::vector<std::uint8_t>& source, std::size_t offset);

} // namespace convertutf

/**
 * @brief Free-function aliases matching the C API names for drop-in use.
 */
} // namespace unicodoid

#endif /* UNICODOID_CONVERTUTF_HPP */
