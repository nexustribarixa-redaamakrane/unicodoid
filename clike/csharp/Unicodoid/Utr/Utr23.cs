namespace Unicodoid;

/// <summary>
/// UTR #23 The Unicode Character Property Model (Revision 15).
///
/// Informative model for properties: enumeration vs. binary vs. numeric
/// vs. string.
/// </summary>
public static class Utr23
{
    /// <summary>Binary property kind.</summary>
    public const int Binary = 0;

    /// <summary>Enumerated property kind.</summary>
    public const int Enumerated = 1;

    /// <summary>Numeric property kind.</summary>
    public const int Numeric = 2;

    /// <summary>String property kind.</summary>
    public const int String = 3;

    /// <summary>Number of supported binary properties.</summary>
    public const int BinaryPropertyCount = 41;

    /// <summary>True when the property at <paramref name="index"/> is binary.</summary>
    public static bool IsBinary(int index) => index >= 0 && index < BinaryPropertyCount;

    /// <summary>Property-kind classification for a named property (Binary for unknown names).</summary>
    public static int PropertyKind(string? name)
    {
        if (name is null) return Binary;
        return name switch
        {
            "General_Category" => Enumerated,
            "Numeric_Type" => Numeric,
            "Unicode_1_Name" => String,
            _ => Binary
        };
    }
}
