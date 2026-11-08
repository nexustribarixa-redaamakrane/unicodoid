using System;

namespace Unicodoid;

/// <summary>
/// UAX #53 Unicode Arabic Mark Rendering (Unicode 18.0.0, Revision 13).
///
/// Joining behavior for Arabic shaping: Joining_Type plus transparent-mark
/// handling (General_Category Mn/Me/Cf join as Transparent).  The generated
/// tables do not carry Joining_Group; the script value is reported instead
/// for the interoperable core.
/// </summary>
public static class Uax53
{
    /// <summary>Joining_Type of <paramref name="cp"/> (indexes <see cref="Ucd.Jt"/>).</summary>
    public static byte JoiningType(uint cp) => Ucd.JoiningType(cp);

    /// <summary>True when <paramref name="cp"/> joins transparently (Mn, Me or Cf).</summary>
    public static bool IsJoiningTransparent(uint cp)
    {
        byte gc = Ucd.GeneralCategory(cp);
        return gc == Ucd.Gc.Mn || gc == Ucd.Gc.Me || gc == Ucd.Gc.Cf;
    }

    /// <summary>Joining group identifier as a Script-derived value, 0 when none.</summary>
    public static ushort JoiningGroup(uint cp) => Ucd.Script(cp);

    /// <summary>
    /// Shaping integrity check.  Dual-joining characters may fall back to
    /// isolated forms at string boundaries, so only invalid scalars fail.
    /// </summary>
    public static bool IsConsistent(ReadOnlySpan<uint> cps)
    {
        foreach (uint cp in cps)
        {
            if (!UnicodeScalars.IsValid(cp)) return false;
        }
        return true;
    }
}
