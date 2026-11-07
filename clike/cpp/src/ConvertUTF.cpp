/* ---------------------------------------------------------------------
 * ConvertUTF.cpp -- Conversions between UTF-32, UTF-16, and UTF-8.
 *
 * Unicodoid native C++ port of the discontinued W3C/Unicode ConvertUTF
 * library, updated to the Unicode 18.0.0 data set.  This is a native
 * implementation: it does not call the C port.
 * --------------------------------------------------------------------- */

#include "unicodoid/ConvertUTF.hpp"

namespace unicodoid {
namespace convertutf {

namespace {

const int kHalfShiftLocal = 10;
const std::uint32_t kHalfBaseLocal = 0x0010000U;
const std::uint32_t kHalfMaskLocal = 0x3FFU;

constexpr std::uint32_t kSurHighStartLocal = 0xD800U;
constexpr std::uint32_t kSurHighEndLocal = 0xDBFFU;
constexpr std::uint32_t kSurLowStartLocal = 0xDC00U;
constexpr std::uint32_t kSurLowEndLocal = 0xDFFFU;

int trailingBytesForUtf8(std::uint8_t first) {
    if (first < 0x80) return 0;
    if (first < 0xC2) return 0;
    if (first < 0xE0) return 1;
    if (first < 0xF0) return 2;
    if (first <= 0xF4) return 3;
    return 4;
}

std::uint32_t offsetsFromUtf8(int extra) {
    static const std::uint32_t kOffsets[6] = {
        0x00000000U, 0x00003080U, 0x000E2080U, 0x03C82080U, 0xFA082080U, 0x82082080U
    };
    if (extra < 0 || extra > 5) return 0;
    return kOffsets[extra];
}

bool isLegalUtf8Sequence(const std::uint8_t* source, int length) {
    if (length < 1 || length > 4) return false;
    std::uint8_t first = source[0];
    if (length == 1) {
        if (first >= 0x80 && first < 0xC2) return false;
        return first <= 0xF4;
    }
    for (int i = 1; i < length; ++i) {
        if (source[i] < 0x80 || source[i] > 0xBF) return false;
    }
    std::uint8_t second = source[1];
    bool legal = true;
    switch (first) {
        case 0xE0: legal = second >= 0xA0; break;
        case 0xED: legal = second <= 0x9F; break;
        case 0xF0: legal = second >= 0x90; break;
        case 0xF4: legal = second <= 0x8F; break;
        default: legal = second >= 0x80; break;
    }
    if (!legal) return false;
    return first <= 0xF4;
}

std::uint8_t firstByteMark(int bytesToWrite) {
    static const std::uint8_t kMarks[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
    if (bytesToWrite < 0 || bytesToWrite > 6) return 0;
    return kMarks[bytesToWrite];
}

void writeUtf8(std::vector<std::uint8_t>& out, std::uint32_t ch, int bytesToWrite) {
    const std::uint32_t byteMask = 0xBF;
    const std::uint32_t byteMark = 0x80;
    std::size_t base = out.size();
    out.resize(base + static_cast<std::size_t>(bytesToWrite));
    std::uint32_t c = ch;
    for (int j = bytesToWrite; j >= 1; --j) {
        if (j == 1) {
            out[base + 0] = static_cast<std::uint8_t>(c | firstByteMark(bytesToWrite));
        } else {
            out[base + static_cast<std::size_t>(j - 1)] =
                static_cast<std::uint8_t>((c | byteMark) & byteMask);
            c >>= 6;
        }
    }
}

} // anonymous namespace

std::vector<std::uint16_t> utf8ToUtf16(
    const std::vector<std::uint8_t>& source,
    ConversionFlags flags,
    ConversionResult* outResult) {
    std::vector<std::uint16_t> target;
    ConversionResult result = ConversionResult::ConversionOk;
    std::size_t idx = 0;
    while (idx < source.size()) {
        int extra = trailingBytesForUtf8(source[idx]);
        if (idx + static_cast<std::size_t>(extra) >= source.size()) {
            result = ConversionResult::SourceExhausted;
            break;
        }
        if (!isLegalUtf8Sequence(&source[idx], extra + 1)) {
            result = ConversionResult::SourceIllegal;
            break;
        }
        std::uint32_t ch = 0;
        int bytesToRead = extra + 1;
        while (true) {
            ch += source[idx++];
            --bytesToRead;
            if (bytesToRead > 0) ch <<= 6;
            else break;
        }
        ch -= offsetsFromUtf8(extra);

        if (ch <= kMaxBmp) {
            if (ch >= kSurHighStart && ch <= kSurLowEnd) {
                if (flags == ConversionFlags::StrictConversion) {
                    result = ConversionResult::SourceIllegal;
                    break;
                }
                target.push_back(static_cast<std::uint16_t>(kReplacementChar));
            } else {
                target.push_back(static_cast<std::uint16_t>(ch));
            }
        } else if (ch > kMaxUtf16) {
            if (flags == ConversionFlags::StrictConversion) {
                result = ConversionResult::SourceIllegal;
                break;
            }
            target.push_back(static_cast<std::uint16_t>(kReplacementChar));
        } else {
            std::uint32_t c = ch - kHalfBaseLocal;
            target.push_back(static_cast<std::uint16_t>((c >> kHalfShiftLocal) + kSurHighStartLocal));
            target.push_back(static_cast<std::uint16_t>((c & kHalfMaskLocal) + kSurLowStartLocal));
        }
    }
    if (outResult) *outResult = result;
    return target;
}

std::vector<std::uint8_t> utf16ToUtf8(
    const std::vector<std::uint16_t>& source,
    ConversionFlags flags,
    ConversionResult* outResult) {
    std::vector<std::uint8_t> target;
    ConversionResult result = ConversionResult::ConversionOk;
    std::size_t idx = 0;
    while (idx < source.size()) {
        std::uint32_t ch = source[idx++];
        if (ch >= kSurHighStartLocal && ch <= kSurHighEndLocal) {
            if (idx < source.size()) {
                std::uint32_t ch2 = source[idx];
                if (ch2 >= kSurLowStartLocal && ch2 <= kSurLowEndLocal) {
                    ch = ((ch - kSurHighStartLocal) << kHalfShiftLocal)
                        + (ch2 - kSurLowStartLocal) + kHalfBaseLocal;
                    ++idx;
                } else if (flags == ConversionFlags::StrictConversion) {
                    result = ConversionResult::SourceIllegal;
                    break;
                }
            } else {
                result = ConversionResult::SourceExhausted;
                break;
            }
        } else if (flags == ConversionFlags::StrictConversion) {
            if (ch >= kSurLowStartLocal && ch <= kSurLowEndLocal) {
                result = ConversionResult::SourceIllegal;
                break;
            }
        }

        int bytesToWrite = 0;
        if (ch < 0x80) bytesToWrite = 1;
        else if (ch < 0x800) bytesToWrite = 2;
        else if (ch < 0x10000) bytesToWrite = 3;
        else if (ch < 0x110000) bytesToWrite = 4;
        else {
            bytesToWrite = 3;
            ch = kReplacementChar;
            result = ConversionResult::SourceIllegal;
        }
        writeUtf8(target, ch, bytesToWrite);
    }
    if (outResult) *outResult = result;
    return target;
}

std::vector<std::uint32_t> utf8ToUtf32(
    const std::vector<std::uint8_t>& source,
    ConversionFlags flags,
    ConversionResult* outResult) {
    std::vector<std::uint32_t> target;
    ConversionResult result = ConversionResult::ConversionOk;
    std::size_t idx = 0;
    while (idx < source.size()) {
        int extra = trailingBytesForUtf8(source[idx]);
        if (idx + static_cast<std::size_t>(extra) >= source.size()) {
            result = ConversionResult::SourceExhausted;
            break;
        }
        if (!isLegalUtf8Sequence(&source[idx], extra + 1)) {
            result = ConversionResult::SourceIllegal;
            break;
        }
        std::uint32_t ch = 0;
        int bytesToRead = extra + 1;
        while (true) {
            ch += source[idx++];
            --bytesToRead;
            if (bytesToRead > 0) ch <<= 6;
            else break;
        }
        ch -= offsetsFromUtf8(extra);

        if (ch <= kMaxLegalUtf32) {
            if (ch >= kSurHighStartLocal && ch <= kSurLowEndLocal) {
                if (flags == ConversionFlags::StrictConversion) {
                    result = ConversionResult::SourceIllegal;
                    break;
                }
                target.push_back(kReplacementChar);
            } else {
                target.push_back(ch);
            }
        } else {
            result = ConversionResult::SourceIllegal;
            target.push_back(kReplacementChar);
        }
    }
    if (outResult) *outResult = result;
    return target;
}

std::vector<std::uint8_t> utf32ToUtf8(
    const std::vector<std::uint32_t>& source,
    ConversionFlags flags,
    ConversionResult* outResult) {
    std::vector<std::uint8_t> target;
    ConversionResult result = ConversionResult::ConversionOk;
    std::size_t idx = 0;
    while (idx < source.size()) {
        std::uint32_t ch = source[idx++];
        if (flags == ConversionFlags::StrictConversion) {
            if (ch >= kSurHighStartLocal && ch <= kSurLowEndLocal) {
                result = ConversionResult::SourceIllegal;
                break;
            }
        }
        int bytesToWrite = 0;
        if (ch < 0x80) bytesToWrite = 1;
        else if (ch < 0x800) bytesToWrite = 2;
        else if (ch < 0x10000) bytesToWrite = 3;
        else if (ch <= kMaxLegalUtf32) bytesToWrite = 4;
        else {
            bytesToWrite = 3;
            ch = kReplacementChar;
            result = ConversionResult::SourceIllegal;
        }
        writeUtf8(target, ch, bytesToWrite);
    }
    if (outResult) *outResult = result;
    return target;
}

std::vector<std::uint32_t> utf16ToUtf32(
    const std::vector<std::uint16_t>& source,
    ConversionFlags flags,
    ConversionResult* outResult) {
    std::vector<std::uint32_t> target;
    ConversionResult result = ConversionResult::ConversionOk;
    std::size_t idx = 0;
    while (idx < source.size()) {
        std::uint32_t ch = source[idx++];
        if (ch >= kSurHighStartLocal && ch <= kSurHighEndLocal) {
            if (idx < source.size()) {
                std::uint32_t ch2 = source[idx];
                if (ch2 >= kSurLowStartLocal && ch2 <= kSurLowEndLocal) {
                    ch = ((ch - kSurHighStartLocal) << kHalfShiftLocal)
                        + (ch2 - kSurLowStartLocal) + kHalfBaseLocal;
                    ++idx;
                } else if (flags == ConversionFlags::StrictConversion) {
                    result = ConversionResult::SourceIllegal;
                    break;
                }
            } else {
                result = ConversionResult::SourceExhausted;
                break;
            }
        } else if (flags == ConversionFlags::StrictConversion) {
            if (ch >= kSurLowStartLocal && ch <= kSurLowEndLocal) {
                result = ConversionResult::SourceIllegal;
                break;
            }
        }
        target.push_back(ch);
    }
    if (outResult) *outResult = result;
    return target;
}

std::vector<std::uint16_t> utf32ToUtf16(
    const std::vector<std::uint32_t>& source,
    ConversionFlags flags,
    ConversionResult* outResult) {
    std::vector<std::uint16_t> target;
    ConversionResult result = ConversionResult::ConversionOk;
    std::size_t idx = 0;
    while (idx < source.size()) {
        std::uint32_t ch = source[idx++];
        if (flags == ConversionFlags::StrictConversion) {
            if (ch >= kSurHighStartLocal && ch <= kSurLowEndLocal) {
                result = ConversionResult::SourceIllegal;
                break;
            }
        }
        if (ch <= kMaxBmp) {
            if (ch >= kSurHighStartLocal && ch <= kSurLowEndLocal) {
                if (flags == ConversionFlags::StrictConversion) {
                    result = ConversionResult::SourceIllegal;
                    break;
                }
                target.push_back(static_cast<std::uint16_t>(kReplacementChar));
            } else {
                target.push_back(static_cast<std::uint16_t>(ch));
            }
        } else if (ch > kMaxUtf16) {
            if (flags == ConversionFlags::StrictConversion) {
                result = ConversionResult::SourceIllegal;
                break;
            }
            target.push_back(static_cast<std::uint16_t>(kReplacementChar));
        } else {
            std::uint32_t c = ch - kHalfBaseLocal;
            target.push_back(static_cast<std::uint16_t>((c >> kHalfShiftLocal) + kSurHighStartLocal));
            target.push_back(static_cast<std::uint16_t>((c & kHalfMaskLocal) + kSurLowStartLocal));
        }
    }
    if (outResult) *outResult = result;
    return target;
}

bool isLegalUtf8(const std::vector<std::uint8_t>& source, std::size_t offset) {
    if (offset >= source.size()) return false;
    int length = trailingBytesForUtf8(source[offset]) + 1;
    if (offset + static_cast<std::size_t>(length) > source.size()) return false;
    return isLegalUtf8Sequence(&source[offset], length);
}

} // namespace convertutf
} // namespace unicodoid
