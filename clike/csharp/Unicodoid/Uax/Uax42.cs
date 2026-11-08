using System;
using System.Text;

namespace Unicodoid;

/// <summary>
/// UAX #42 Unicode Character Database in XML (Unicode 18.0.0, Revision 40).
///
/// UCD XML is a distribution format, not a runtime algorithm.  This module
/// provides the format identifier and a minimal writer that emits a
/// <c>&lt;char&gt;</c> record for one scalar from the generated tables.
/// </summary>
public static class Uax42
{
    /// <summary>Format identifier for Unicode 18.0.0.</summary>
    public const string Format = "UCDxml";

    /// <summary>True when <paramref name="cp"/> is a valid scalar (not a surrogate).</summary>
    public static bool IsRepresentable(uint cp) => UnicodeScalars.IsValid(cp);

    /// <summary>
    /// Emit a single-character XML record for <paramref name="cp"/>.
    /// Format: <c>&lt;char cp="XXXX" GC=".." Bidi_C=".." EA=".." LB=".." sc=".." ccc=".."/&gt;</c>
    /// </summary>
    public static string CharRecord(uint cp)
    {
        string gc = Ucd.GeneralCategoryName(Ucd.GeneralCategory(cp)) ?? "Cn";
        string bc = Ucd.BidiClassName(Ucd.BidiClass(cp)) ?? "L";
        string ea = Ucd.EastAsianWidthName(Ucd.EastAsianWidth(cp)) ?? "N";
        string lb = Ucd.LineBreakName(Ucd.LineBreak(cp)) ?? "XX";
        string sc = Ucd.ScriptName(Ucd.Script(cp)) ?? "Unknown";
        int ccc = Ucd.CombiningClass(cp);

        var builder = new StringBuilder(96);
        builder.Append("<char cp=\"");
        builder.Append(cp.ToString("X4"));
        builder.Append("\" GC=\"");
        builder.Append(gc);
        builder.Append("\" Bidi_C=\"");
        builder.Append(bc);
        builder.Append("\" EA=\"");
        builder.Append(ea);
        builder.Append("\" LB=\"");
        builder.Append(lb);
        builder.Append("\" sc=\"");
        builder.Append(sc);
        builder.Append("\" ccc=\"");
        builder.Append(ccc);
        builder.Append("\"/>\n");
        return builder.ToString();
    }
}
