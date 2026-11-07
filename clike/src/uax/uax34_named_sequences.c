#include "unicodoid/uax/uax34_named_sequences.h"
#include "unicodoid/unicodoid_ucd.h"

#include <string.h>

static bool valid_scalar(uint32_t cp) {
    return cp <= 0x10FFFF && !(cp >= 0xD800 && cp <= 0xDFFF);
}

size_t unicodoid_uax34_count(void) {
    return unicodoid_ucd_named_sequences_count();
}

const char* unicodoid_uax34_name(size_t index) {
    return unicodoid_ucd_named_seq_name(index);
}

size_t unicodoid_uax34_sequence(size_t index, uint32_t* out, size_t cap) {
    return unicodoid_ucd_named_seq(index, out, cap);
}

bool unicodoid_uax34_find(
    const char* name, uint32_t* out, size_t cap, size_t* length) {
    size_t index = 0;
    if (!unicodoid_ucd_named_seq_find(name, &index)) return false;
    size_t n = unicodoid_ucd_named_seq(index, out, cap);
    if (n == (size_t)-1) return false;
    if (length) *length = n;
    if (out && cap < n) return false;
    return true;
}

bool unicodoid_uax34_is_registered(const uint32_t* cps, size_t length) {
    if (!cps || length == 0) return false;
    for (size_t i = 0; i < length; ++i) {
        if (!valid_scalar(cps[i])) return false;
    }

    size_t count = unicodoid_uax34_count();
    uint32_t buffer[32];
    for (size_t i = 0; i < count; ++i) {
        size_t n = unicodoid_uax34_sequence(i, buffer, 32);
        if (n != length) continue;
        if (memcmp(buffer, cps, length * sizeof(uint32_t)) == 0) return true;
    }
    return false;
}
