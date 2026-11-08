using System;
using System.Collections.Generic;

namespace Unicodoid;

/// <summary>
/// UTS #46 Unicode IDNA Compatibility Processing (18.0.0, Revision 36).
///
/// IDNA processing steps 1..7 (map, NFC, split, validate, convert) using the
/// generated IdnaMappingTable status.  Punycode and full IDNA2008 label
/// validation are out of scope.
/// </summary>
public static class Uts46
{
    /// <summary>Scalar is valid in IDNA.</summary>
    public const int Valid = 0;

    /// <summary>Scalar is mapped to another sequence.</summary>
    public const int Mapped = 1;

    /// <summary>Scalar has a deviation disposition.</summary>
    public const int Deviation = 2;

    /// <summary>Scalar is ignored.</summary>
    public const int Ignored = 3;

    /// <summary>Scalar is disallowed.</summary>
    public const int Disallowed = 4;

    /// <summary>UTS #46 disposition of one scalar.</summary>
    public static int Disposition(uint cp) => Ucd.IdnaStatus(cp, out _);

    /// <summary>Map a scalar sequence (UTS #46 steps 4..6: map + NFC).</summary>
    public static uint[] Map(ReadOnlySpan<uint> input)
    {
        if (input.IsEmpty) return Array.Empty<uint>();

        var mapped = new List<uint>(input.Length * 2 + 4);
        foreach (uint cp in input)
        {
            int status = Ucd.IdnaStatus(cp, out uint[]? mapping);
            if (status == Ignored) continue;
            if ((status == Mapped || status == Deviation) && mapping is not null)
            {
                mapped.AddRange(mapping);
            }
            else
            {
                mapped.Add(cp);
            }
        }
        return Normalization.Normalize(mapped.ToArray(), NormalizationForm.Nfc);
    }

    /// <summary>True when <paramref name="label"/> survives mapping and has no disallowed scalars.</summary>
    public static bool CheckLabel(ReadOnlySpan<uint> label)
    {
        if (label.IsEmpty) return false;
        foreach (uint cp in label)
        {
            if (Disposition(cp) == Disallowed) return false;
        }
        return true;
    }

    /// <summary>True when <paramref name="cp"/> must not appear in a transitional label.</summary>
    public static bool IsTransitionalDisallowed(uint cp) => Ucd.IdnaNv8(cp);
}
