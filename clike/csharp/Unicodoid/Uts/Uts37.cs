using System;

namespace Unicodoid;

/// <summary>
/// UTS #37 Unicode Ideographic Variation Database (Revision 16).
///
/// IVS records are a base character plus an ideographic variation selector
/// (U+E0100..U+E01EF).  The IVD file is not in the generated tables; this
/// class provides well-formedness checks and selector range helpers.
/// </summary>
public static class Uts37
{
    /// <summary>True when <paramref name="cp"/> is an ideographic variation selector.</summary>
    public static bool IsVariationSelector(uint cp) => cp >= 0xE0100 && cp <= 0xE01EF;

    /// <summary>Selector offset (0-based) of <paramref name="cp"/>, or -1 when not a selector.</summary>
    public static int SelectorOffset(uint cp) =>
        IsVariationSelector(cp) ? (int)(cp - 0xE0100) : -1;

    /// <summary>True when <paramref name="cps"/> is an IVS (two scalars, second a selector).</summary>
    public static bool IsIvs(ReadOnlySpan<uint> cps)
    {
        if (cps.Length != 2) return false;
        if (!UnicodeScalars.IsValid(cps[0])) return false;
        return IsVariationSelector(cps[1]);
    }
}
