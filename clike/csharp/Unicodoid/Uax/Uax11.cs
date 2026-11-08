using System;

namespace Unicodoid;

/// <summary>UAX #11 East Asian Width (Unicode 18.0.0, Revision 46).</summary>
public static class Uax11
{
    /// <summary>Raw East_Asian_Width of <paramref name="cp"/> (indexes <see cref="Ucd.Eaw"/>).</summary>
    public static byte Eaw(uint cp) => Ucd.EastAsianWidth(cp);

    /// <summary>True when <paramref name="cp"/> is Fullwidth or Wide.</summary>
    public static bool IsWide(uint cp)
    {
        byte value = Eaw(cp);
        return value is Ucd.Eaw.F or Ucd.Eaw.W;
    }

    /// <summary>Terminal column count: W/F = 2, A = 2 when opted in, otherwise 1.</summary>
    public static int ColumnCount(uint cp, bool ambiguousAsWide)
    {
        byte value = Eaw(cp);
        if (value is Ucd.Eaw.F or Ucd.Eaw.W) return 2;
        if (value == Ucd.Eaw.A && ambiguousAsWide) return 2;
        return 1;
    }

    /// <summary>Sum of column counts for a scalar string.</summary>
    public static int Columns(ReadOnlySpan<uint> cps, bool ambiguousAsWide)
    {
        int total = 0;
        foreach (uint cp in cps) total += ColumnCount(cp, ambiguousAsWide);
        return total;
    }
}
