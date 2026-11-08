#ifndef UNICODOID_UTS18_REGEX_H
#define UNICODOID_UTS18_REGEX_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file uts18_regex.h
 * @brief UTS #18 Unicode Regular Expressions (Revision 25, RL1.1 level).
 *
 * Unicode-aware regular-expression character classes: \p{...} / \P{...}
 * property escapes.  This module evaluates a single property escape
 * against the generated UCD tables and reports syntax validity.  Full
 * pattern compilation, backreferences, and UnicodeSet operations
 * (UTS #61) are out of scope.
 */

typedef enum unicodoid_uts18_match {
    UNICODOID_UTS18_NONE = 0,
    UNICODOID_UTS18_GENERAL_CATEGORY = 1,
    UNICODOID_UTS18_SCRIPT = 2,
    UNICODOID_UTS18_BINARY = 3,
    UNICODOID_UTS18_EAW = 4,
    UNICODOID_UTS18_INVALID = 5
} unicodoid_uts18_match_t;

/** Width in bytes/chars of a "\p{...}" escape starting at @p pat. */
size_t unicodoid_uts18_property_escape_length(const char* pat);

/**
 * @brief Evaluate whether @p cp matches the escape at @p pat (negated
 *        when the escape starts with "\P").
 * @return UNICODOID_UTS18_NONE when @p pat is not a property escape.
 */
int unicodoid_uts18_property_match(const char* pat, uint32_t cp);

/** True when @p pat is a syntactically valid property escape. */
bool unicodoid_uts18_is_valid_escape(const char* pat);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UTS18_REGEX_H */
