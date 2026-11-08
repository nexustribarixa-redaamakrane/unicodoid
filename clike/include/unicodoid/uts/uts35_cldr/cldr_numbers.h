#ifndef UNICODOID_CLDR_NUMBERS_H
#define UNICODOID_CLDR_NUMBERS_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cldr_numbers.h
 * @brief UTS #35 Numbers (number formatting symbols).
 *
 * CLDR number symbols (grouping/decimal) and compact-notation helpers.
 * Full CLDR number-pattern rendering is out of scope; this module covers
 * the symbols and decimal/group separators keyed by region.
 */

typedef struct unicodoid_cldr_number_symbols {
    char group[8];
    char decimal[8];
    char minus[8];
    char percent[8];
} unicodoid_cldr_number_symbols_t;

/** Symbols for @p region (ISO 3166); root defaults when @p region is NULL/empty. */
void unicodoid_cldr_number_symbols(const char* region, unicodoid_cldr_number_symbols_t* out);

/** True when @p decimal string uses only ASCII digits and at most one '.' separator. */
bool unicodoid_cldr_is_decimal(const char* text);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_CLDR_NUMBERS_H */
