#include "unicodoid/uax/uax41_common_refs.hpp"
#include "unicodoid/uax/uax41_common_refs.h"

#include <string_view>

namespace unicodoid {
namespace uax41 {

std::string_view revision() { return unicodoid_uax41_revision(); }
std::string_view date() { return unicodoid_uax41_date(); }
std::string_view url() { return unicodoid_uax41_url(); }

} // namespace uax41
} // namespace unicodoid
