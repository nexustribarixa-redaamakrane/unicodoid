#ifndef UNICODOID_SYSTEM_H
#define UNICODOID_SYSTEM_H

#include "unicodoid_types.h"
#include "unicodoid_unicode.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declaration. */
typedef struct unicodoid_system unicodoid_system_t;

/**
 * @file unicodoid_system.h
 * @brief Descriptor and operation table for an encoding system.
 *
 * Operations are state-aware.  Stateless systems ignore the @c state pointer.
 * Stateful systems receive a pointer to a buffer of @ref
 * unicodoid_system::state_size bytes, normally owned by a
 * @ref unicodoid_codec_t or a @ref unicodoid_string_t.
 */

/** Identity and characteristics of an encoding system. */
typedef struct unicodoid_system_meta {
    const char*              id;                 /**< Unique reverse-DNS identifier */
    const char*              name;               /**< Human-readable name */
    const char*              version;            /**< System version */
    const char*              author;             /**< Author or organization */
    const char*              description;        /**< Design summary */
    uint32_t                 code_unit_bits;     /**< 8, 16, 32, or 64 */
    uint32_t                 min_bytes_per_char; /**< Minimum encoded bytes per character */
    uint32_t                 max_bytes_per_char; /**< Maximum encoded bytes per character */
    unicodoid_char_model_t   model;              /**< Character modeling type */
    unicodoid_endianness_t   endianness;         /**< Endianness of multi-byte code units */
    unicodoid_compat_level_t compat_level;       /**< Unicode compatibility tier */
    unicodoid_char64_t       max_char_val;       /**< Maximum native character value */
    unicodoid_char64_t       replacement_char;   /**< Native replacement character */
} unicodoid_system_meta_t;

/** Functional operation table implemented by an encoding system. */
typedef struct unicodoid_system_ops {
    /**
     * @brief Encode one character value.
     *
     * @param sys    Encoding system.
     * @param state  Mutable codec state, or NULL for stateless systems.
     * @param ch     Character value.
     * @param out    Destination buffer.
     * @param cap    Destination capacity in bytes.
     * @param written Receives the number of bytes written.
     */
    unicodoid_status_t (*encode_char)(
        const unicodoid_system_t* sys,
        void* state,
        unicodoid_char64_t ch,
        void* out,
        size_t cap,
        size_t* written
    );

    /**
     * @brief Decode one character value.
     *
     * @param read Receives the number of bytes consumed.
     */
    unicodoid_status_t (*decode_char)(
        const unicodoid_system_t* sys,
        void* state,
        const void* in,
        size_t in_len,
        unicodoid_char64_t* out_ch,
        size_t* read
    );

    /**
     * @brief Validate a complete byte buffer.
     *
     * @param invalid_offset Receives the first invalid byte offset on failure.
     */
    unicodoid_status_t (*validate)(
        const unicodoid_system_t* sys,
        void* state,
        const void* in,
        size_t in_len,
        size_t* invalid_offset
    );

    /** Count characters in a complete byte buffer. */
    unicodoid_status_t (*char_count)(
        const unicodoid_system_t* sys,
        void* state,
        const void* in,
        size_t in_len,
        size_t* out_count
    );

    /**
     * @brief Exact number of bytes needed to encode an array of character values.
     *
     * This is a pure sizing operation and does not mutate state.
     */
    unicodoid_status_t (*byte_count)(
        const unicodoid_system_t* sys,
        const unicodoid_char64_t* chars,
        size_t num_chars,
        size_t* out_bytes
    );
} unicodoid_system_ops_t;

/** Complete encoding system descriptor. */
struct unicodoid_system {
    unicodoid_system_meta_t            meta;
    unicodoid_system_ops_t             ops;

    /** Size in bytes of one codec state object; 0 for stateless systems. */
    size_t                             state_size;

    /** Optional state initializer.  Called on freshly allocated zeroed memory. */
    unicodoid_status_t (*state_init)(const unicodoid_system_t* sys, void* state);

    /** Optional state reset.  Must restore the initial state. */
    void (*state_reset)(const unicodoid_system_t* sys, void* state);

    /** Optional Unicode profile; NULL means the system has no Unicode relationship. */
    const unicodoid_unicode_profile_t* unicode_profile;

    /** Implementation-defined payload. */
    void*                              user_data;
};

/** True when the system declares and provides a usable Unicode profile. */
static inline bool unicodoid_system_has_unicode_profile(const unicodoid_system_t* sys) {
    return (sys != NULL) && (sys->unicode_profile != NULL) &&
           (sys->meta.compat_level != UNICODOID_COMPAT_NONE) &&
           (sys->unicode_profile->to_unicode != NULL) &&
           (sys->unicode_profile->from_unicode != NULL);
}

/** True when the system requires codec state. */
static inline bool unicodoid_system_is_stateful(const unicodoid_system_t* sys) {
    return (sys != NULL) && (sys->state_size > 0);
}

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_SYSTEM_H */
