#ifndef UNICODOID_CLDR_KEYBOARDS_H
#define UNICODOID_CLDR_KEYBOARDS_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cldr_keyboards.h
 * @brief UTS #35 Keyboards (keyboard layout identifiers).
 *
 * Keyboard types and layout transparency.  Full LXML data is external;
 * this module validates keyboard IDs and reports layout transparency.
 */

/** True when @p id is a CLDR keyboard id (e.g. "en_US", "de_DE", "dvorak"). */
bool unicodoid_cldr_is_keyboard_id(const char* id);

/** True when the layout is transparent (QWERTY-family default). */
bool unicodoid_cldr_is_transparent(const char* id);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_CLDR_KEYBOARDS_H */
