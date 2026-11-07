#ifndef UNICODOID_TYPES_H
#define UNICODOID_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file unicodoid_types.h
 * @brief Fundamental value types shared by every Unicodoid encoding system.
 *
 * Unicodoid does not assume that a character is a Unicode scalar value.  A
 * character is an integer in the value space declared by its encoding system.
 * The value may be a scalar, a token identifier, or a composite reference.
 */

/** A compact character value (32-bit scalar or token). */
typedef uint32_t unicodoid_char_t;

/** The general character value type used at every SDK boundary. */
typedef uint64_t unicodoid_char64_t;

/** Sentinel used where no character value exists. */
#define UNICODOID_INVALID_CHAR ((unicodoid_char64_t)UINT64_MAX)

/**
 * @brief Status and error codes returned across the Unicodoid SDK.
 *
 * Every fallible function returns one of these values.  Negative values are
 * errors; zero is success.  The numeric values are part of the ABI and are
 * mirrored by the other language bindings.
 */
typedef enum unicodoid_status {
    UNICODOID_OK                         = 0,
    UNICODOID_ERR_INVALID_PARAM          = -1,
    UNICODOID_ERR_BUFFER_TOO_SMALL       = -2,
    UNICODOID_ERR_INVALID_CODE_UNIT      = -3,
    UNICODOID_ERR_UNMAPPABLE_CHAR        = -4,
    UNICODOID_ERR_INCOMPLETE_SEQUENCE    = -5,
    UNICODOID_ERR_NOT_FOUND              = -6,
    UNICODOID_ERR_ALREADY_EXISTS         = -7,
    UNICODOID_ERR_UNSUPPORTED            = -8,
    UNICODOID_ERR_CAPACITY_EXCEEDED      = -9,
    UNICODOID_ERR_ALLOCATION_FAILED      = -10,
    UNICODOID_ERR_INTERNAL               = -11,
    UNICODOID_ERR_STATE                  = -12,
    UNICODOID_ERR_OVERFLOW               = -13,
    /**
     * Input was consumed but produced no character value.  This is not a
     * failure: modal systems use it for shift controls.  Streaming loops must
     * advance by the reported byte count and continue.
     */
    UNICODOID_ERR_NO_CHARACTER           = -14
} unicodoid_status_t;

/**
 * @brief Human-readable, stable name for a status code.
 */
const char* unicodoid_status_string(unicodoid_status_t status);

/**
 * @brief Character modeling architecture declared by an encoding system.
 *
 * This is descriptive metadata.  The SDK never derives behaviour from the
 * model; behaviour always comes from the system's operations.
 */
typedef enum unicodoid_char_model {
    UNICODOID_MODEL_FIXED_WIDTH          = 0, /**< Every character has one code unit. */
    UNICODOID_MODEL_VARIABLE_WIDTH       = 1, /**< Character width varies with the value. */
    UNICODOID_MODEL_MODAL_STATEFUL       = 2, /**< Encoding mode depends on preceding input. */
    UNICODOID_MODEL_TOKEN_BASED          = 3, /**< Values are token identifiers, not scalars. */
    UNICODOID_MODEL_CUSTOM               = 4  /**< System-defined model. */
} unicodoid_char_model_t;

/**
 * @brief Byte order expectation for multi-byte code units.
 */
typedef enum unicodoid_endianness {
    UNICODOID_ENDIAN_NONE                = 0,
    UNICODOID_ENDIAN_LITTLE              = 1,
    UNICODOID_ENDIAN_BIG                 = 2
} unicodoid_endianness_t;

/**
 * @brief Level of optional Unicode compatibility declared by an encoding system.
 *
 * UNICODOID_COMPAT_NONE is a first-class state.  Encoding systems are not
 * required to have any relationship with Unicode.
 */
typedef enum unicodoid_compat_level {
    UNICODOID_COMPAT_NONE                = 0, /**< No Unicode mapping. */
    UNICODOID_COMPAT_PARTIAL             = 1, /**< Maps a subset of values to Unicode. */
    UNICODOID_COMPAT_FULL                = 2, /**< Isomorphic mapping for the whole value space. */
    UNICODOID_COMPAT_PROFILE             = 3  /**< Mapping plus declared UAX/UTS/UTR profile. */
} unicodoid_compat_level_t;

/**
 * @brief Strategy applied during transcoding when a character cannot be mapped.
 */
typedef enum unicodoid_transcode_policy {
    UNICODOID_POLICY_STRICT              = 0, /**< Fail immediately. */
    UNICODOID_POLICY_REPLACE             = 1, /**< Emit the destination replacement character. */
    UNICODOID_POLICY_SKIP                = 2, /**< Omit the character. */
    UNICODOID_POLICY_CALLBACK            = 3  /**< Delegate to a user callback. */
} unicodoid_transcode_policy_t;

/* Unicode scalar value constants.  These are only meaningful to systems that
 * declare a Unicode profile; they do not constrain independent systems. */
#define UNICODOID_UNICODE_MAX_SCALAR     ((uint32_t)0x0010FFFFUL)
#define UNICODOID_UNICODE_SURROGATE_MIN  ((uint32_t)0x0000D800UL)
#define UNICODOID_UNICODE_SURROGATE_MAX  ((uint32_t)0x0000DFFFUL)
#define UNICODOID_UNICODE_REPLACEMENT    ((uint32_t)0x0000FFFDUL)

/**
 * @brief Check whether a 32-bit value is a valid Unicode scalar value.
 */
static inline bool unicodoid_is_valid_unicode_scalar(uint32_t cp) {
    return (cp <= UNICODOID_UNICODE_MAX_SCALAR) &&
           (cp < UNICODOID_UNICODE_SURROGATE_MIN || cp > UNICODOID_UNICODE_SURROGATE_MAX);
}

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_TYPES_H */
