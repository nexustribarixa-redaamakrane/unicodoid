#include "unicodoid/uts/uts46_idna_compat.h"
#include "unicodoid/unicodoid_ucd.h"
#include "unicodoid/uax/uax15_normalization.h"

#include <stdlib.h>

int unicodoid_uts46_disposition(uint32_t cp) {
    size_t len = 0;
    return unicodoid_ucd_idna_status(cp, NULL, &len);
}

size_t unicodoid_uts46_map(const uint32_t* in, size_t length, uint32_t* out, size_t cap) {
    if (!in || length == 0) return 0;
    /* Step 4: per-scalar mapping (valid scalars pass through). */
    size_t mapped_cap = length * 4 + 4;
    uint32_t* mapped = (uint32_t*)malloc(mapped_cap * sizeof(uint32_t));
    if (!mapped) return 0;
    size_t mlen = 0;
    for (size_t i = 0; i < length; ++i) {
        size_t map_len = 0;
        int st = unicodoid_ucd_idna_status(in[i], NULL, &map_len);
        if (st == UNICODOID_UTS46_IGNORED) continue;
        if (st == UNICODOID_UTS46_MAPPED || st == UNICODOID_UTS46_DEVIATION) {
            uint32_t seq[4];
            size_t slen = 0;
            unicodoid_ucd_idna_status(in[i], seq, &slen);
            for (size_t k = 0; k < slen && mlen < mapped_cap; ++k) mapped[mlen++] = seq[k];
        } else if (st == UNICODOID_UTS46_DISALLOWED) {
            mapped[mlen++] = in[i];
        } else {
            if (mlen < mapped_cap) mapped[mlen++] = in[i];
        }
    }
    /* Step 5: NFC normalize the mapped sequence. */
    size_t total = unicodoid_uax15_normalize(mapped, mlen, 0, NULL, 0);
    if (out && cap >= total) unicodoid_uax15_normalize(mapped, mlen, 0, out, cap);
    free(mapped);
    return total;
}

bool unicodoid_uts46_check_label(const uint32_t* label, size_t length) {
    if (!label || length == 0) return false;
    for (size_t i = 0; i < length; ++i) {
        if (unicodoid_uts46_disposition(label[i]) == UNICODOID_UTS46_DISALLOWED) return false;
    }
    return true;
}

bool unicodoid_uts46_is_transitional_disallowed(uint32_t cp) {
    return unicodoid_ucd_idna_nv8(cp);
}
