#ifndef UNICODOID_UAX60_EA_SCRIPT_DATA_H
#define UNICODOID_UAX60_EA_SCRIPT_DATA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file uax60_ea_script_data.h
 * @brief UAX #60 Data for East Asian Scripts (Unicode 18.0.0, Rev. 3).
 *
 * Data consumed by UAX #11 / #14 / #50 for East Asian scripts (Han, Kana,
 * Hangul, Bopomofo, Yi, etc.): script membership, East_Asian_Width
 * interaction, and script-code tables.  This module exposes the script
 * membership core over the generated Script property (the full UAX #60
 * data file is a distribution reference, not a runtime table).
 */

/** True when @p cp has an East Asian Script (Han, Kana, Hangul, Bopomofo, Yi, Tangut, Nushu, Khitan, Kawi...). */
bool unicodoid_uax60_is_ea_script(uint32_t cp);

/** True when @p cp is Han (Script=Hani). */
bool unicodoid_uax60_is_han(uint32_t cp);

/** True when @p cp is an East Asian Wide/Fullwidth script character (Wideness follows UAX #11). */
bool unicodoid_uax60_is_ea_wide(uint32_t cp);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UAX60_EA_SCRIPT_DATA_H */
