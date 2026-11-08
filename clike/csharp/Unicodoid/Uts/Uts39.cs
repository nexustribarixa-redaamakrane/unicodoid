using System;
using System.Collections.Generic;

namespace Unicodoid;

/// <summary>
/// UTS #39 Unicode Security Mechanisms (Unicode 18.0.0, Revision 34).
///
/// Confusable skeletons using the generated confusables table.  Identifier
/// status files are distribution data; full-file checks are out of scope.
/// </summary>
public static class Uts39
{
    private const int MaxSkeletonPrefix = 8;
    private const int MaxFoldRounds = 3;

    /// <summary>Visual confusable skeleton of one scalar.</summary>
    public static int Confusable(uint cp, Span<uint> output)
    {
        uint[]? mapping = Ucd.Confusable(cp);
        if (mapping is null || mapping.Length == 0) return 0;
        int count = Math.Min(mapping.Length, output.Length);
        mapping.AsSpan(0, count).CopyTo(output);
        return mapping.Length;
    }

    /// <summary>Confusable mapping of one scalar.</summary>
    public static uint[] Confusable(uint cp) => Ucd.Confusable(cp) ?? Array.Empty<uint>();

    /// <summary>True when <paramref name="cp"/> is Default_Ignorable_Code_Point.</summary>
    public static bool IsDefaultIgnorable(uint cp) =>
        Ucd.HasBinaryProperty(cp, Ucd.BinaryProperty.DefaultIgnorableCodePoint);

    /// <summary>Visual confusable skeleton of a scalar sequence.</summary>
    public static uint[] Skeleton(ReadOnlySpan<uint> cps)
    {
        if (cps.IsEmpty) return Array.Empty<uint>();

        var output = new List<uint>(cps.Length);
        foreach (uint first in cps)
        {
            uint[] current = [first];
            for (int round = 0; round < MaxFoldRounds; ++round)
            {
                var next = new List<uint>(MaxSkeletonPrefix);
                foreach (uint value in current)
                {
                    uint[] mapping = Confusable(value);
                    if (mapping.Length == 0)
                    {
                        if (next.Count < MaxSkeletonPrefix) next.Add(value);
                    }
                    else
                    {
                        foreach (uint mapped in mapping)
                        {
                            if (next.Count >= MaxSkeletonPrefix) break;
                            next.Add(mapped);
                        }
                    }
                }

                uint[] folded = next.ToArray();
                if (folded.Length == current.Length && folded.AsSpan().SequenceEqual(current))
                {
                    break;
                }
                current = folded;
                if (current.Length == 0) break;
            }
            output.AddRange(current);
        }
        return output.ToArray();
    }

    /// <summary>Write the skeleton to <paramref name="output"/>; returns the required length.</summary>
    public static int Skeleton(ReadOnlySpan<uint> cps, Span<uint> output)
    {
        uint[] skeleton = Skeleton(cps);
        int count = Math.Min(skeleton.Length, output.Length);
        skeleton.AsSpan(0, count).CopyTo(output);
        return skeleton.Length;
    }

    /// <summary>True when the skeletons of the two sequences are equal.</summary>
    public static bool AreConfusable(ReadOnlySpan<uint> a, ReadOnlySpan<uint> b)
    {
        if (a.IsEmpty || b.IsEmpty) return a.Length == b.Length;
        return Skeleton(a).AsSpan().SequenceEqual(Skeleton(b));
    }
}
