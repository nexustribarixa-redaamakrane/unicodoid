#include "unicodoid/unicodoid_transcode.h"
#include "unicodoid/unicodoid_codec.h"
#include <stdlib.h>
#include <string.h>

typedef struct byte_buffer {
    uint8_t* data;
    size_t   length;
    size_t   capacity;
} byte_buffer_t;

static unicodoid_status_t buffer_reserve(byte_buffer_t* buf, size_t extra) {
    /* Check for size_t overflow before adding: length + extra could wrap. */
    if (extra > SIZE_MAX - buf->length) return UNICODOID_ERR_OVERFLOW;
    size_t needed = buf->length + extra;
    if (needed <= buf->capacity) return UNICODOID_OK;

    size_t new_cap = buf->capacity ? buf->capacity : 64;
    while (new_cap < needed) {
        if (new_cap > (SIZE_MAX / 2)) return UNICODOID_ERR_OVERFLOW;
        new_cap *= 2;
    }
    uint8_t* p = (uint8_t*)realloc(buf->data, new_cap);
    if (!p) return UNICODOID_ERR_ALLOCATION_FAILED;
    buf->data = p;
    buf->capacity = new_cap;
    return UNICODOID_OK;
}

static unicodoid_status_t buffer_append(byte_buffer_t* buf, const void* data, size_t len) {
    if (len == 0) return UNICODOID_OK;
    unicodoid_status_t st = buffer_reserve(buf, len);
    if (st != UNICODOID_OK) return st;
    memcpy(buf->data + buf->length, data, len);
    buf->length += len;
    return UNICODOID_OK;
}

static void buffer_free(byte_buffer_t* buf) {
    free(buf->data);
    memset(buf, 0, sizeof(*buf));
}

static bool same_system(const unicodoid_system_t* a, const unicodoid_system_t* b) {
    if (a == b) return true;
    if (!a || !b) return false;
    return a->meta.id && b->meta.id && strcmp(a->meta.id, b->meta.id) == 0;
}

unicodoid_status_t unicodoid_transcode_char(
    const unicodoid_system_t* src_sys,
    unicodoid_char64_t src_ch,
    const unicodoid_system_t* dst_sys,
    unicodoid_char64_t* out_dst_ch,
    const unicodoid_transcode_config_t* cfg
) {
    if (!src_sys || !dst_sys || !out_dst_ch) return UNICODOID_ERR_INVALID_PARAM;

    if (same_system(src_sys, dst_sys)) {
        *out_dst_ch = src_ch;
        return UNICODOID_OK;
    }

    if (unicodoid_system_has_unicode_profile(src_sys) &&
        unicodoid_system_has_unicode_profile(dst_sys)) {
        uint32_t scalar = 0;
        unicodoid_status_t st = src_sys->unicode_profile->to_unicode(
            src_sys->unicode_profile, src_ch, &scalar
        );
        if (st == UNICODOID_OK) {
            st = dst_sys->unicode_profile->from_unicode(
                dst_sys->unicode_profile, scalar, out_dst_ch
            );
        }
        return st;
    }

    if (cfg && cfg->bridge) {
        return cfg->bridge(
            cfg->bridge_user_data, src_sys, src_ch, dst_sys, out_dst_ch
        );
    }

    /* No implicit identity mapping between unrelated systems. */
    return UNICODOID_ERR_UNMAPPABLE_CHAR;
}

static unicodoid_status_t encode_into_buffer(
    unicodoid_codec_t* codec,
    unicodoid_char64_t ch,
    byte_buffer_t* out
) {
    size_t reserve = codec->system->meta.max_bytes_per_char;
    if (reserve == 0) reserve = 8;

    unicodoid_status_t st = buffer_reserve(out, reserve);
    if (st != UNICODOID_OK) return st;

    size_t written = 0;
    st = unicodoid_codec_encode_char(
        codec, ch, out->data + out->length, out->capacity - out->length, &written
    );
    if (st != UNICODOID_OK) return st;
    out->length += written;
    return UNICODOID_OK;
}

static unicodoid_status_t emit_substitute(
    const unicodoid_transcode_config_t* cfg,
    const unicodoid_system_t* src_sys,
    const unicodoid_system_t* dst_sys,
    unicodoid_char64_t ch,
    byte_buffer_t* out,
    bool* handled
) {
    *handled = false;
    if (cfg->policy != UNICODOID_POLICY_CALLBACK || !cfg->callback) return UNICODOID_OK;

    uint8_t temp[256];
    size_t temp_len = sizeof(temp);
    bool cont = cfg->callback(
        src_sys, dst_sys, ch, temp, &temp_len, cfg->callback_user_data
    );
    if (!cont) return UNICODOID_ERR_UNMAPPABLE_CHAR;
    if (temp_len > sizeof(temp)) return UNICODOID_ERR_INTERNAL;

    unicodoid_status_t st = buffer_append(out, temp, temp_len);
    if (st != UNICODOID_OK) return st;
    *handled = true;
    return UNICODOID_OK;
}

