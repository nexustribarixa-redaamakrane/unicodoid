namespace Unicodoid;

/// <summary>
/// UAX #45 U-source Ideographs (Unicode 18.0.0, Revision 34).
///
/// U-source (USourceData.txt) records ideographs encoded from the Unicode
/// sources project.  The generated tables carry Unified_Ideograph; this
/// module exposes membership and the US-source family classification.  Full
/// USourceData.txt field parsing is out of scope.
/// </summary>
public static class Uax45
{
    /// <summary>True when <paramref name="cp"/> is a U-source ideograph.</summary>
    public static bool IsUSource(uint cp)
    {
        if (Ucd.HasBinaryProperty(cp, Ucd.BinaryProperty.UnifiedIdeograph)) return true;
        // CJK Ext blocks carry U-source ideographs even before
        // Unified_Ideograph assignment in a given release.
        return (cp >= 0x20000 && cp <= 0x2FA1F) || (cp >= 0x30000 && cp <= 0x3134F);
    }

    /// <summary>True when <paramref name="cp"/> is in the U-source PUA staging area.</summary>
    public static bool IsUs1Staging(uint cp) => cp >= 0xF900 && cp <= 0xFAFF;

    /// <summary>US-source family name ("US1" for staging, "US" for CJK), or null.</summary>
    public static string? USourceFamily(uint cp)
    {
        if (IsUs1Staging(cp)) return "US1";
        if (IsUSource(cp)) return "US";
        return null;
    }
}
