#include "unicodoid/unicodoid_string.h"
#include "unicodoid/unicodoid_codec.h"
#include <stdlib.h>
#include <string.h>

static unicodoid_status_t alloc_state(unicodoid_string_t* str, const unicodoid_system_t* sys) {
    str->state = NULL;
    str->state_size = 0;
    if (sys->state_size == 0) return UNICODOID_OK;

    str->state = calloc(1, sys->state_size);
    if (!str->state) return UNICODOID_ERR_ALLOCATION_FAILED;
    str->state_size = sys->state_size;

    if (sys->state_init) {
        unicodoid_status_t st = sys->state_init(sys, str->state);
        if (st != UNICODOID_OK) {
            free(str->state);
            str->state = NULL;
            str->state_size = 0;
            return st;
        }
    }
    return UNICODOID_OK;
}

unicodoid_status_t unicodoid_string_init(
    unicodoid_string_t* str,
    const unicodoid_system_t* sys,
    size_t initial_capacity
) {
    if (!str || !sys) return UNICODOID_ERR_INVALID_PARAM;

    memset(str, 0, sizeof(*str));
    str->system = sys;
    str->capacity = initial_capacity > 0 ? initial_capacity : 16;
    str->data = (uint8_t*)malloc(str->capacity);
    if (!str->data) {
        str->capacity = 0;
        str->system = NULL;
        return UNICODOID_ERR_ALLOCATION_FAILED;
    }
    str->is_owned = true;

    unicodoid_status_t st = alloc_state(str, sys);
    if (st != UNICODOID_OK) {
        free(str->data);
        str->data = NULL;
        str->capacity = 0;
        str->is_owned = false;
        str->system = NULL;
        return st;
    }
    return UNICODOID_OK;
}

unicodoid_status_t unicodoid_string_from_slice(
    unicodoid_string_t* str,
    const unicodoid_system_t* sys,
    const void* buffer,
    size_t length_bytes
) {
    if (!str || !sys) return UNICODOID_ERR_INVALID_PARAM;
    if (length_bytes > 0 && !buffer) return UNICODOID_ERR_INVALID_PARAM;

    if (length_bytes > 0) {
        unicodoid_codec_t codec;
        unicodoid_status_t st = unicodoid_codec_init(&codec, sys);
        if (st != UNICODOID_OK) return st;
        st = unicodoid_codec_validate(&codec, buffer, length_bytes, NULL);
        unicodoid_codec_free(&codec);
        if (st != UNICODOID_OK) return st;
    }

    memset(str, 0, sizeof(*str));
    str->system = sys;
    str->data = (uint8_t*)buffer;
    str->length = length_bytes;
    str->capacity = length_bytes;
    str->is_owned = false;

    return alloc_state(str, sys);
}

unicodoid_status_t unicodoid_string_from_chars(
    unicodoid_string_t* str,
    const unicodoid_system_t* sys,
    const unicodoid_char64_t* chars,
    size_t num_chars
) {
    if (!str || !sys) return UNICODOID_ERR_INVALID_PARAM;
    if (num_chars > 0 && !chars) return UNICODOID_ERR_INVALID_PARAM;

    size_t req_bytes = 0;
    if (sys->ops.byte_count) {
        unicodoid_status_t st = sys->ops.byte_count(sys, chars, num_chars, &req_bytes);
        if (st != UNICODOID_OK) return st;
    } else {
        size_t per_char = sys->meta.max_bytes_per_char ? sys->meta.max_bytes_per_char : 8;
        /* Check for size_t overflow before multiplying. */
        if (per_char > 0 && num_chars > (SIZE_MAX / per_char)) return UNICODOID_ERR_OVERFLOW;
        req_bytes = num_chars * per_char;
    }

    unicodoid_status_t st = unicodoid_string_init(str, sys, req_bytes > 0 ? req_bytes : 16);
    if (st != UNICODOID_OK) return st;

    for (size_t i = 0; i < num_chars; ++i) {
        st = unicodoid_string_append_char(str, chars[i]);
        if (st != UNICODOID_OK) {
            unicodoid_string_free(str);
            return st;
        }
    }
    return UNICODOID_OK;
}

