#include "unicodoid/uts/uts39_security.h"
#include "unicodoid/unicodoid_ucd.h"

#include <stdlib.h>
#include <string.h>

size_t unicodoid_uts39_confusable(uint32_t cp, uint32_t* out, size_t cap) {
    return unicodoid_ucd_confusable(cp, out, cap);
}

bool unicodoid_uts39_is_default_ignorable(uint32_t cp) {
    return unicodoid_ucd_has_binary_property(
        cp, UNICODOID_BP_DEFAULT_IGNORABLE_CODE_POINT);
}

size_t unicodoid_uts39_skeleton(const uint32_t* cps, size_t length, uint32_t* out, size_t cap) {
    if (!cps || length == 0) return 0;
    /* Fold: repeatedly apply confusable mapping until fixpoint (bounded 3 rounds). */
    size_t total = 0;
    for (size_t i = 0; i < length; ++i) {
        uint32_t cur[8];
        size_t cur_len = 1;
        cur[0] = cps[i];
        for (int round = 0; round < 3; ++round) {
            uint32_t next[8];
            size_t next_len = 0;
            for (size_t k = 0; k < cur_len; ++k) {
                uint32_t mapped[4];
                size_t m = unicodoid_ucd_confusable(cur[k], mapped, 4);
                if (m == 0) {
                    if (next_len >= 8) break;
                    next[next_len++] = cur[k];
                } else {
                    for (size_t j = 0; j < m && next_len < 8; ++j) next[next_len++] = mapped[j];
                }
            }
            if (next_len == cur_len && memcmp(cur, next, cur_len * sizeof(uint32_t)) == 0) break;
            memcpy(cur, next, next_len * sizeof(uint32_t));
            cur_len = next_len;
            if (cur_len == 0) break;
        }
        if (out && total + cur_len <= cap) {
            memcpy(out + total, cur, cur_len * sizeof(uint32_t));
        }
        total += cur_len;
    }
    return total;
}

bool unicodoid_uts39_are_confusable(
    const uint32_t* a, size_t a_len, const uint32_t* b, size_t b_len) {
    if (!a || !b) return false;
    if (a_len == 0 || b_len == 0) return a_len == b_len;
    size_t na = unicodoid_uts39_skeleton(a, a_len, NULL, 0);
    size_t nb = unicodoid_uts39_skeleton(b, b_len, NULL, 0);
    if (na != nb) return false;
    if (na == 0) return true;
    uint32_t* xa = (uint32_t*)malloc(na * sizeof(uint32_t));
    uint32_t* xb = (uint32_t*)malloc(nb * sizeof(uint32_t));
    if (!xa || !xb) { free(xa); free(xb); return false; }
    unicodoid_uts39_skeleton(a, a_len, xa, na);
    unicodoid_uts39_skeleton(b, b_len, xb, nb);
    bool eq = (memcmp(xa, xb, na * sizeof(uint32_t)) == 0);
    free(xa); free(xb);
    return eq;
}
