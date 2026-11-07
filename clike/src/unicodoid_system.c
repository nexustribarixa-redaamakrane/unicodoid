#include "unicodoid/unicodoid_system.h"

const char* unicodoid_status_string(unicodoid_status_t status) {
    switch (status) {
        case UNICODOID_OK:                      return "ok";
        case UNICODOID_ERR_INVALID_PARAM:       return "invalid parameter";
        case UNICODOID_ERR_BUFFER_TOO_SMALL:    return "buffer too small";
        case UNICODOID_ERR_INVALID_CODE_UNIT:   return "invalid code unit";
        case UNICODOID_ERR_UNMAPPABLE_CHAR:     return "unmappable character";
        case UNICODOID_ERR_INCOMPLETE_SEQUENCE: return "incomplete sequence";
        case UNICODOID_ERR_NOT_FOUND:           return "not found";
        case UNICODOID_ERR_ALREADY_EXISTS:      return "already exists";
        case UNICODOID_ERR_UNSUPPORTED:         return "unsupported";
        case UNICODOID_ERR_CAPACITY_EXCEEDED:   return "capacity exceeded";
        case UNICODOID_ERR_ALLOCATION_FAILED:   return "allocation failed";
        case UNICODOID_ERR_INTERNAL:            return "internal error";
        case UNICODOID_ERR_STATE:               return "invalid codec state";
        case UNICODOID_ERR_OVERFLOW:            return "numeric overflow";
        case UNICODOID_ERR_NO_CHARACTER:        return "no character produced";
        default:                                return "unknown status";
    }
}
