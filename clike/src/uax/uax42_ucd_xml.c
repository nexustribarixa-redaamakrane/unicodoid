#include "unicodoid/uax/uax42_ucd_xml.h"
#include "unicodoid/unicodoid_ucd.h"

#include <stdio.h>
#include <string.h>

const char* unicodoid_uax42_format(void) { return "UCDxml"; }

int unicodoid_uax42_is_representable(uint32_t cp) {
    return (cp <= 0x10FFFF && !(cp >= 0xD800 && cp <= 0xDFFF)) ? 1 : 0;
}

size_t unicodoid_uax42_char_record(uint32_t cp, char* out, size_t cap) {
    const char* gc = unicodoid_ucd_general_category_name(unicodoid_ucd_general_category(cp));
    const char* bc = unicodoid_ucd_bidi_class_name(unicodoid_ucd_bidi_class(cp));
    const char* ea = unicodoid_ucd_east_asian_width_name(unicodoid_ucd_east_asian_width(cp));
    const char* lb = unicodoid_ucd_line_break_name(unicodoid_ucd_line_break(cp));
    const char* sc = unicodoid_ucd_script_name(unicodoid_ucd_script(cp));
    int ccc = unicodoid_ucd_canonical_combining_class(cp);
    char buf[256];
    int n = snprintf(buf, sizeof(buf),
                     "<char cp=\"%04X\" GC=\"%s\" Bidi_C=\"%s\" EA=\"%s\" LB=\"%s\" sc=\"%s\" ccc=\"%d\"/>\n",
                     cp, gc, bc, ea, lb, sc, ccc);
    if (n < 0) return 0;
    if (out && cap >= (size_t)n + 1) {
        memcpy(out, buf, (size_t)n + 1);
    }
    return (size_t)n;
}
