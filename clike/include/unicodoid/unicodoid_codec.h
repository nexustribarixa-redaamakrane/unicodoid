#ifndef UNICODOID_CODEC_H
#define UNICODOID_CODEC_H

#include "unicodoid_system.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file unicodoid_codec.h
 * @brief Stateful stream facade over an encoding system.
 *
 * A codec owns one instance of a system's state.  For stateless systems the
 * state buffer is NULL and every operation is equivalent to a direct call on
 * the system operation table.
 */

/** A codec instance. */
typedef struct unicodoid_codec {
    const unicodoid_system_t* system;
    void*                     state;      /**< Owned state buffer, or NULL */
    size_t                    state_size; /**< Size of the state buffer */
} unicodoid_codec_t;

/**
 * @brief Initialize a codec for @p sys.
 *
 * Allocates and initializes state when the system is stateful.  The codec must
 * be released with unicodoid_codec_free().
 */
unicodoid_status_t unicodoid_codec_init(
    unicodoid_codec_t* codec,
    const unicodoid_system_t* sys
);

/** Release the codec state. */
void unicodoid_codec_free(unicodoid_codec_t* codec);

/** Reset the codec to its initial state. */
unicodoid_status_t unicodoid_codec_reset(unicodoid_codec_t* codec);

/** Encode one character, updating codec state. */
unicodoid_status_t unicodoid_codec_encode_char(
    unicodoid_codec_t* codec,
    unicodoid_char64_t ch,
    void* out,
    size_t out_cap,
    size_t* out_written
);

/** Decode one character, updating codec state. */
unicodoid_status_t unicodoid_codec_decode_char(
    unicodoid_codec_t* codec,
    const void* in,
    size_t in_len,
    unicodoid_char64_t* out_ch,
    size_t* out_read
);

/** Validate a complete buffer with this codec's state. */
unicodoid_status_t unicodoid_codec_validate(
    unicodoid_codec_t* codec,
    const void* in,
    size_t in_len,
    size_t* out_invalid_offset
);

/** Count characters in a complete buffer with this codec's state. */
unicodoid_status_t unicodoid_codec_char_count(
    unicodoid_codec_t* codec,
    const void* in,
    size_t in_len,
    size_t* out_count
);

/**
 * @brief Encode a sequence of character values as one stream.
 *
 * State persists across the whole sequence, which is required for modal
 * systems.  On UNICODOID_ERR_BUFFER_TOO_SMALL, @p out_written receives the
 * number of bytes required for the prefix that was encoded.
 *
 * @p out may be NULL only when @p out_cap is 0.  In that case no bytes are
 * written; encoding a non-empty sequence reports
 * UNICODOID_ERR_BUFFER_TOO_SMALL (or UNICODOID_ERR_UNMAPPABLE_CHAR for an
 * invalid value) with @p *out_written == 0, matching a size-probe call.
 */
unicodoid_status_t unicodoid_codec_encode(
    unicodoid_codec_t* codec,
    const unicodoid_char64_t* chars,
    size_t num_chars,
    void* out,
    size_t out_cap,
    size_t* out_written
);

/**
 * @brief Decode a complete byte stream into character values.
 *
 * @param out_chars    Destination array, or NULL to only count.
 * @param out_capacity Capacity of @p out_chars in elements.
 * @param out_count    Receives the number of decoded characters.
 */
unicodoid_status_t unicodoid_codec_decode(
    unicodoid_codec_t* codec,
    const void* in,
    size_t in_len,
    unicodoid_char64_t* out_chars,
    size_t out_capacity,
    size_t* out_count
);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_CODEC_H */
