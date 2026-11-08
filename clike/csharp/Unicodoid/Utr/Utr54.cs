namespace Unicodoid;

/// <summary>
/// UTR #54 Unicode Mongolian 12.1 Baseline (Revision 1).
///
/// Baseline repertoire and shaping rules for Mongolian (U+1800..U+18AF).
/// Full baseline shaping is a font/layout concern; this class is the
/// repertoire view.
/// </summary>
public static class Utr54
{
    /// <summary>True when <paramref name="cp"/> is in the Mongolian block.</summary>
    public static bool IsMongolian(uint cp) => cp >= 0x1800 && cp <= 0x18AF;

    /// <summary>True when <paramref name="cp"/> is a Mongolian Free Variation Selector.</summary>
    public static bool IsFvs(uint cp) =>
        cp == 0x180B || cp == 0x180C || cp == 0x180D || cp == 0x180F;

    /// <summary>True when <paramref name="cp"/> is the Mongolian Vowel Separator.</summary>
    public static bool IsMvs(uint cp) => cp == 0x180E;
}
