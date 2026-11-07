#ifndef UNICODOID_H
#define UNICODOID_H

/**
 * @file unicodoid.h
 * @brief Master umbrella header for the Unicodoid C SDK.
 */

#include "unicodoid_types.h"
#include "unicodoid_unicode.h"
#include "unicodoid_system.h"
#include "unicodoid_codec.h"
#include "unicodoid_registry.h"
#include "unicodoid_string.h"
#include "unicodoid_transcode.h"
#include "unicodoid_properties.h"
#include "unicodoid_segmentation.h"
#include "unicodoid_normalization.h"
#include "unicodoid_collation.h"
#include "ConvertUTF.h"

/* Built-in reference systems. */
#include "systems/unicodoid_utf8.h"
#include "systems/unicodoid_utf16.h"
#include "systems/unicodoid_utf32.h"
#include "systems/unicodoid_ascii.h"
#include "systems/unicodoid_custom_conlang.h"
#include "systems/unicodoid_shiftmodal16.h"

#define UNICODOID_VERSION_MAJOR 1
#define UNICODOID_VERSION_MINOR 0
#define UNICODOID_VERSION_PATCH 0
#define UNICODOID_VERSION_STRING "1.0.0"

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize all SDK subsystems. */
static inline unicodoid_status_t unicodoid_init(void) {
    return unicodoid_registry_init();
}

/** Shut down global SDK resources. */
static inline void unicodoid_shutdown(void) {
    unicodoid_registry_cleanup();
}

/** SDK version string. */
static inline const char* unicodoid_version(void) {
    return UNICODOID_VERSION_STRING;
}

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_H */
