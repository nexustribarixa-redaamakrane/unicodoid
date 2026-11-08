namespace Unicodoid;

/// <summary>
/// UTR #17 Unicode Character Encoding Model (Revision 9).
///
/// Informative model: code points, code units, and the transformation
/// formats.  This class answers the model queries that encoders need.
/// </summary>
public static class Utr17
{
    /// <summary>Number of UTF-8 code units for a valid scalar, 0 for invalid.</summary>
    public static int Utf8Length(uint cp)
    {
        if (!UnicodeScalars.IsValid(cp)) return 0;
        return cp <= 0x7F ? 1 : cp <= 0x7FF ? 2 : cp <= 0xFFFF ? 3 : 4;
    }

    /// <summary>Number of UTF-16 code units for a valid scalar, 0 for invalid.</summary>
    public static int Utf16Length(uint cp)
    {
        if (!UnicodeScalars.IsValid(cp)) return 0;
        return cp <= 0xFFFF ? 1 : 2;
    }

    /// <summary>True when <paramref name="cp"/> is a valid Unicode scalar value.</summary>
    public static bool IsValid(uint cp) => UnicodeScalars.IsValid(cp);

    /// <summary>True when <paramref name="cp"/> is a replacement candidate (noncharacter or unassigned).</summary>
    public static bool IsReplacement(uint cp)
    {
        if (Ucd.HasBinaryProperty(cp, Ucd.BinaryProperty.NoncharacterCodePoint)) return true;
        return Ucd.GeneralCategory(cp) == Ucd.Gc.Cn;
    }
}
