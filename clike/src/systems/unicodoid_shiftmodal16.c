#include "unicodoid/systems/unicodoid_shiftmodal16.h"
#include <string.h>

/*
 * ShiftModal-16: a deliberately non-Unicode, modal, stateful example system.
 *
 * Base mode:
 *   0x00..0x7F  -> values 0x0000..0x007F, except the three control bytes below.
 *   0x1B b      -> value 0x0100 | b   (escape; consumes the following byte)
 *   0x0E        -> switch to shifted mode (emits no character)
 *   0x0F        -> switch to base mode    (emits no character)
 *
 * Shifted mode:
 *   0x0F        -> switch to base mode    (emits no character)
 *   0x1B b      -> value 0x0100 | b      (escape; mode is unchanged)
 *   b           -> value 0x4000 | b      (0x0E is ordinary data here)
 *
 * Reserved / unrepresentable native values:
 *   0x000E, 0x000F, 0x001B in base mode, and 0x400F, 0x401B in shifted mode.
 *   The encoder reports UNICODOID_ERR_UNMAPPABLE_CHAR for those values.
 *
 * decode_char() consumes shift controls as part of the next character, so a
 * stream consisting only of controls is reported as an incomplete sequence.
 *
 * The system intentionally has no Unicode profile: it exists to prove that the
 * codec, string, and transcoding layers do not assume a Unicode relationship.
 */

typedef enum shiftmodal_mode {
    SHIFTMODAL_BASE = 0,
    SHIFTMODAL_SHIFTED = 1
} shiftmodal_mode_t;

typedef struct shiftmodal_state {
    uint8_t mode; /* shiftmodal_mode_t */
} shiftmodal_state_t;

#define SHIFTMODAL_ESC 0x1Bu
#define SHIFTMODAL_SO  0x0Eu
#define SHIFTMODAL_SI  0x0Fu

static unicodoid_status_t shiftmodal_state_init(
    const unicodoid_system_t* sys,
    void* state
) {
    (void)sys;
    if (!state) return UNICODOID_ERR_INVALID_PARAM;
    ((shiftmodal_state_t*)state)->mode = SHIFTMODAL_BASE;
    return UNICODOID_OK;
}

static void shiftmodal_state_reset(
    const unicodoid_system_t* sys,
    void* state
) {
    (void)sys;
    if (state) ((shiftmodal_state_t*)state)->mode = SHIFTMODAL_BASE;
}

/* Return the caller's state, or a local base-mode state for direct calls. */
static shiftmodal_state_t* shiftmodal_get_state(void* state, shiftmodal_state_t* local) {
    if (state) return (shiftmodal_state_t*)state;
    local->mode = SHIFTMODAL_BASE;
    return local;
}

static bool shiftmodal_base_reserved(uint8_t b) {
    return b == SHIFTMODAL_ESC || b == SHIFTMODAL_SO || b == SHIFTMODAL_SI;
}

static unicodoid_status_t shiftmodal_encode_char(
    const unicodoid_system_t* sys,
    void* state,
    unicodoid_char64_t ch,
    void* out_buf,
    size_t out_cap,
    size_t* out_written
) {
    (void)sys;
    if (!out_buf || !out_written) return UNICODOID_ERR_INVALID_PARAM;

    shiftmodal_state_t local;
    shiftmodal_state_t* st = shiftmodal_get_state(state, &local);
    uint8_t* p = (uint8_t*)out_buf;

    if (ch <= 0x7F) {
        uint8_t b = (uint8_t)ch;
        if (shiftmodal_base_reserved(b)) return UNICODOID_ERR_UNMAPPABLE_CHAR;

        if (st->mode == SHIFTMODAL_SHIFTED) {
            if (out_cap < 2) return UNICODOID_ERR_BUFFER_TOO_SMALL;
            p[0] = SHIFTMODAL_SI;
            p[1] = b;
            st->mode = SHIFTMODAL_BASE;
            *out_written = 2;
        } else {
            if (out_cap < 1) return UNICODOID_ERR_BUFFER_TOO_SMALL;
            p[0] = b;
            *out_written = 1;
        }
        return UNICODOID_OK;
    }

    if (ch >= 0x0100 && ch <= 0x01FF) {
        if (out_cap < 2) return UNICODOID_ERR_BUFFER_TOO_SMALL;
        p[0] = SHIFTMODAL_ESC;
        p[1] = (uint8_t)(ch & 0xFF);
        *out_written = 2;
        return UNICODOID_OK;
    }

    if (ch >= 0x4000 && ch <= 0x40FF) {
        uint8_t b = (uint8_t)(ch & 0xFF);
        /* In shifted mode SI and ESC are controls, not data. */
        if (b == SHIFTMODAL_SI || b == SHIFTMODAL_ESC) {
            return UNICODOID_ERR_UNMAPPABLE_CHAR;
        }

        if (st->mode == SHIFTMODAL_BASE) {
            if (out_cap < 2) return UNICODOID_ERR_BUFFER_TOO_SMALL;
            p[0] = SHIFTMODAL_SO;
            p[1] = b;
            st->mode = SHIFTMODAL_SHIFTED;
            *out_written = 2;
        } else {
            if (out_cap < 1) return UNICODOID_ERR_BUFFER_TOO_SMALL;
            p[0] = b;
            *out_written = 1;
        }
        return UNICODOID_OK;
    }

    return UNICODOID_ERR_UNMAPPABLE_CHAR;
}

