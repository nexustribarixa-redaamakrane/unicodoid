#ifndef UNICODOID_SYSTEM_UTF32_H
#define UNICODOID_SYSTEM_UTF32_H

#include "../unicodoid_system.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get the standard UTF-32LE encoding system singleton.
 */
const unicodoid_system_t* unicodoid_system_get_utf32le(void);

/**
 * @brief Get the standard UTF-32BE encoding system singleton.
 */
const unicodoid_system_t* unicodoid_system_get_utf32be(void);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_SYSTEM_UTF32_H */
