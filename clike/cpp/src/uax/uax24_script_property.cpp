#include "unicodoid/uax/uax24_script_property.hpp"
#include "unicodoid/unicodoid_ucd.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace unicodoid {
namespace uax24 {

std::size_t count() {
    return static_cast<std::size_t>(unicodoid_ucd_script_names_count);
}

std::string_view script_name(uint16_t value) {
    const char* name = unicodoid_ucd_script_name(value);
    return name ? std::string_view(name) : std::string_view();
}

uint16_t script(uint32_t cp) {
    return unicodoid_ucd_script(cp);
}

uint16_t find(std::string_view name) {
    const std::string owned(name);
    for (std::size_t i = 0; i < count(); ++i) {
        if (owned == unicodoid_ucd_script_names[i]) {
            return static_cast<uint16_t>(i);
        }
    }
    return 0;
}

bool is_script(uint32_t cp, uint16_t script_value) {
    return unicodoid_ucd_script(cp) == script_value;
}

} // namespace uax24
} // namespace unicodoid