unicodoid_status_t unicodoid_transcode(
    const unicodoid_system_t* src_sys,
    const void* in_buf,
    size_t in_len,
    const unicodoid_system_t* dst_sys,
    void* out_buf,
    size_t out_cap,
    size_t* out_written,
    const unicodoid_transcode_config_t* cfg
) {
    if (!src_sys || !dst_sys || !out_written) return UNICODOID_ERR_INVALID_PARAM;
    if (in_len > 0 && !in_buf) return UNICODOID_ERR_INVALID_PARAM;
    if (out_cap > 0 && !out_buf) return UNICODOID_ERR_INVALID_PARAM;

    unicodoid_transcode_config_t default_cfg = unicodoid_transcode_config_default();
    if (!cfg) cfg = &default_cfg;

    *out_written = 0;
    if (in_len == 0) return UNICODOID_OK;

    /* Same system: validate once, then copy the byte stream unchanged. */
    if (same_system(src_sys, dst_sys)) {
        unicodoid_codec_t check;
        unicodoid_status_t check_st = unicodoid_codec_init(&check, src_sys);
        if (check_st != UNICODOID_OK) return check_st;
        check_st = unicodoid_codec_validate(&check, in_buf, in_len, NULL);
        unicodoid_codec_free(&check);
        if (check_st != UNICODOID_OK) return check_st;

        *out_written = in_len;
        if (in_len > out_cap || (in_len > 0 && !out_buf)) {
            return UNICODOID_ERR_BUFFER_TOO_SMALL;
        }
        memcpy(out_buf, in_buf, in_len);
        return UNICODOID_OK;
    }

    if (!src_sys->ops.decode_char || !dst_sys->ops.encode_char) {
        return UNICODOID_ERR_UNSUPPORTED;
    }

    byte_buffer_t out;
    memset(&out, 0, sizeof(out));

    unicodoid_codec_t src_codec;
    unicodoid_codec_t dst_codec;
    unicodoid_status_t st = unicodoid_codec_init(&src_codec, src_sys);
    if (st != UNICODOID_OK) return st;
    st = unicodoid_codec_init(&dst_codec, dst_sys);
    if (st != UNICODOID_OK) {
        unicodoid_codec_free(&src_codec);
        return st;
    }

    const uint8_t* in = (const uint8_t*)in_buf;
    size_t in_off = 0;

    while (in_off < in_len) {
        unicodoid_char64_t src_ch = 0;
        size_t read = 0;

        st = unicodoid_codec_decode_char(
            &src_codec, in + in_off, in_len - in_off, &src_ch, &read
        );

        if (st == UNICODOID_ERR_NO_CHARACTER) {
            if (read == 0) {
                st = UNICODOID_ERR_INTERNAL;
                goto fail;
            }
            in_off += read;
            continue;
        }

        if (st != UNICODOID_OK) {
            if (cfg->policy == UNICODOID_POLICY_STRICT) {
                goto fail;
            }

            size_t advance = read > 0 ? read : 1;
            if (advance > in_len - in_off) advance = in_len - in_off;
            in_off += advance;

            if (cfg->policy == UNICODOID_POLICY_SKIP) continue;

            bool handled = false;
            st = emit_substitute(
                cfg, src_sys, dst_sys, UNICODOID_INVALID_CHAR, &out, &handled
            );
            if (st != UNICODOID_OK) goto fail;
            if (handled) continue;

            unicodoid_char64_t repl =
                cfg->custom_replacement ? cfg->custom_replacement
                                        : dst_sys->meta.replacement_char;
            st = encode_into_buffer(&dst_codec, repl, &out);
            if (st != UNICODOID_OK) goto fail;
            continue;
        }

        in_off += read;

        unicodoid_char64_t dst_ch = 0;
        st = unicodoid_transcode_char(src_sys, src_ch, dst_sys, &dst_ch, cfg);

        if (st != UNICODOID_OK) {
            if (cfg->policy == UNICODOID_POLICY_STRICT) goto fail;

            if (cfg->policy == UNICODOID_POLICY_SKIP) continue;

            bool handled = false;
            st = emit_substitute(cfg, src_sys, dst_sys, src_ch, &out, &handled);
            if (st != UNICODOID_OK) goto fail;
            if (handled) continue;

            dst_ch = cfg->custom_replacement ? cfg->custom_replacement
                                             : dst_sys->meta.replacement_char;
        }

        st = encode_into_buffer(&dst_codec, dst_ch, &out);
        if (st != UNICODOID_OK) goto fail;
    }

    unicodoid_codec_free(&src_codec);
    unicodoid_codec_free(&dst_codec);

    *out_written = out.length;
    if (out.length > out_cap || (out.length > 0 && !out_buf)) {
        buffer_free(&out);
        return UNICODOID_ERR_BUFFER_TOO_SMALL;
    }
    if (out.length > 0) memcpy(out_buf, out.data, out.length);
    buffer_free(&out);
    return UNICODOID_OK;

fail:
    unicodoid_codec_free(&src_codec);
    unicodoid_codec_free(&dst_codec);
    *out_written = out.length;
    buffer_free(&out);
    return st;
}

unicodoid_status_t unicodoid_transcode_string(
    const unicodoid_string_t* src,
    const unicodoid_system_t* dst_sys,
    const unicodoid_transcode_config_t* cfg,
    unicodoid_string_t* out_dst
) {
    if (!src || !dst_sys || !out_dst) return UNICODOID_ERR_INVALID_PARAM;

    size_t required = 0;
    unicodoid_status_t st = unicodoid_transcode(
        src->system, src->data, src->length, dst_sys,
        NULL, 0, &required, cfg
    );

    if (st != UNICODOID_OK && st != UNICODOID_ERR_BUFFER_TOO_SMALL) {
        return st;
    }

    st = unicodoid_string_init(out_dst, dst_sys, required > 0 ? required : 16);
    if (st != UNICODOID_OK) return st;

    size_t written = 0;
    st = unicodoid_transcode(
        src->system, src->data, src->length, dst_sys,
        out_dst->data, out_dst->capacity, &written, cfg
    );
    if (st != UNICODOID_OK) {
        unicodoid_string_free(out_dst);
        return st;
    }
    out_dst->length = written;
    return UNICODOID_OK;
}
