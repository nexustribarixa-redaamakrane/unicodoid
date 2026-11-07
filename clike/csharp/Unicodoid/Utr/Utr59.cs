using System;
using System.Collections.Generic;

namespace Unicodoid;

/// <summary>UTR #59 East_Asian_Spacing property values (Table 1).</summary>
public static class Eas
{
    /// <summary>Wide: East Asian script character.</summary>
    public const int W = 1;

    /// <summary>Narrow: needs spacing next to W.</summary>
    public const int N = 2;

    /// <summary>Other: no auto-spacing.</summary>
    public const int O = 3;

    /// <summary>Conditional: N for Chinese, else O.</summary>
    public const int C = 4;
}

/// <summary>
/// UTR #59 East Asian Spacing (Unicode 18.0.0, Revision 1 draft).
///
/// Implements the informative East_Asian_Spacing property and the spacing
/// algorithm of UTR59 §3.3 against the official derived data file
/// (<c>east-asian-spacing.txt</c>) generated for Unicode 18.0.0.
///
/// The algorithm operates on extended grapheme clusters (UAX #29).  It does not
/// decide the visual width of the inserted space; callers choose whether to add
/// a glyph advance or a space character such as U+2009.
/// </summary>
public static class Utr59
{
    /// <summary>Raw East_Asian_Spacing value for a code point (never resolves C).</summary>
    public static int EastAsianSpacing(uint cp) => Ucd.EastAsianSpacing(cp);

    /// <summary>UTR59 §3.3.1 step 4: upright when Vertical_Orientation is U, Tu or Tr.</summary>
    private static bool IsUpright(uint cp)
    {
        byte vo = Ucd.VerticalOrientation(cp);
        return vo is Ucd.Vo.U or Ucd.Vo.Tu or Ucd.Vo.Tr;
    }

    /// <summary>Resolve the property value of one extended grapheme cluster.</summary>
    public static int ClusterValue(ReadOnlySpan<uint> cps, bool chinese, bool vertical)
    {
        if (cps.IsEmpty) return Eas.O;

        // Step 3: the first code point determines the initial value.
        int value = EastAsianSpacing(cps[0]);

        // UTR59 §3.4.1: an enclosing mark forces the whole cluster to O.
        foreach (uint cp in cps)
        {
            if (Ucd.GeneralCategory(cp) == Ucd.Gc.Me)
            {
                value = Eas.O;
                break;
            }
        }

        // Steps 4-5: vertical layout, upright Narrow characters become Other.
        if (vertical && value == Eas.N && IsUpright(cps[0])) value = Eas.O;

        // Step 6: resolve Conditional.
        if (value == Eas.C) value = chinese ? Eas.N : Eas.O;

        return value;
    }

    /// <summary>
    /// Compute where auto-spacing is inserted.  Iterates extended grapheme
    /// clusters and records every boundary between a W cluster and an N cluster
    /// (in either order).  Each offset is the scalar index of the first code
    /// point of the following cluster.
    /// </summary>
    public static int[] SpacingOffsets(ReadOnlySpan<uint> cps, bool chinese, bool vertical)
    {
        var offsets = new List<int>();
        if (cps.IsEmpty) return offsets.ToArray();

        int start = 0;
        int end = Segmenter.NextGraphemeCluster(cps, start);
        int prev = ClusterValue(cps[start..end], chinese, vertical);

        while (end < cps.Length)
        {
            int nextStart = end;
            int nextEnd = Segmenter.NextGraphemeCluster(cps, nextStart);
            int next = ClusterValue(cps[nextStart..nextEnd], chinese, vertical);

            bool insert = (prev == Eas.W && next == Eas.N) || (prev == Eas.N && next == Eas.W);
            if (insert) offsets.Add(nextStart);

            prev = next;
            start = nextStart;
            end = nextEnd;
        }

        _ = start;
        return offsets.ToArray();
    }
}
