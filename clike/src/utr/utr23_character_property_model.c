#include "unicodoid/utr/utr23_character_property_model.h"

#include <string.h>

size_t unicodoid_utr23_binary_property_count(void) { return 41; }

bool unicodoid_utr23_is_binary(size_t index) { return index < 41; }

int unicodoid_utr23_property_kind(const char* name) {
    if (!name) return 0;
    if (strcmp(name, "General_Category") == 0) return UNICODOID_PROP_ENUMERATED;
    if (strcmp(name, "Numeric_Type") == 0) return UNICODOID_PROP_NUMERIC;
    if (strcmp(name, "Unicode_1_Name") == 0) return UNICODOID_PROP_STRING;
    return UNICODOID_PROP_BINARY; /* default: binary */
}
