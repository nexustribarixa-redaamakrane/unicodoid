namespace Unicodoid;

/// <summary>
/// UAX #60 Data for East Asian Scripts (Unicode 18.0.0, Revision 3).
///
/// Data consumed by UAX #11 / #14 / #50 for East Asian scripts.  This class
/// exposes the script membership core over the generated Script property.
/// </summary>
public static class Uax60
{
    private static bool ScriptIs(uint cp, string name) =>
        string.Equals(Ucd.ScriptName(Ucd.Script(cp)), name, System.StringComparison.Ordinal);

    /// <summary>True when <paramref name="cp"/> is Han (Script=Hani).</summary>
    public static bool IsHan(uint cp) => ScriptIs(cp, "Han");

    /// <summary>True when <paramref name="cp"/> has an East Asian Script.</summary>
    public static bool IsEaScript(uint cp)
    {
        string? script = Ucd.ScriptName(Ucd.Script(cp));
        if (script is null) return false;
        return script is "Han" or "Hiragana" or "Katakana" or "Hangul" or
            "Bopomofo" or "Yi" or "Tangut" or "Nushu" or
            "Khitan_Small_Script" or "Jurchen";
    }

    /// <summary>True when <paramref name="cp"/> is East Asian Wide or Fullwidth.</summary>
    public static bool IsEaWide(uint cp)
    {
        byte width = Ucd.EastAsianWidth(cp);
        return width is Ucd.Eaw.W or Ucd.Eaw.F;
    }
}
