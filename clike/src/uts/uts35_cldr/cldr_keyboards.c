#include "unicodoid/uts/uts35_cldr/cldr_keyboards.h"

#include <string.h>

bool unicodoid_cldr_is_keyboard_id(const char* id) {
    if (!id || !*id) return false;
    for (const char* p = id; *p; ++p) {
        if (!((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') ||
              (*p >= '0' && *p <= '9') || *p == '_' || *p == '-')) return false;
    }
    return true;
}

bool unicodoid_cldr_is_transparent(const char* id) {
    if (!id) return true;
    return strstr(id, "dvorak") == NULL && strstr(id, "colemak") == NULL;
}