void unicodoid_string_free(unicodoid_string_t* str) {
    if (!str) return;
    if (str->is_owned && str->data) free(str->data);
    free(str->state);
    memset(str, 0, sizeof(*str));
}

unicodoid_status_t unicodoid_string_clone(
    const unicodoid_string_t* src,
    unicodoid_string_t* out_clone
) {
    if (!src || !out_clone) return UNICODOID_ERR_INVALID_PARAM;

    unicodoid_status_t st = unicodoid_string_init(
        out_clone, src->system, src->length > 0 ? src->length : 16
    );
    if (st != UNICODOID_OK) return st;

    if (src->length > 0) {
        memcpy(out_clone->data, src->data, src->length);
        out_clone->length = src->length;
    }
    if (src->state_size > 0 && src->state) {
        if (out_clone->state_size != src->state_size || !out_clone->state) {
            unicodoid_string_free(out_clone);
            return UNICODOID_ERR_STATE;
        }
        memcpy(out_clone->state, src->state, src->state_size);
    }
    return UNICODOID_OK;
}

static unicodoid_status_t ensure_capacity(unicodoid_string_t* str, size_t needed) {
    /* Check for size_t overflow before adding: length + needed could wrap. */
    if (needed > SIZE_MAX - str->length) return UNICODOID_ERR_OVERFLOW;
    size_t total = str->length + needed;

    if (!str->is_owned) {
        /* Grow to at least double the required total (min 16), without overflow. */
        size_t new_cap = total;
        if (new_cap <= (SIZE_MAX / 2)) new_cap *= 2;
        if (new_cap < 16) new_cap = 16;
        uint8_t* new_data = (uint8_t*)malloc(new_cap);
        if (!new_data) return UNICODOID_ERR_ALLOCATION_FAILED;
        if (str->length > 0 && str->data) memcpy(new_data, str->data, str->length);
        str->data = new_data;
        str->capacity = new_cap;
        str->is_owned = true;
        return UNICODOID_OK;
    }

    if (total > str->capacity) {
        size_t new_cap = str->capacity ? str->capacity : 16;
        if (new_cap <= (SIZE_MAX / 2)) new_cap *= 2;
        if (new_cap < total) {
            /* total + 16 could overflow; total alone is an exact fit. */
            new_cap = (total <= (SIZE_MAX - 16)) ? total + 16 : total;
        }
        uint8_t* new_data = (uint8_t*)realloc(str->data, new_cap);
        if (!new_data) return UNICODOID_ERR_ALLOCATION_FAILED;
        str->data = new_data;
        str->capacity = new_cap;
    }
    return UNICODOID_OK;
}

unicodoid_status_t unicodoid_string_append_char(
    unicodoid_string_t* str,
    unicodoid_char64_t ch
) {
    if (!str || !str->system || !str->system->ops.encode_char) {
        return UNICODOID_ERR_INVALID_PARAM;
    }
    if (str->state_size > 0 && !str->state) return UNICODOID_ERR_STATE;

    size_t max_bytes = str->system->meta.max_bytes_per_char;
    if (max_bytes == 0) max_bytes = 8;

    unicodoid_status_t st = ensure_capacity(str, max_bytes);
    if (st != UNICODOID_OK) return st;

    size_t written = 0;
    st = str->system->ops.encode_char(
        str->system,
        str->state,
        ch,
        str->data + str->length,
        str->capacity - str->length,
        &written
    );
    if (st != UNICODOID_OK) return st;

    str->length += written;
    return UNICODOID_OK;
}

