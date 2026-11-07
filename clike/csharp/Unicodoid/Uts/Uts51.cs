using System;

namespace Unicodoid;

/// <summary>
/// UTS #51 Unicode Emoji sequence validation (Unicode 18.0.0, Revision 4).
///
/// Implements the machine-readable RGI emoji sequence sets distributed in
/// <c>emoji-sequences.txt</c> and <c>emoji-zwj-sequences.txt</c>:
/// Basic_Emoji, Emoji_Keycap_Sequence, RGI_Emoji_Modifier_Sequence,
/// RGI_Emoji_Flag_Sequence, RGI_Emoji_Tag_Sequence and
/// RGI_Emoji_ZWJ_Sequence.
///
/// Glyph selection, presentation and emoji-data derived properties are outside
/// this module; only sequence recognition and structural validation are
/// provided.
/// </summary>
public static class Uts51
{
    /// <summary>Number of RGI emoji sequences in the generated data.</summary>
    public static int Count => Ucd.EmojiSeqCount;

    /// <summary>RGI type name of sequence <paramref name="index"/>, or null.</summary>
    public static string? TypeName(int index) => Ucd.EmojiSeqType(index);

    /// <summary>Code points of sequence <paramref name="index"/>, or null.</summary>
    public static uint[]? Sequence(int index) => Ucd.EmojiSeqCps(index);

    private static bool ValidScalar(uint cp) =>
        cp <= 0x10FFFF && cp is < 0xD800 or > 0xDFFF;

    private static bool IsRegionalIndicator(uint cp) => cp is >= 0x1F1E6 and <= 0x1F1FF;

    private static bool IsEmojiModifier(uint cp) => cp is >= 0x1F3FB and <= 0x1F3FF;

    private static bool IsTagChar(uint cp) => cp is >= 0xE0020 and <= 0xE007E;

    /// <summary>True when <paramref name="cps"/> is an exact RGI sequence; returns its type.</summary>
    public static string? IsRgi(ReadOnlySpan<uint> cps)
    {
        if (cps.IsEmpty) return null;
        foreach (uint cp in cps)
        {
            if (!ValidScalar(cp)) return null;
        }

        for (int i = 0; i < Count; ++i)
        {
            uint[]? sequence = Sequence(i);
            if (sequence is not null && cps.SequenceEqual(sequence)) return TypeName(i);
        }
        return null;
    }

    /// <summary>
    /// Structural validation independent of the RGI table.  Checks the
    /// UTS #51 shapes: keycap, flag, tag, modifier and ZWJ sequences.
    /// </summary>
    public static bool IsWellFormed(ReadOnlySpan<uint> cps)
    {
        if (cps.IsEmpty) return false;
        foreach (uint cp in cps)
        {
            if (!ValidScalar(cp)) return false;
        }

        // Emoji_Keycap_Sequence: [0-9#*] FE0F 20E3.
        if (cps.Length == 3 && cps[1] == 0xFE0F && cps[2] == 0x20E3)
        {
            uint b = cps[0];
            return b is >= (uint)'0' and <= (uint)'9' || b == '#' || b == '*';
        }

        // RGI_Emoji_Flag_Sequence: exactly two regional indicators.
        if (cps.Length == 2 && IsRegionalIndicator(cps[0]) && IsRegionalIndicator(cps[1]))
        {
            return true;
        }

        // RGI_Emoji_Tag_Sequence: 1F3F4 (tag chars)* E007F.
        if (cps.Length >= 3 && cps[0] == 0x1F3F4 && cps[^1] == 0xE007F)
        {
            for (int i = 1; i < cps.Length - 1; ++i)
            {
                if (!IsTagChar(cps[i])) return false;
            }
            return true;
        }

        // RGI_Emoji_Modifier_Sequence: modifier base + modifier.
        if (cps.Length == 2 && Ucd.EmojiModifierBase(cps[0]) && IsEmojiModifier(cps[1]))
        {
            return true;
        }

        // RGI_Emoji_ZWJ_Sequence: ExtPict (modifier/VS16)* (ZWJ ExtPict ...).
        if (cps.IndexOf(0x200Du) >= 0)
        {
            if (!Ucd.ExtendedPictographic(cps[0])) return false;
            int i = 1;
            while (i < cps.Length)
            {
                if (cps[i] == 0xFE0F || IsEmojiModifier(cps[i]))
                {
                    ++i;
                    continue;
                }
                if (cps[i] != 0x200D) return false;
                ++i;
                if (i >= cps.Length || !Ucd.ExtendedPictographic(cps[i])) return false;
                ++i;
            }
            return true;
        }

        // Basic_Emoji: a single emoji code point, optionally followed by VS16.
        // Emoji modifiers are Basic_Emoji in their own right even though they
        // are not Extended_Pictographic.
        if (cps.Length == 1 || (cps.Length == 2 && cps[1] == 0xFE0F))
        {
            uint cp = cps[0];
            if (Ucd.ExtendedPictographic(cp) || Ucd.Emoji(cp)) return true;
        }

        return false;
    }
}
