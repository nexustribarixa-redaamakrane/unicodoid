#ifndef UNICODOID_SYSTEM_UTF16_H
#define UNICODOID_SYSTEM_UTF16_H

#include "../unicodoid_system.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get the standard UTF-16LE encoding system singleton.
 */
const unicodoid_system_t* unicodoid_system_get_utf16le(void);

/**
 * @brief Get the standard UTF-16BE encoding system singleton.
 */
const unicodoid_system_t* unicodoid_system_get_utf16be(void);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_SYSTEM_UTF16_H */