unicodoid_status_t unicodoid_string_append_bytes(
    unicodoid_string_t* str,
    const void* bytes,
    size_t num_bytes
) {
    if (!str || !str->system) return UNICODOID_ERR_INVALID_PARAM;
    if (num_bytes == 0) return UNICODOID_OK;
    if (!bytes) return UNICODOID_ERR_INVALID_PARAM;

    /*
     * Validate against a copy of the string's current codec state, then commit
     * that state only if the bytes are valid.  This keeps append_bytes()
     * coherent for modal systems without interpreting the bytes twice.
     */
    if (str->system->ops.validate) {
        void* trial_state = NULL;
        if (str->state_size > 0) {
            if (!str->state) return UNICODOID_ERR_STATE;
            trial_state = malloc(str->state_size);
            if (!trial_state) return UNICODOID_ERR_ALLOCATION_FAILED;
            memcpy(trial_state, str->state, str->state_size);
        }

        unicodoid_status_t st = str->system->ops.validate(
            str->system, trial_state, bytes, num_bytes, NULL
        );
        if (st == UNICODOID_OK && str->state_size > 0) {
            memcpy(str->state, trial_state, str->state_size);
        }
        free(trial_state);
        if (st != UNICODOID_OK) return st;
    }

    unicodoid_status_t st = ensure_capacity(str, num_bytes);
    if (st != UNICODOID_OK) return st;

    memcpy(str->data + str->length, bytes, num_bytes);
    str->length += num_bytes;
    return UNICODOID_OK;
}

unicodoid_status_t unicodoid_string_char_length(
    const unicodoid_string_t* str,
    size_t* out_char_count
) {
    if (!str || !str->system || !out_char_count) return UNICODOID_ERR_INVALID_PARAM;
    if (str->length == 0) {
        *out_char_count = 0;
        return UNICODOID_OK;
    }

    unicodoid_codec_t codec;
    unicodoid_status_t st = unicodoid_codec_init(&codec, str->system);
    if (st != UNICODOID_OK) return st;
    st = unicodoid_codec_char_count(&codec, str->data, str->length, out_char_count);
    unicodoid_codec_free(&codec);
    return st;
}

unicodoid_status_t unicodoid_string_get_char_at(
    const unicodoid_string_t* str,
    size_t char_index,
    unicodoid_char64_t* out_ch
) {
    if (!str || !str->system || !out_ch) return UNICODOID_ERR_INVALID_PARAM;

    unicodoid_codec_t codec;
    unicodoid_status_t st = unicodoid_codec_init(&codec, str->system);
    if (st != UNICODOID_OK) return st;

    size_t index = 0;
    size_t off = 0;
    while (off < str->length) {
        unicodoid_char64_t ch = 0;
        size_t read = 0;
        st = unicodoid_codec_decode_char(
            &codec, str->data + off, str->length - off, &ch, &read
        );
        if (st == UNICODOID_ERR_NO_CHARACTER) {
            if (read == 0) {
                unicodoid_codec_free(&codec);
                return UNICODOID_ERR_INTERNAL;
            }
            off += read;
            continue;
        }
        if (st != UNICODOID_OK) {
            unicodoid_codec_free(&codec);
            return st;
        }
        if (index == char_index) {
            *out_ch = ch;
            unicodoid_codec_free(&codec);
            return UNICODOID_OK;
        }
        off += read;
        ++index;
    }

    unicodoid_codec_free(&codec);
    return UNICODOID_ERR_NOT_FOUND;
}

unicodoid_status_t unicodoid_string_reset_state(unicodoid_string_t* str) {
    if (!str || !str->system) return UNICODOID_ERR_INVALID_PARAM;
    if (str->state_size == 0) return UNICODOID_OK;
    if (!str->state) return UNICODOID_ERR_STATE;

    if (str->system->state_reset) {
        str->system->state_reset(str->system, str->state);
    } else {
        memset(str->state, 0, str->state_size);
    }
    return UNICODOID_OK;
}

bool unicodoid_string_equals(
    const unicodoid_string_t* a,
    const unicodoid_string_t* b
) {
    if (!a || !b) return false;
    if (a == b) return true;
    if (a->length != b->length) return false;
    if (a->length == 0) return true;
    return memcmp(a->data, b->data, a->length) == 0;
}

int unicodoid_string_compare(
    const unicodoid_string_t* a,
    const unicodoid_string_t* b
) {
    if (!a || !b) return 0;
    if (a == b) return 0;
    size_t min_len = a->length < b->length ? a->length : b->length;
    if (min_len > 0) {
        int r = memcmp(a->data, b->data, min_len);
        if (r != 0) return r;
    }
    if (a->length < b->length) return -1;
    if (a->length > b->length) return 1;
    return 0;
}
