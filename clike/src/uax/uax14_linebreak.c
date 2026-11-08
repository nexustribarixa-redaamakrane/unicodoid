#include "unicodoid/uax/uax14_linebreak.h"
#include "unicodoid/unicodoid_segmentation.h"
#include "unicodoid/unicodoid_ucd.h"
#include "unicodoid/systems/unicodoid_utf8.h"

#include <stdlib.h>

uint8_t unicodoid_uax14_line_break(uint32_t cp) {
    return unicodoid_ucd_line_break(cp);
}

/* Encode scalars to UTF-8 bytes (scalar values must be valid). */
static size_t utf8_encode(const uint32_t* cps, size_t length, uint8_t* out, size_t cap,
                          size_t* offsets) {
    size_t bytes = 0;
    for (size_t i = 0; i < length; ++i) {
        uint32_t cp = cps[i];
        if (offsets) offsets[i] = bytes;
        size_t n = (cp <= 0x7F) ? 1 : (cp <= 0x7FF) ? 2 : (cp <= 0xFFFF) ? 3 : 4;
        if (out && bytes + n <= cap) {
            if (cp <= 0x7F) {
                out[bytes] = (uint8_t)cp;
            } else if (cp <= 0x7FF) {
                out[bytes] = (uint8_t)(0xC0 | (cp >> 6));
                out[bytes + 1] = (uint8_t)(0x80 | (cp & 0x3F));
            } else if (cp <= 0xFFFF) {
                out[bytes] = (uint8_t)(0xE0 | (cp >> 12));
                out[bytes + 1] = (uint8_t)(0x80 | ((cp >> 6) & 0x3F));
                out[bytes + 2] = (uint8_t)(0x80 | (cp & 0x3F));
            } else {
                out[bytes] = (uint8_t)(0xF0 | (cp >> 18));
                out[bytes + 1] = (uint8_t)(0x80 | ((cp >> 12) & 0x3F));
                out[bytes + 2] = (uint8_t)(0x80 | ((cp >> 6) & 0x3F));
                out[bytes + 3] = (uint8_t)(0x80 | (cp & 0x3F));
            }
        }
        bytes += n;
    }
    if (offsets) offsets[length] = bytes;
    return bytes;
}

static size_t decode_offsets(const uint8_t* bytes, size_t len,
                             size_t* scalar_offsets, size_t scalar_cap) {
    /* Count scalars / record byte offsets by decoding UTF-8 lengths. */
    size_t s = 0, b = 0;
    while (b < len && s < scalar_cap) {
        scalar_offsets[s++] = b;
        uint8_t b0 = bytes[b];
        if ((b0 & 0x80) == 0) b += 1;
        else if ((b0 & 0xE0) == 0xC0) b += 2;
        else if ((b0 & 0xF0) == 0xE0) b += 3;
        else b += 4;
    }
    if (s < scalar_cap) scalar_offsets[s] = len;
    return s;
}

size_t unicodoid_uax14_next_break(const uint32_t* cps, size_t length, size_t start) {
    if (!cps || start >= length) return length;

    size_t need = utf8_encode(cps, length, NULL, 0, NULL);
    if (need == 0) return length;
    uint8_t* bytes = (uint8_t*)malloc(need);
    size_t* b_off = (size_t*)malloc((length + 1) * sizeof(size_t));
    if (!bytes || !b_off) { free(bytes); free(b_off); return start + 1; }
    utf8_encode(cps, length, bytes, need, b_off);

    const unicodoid_system_t* sys = unicodoid_system_get_utf8();
    unicodoid_segmenter_t seg;
    size_t result = start + 1;
    if (unicodoid_segmenter_init(&seg, sys, bytes, need, UNICODOID_BOUNDARY_LINE) == UNICODOID_OK) {
        size_t byte_start = b_off[start];
        size_t byte_end = b_off[length];
        unicodoid_segment_t out;
        while (unicodoid_segmenter_next(&seg, &out)) {
            if (out.byte_offset == byte_start) {
                byte_end = out.byte_offset + out.byte_length;
                break;
            }
            if (out.byte_offset > byte_start) { byte_end = out.byte_offset; break; }
        }
        unicodoid_segmenter_free(&seg);
        /* Map byte end back to scalar index. */
        size_t* s_off = (size_t*)malloc((length + 2) * sizeof(size_t));
        if (s_off) {
            size_t n = decode_offsets(bytes, need, s_off, length + 1);
            result = n;
            for (size_t k = 0; k <= n && k <= length + 1; ++k) {
                if (s_off[k] == byte_end) { result = k; break; }
                if (s_off[k] > byte_end) { result = k; break; }
            }
            if (result < start + 1) result = start + 1;
            if (result > length) result = length;
            free(s_off);
        }
    }
    free(bytes);
    free(b_off);
    return result;
}

bool unicodoid_uax14_is_break(const uint32_t* cps, size_t length, size_t index) {
    if (index == 0 || index >= length) return true;
    size_t start = 0;
    while (start < index) {
        size_t next = unicodoid_uax14_next_break(cps, length, start);
        if (next <= start) return false;
        start = next;
    }
    return start == index;
}
