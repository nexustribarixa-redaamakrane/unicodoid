#ifndef UNICODOID_TRANSCODE_H
#define UNICODOID_TRANSCODE_H

#include "unicodoid_string.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file unicodoid_transcode.h
 * @brief Explicit transcoding between independent encoding systems.
 *
 * Two systems are bridged in exactly one of these ways:
 *  - they are the same system (byte copy after validation);
 *  - both declare Unicode profiles (route through Unicode scalar values);
 *  - the caller supplies a bridge callback for non-Unicode value mapping.
 *
 * There is no implicit identity mapping between unrelated systems.
 */

/** Substitute-output callback for UNICODOID_POLICY_CALLBACK. */
typedef bool (*unicodoid_transcode_callback_t)(
    const unicodoid_system_t* src_sys,
    const unicodoid_system_t* dst_sys,
    unicodoid_char64_t unmappable_char,
    void* out_subst,
    size_t* out_subst_len,
    void* user_data
);

/**
 * @brief Explicit value bridge for systems that do not share a Unicode profile.
 *
 * Implementations map one native source character value to one native
 * destination character value.  Return UNICODOID_ERR_UNMAPPABLE_CHAR when the
 * value cannot be bridged.
 */
typedef unicodoid_status_t (*unicodoid_transcode_bridge_t)(
    void* user_data,
    const unicodoid_system_t* src_sys,
    unicodoid_char64_t src_ch,
    const unicodoid_system_t* dst_sys,
    unicodoid_char64_t* out_dst_ch
);

/** Transcoding configuration. */
typedef struct unicodoid_transcode_config {
    unicodoid_transcode_policy_t   policy;
    unicodoid_char64_t             custom_replacement; /**< 0 means use destination default */
    unicodoid_transcode_callback_t callback;
    void*                          callback_user_data;
    unicodoid_transcode_bridge_t   bridge;
    void*                          bridge_user_data;
} unicodoid_transcode_config_t;

/** Default configuration: strict policy, no bridge. */
static inline unicodoid_transcode_config_t unicodoid_transcode_config_default(void) {
    unicodoid_transcode_config_t cfg;
    cfg.policy = UNICODOID_POLICY_STRICT;
    cfg.custom_replacement = 0;
    cfg.callback = NULL;
    cfg.callback_user_data = NULL;
    cfg.bridge = NULL;
    cfg.bridge_user_data = NULL;
    return cfg;
}

/**
 * @brief Transcode one native character value into another system.
 *
 * This performs mapping only; it does not encode bytes.
 */
unicodoid_status_t unicodoid_transcode_char(
    const unicodoid_system_t* src_sys,
    unicodoid_char64_t src_ch,
    const unicodoid_system_t* dst_sys,
    unicodoid_char64_t* out_dst_ch,
    const unicodoid_transcode_config_t* cfg
);

/**
 * @brief Transcode a raw byte buffer.
 *
 * On UNICODOID_ERR_BUFFER_TOO_SMALL, @p out_written receives the required
 * destination size in bytes.
 */
unicodoid_status_t unicodoid_transcode(
    const unicodoid_system_t* src_sys,
    const void* in_buf,
    size_t in_len,
    const unicodoid_system_t* dst_sys,
    void* out_buf,
    size_t out_cap,
    size_t* out_written,
    const unicodoid_transcode_config_t* cfg
);

/** Transcode a string into a newly allocated destination string. */
unicodoid_status_t unicodoid_transcode_string(
    const unicodoid_string_t* src,
    const unicodoid_system_t* dst_sys,
    const unicodoid_transcode_config_t* cfg,
    unicodoid_string_t* out_dst
);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_TRANSCODE_H */
