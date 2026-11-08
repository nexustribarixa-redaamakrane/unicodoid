namespace Unicodoid;

/// <summary>
/// UAX #50 Unicode Vertical Text Layout (Unicode 18.0.0, Revision 35).
///
/// The generated Vertical_Orientation values (U, R, Tu, Tr) are the input;
/// this class is the resolution step (UAX #50 Table 1).
/// </summary>
public static class Uax50
{
    /// <summary>Raw Vertical_Orientation of <paramref name="cp"/>.</summary>
    public static byte VerticalOrientation(uint cp) => Ucd.VerticalOrientation(cp);

    /// <summary>
    /// Resolved orientation (U or R) of <paramref name="cp"/>.
    /// U-&gt;U, R-&gt;R, Tu-&gt;U, Tr-&gt;R per UAX #50 Table 1.
    /// </summary>
    public static byte ResolvedOrientation(uint cp)
    {
        byte value = Ucd.VerticalOrientation(cp);
        if (value == Ucd.Vo.Tr) return Ucd.Vo.R;
        if (value == Ucd.Vo.Tu) return Ucd.Vo.U;
        return value;
    }

    /// <summary>True when <paramref name="cp"/> should be displayed upright in vertical layout.</summary>
    public static bool IsUpright(uint cp) => ResolvedOrientation(cp) == Ucd.Vo.U;
}
