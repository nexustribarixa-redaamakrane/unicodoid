using System;

namespace Unicodoid;

/// <summary>
/// UAX #31 Unicode Identifiers and Syntax (Unicode 18.0.0, Revision 45).
///
/// Default identifier profile: XID_Start (XID_Continue | ZWNJ | ZWJ)*.
/// </summary>
public static class Uax31
{
    /// <summary>True when <paramref name="cp"/> is XID_Start.</summary>
    public static bool IsXidStart(uint cp) =>
        Ucd.HasBinaryProperty(cp, Ucd.BinaryProperty.XidStart);

    /// <summary>True when <paramref name="cp"/> is XID_Continue.</summary>
    public static bool IsXidContinue(uint cp) =>
        Ucd.HasBinaryProperty(cp, Ucd.BinaryProperty.XidContinue);

    /// <summary>True when <paramref name="cp"/> is ID_Start.</summary>
    public static bool IsIdStart(uint cp) =>
        Ucd.HasBinaryProperty(cp, Ucd.BinaryProperty.IdStart);

    /// <summary>True when <paramref name="cp"/> is ID_Continue.</summary>
    public static bool IsIdContinue(uint cp) =>
        Ucd.HasBinaryProperty(cp, Ucd.BinaryProperty.IdContinue);

    private static bool IsValidScalar(uint cp) =>
        cp <= 0x10FFFF && cp is < 0xD800 or > 0xDFFF;

    /// <summary>Validate a whole identifier against the default profile.</summary>
    public static bool IsValid(ReadOnlySpan<uint> cps)
    {
        if (cps.IsEmpty) return false;
        foreach (uint cp in cps)
        {
            if (!IsValidScalar(cp)) return false;
        }
        if (!IsXidStart(cps[0])) return false;
        for (int i = 1; i < cps.Length; ++i)
        {
            uint cp = cps[i];
            if (cp is 0x200C or 0x200D) continue;
            if (!IsXidContinue(cp)) return false;
        }
        return true;
    }

    /// <summary>True when <paramref name="cps"/> is NFKC-stable (UAX #31 stability check).</summary>
    public static bool IsNfkcStable(ReadOnlySpan<uint> cps)
    {
        if (cps.IsEmpty) return true;
        uint[] normalized = Normalization.Normalize(cps, NormalizationForm.Nfkc);
        return normalized.AsSpan().SequenceEqual(cps);
    }
}
