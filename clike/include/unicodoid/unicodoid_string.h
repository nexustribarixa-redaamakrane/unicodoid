#ifndef UNICODOID_STRING_H
#define UNICODOID_STRING_H

#include "unicodoid_system.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file unicodoid_string.h
 * @brief Length-bounded byte string parameterized by an encoding system.
 *
 * Strings never depend on null terminators and may contain embedded zero
 * bytes.  A string keeps its own codec state so that appending characters to a
 * modal encoding produces one coherent stream.
 */

typedef struct unicodoid_string {
    const unicodoid_system_t* system;
    uint8_t*                  data;
    size_t                    length;   /**< Length in bytes */
    size_t                    capacity; /**< Buffer capacity in bytes */
    bool                      is_owned; /**< True when the SDK allocated data */
    void*                     state;    /**< Optional owned codec state */
    size_t                    state_size;
} unicodoid_string_t;

/** Initialize an empty, owned string. */
unicodoid_status_t unicodoid_string_init(
    unicodoid_string_t* str,
    const unicodoid_system_t* sys,
    size_t initial_capacity
);

/** Wrap an existing buffer as a non-owned string. */
unicodoid_status_t unicodoid_string_from_slice(
    unicodoid_string_t* str,
    const unicodoid_system_t* sys,
    const void* buffer,
    size_t length_bytes
);

/** Create an owned string by encoding an array of character values. */
unicodoid_status_t unicodoid_string_from_chars(
    unicodoid_string_t* str,
    const unicodoid_system_t* sys,
    const unicodoid_char64_t* chars,
    size_t num_chars
);

/** Free owned storage (data and codec state). */
void unicodoid_string_free(unicodoid_string_t* str);

/** Clone a string, including codec state. */
unicodoid_status_t unicodoid_string_clone(
    const unicodoid_string_t* src,
    unicodoid_string_t* out_clone
);

/** Append one character, using the string's persistent codec state. */
unicodoid_status_t unicodoid_string_append_char(
    unicodoid_string_t* str,
    unicodoid_char64_t ch
);

/** Append already-validated bytes without interpreting them. */
unicodoid_status_t unicodoid_string_append_bytes(
    unicodoid_string_t* str,
    const void* bytes,
    size_t num_bytes
);

/** Count characters by decoding from the initial state. */
unicodoid_status_t unicodoid_string_char_length(
    const unicodoid_string_t* str,
    size_t* out_char_count
);

/** Retrieve a character by zero-based character index. */
unicodoid_status_t unicodoid_string_get_char_at(
    const unicodoid_string_t* str,
    size_t char_index,
    unicodoid_char64_t* out_ch
);

/** Reset the string's persistent codec state to its initial value. */
unicodoid_status_t unicodoid_string_reset_state(unicodoid_string_t* str);

/** Byte-wise equality. */
bool unicodoid_string_equals(
    const unicodoid_string_t* a,
    const unicodoid_string_t* b
);

/** Lexicographic comparison by raw bytes. */
int unicodoid_string_compare(
    const unicodoid_string_t* a,
    const unicodoid_string_t* b
);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_STRING_H */
