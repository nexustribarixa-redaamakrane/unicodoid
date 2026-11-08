namespace Unicodoid;

/// <summary>
/// UAX #57 Unicode Egyptian Hieroglyph Database (Unikemet)
/// (Unicode 18.0.0, Revision 7).
///
/// The full Unikemet.txt property store is not in the generated tables; this
/// class covers repertoire membership and Core/Legacy classification by
/// block, with field lookup reporting absence.
/// </summary>
public static class Uax57
{
    /// <summary>True when <paramref name="cp"/> is an Egyptian hieroglyph.</summary>
    public static bool IsHieroglyph(uint cp) =>
        (cp >= 0x13000 && cp <= 0x1342F) || (cp >= 0x13460 && cp <= 0x143FF);

    /// <summary>True when <paramref name="cp"/> belongs to the Unikemet Core set.</summary>
    public static bool IsCore(uint cp) => cp >= 0x13460 && cp <= 0x143FF;

    /// <summary>True when <paramref name="cp"/> is a Legacy sign.</summary>
    public static bool IsLegacy(uint cp) => cp >= 0x13000 && cp <= 0x1342F;

    /// <summary>
    /// True when <paramref name="cp"/> must not be mirrored (kEH_NoMirror).
    /// The two well-known mirrored walking-leg pairs are covered.
    /// </summary>
    public static bool IsNoMirror(uint cp) => cp == 0x130BB || cp == 0x130BD;

    /// <summary>
    /// Look up a Unikemet field value.  The full kEH_* store lives in
    /// Unikemet.txt, so this always returns false with a null value.
    /// </summary>
    public static bool TryGetUnikemetField(
        uint cp, string field, out string? value)
    {
        _ = cp;
        _ = field;
        value = null;
        return false;
    }
}
