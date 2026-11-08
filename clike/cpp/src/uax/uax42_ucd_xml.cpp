#include "unicodoid/uax/uax42_ucd_xml.hpp"
#include "unicodoid/unicodoid_ucd.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>

namespace unicodoid {
namespace uax42 {

std::string_view format() { return "UCDxml"; }

bool is_representable(uint32_t cp) {
    return cp <= 0x10FFFF && !(cp >= 0xD800 && cp <= 0xDFFF);
}

std::string char_record(uint32_t cp) {
    const char* gc =
        unicodoid_ucd_general_category_name(unicodoid_ucd_general_category(cp));
    const char* bc =
        unicodoid_ucd_bidi_class_name(unicodoid_ucd_bidi_class(cp));
    const char* ea =
        unicodoid_ucd_east_asian_width_name(unicodoid_ucd_east_asian_width(cp));
    const char* lb =
        unicodoid_ucd_line_break_name(unicodoid_ucd_line_break(cp));
    const char* sc = unicodoid_ucd_script_name(unicodoid_ucd_script(cp));
    const int ccc = unicodoid_ucd_canonical_combining_class(cp);

    char buffer[256];
    const int n = std::snprintf(
        buffer, sizeof(buffer),
        "<char cp=\"%04X\" GC=\"%s\" Bidi_C=\"%s\" EA=\"%s\" LB=\"%s\" "
        "sc=\"%s\" ccc=\"%d\"/>\n",
        cp, gc ? gc : "", bc ? bc : "", ea ? ea : "", lb ? lb : "",
        sc ? sc : "", ccc);
    if (n <= 0) return {};
    return std::string(buffer, static_cast<std::size_t>(n));
}

} // namespace uax42
} // namespace unicodoid