static unicodoid_status_t shiftmodal_decode_char(
    const unicodoid_system_t* sys,
    void* state,
    const void* in_buf,
    size_t in_len,
    unicodoid_char64_t* out_ch,
    size_t* out_read
) {
    (void)sys;
    if (!in_buf || !out_ch || !out_read) return UNICODOID_ERR_INVALID_PARAM;
    if (in_len == 0) return UNICODOID_ERR_INCOMPLETE_SEQUENCE;

    shiftmodal_state_t local;
    shiftmodal_state_t* st = shiftmodal_get_state(state, &local);
    const uint8_t* p = (const uint8_t*)in_buf;

    /* Do not commit a partial control run if the buffer ends too early. */
    shiftmodal_state_t saved = *st;
    size_t consumed = 0;

    while (consumed < in_len) {
        uint8_t b = p[consumed];

        if (b == SHIFTMODAL_ESC) {
            if (consumed + 1 >= in_len) {
                *st = saved;
                return UNICODOID_ERR_INCOMPLETE_SEQUENCE;
            }
            *out_ch = (unicodoid_char64_t)(0x0100u | p[consumed + 1]);
            *out_read = consumed + 2;
            return UNICODOID_OK;
        }

        if (b == SHIFTMODAL_SI) {
            st->mode = SHIFTMODAL_BASE;
            ++consumed;
            continue;
        }

        if (st->mode == SHIFTMODAL_SHIFTED) {
            *out_ch = (unicodoid_char64_t)(0x4000u | b);
            *out_read = consumed + 1;
            return UNICODOID_OK;
        }

        if (b == SHIFTMODAL_SO) {
            st->mode = SHIFTMODAL_SHIFTED;
            ++consumed;
            continue;
        }

        *out_ch = (unicodoid_char64_t)b;
        *out_read = consumed + 1;
        return UNICODOID_OK;
    }

    /* Only shift controls were present: the state change is committed. */
    *out_read = consumed;
    return UNICODOID_ERR_NO_CHARACTER;
}

static unicodoid_status_t shiftmodal_validate(
    const unicodoid_system_t* sys,
    void* state,
    const void* in_buf,
    size_t in_len,
    size_t* out_invalid_offset
) {
    shiftmodal_state_t local;
    shiftmodal_state_t* st = shiftmodal_get_state(state, &local);
    const uint8_t* p = (const uint8_t*)in_buf;

    size_t off = 0;
    while (off < in_len) {
        unicodoid_char64_t ch = 0;
        size_t read = 0;
        unicodoid_status_t status = shiftmodal_decode_char(
            sys, st, p + off, in_len - off, &ch, &read
        );
        if (status == UNICODOID_ERR_NO_CHARACTER) {
            if (read == 0) {
                if (out_invalid_offset) *out_invalid_offset = off;
                return UNICODOID_ERR_INTERNAL;
            }
            off += read;
            continue;
        }
        if (status != UNICODOID_OK) {
            if (out_invalid_offset) *out_invalid_offset = off;
            return status;
        }
        if (read == 0) {
            if (out_invalid_offset) *out_invalid_offset = off;
            return UNICODOID_ERR_INTERNAL;
        }
        off += read;
    }
    return UNICODOID_OK;
}

