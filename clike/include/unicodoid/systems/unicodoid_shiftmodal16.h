#ifndef UNICODOID_SYSTEM_SHIFTMODAL16_H
#define UNICODOID_SYSTEM_SHIFTMODAL16_H

#include "../unicodoid_system.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get the built-in ShiftModal-16 modal, stateful example system.
 *
 * ShiftModal-16 is deliberately independent of Unicode
 * (UNICODOID_COMPAT_NONE).  It demonstrates that the SDK's codec and string
 * layers can carry genuine per-stream state:
 *
 * - Base mode: 0x00..0x7F are literal values 0x0000..0x007F.
 * - ESC (0x1B) followed by one byte b encodes value 0x0100 | b.
 * - SO  (0x0E) switches to shifted mode.
 * - Shifted mode: each byte b encodes value 0x4000 | b.
 * - SI  (0x0F) returns to base mode.
 */
const unicodoid_system_t* unicodoid_system_get_shiftmodal16(void);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_SYSTEM_SHIFTMODAL16_H */
