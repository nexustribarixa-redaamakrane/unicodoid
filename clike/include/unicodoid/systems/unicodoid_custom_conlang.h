#ifndef UNICODOID_SYSTEM_CUSTOM_CONLANG_H
#define UNICODOID_SYSTEM_CUSTOM_CONLANG_H

#include "../unicodoid_system.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get the custom Conlang (Tengwar/Kupogram-inspired) independent encoding system.
 *
 * This system is completely independent of Unicode (UNICODOID_COMPAT_NONE).
 * It features a 14-bit character space (0x0000 - 0x3FFF), encoded in variable 1-to-2 bytes:
 * - 0x0000 - 0x007F: Single byte (0xxxxxxx)
 * - 0x0080 - 0x3FFF: Two bytes (10xxxxxx xxxxxxxx)
 */
const unicodoid_system_t* unicodoid_system_get_custom_conlang(void);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_SYSTEM_CUSTOM_CONLANG_H */
