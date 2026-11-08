#ifndef UNICODOID_UTR23_CHARACTER_PROPERTY_MODEL_H
#define UNICODOID_UTR23_CHARACTER_PROPERTY_MODEL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file utr23_character_property_model.h
 * @brief UTR #23 The Unicode Character Property Model (Revision 15).
 *
 * Informative model for properties: enumeration vs. binary vs. numeric,
 * and how derived properties (Normalizations, Emoji, etc.) are defined
 * from the base UCD.  This module exposes the model queries the SDK
 * itself uses: list the supported property families and answer whether a
 * scalar has a derived binary property.
 */

typedef enum unicodoid_utr23_property_kind {
    UNICODOID_PROP_BINARY = 0,
    UNICODOID_PROP_ENUMERATED = 1,
    UNICODOID_PROP_NUMERIC = 2,
    UNICODOID_PROP_STRING = 3
} unicodoid_utr23_property_kind_t;

/** Number of supported binary properties (unicodoid_binary_property_t count). */
size_t unicodoid_utr23_binary_property_count(void);

/** True when the property at @p index is binary. */
bool unicodoid_utr23_is_binary(size_t index);

/** Property-kind classification for a named property (0 for unknown). */
int unicodoid_utr23_property_kind(const char* name);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UTR23_CHARACTER_PROPERTY_MODEL_H */
