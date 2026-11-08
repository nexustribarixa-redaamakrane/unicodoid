#ifndef UNICODOID_UAX50_VTL_HPP
#define UNICODOID_UAX50_VTL_HPP

#include <cstdint>

namespace unicodoid {
namespace uax50 {

/** UAX #50 Unicode Vertical Text Layout (Unicode 18.0.0, Rev. 35). */

/** Resolved display orientation. */
enum class Orientation : uint8_t {
    Upright = 0,
    Rotated = 1
};

/** Raw Vertical_Orientation of @p cp (unicodoid_vo_t value). */
uint8_t vertical_orientation(uint32_t cp);

/** Resolved orientation (U or R): U->U, R->R, Tu->U, Tr->R. */
uint8_t resolved_orientation(uint32_t cp);

/** True when @p cp should be displayed upright in vertical layout. */
bool is_upright(uint32_t cp);

} // namespace uax50
} // namespace unicodoid

#endif /* UNICODOID_UAX50_VTL_HPP */
