#ifndef UNICODOID_UAX60_EA_SCRIPT_DATA_HPP
#define UNICODOID_UAX60_EA_SCRIPT_DATA_HPP

#include <cstdint>

namespace unicodoid {
namespace uax60 {

/** UAX #60 Data for East Asian Scripts (Unicode 18.0.0, Rev. 3). */

/** True when @p cp has an East Asian script. */
bool is_ea_script(uint32_t cp);

/** True when @p cp is Han (Script=Han). */
bool is_han(uint32_t cp);

/** True when @p cp is East Asian Wide or Fullwidth. */
bool is_ea_wide(uint32_t cp);

} // namespace uax60
} // namespace unicodoid

#endif /* UNICODOID_UAX60_EA_SCRIPT_DATA_HPP */
