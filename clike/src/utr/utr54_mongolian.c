#include "unicodoid/utr/utr54_mongolian.h"

bool unicodoid_utr54_is_mongolian(uint32_t cp) {
    return cp >= 0x1800 && cp <= 0x18AF;
}

bool unicodoid_utr54_is_fvs(uint32_t cp) {
    return cp == 0x180B || cp == 0x180C || cp == 0x180D || cp == 0x180F;
}

bool unicodoid_utr54_is_mvs(uint32_t cp) {
    return cp == 0x180E;
}
