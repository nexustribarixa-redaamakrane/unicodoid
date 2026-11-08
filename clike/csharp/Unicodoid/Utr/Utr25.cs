namespace Unicodoid;

/// <summary>
/// UTR #25 Unicode Support for Mathematics (Revision 16, Draft 5).
///
/// Mathematical alphanumeric symbols and the Math binary property.
/// </summary>
public static class Utr25
{
    /// <summary>True when <paramref name="cp"/> has Math = Yes.</summary>
    public static bool IsMath(uint cp) => Ucd.HasBinaryProperty(cp, Ucd.BinaryProperty.Math);

    /// <summary>True when <paramref name="cp"/> is in Mathematical Alphanumeric Symbols.</summary>
    public static bool IsMathAlphanumeric(uint cp) => cp >= 0x1D400 && cp <= 0x1D7FF;

    /// <summary>True when <paramref name="cp"/> is a mathematical operator (Math + Sm/Sc).</summary>
    public static bool IsOperator(uint cp)
    {
        if (!IsMath(cp)) return false;
        byte gc = Ucd.GeneralCategory(cp);
        return gc == Ucd.Gc.Sm || gc == Ucd.Gc.Sc;
    }
}
