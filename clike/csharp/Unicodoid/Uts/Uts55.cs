using System;

namespace Unicodoid;

/// <summary>
/// UTS #55 Unicode Source Code Handling (Revision 5).
///
/// Source-code safety checks: Bidi embedding/override marks and joiners in
/// source.  Full language-grammar handling is out of scope.
/// </summary>
public static class Uts55
{
    /// <summary>True when <paramref name="cp"/> is a Bidi control (Embed/Override/Isolate).</summary>
    public static bool IsBidiControl(uint cp) =>
        Ucd.HasBinaryProperty(cp, Ucd.BinaryProperty.BidiControl);

    /// <summary>True when <paramref name="cp"/> is ZWJ or ZWNJ.</summary>
    public static bool IsJoiner(uint cp) => cp == 0x200C || cp == 0x200D;

    /// <summary>Number of UTS #55 flagged characters in <paramref name="cps"/>.</summary>
    public static int FlagCount(ReadOnlySpan<uint> cps)
    {
        int count = 0;
        foreach (uint cp in cps)
        {
            if (IsBidiControl(cp) || IsJoiner(cp)) ++count;
        }
        return count;
    }
}
