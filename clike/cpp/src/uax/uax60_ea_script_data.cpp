#include "unicodoid/uax/uax60_ea_script_data.hpp"
#include "unicodoid/uax/uax11_eaw.hpp"
#include "unicodoid/unicodoid_ucd.h"

#include <cstdint>
#include <cstring>

namespace unicodoid {
namespace uax60 {
namespace {

bool script_is(uint32_t cp, const char* name) {
    const char* script = unicodoid_ucd_script_name(unicodoid_ucd_script(cp));
    return script != nullptr && std::strcmp(script, name) == 0;
}

} // namespace

bool is_han(uint32_t cp) {
    return script_is(cp, "Han");
}

bool is_ea_script(uint32_t cp) {
    const char* script = unicodoid_ucd_script_name(unicodoid_ucd_script(cp));
    if (!script) return false;
    return std::strcmp(script, "Han") == 0 ||
           std::strcmp(script, "Hiragana") == 0 ||
           std::strcmp(script, "Katakana") == 0 ||
           std::strcmp(script, "Hangul") == 0 ||
           std::strcmp(script, "Bopomofo") == 0 ||
           std::strcmp(script, "Yi") == 0 ||
           std::strcmp(script, "Tangut") == 0 ||
           std::strcmp(script, "Nushu") == 0 ||
           std::strcmp(script, "Khitan_Small_Script") == 0 ||
           std::strcmp(script, "Jurchen") == 0;
}

bool is_ea_wide(uint32_t cp) {
    const uint8_t value = unicodoid_ucd_east_asian_width(cp);
    return value == static_cast<uint8_t>(uax11::Width::Wide) ||
           value == static_cast<uint8_t>(uax11::Width::Fullwidth);
}

} // namespace uax60
} // namespace unicodoid
