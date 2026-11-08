#include "unicodoid/uax/uax60_ea_script_data.h"
#include "unicodoid/unicodoid_ucd.h"

#include <string.h>

static bool script_is(uint32_t cp, const char* name) {
    const char* sc = unicodoid_ucd_script_name(unicodoid_ucd_script(cp));
    return sc && strcmp(sc, name) == 0;
}

bool unicodoid_uax60_is_han(uint32_t cp) {
    return script_is(cp, "Han");
}

bool unicodoid_uax60_is_ea_script(uint32_t cp) {
    const char* sc = unicodoid_ucd_script_name(unicodoid_ucd_script(cp));
    if (!sc) return false;
    return strcmp(sc, "Han") == 0 || strcmp(sc, "Hiragana") == 0 ||
           strcmp(sc, "Katakana") == 0 || strcmp(sc, "Hangul") == 0 ||
           strcmp(sc, "Bopomofo") == 0 || strcmp(sc, "Yi") == 0 ||
           strcmp(sc, "Tangut") == 0 || strcmp(sc, "Nushu") == 0 ||
           strcmp(sc, "Khitan_Small_Script") == 0 || strcmp(sc, "Jurchen") == 0;
}

bool unicodoid_uax60_is_ea_wide(uint32_t cp) {
    uint8_t w = unicodoid_ucd_east_asian_width(cp);
    return w == 5 /* W */ || w == 3 /* F */;
}
