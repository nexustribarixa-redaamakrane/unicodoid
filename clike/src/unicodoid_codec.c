#include "unicodoid/unicodoid_codec.h"
#include <stdlib.h>
#include <string.h>

unicodoid_status_t unicodoid_codec_init(
    unicodoid_codec_t* codec,
    const unicodoid_system_t* sys
) {
    if (!codec || !sys) return UNICODOID_ERR_INVALID_PARAM;

    memset(codec, 0, sizeof(*codec));
    codec->system = sys;
    codec->state_size = sys->state_size;

    if (sys->state_size > 0) {
        codec->state = calloc(1, sys->state_size);
        if (!codec->state) {
            codec->system = NULL;
            codec->state_size = 0;
            return UNICODOID_ERR_ALLOCATION_FAILED;
        }
        if (sys->state_init) {
            unicodoid_status_t st = sys->state_init(sys, codec->state);
            if (st != UNICODOID_OK) {
                free(codec->state);
                codec->state = NULL;
                codec->system = NULL;
                codec->state_size = 0;
                return st;
            }
        }
    }

    return UNICODOID_OK;
}

void unicodoid_codec_free(unicodoid_codec_t* codec) {
    if (!codec) return;
    free(codec->state);
    codec->state = NULL;
    codec->system = NULL;
    codec->state_size = 0;
}

unicodoid_status_t unicodoid_codec_reset(unicodoid_codec_t* codec) {
    if (!codec || !codec->system) return UNICODOID_ERR_INVALID_PARAM;

    if (codec->state_size > 0) {
        if (!codec->state) return UNICODOID_ERR_STATE;
        if (codec->system->state_reset) {
            codec->system->state_reset(codec->system, codec->state);
        } else {
            memset(codec->state, 0, codec->state_size);
        }
    }
    return UNICODOID_OK;
}

unicodoid_status_t unicodoid_codec_encode_char(
    unicodoid_codec_t* codec,
    unicodoid_char64_t ch,
    void* out,
    size_t out_cap,
    size_t* out_written
) {
    if (!codec || !codec->system || !codec->system->ops.encode_char) {
        return UNICODOID_ERR_INVALID_PARAM;
    }
    if (codec->state_size > 0 && !codec->state) return UNICODOID_ERR_STATE;
    return codec->system->ops.encode_char(
        codec->system, codec->state, ch, out, out_cap, out_written
    );
}

unicodoid_status_t unicodoid_codec_decode_char(
    unicodoid_codec_t* codec,
    const void* in,
    size_t in_len,
    unicodoid_char64_t* out_ch,
    size_t* out_read
) {
    if (!codec || !codec->system || !codec->system->ops.decode_char) {
        return UNICODOID_ERR_INVALID_PARAM;
    }
    if (codec->state_size > 0 && !codec->state) return UNICODOID_ERR_STATE;
    return codec->system->ops.decode_char(
        codec->system, codec->state, in, in_len, out_ch, out_read
    );
}

unicodoid_status_t unicodoid_codec_validate(
    unicodoid_codec_t* codec,
    const void* in,
    size_t in_len,
    size_t* out_invalid_offset
) {
    if (!codec || !codec->system) return UNICODOID_ERR_INVALID_PARAM;
    if (in_len > 0 && !in) return UNICODOID_ERR_INVALID_PARAM;

    if (codec->system->ops.validate) {
        if (codec->state_size > 0 && !codec->state) return UNICODOID_ERR_STATE;
        return codec->system->ops.validate(
            codec->system, codec->state, in, in_len, out_invalid_offset
        );
    }

    /* Fallback: decode the whole buffer. */
    size_t off = 0;
    while (off < in_len) {
        unicodoid_char64_t ch = 0;
        size_t read = 0;
        unicodoid_status_t st = unicodoid_codec_decode_char(
            codec, (const uint8_t*)in + off, in_len - off, &ch, &read
        );
        if (st == UNICODOID_ERR_NO_CHARACTER) {
            if (read == 0) {
                if (out_invalid_offset) *out_invalid_offset = off;
                return UNICODOID_ERR_INTERNAL;
            }
            off += read;
            continue;
        }
        if (st != UNICODOID_OK) {
            if (out_invalid_offset) *out_invalid_offset = off;
            return st;
        }
        if (read == 0) {
            if (out_invalid_offset) *out_invalid_offset = off;
            return UNICODOID_ERR_INTERNAL;
        }
        off += read;
    }
    return UNICODOID_OK;
}

