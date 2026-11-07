#ifndef UNICODOID_SYSTEM_UTF8_H
#define UNICODOID_SYSTEM_UTF8_H

#include "../unicodoid_system.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get the standard UTF-8 encoding system singleton.
 */
const unicodoid_system_t* unicodoid_system_get_utf8(void);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_SYSTEM_UTF8_H */
