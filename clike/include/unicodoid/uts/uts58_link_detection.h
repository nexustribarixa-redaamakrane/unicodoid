#ifndef UNICODOID_UTS58_LINK_DETECTION_H
#define UNICODOID_UTS58_LINK_DETECTION_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file uts58_link_detection.h
 * @brief UTS #58 Link Detection and Formatting: URLs and Email Addresses
 *        (Unicode 18.0.0, Revision 3).
 *
 * Implements the code point machinery of UTS #58 that operates after the
 * host has been parsed:
 *
 *   - Link_Term / Link_Bracket / Link_Email property lookups (backed by the
 *     generated Unicode 18.0.0 tables, see unicodoid_ucd.h),
 *   - the link termination algorithm (UTS58 §3.5.1),
 *   - the URL minimal escaping algorithm (UTS58 §4.1),
 *   - the email local-part backward scan (UTS58 §5.2).
 *
 * URL *initiation* (finding the Scheme, Host and optional Port, including
 * registrable-domain and TLD checks) is explicitly out of scope, as it is in
 * the specification itself (UTS58 §1.4 and §3.2).
 */

/** URL parts for minimal escaping (UTS58 §4.1). */
typedef enum unicodoid_url_part {
    UNICODOID_URL_PART_PATH              = 0,
    UNICODOID_URL_PART_QUERY             = 1,
    UNICODOID_URL_PART_FRAGMENT          = 2,
    UNICODOID_URL_PART_FRAGMENT_DIRECTIVE = 3
} unicodoid_url_part_t;

/**
 * @brief Link detection termination algorithm (UTS58 §3.5.1).
 *
 * @param cps    the code points of the text to scan,
 * @param length number of code points,
 * @param start  index of the first code point to process (i.e. the index
 *               immediately after the host, at the beginning of the Path,
 *               Query or Fragment, if any).
 *
 * @return the offset one past the end of the final URL part, i.e. the end of
 *         the detected link.  Returns @p start when no URL part is found.
 */
size_t unicodoid_uts58_link_detection_end(
    const uint32_t* cps, size_t length, size_t start);

/**
 * @brief Email local-part detection (UTS58 §5.2).
 *
 * Scans backwards from the '@' sign at @p at_index and finds the beginning
 * of the local part, per the rules of the specification (no leading or
 * trailing dots, no consecutive dots, only Link_Email characters).
 *
 * @return the offset of the beginning of the local part, optionally
 *         including the "mailto:" prefix; or SIZE_MAX when the text does not
 *         contain a valid local part.
 */
size_t unicodoid_uts58_email_local_part_start(
    const uint32_t* cps, size_t at_index);

/**
 * @brief URL Minimal Escaping Algorithm (UTS58 §4.1).
 *
 * @param part     which URL part is being escaped,
 * @param cps      the decoded code points of the part, WITHOUT the initiator
 *                 (which is added by the algorithm),
 * @param length   number of code points,
 * @param is_last  true when this is the last part of the URL (a final Soft
 *                 code point of the whole URL must be percent-escaped),
 * @param out      destination buffer for the minimally escaped UTF-8 bytes,
 *                 or NULL to query the required size,
 * @param cap      capacity of @p out.
 *
 * @return the number of bytes required/written (excluding the NUL), like
 *         the rest of the SDK's buffer-query APIs.
 */
size_t unicodoid_uts58_minimal_escape_part(
    unicodoid_url_part_t part,
    const uint32_t* cps, size_t length,
    bool is_last,
    uint8_t* out, size_t cap);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UTS58_LINK_DETECTION_H */
