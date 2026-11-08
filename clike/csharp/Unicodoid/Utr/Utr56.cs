namespace Unicodoid;

/// <summary>
/// UTR #56 Unicode Cuneiform Sign Lists (Revision 5).
///
/// Sign-list repertoire for Cuneiform (U+12000..U+1247F, plus extensions).
/// Membership helpers over the generated Script/Block data; the sign-list
/// documents are references, not runtime tables.
/// </summary>
public static class Utr56
{
    /// <summary>True when <paramref name="cp"/> is in Cuneiform or its extensions.</summary>
    public static bool IsCuneiform(uint cp) =>
        (cp >= 0x12000 && cp <= 0x1247F) ||
        (cp >= 0x12480 && cp <= 0x125FF) ||
        (cp >= 0x14400 && cp <= 0x1467F);

    /// <summary>True when <paramref name="cp"/> is a Cuneiform number.</summary>
    public static bool IsNumber(uint cp) => cp >= 0x12480 && cp <= 0x125FF;
}
