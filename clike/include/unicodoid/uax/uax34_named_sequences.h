#ifndef UNICODOID_UAX34_NAMED_SEQUENCES_H
#define UNICODOID_UAX34_NAMED_SEQUENCES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file uax34_named_sequences.h
 * @brief UAX #34 Unicode Named Character Sequences (Unicode 18.0.0).
 *
 * Provides access to the normative NamedSequences.txt data and validates
 * sequences against it.  Names are compared exactly, as required by UAX #34;
 * callers that need loose matching apply their own normalization.
 */

/** Number of registered named character sequences. */
size_t unicodoid_uax34_count(void);

/** Name of the sequence at @p index, or NULL when out of range. */
const char* unicodoid_uax34_name(size_t index);

/**
 * @brief Fetch the code points of the sequence at @p index.
 *
 * @return the sequence length, or SIZE_MAX when @p index is out of range.
 */
size_t unicodoid_uax34_sequence(size_t index, uint32_t* out, size_t cap);

/**
 * @brief Look up a sequence by exact name.
 *
 * @param name    NUL-terminated sequence name,
 * @param out     destination buffer, or NULL to query the length,
 * @param cap     capacity of @p out,
 * @param length  receives the sequence length on success.
 *
 * @return true when the name is registered and the sequence fits in @p out.
 */
bool unicodoid_uax34_find(
    const char* name, uint32_t* out, size_t cap, size_t* length);

/**
 * @brief True when @p cps is exactly one of the registered sequences.
 *
 * Also rejects ill-formed scalar values (surrogates and out-of-range code
 * points) before consulting the table.
 */
bool unicodoid_uax34_is_registered(const uint32_t* cps, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UAX34_NAMED_SEQUENCES_H */
