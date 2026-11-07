#ifndef UNICODOID_COLLATION_H
#define UNICODOID_COLLATION_H

#include "unicodoid_string.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file unicodoid_collation.h
 * @brief Partial collation for Unicode-compatible systems.
 *
 * Provides a small, deterministic multi-level ordering suitable for tests and
 * demonstrations.  It is inspired by UTS #10 but is not a conformant UCA
 * implementation and does not ship DUCET or CLDR data.
 */

typedef enum unicodoid_collation_strength {
    UNICODOID_COLLATE_PRIMARY    = 1,
    UNICODOID_COLLATE_SECONDARY  = 2,
    UNICODOID_COLLATE_TERTIARY   = 3,
    UNICODOID_COLLATE_QUATERNARY = 4,
    UNICODOID_COLLATE_IDENTICAL  = 5
} unicodoid_collation_strength_t;

typedef struct unicodoid_collation_options {
    unicodoid_collation_strength_t strength;
    bool                           case_level;
    bool                           numeric_ordering;
} unicodoid_collation_options_t;

static inline unicodoid_collation_options_t unicodoid_collation_options_default(void) {
    unicodoid_collation_options_t opts;
    opts.strength = UNICODOID_COLLATE_TERTIARY;
    opts.case_level = false;
    opts.numeric_ordering = false;
    return opts;
}

/** Compare two strings.  Requires Unicode profiles on both systems. */
int unicodoid_collate(
    const unicodoid_string_t* a,
    const unicodoid_string_t* b,
    const unicodoid_collation_options_t* opts
);

/**
 * @brief Generate a binary sort key.
 *
 * On UNICODOID_ERR_BUFFER_TOO_SMALL, @p out_written receives the required size.
 */
unicodoid_status_t unicodoid_collation_sort_key(
    const unicodoid_string_t* str,
    const unicodoid_collation_options_t* opts,
    uint8_t* out_key,
    size_t out_cap,
    size_t* out_written
);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_COLLATION_H */
