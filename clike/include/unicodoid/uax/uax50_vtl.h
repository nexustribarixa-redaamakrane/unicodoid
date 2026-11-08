#ifndef UNICODOID_UAX50_VTL_H
#define UNICODOID_UAX50_VTL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file uax50_vtl.h
 * @brief UAX #50 Unicode Vertical Text Layout (Unicode 18.0.0, Rev. 35).
 *
 * Resolved orientation per UAX #50 §2: V = R for Upright, V = Tu/Tr
 * handling, V = U otherwise; plus the upright/rotated layout decision
 * used by UTR #59.  The generated Vertical_Orientation values (U, R, Tu,
 * Tr) are the input; this module is the resolution step.
 */

typedef enum unicodoid_vtl_orientation {
    UNICODOID_VTL_UPRIGHT = 0, /**< Display upright (U). */
    UNICODOID_VTL_ROTATED = 1  /**< Display rotated (R). */
} unicodoid_vtl_orientation_t;

/** Raw Vertical_Orientation of @p cp (unicodoid_vo_t value). */
uint8_t unicodoid_uax50_vertical_orientation(uint32_t cp);

/**
 * @brief Resolved orientation (U or R) of @p cp.
 * U->U, R->R, Tu->U, Tr->R per UAX #50 table 1.
 */
uint8_t unicodoid_uax50_resolved_orientation(uint32_t cp);

/** True when @p cp should be displayed upright in vertical layout. */
bool unicodoid_uax50_is_upright(uint32_t cp);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UAX50_VTL_H */