static unicodoid_status_t shiftmodal_char_count(
    const unicodoid_system_t* sys,
    void* state,
    const void* in_buf,
    size_t in_len,
    size_t* out_count
) {
    if (!in_buf || !out_count) return UNICODOID_ERR_INVALID_PARAM;

    shiftmodal_state_t local;
    shiftmodal_state_t* st = shiftmodal_get_state(state, &local);
    const uint8_t* p = (const uint8_t*)in_buf;

    size_t count = 0;
    size_t off = 0;
    while (off < in_len) {
        unicodoid_char64_t ch = 0;
        size_t read = 0;
        unicodoid_status_t status = shiftmodal_decode_char(
            sys, st, p + off, in_len - off, &ch, &read
        );
        if (status == UNICODOID_ERR_NO_CHARACTER) {
            if (read == 0) return UNICODOID_ERR_INTERNAL;
            off += read;
            continue;
        }
        if (status != UNICODOID_OK) return status;
        if (read == 0) return UNICODOID_ERR_INTERNAL;
        off += read;
        ++count;
    }
    *out_count = count;
    return UNICODOID_OK;
}

/*
 * Byte sizing is a pure operation.  It simulates a fresh stream starting in
 * base mode; a mid-stream count may differ because the current mode affects
 * whether a shift byte is required.
 */
static unicodoid_status_t shiftmodal_byte_count(
    const unicodoid_system_t* sys,
    const unicodoid_char64_t* chars,
    size_t num_chars,
    size_t* out_bytes
) {
    (void)sys;
    if (!chars || !out_bytes) return UNICODOID_ERR_INVALID_PARAM;

    shiftmodal_mode_t mode = SHIFTMODAL_BASE;
    size_t total = 0;

    for (size_t i = 0; i < num_chars; ++i) {
        unicodoid_char64_t ch = chars[i];
        if (ch <= 0x7F) {
            if (shiftmodal_base_reserved((uint8_t)ch)) return UNICODOID_ERR_UNMAPPABLE_CHAR;
            if (mode == SHIFTMODAL_SHIFTED) {
                total += 2;
                mode = SHIFTMODAL_BASE;
            } else {
                total += 1;
            }
        } else if (ch >= 0x0100 && ch <= 0x01FF) {
            total += 2;
        } else if (ch >= 0x4000 && ch <= 0x40FF) {
            uint8_t b = (uint8_t)(ch & 0xFF);
            if (b == SHIFTMODAL_SI || b == SHIFTMODAL_ESC) {
                return UNICODOID_ERR_UNMAPPABLE_CHAR;
            }
            if (mode == SHIFTMODAL_BASE) {
                total += 2;
                mode = SHIFTMODAL_SHIFTED;
            } else {
                total += 1;
            }
        } else {
            return UNICODOID_ERR_UNMAPPABLE_CHAR;
        }
    }

    *out_bytes = total;
    return UNICODOID_OK;
}

static const unicodoid_system_t s_shiftmodal16_system = {
    .meta = {
        .id = "org.unicodoid.shiftmodal16",
        .name = "ShiftModal-16",
        .version = "1.0.0",
        .author = "Unicodoid Reference Systems",
        .description = "Modal, stateful 16-bit example system with no Unicode profile",
        .code_unit_bits = 8,
        .min_bytes_per_char = 1,
        .max_bytes_per_char = 2,
        .model = UNICODOID_MODEL_MODAL_STATEFUL,
        .endianness = UNICODOID_ENDIAN_NONE,
        .compat_level = UNICODOID_COMPAT_NONE,
        .max_char_val = 0x40FF,
        .replacement_char = 0x003F
    },
    .ops = {
        .encode_char = shiftmodal_encode_char,
        .decode_char = shiftmodal_decode_char,
        .validate = shiftmodal_validate,
        .char_count = shiftmodal_char_count,
        .byte_count = shiftmodal_byte_count
    },
    .state_size = sizeof(shiftmodal_state_t),
    .state_init = shiftmodal_state_init,
    .state_reset = shiftmodal_state_reset,
    .unicode_profile = NULL,
    .user_data = NULL
};

const unicodoid_system_t* unicodoid_system_get_shiftmodal16(void) {
    return &s_shiftmodal16_system;
}
