#ifndef UNICODOID_UTS39_SECURITY_HPP
#define UNICODOID_UTS39_SECURITY_HPP

#include <cstdint>
#include <string>

namespace unicodoid {
namespace uts39 {

/** UTS #39 Unicode Security Mechanisms (Unicode 18.0.0, Rev. 34). */

/** Confusable mapping of one scalar (empty when none). */
std::u32string confusable(uint32_t cp);

/** Visual confusable skeleton of a scalar sequence. */
std::u32string skeleton(const std::u32string& cps);

/** True when the skeletons of @p a and @p b are equal. */
bool are_confusable(const std::u32string& a, const std::u32string& b);

/** True when @p cp is Default_Ignorable_Code_Point. */
bool is_default_ignorable(uint32_t cp);

} // namespace uts39
} // namespace unicodoid

#endif /* UNICODOID_UTS39_SECURITY_HPP */
