#include "unicodoid/utr/utr33_conformance.hpp"

#include <string_view>

namespace unicodoid {
namespace utr33 {

std::string_view level_name(int level) {
    switch (level) {
        case static_cast<int>(Level::WellFormed): return "Well_Formed";
        case static_cast<int>(Level::Valid): return "Valid";
        case static_cast<int>(Level::Stable): return "Stable";
        default: return "Unspecified";
    }
}

} // namespace utr33
} // namespace unicodoid