unicodoid_status_t unicodoid_codec_char_count(
    unicodoid_codec_t* codec,
    const void* in,
    size_t in_len,
    size_t* out_count
) {
    if (!codec || !codec->system || !out_count) return UNICODOID_ERR_INVALID_PARAM;
    if (in_len > 0 && !in) return UNICODOID_ERR_INVALID_PARAM;

    if (codec->system->ops.char_count) {
        if (codec->state_size > 0 && !codec->state) return UNICODOID_ERR_STATE;
        return codec->system->ops.char_count(
            codec->system, codec->state, in, in_len, out_count
        );
    }

    size_t count = 0;
    size_t off = 0;
    while (off < in_len) {
        unicodoid_char64_t ch = 0;
        size_t read = 0;
        unicodoid_status_t st = unicodoid_codec_decode_char(
            codec, (const uint8_t*)in + off, in_len - off, &ch, &read
        );
        if (st == UNICODOID_ERR_NO_CHARACTER) {
            if (read == 0) return UNICODOID_ERR_INTERNAL;
            off += read;
            continue;
        }
        if (st != UNICODOID_OK) return st;
        if (read == 0) return UNICODOID_ERR_INTERNAL;
        off += read;
        ++count;
    }
    *out_count = count;
    return UNICODOID_OK;
}

unicodoid_status_t unicodoid_codec_encode(
    unicodoid_codec_t* codec,
    const unicodoid_char64_t* chars,
    size_t num_chars,
    void* out,
    size_t out_cap,
    size_t* out_written
) {
    if (!codec || !out_written) return UNICODOID_ERR_INVALID_PARAM;
    if (num_chars > 0 && !chars) return UNICODOID_ERR_INVALID_PARAM;
    if (out_cap > 0 && !out) return UNICODOID_ERR_INVALID_PARAM;

    *out_written = 0;
    /* out may be NULL only when out_cap == 0 (checked above).  Pointer
     * arithmetic on a NULL pointer is undefined behaviour in C even with
     * offset zero, so when out is NULL pass a valid dummy pointer instead;
     * encoders check out_cap before writing and will report
     * UNICODOID_ERR_BUFFER_TOO_SMALL (or UNMAPPABLE_CHAR for an invalid
     * value) without dereferencing it. */
    uint8_t dummy_slot = 0;
    size_t off = 0;
    for (size_t i = 0; i < num_chars; ++i) {
        if (off > out_cap) {
            *out_written = off;
            return UNICODOID_ERR_OVERFLOW;
        }
        void* dst = out ? (void*)((uint8_t*)out + off) : (void*)&dummy_slot;
        size_t written = 0;
        unicodoid_status_t st = unicodoid_codec_encode_char(
            codec, chars[i], dst, out_cap - off, &written
        );
        if (st != UNICODOID_OK) {
            *out_written = off;
            return st;
        }
        off += written;
    }
    *out_written = off;
    return UNICODOID_OK;
}

unicodoid_status_t unicodoid_codec_decode(
    unicodoid_codec_t* codec,
    const void* in,
    size_t in_len,
    unicodoid_char64_t* out_chars,
    size_t out_capacity,
    size_t* out_count
) {
    if (!codec || !out_count) return UNICODOID_ERR_INVALID_PARAM;
    if (in_len > 0 && !in) return UNICODOID_ERR_INVALID_PARAM;

    size_t count = 0;
    size_t off = 0;
    while (off < in_len) {
        unicodoid_char64_t ch = 0;
        size_t read = 0;
        unicodoid_status_t st = unicodoid_codec_decode_char(
            codec, (const uint8_t*)in + off, in_len - off, &ch, &read
        );
        if (st == UNICODOID_ERR_NO_CHARACTER) {
            if (read == 0) {
                *out_count = count;
                return UNICODOID_ERR_INTERNAL;
            }
            off += read;
            continue;
        }
        if (st != UNICODOID_OK) {
            *out_count = count;
            return st;
        }
        if (read == 0) {
            *out_count = count;
            return UNICODOID_ERR_INTERNAL;
        }
        if (out_chars) {
            if (count >= out_capacity) {
                *out_count = count;
                return UNICODOID_ERR_BUFFER_TOO_SMALL;
            }
            out_chars[count] = ch;
        }
        off += read;
        ++count;
    }
    *out_count = count;
    return UNICODOID_OK;
}
