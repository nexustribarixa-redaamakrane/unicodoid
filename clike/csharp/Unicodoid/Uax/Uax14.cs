using System;

namespace Unicodoid;

/// <summary>
/// UAX #14 Unicode Line Breaking Algorithm (Unicode 18.0.0, Revision 57).
///
/// The full LB1..LB31 rule engine is the existing <see cref="Segmenter"/>
/// implementation; these helpers expose it through a scalar API.  Break
/// positions are scalar indices, and mandatory breaks (BK/CR/LF/NL) are
/// reported like any other boundary.
/// </summary>
public static class Uax14
{
    /// <summary>Line_Break class of <paramref name="cp"/> (indexes <see cref="Ucd.Lb"/>).</summary>
    public static byte LineBreak(uint cp) => Ucd.LineBreak(cp);

    /// <summary>Index one past the end of the line segment starting at <paramref name="start"/>.</summary>
    public static int NextBreak(ReadOnlySpan<uint> cps, int start)
    {
        if (start < 0 || start >= cps.Length) return cps.Length;
        int[] boundaries = Segmenter.LineBoundaries(cps);
        for (int i = 0; i < boundaries.Length; ++i)
        {
            if (boundaries[i] > start) return boundaries[i];
        }
        return cps.Length;
    }

    /// <summary>True when a break occurs immediately before <paramref name="index"/> (1..size).</summary>
    public static bool IsBreak(ReadOnlySpan<uint> cps, int index)
    {
        if (index <= 0 || index >= cps.Length) return true;
        return Array.BinarySearch(Segmenter.LineBoundaries(cps), index) >= 0;
    }
}
