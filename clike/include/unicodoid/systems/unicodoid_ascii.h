#ifndef UNICODOID_SYSTEM_ASCII_H
#define UNICODOID_SYSTEM_ASCII_H

#include "../unicodoid_system.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get the standard 7-bit ASCII encoding system singleton.
 */
const unicodoid_system_t* unicodoid_system_get_ascii(void);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_SYSTEM_ASCII_H */
