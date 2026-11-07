#ifndef UNICODOID_UTS51_EMOJI_SEQUENCES_H
#define UNICODOID_UTS51_EMOJI_SEQUENCES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file uts51_emoji_sequences.h
 * @brief UTS #51 Unicode Emoji sequence validation (Unicode 18.0.0).
 *
 * Implements the machine-readable RGI emoji sequence sets defined by
 * UTS #51 and distributed in emoji-sequences.txt and emoji-zwj-sequences.txt:
 *
 *   - Basic_Emoji,
 *   - Emoji_Keycap_Sequence,
 *   - RGI_Emoji_Modifier_Sequence,
 *   - RGI_Emoji_Flag_Sequence,
 *   - RGI_Emoji_Tag_Sequence,
 *   - RGI_Emoji_ZWJ_Sequence.
 *
 * Glyph selection, presentation and emoji-data derived properties are
 * outside the scope of this module; only sequence recognition is provided.
 */

/** Number of RGI emoji sequences in the generated data. */
size_t unicodoid_uts51_count(void);

/** RGI type name of sequence @p index, or NULL when out of range. */
const char* unicodoid_uts51_type(size_t index);

/** Code points of sequence @p index, or SIZE_MAX when out of range. */
size_t unicodoid_uts51_sequence(size_t index, uint32_t* out, size_t cap);

/**
 * @brief True when @p cps is an exact RGI emoji sequence.
 *
 * @param type receives the RGI type name when the sequence is found and
 *             @p type is non-NULL.
 */
bool unicodoid_uts51_is_rgi(
    const uint32_t* cps, size_t length, const char** type);

/**
 * @brief Structural validation independent of the RGI table.
 *
 * Checks the UTS #51 shapes: keycap (base FE0F 20E3), flag (two regional
 * indicators), tag (1F3F4 tag-spec E007F), modifier (base modifier), and ZWJ
 * (Extended_Pictographic elements joined by U+200D).  This is intentionally
 * more permissive than RGI membership, which also depends on which base
 * characters have been assigned.
 */
bool unicodoid_uts51_is_well_formed(const uint32_t* cps, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UTS51_EMOJI_SEQUENCES_H */
