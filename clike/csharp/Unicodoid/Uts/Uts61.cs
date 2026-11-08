using System;
using System.Globalization;

namespace Unicodoid;

/// <summary>
/// UTS #61 Unicode Set Notation (Version 1, Draft 4).
///
/// Draft set-notation syntax: <c>\p{...}</c> property sets and "X..Y" ranges.
/// Full UnicodeSet algebra is out of scope.
/// </summary>
public static class Uts61
{
    /// <summary>
    /// Membership of <paramref name="cp"/> in the set expression
    /// <paramref name="set"/>.  Returns 1 in, 0 out, -1 on parse error.
    /// </summary>
    public static int SetMembership(string? set, uint cp)
    {
        if (set is null) return -1;

        // Single property escape: delegate to UTS #18.
        if (set.Length >= 2 && set[0] == '\\' && (set[1] == 'p' || set[1] == 'P'))
        {
            int match = Uts18.PropertyMatch(set, cp);
            if (match == Uts18.Invalid) return -1;
            return match == Uts18.None ? 0 : 1;
        }

        if (TryParseRange(set, out uint lo, out uint hi))
        {
            return InRange(cp, lo, hi) ? 1 : 0;
        }
        return -1;
    }

    /// <summary>Parse "X..Y" range text (hex scalars separated by "..").</summary>
    public static bool TryParseRange(string? text, out uint lo, out uint hi)
    {
        lo = 0;
        hi = 0;
        if (text is null) return false;

        int dots = text.IndexOf("..", StringComparison.Ordinal);
        if (dots <= 0) return false;

        string loText = text.Substring(0, dots);
        string hiText = text.Substring(dots + 2);
        if (!TryParseHex(loText, out uint a)) return false;
        if (!TryParseHex(hiText, out uint b)) return false;
        if (a > 0x10FFFF || b > 0x10FFFF || a > b) return false;

        lo = a;
        hi = b;
        return true;
    }

    /// <summary>True when <paramref name="cp"/> lies in [lo, hi].</summary>
    public static bool InRange(uint cp, uint lo, uint hi) => cp >= lo && cp <= hi;

    private static bool TryParseHex(string text, out uint value)
    {
        return uint.TryParse(
            text,
            NumberStyles.AllowHexSpecifier,
            CultureInfo.InvariantCulture,
            out value);
    }
}
