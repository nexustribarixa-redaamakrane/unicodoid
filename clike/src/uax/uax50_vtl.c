#include "unicodoid/uax/uax50_vtl.h"
#include "unicodoid/unicodoid_ucd.h"

uint8_t unicodoid_uax50_vertical_orientation(uint32_t cp) {
    return unicodoid_ucd_vertical_orientation(cp);
}

uint8_t unicodoid_uax50_resolved_orientation(uint32_t cp) {
    uint8_t vo = unicodoid_ucd_vertical_orientation(cp);
    /* UAX #50 Table 1: U->U, Tu->U, R->R, Tr->R. */
    if (vo == (uint8_t)UNICODOID_VO_TR) return (uint8_t)UNICODOID_VO_R;
    if (vo == (uint8_t)UNICODOID_VO_TU) return (uint8_t)UNICODOID_VO_U;
    return vo;
}

bool unicodoid_uax50_is_upright(uint32_t cp) {
    return unicodoid_uax50_resolved_orientation(cp) == (uint8_t)UNICODOID_VO_U;
}
