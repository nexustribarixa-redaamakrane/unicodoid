#ifndef UNICODOID_SEGMENTATION_H
#define UNICODOID_SEGMENTATION_H

#include "unicodoid_codec.h"
#include "unicodoid_string.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file unicodoid_segmentation.h
 * @brief Partial text segmentation for Unicode-compatible systems.
 *
 * Implements simplified grapheme, word, line, and sentence iteration.  The
 * module requires a Unicode profile and is explicitly documented as a
 * pragmatic subset, not a certified UAX #29 / UAX #14 implementation.
 */

typedef enum unicodoid_boundary_type {
    UNICODOID_BOUNDARY_GRAPHEME = 0,
    UNICODOID_BOUNDARY_WORD     = 1,
    UNICODOID_BOUNDARY_LINE     = 2,
    UNICODOID_BOUNDARY_SENTENCE = 3
} unicodoid_boundary_type_t;

typedef struct unicodoid_segmenter {
    const unicodoid_system_t* system;
    const uint8_t*            data;
    size_t                    length_bytes;
    unicodoid_boundary_type_t boundary_type;
    size_t                    current_offset;
    unicodoid_codec_t         codec;
    void*                     internal; /**< Private decoded-scalar cache */
} unicodoid_segmenter_t;

typedef struct unicodoid_segment {
    size_t byte_offset;
    size_t byte_length;
    size_t char_count;
    bool   is_boundary;
} unicodoid_segment_t;

/** Initialize a segmenter.  Requires a Unicode profile on @p sys. */
unicodoid_status_t unicodoid_segmenter_init(
    unicodoid_segmenter_t* seg,
    const unicodoid_system_t* sys,
    const void* data,
    size_t length_bytes,
    unicodoid_boundary_type_t boundary_type
);

/** Release resources held by a segmenter. */
void unicodoid_segmenter_free(unicodoid_segmenter_t* seg);

/**
 * @brief Produce the next segment.
 *
 * @return true when a segment was produced, false at end of input.
 */
bool unicodoid_segmenter_next(
    unicodoid_segmenter_t* seg,
    unicodoid_segment_t* out_segment
);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_SEGMENTATION_H */
