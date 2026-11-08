#include "unicodoid/utr/utr33_conformance.h"

const char* unicodoid_utr33_level_name(int level) {
    switch (level) {
        case UNICODOID_CONFORMANCE_WELL_FORMED: return "Well_Formed";
        case UNICODOID_CONFORMANCE_VALID: return "Valid";
        case UNICODOID_CONFORMANCE_STABLE: return "Stable";
        default: return "Unspecified";
    }
}
