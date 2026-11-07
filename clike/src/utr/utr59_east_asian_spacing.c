#include "unicodoid/utr/utr59_east_asian_spacing.h"
#include "unicodoid/uax/uax29_segmentation.h"
#include "unicodoid/unicodoid_ucd.h"

uint8_t unicodoid_utr59_east_asian_spacing(uint32_t cp) {
    return unicodoid_ucd_east_asian_spacing(cp);
}

/* UTR59 §3.3.1 step 4: upright when Vertical_Orientation is U, Tu or Tr. */
static bool is_upright(uint32_t cp) {
    uint8_t vo = unicodoid_ucd_vertical_orientation(cp);
    return vo == (uint8_t)UNICODOID_VO_U ||
           vo == (uint8_t)UNICODOID_VO_TU ||
           vo == (uint8_t)UNICODOID_VO_TR;
}

uint8_t unicodoid_utr59_cluster_value(
    const uint32_t* cps, size_t length, bool chinese, bool vertical) {
    if (!cps || length == 0) return (uint8_t)UNICODOID_EAS_O;

    /* Step 3: the first code point determines the initial value. */
    uint8_t value = unicodoid_utr59_east_asian_spacing(cps[0]);

    /* UTR59 §3.4.1: an enclosing mark forces the whole cluster to O. */
    for (size_t i = 0; i < length; ++i) {
        if (unicodoid_ucd_general_category(cps[i]) == (uint8_t)UNICODOID_GC_ME) {
            value = (uint8_t)UNICODOID_EAS_O;
            break;
        }
    }

    /* Steps 4-5: vertical layout, upright Narrow characters become Other. */
    if (vertical && value == (uint8_t)UNICODOID_EAS_N && is_upright(cps[0])) {
        value = (uint8_t)UNICODOID_EAS_O;
    }

    /* Step 6: resolve Conditional. */
    if (value == (uint8_t)UNICODOID_EAS_C) {
        value = chinese ? (uint8_t)UNICODOID_EAS_N
                        : (uint8_t)UNICODOID_EAS_O;
    }

    return value;
}

size_t unicodoid_utr59_spacing_offsets(
    const uint32_t* cps, size_t length,
    bool chinese, bool vertical,
    size_t* out, size_t cap) {
    if (!cps || length == 0) return 0;

    size_t count = 0;
    size_t start = 0;
    size_t end = unicodoid_uax29_next_grapheme(cps, length, start);
    uint8_t prev = unicodoid_utr59_cluster_value(
        cps + start, end - start, chinese, vertical);

    while (end < length) {
        size_t next_start = end;
        size_t next_end = unicodoid_uax29_next_grapheme(cps, length, next_start);
        uint8_t next = unicodoid_utr59_cluster_value(
            cps + next_start, next_end - next_start, chinese, vertical);

        bool insert =
            (prev == (uint8_t)UNICODOID_EAS_W &&
             next == (uint8_t)UNICODOID_EAS_N) ||
            (prev == (uint8_t)UNICODOID_EAS_N &&
             next == (uint8_t)UNICODOID_EAS_W);
        if (insert) {
            if (out && count < cap) out[count] = next_start;
            ++count;
        }

        prev = next;
        start = next_start;
        end = next_end;
    }

    return count;
}
