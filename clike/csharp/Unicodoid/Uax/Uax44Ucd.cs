using System;
using System.Collections.Generic;

namespace Unicodoid;

/// <summary>
/// UAX #44 Unicode Character Database alias and file-format layer
/// (Unicode 18.0.0, Revision 38).
///
/// The generated tables already follow UAX #44's range and default
/// conventions; this class exposes the alias tables and the block/casefold
/// helpers so embedders can cite UAX #44 uniformly.
/// </summary>
public static class Uax44
{
    /// <summary>Canonical UCD version string.</summary>
    public static string Version => Ucd.Version;

    private static readonly Dictionary<string, string> s_propertyAliases = new(StringComparer.Ordinal)
    {
        ["gc"] = "General_Category",
        ["bc"] = "Bidi_Class",
        ["ea"] = "East_Asian_Width",
        ["lb"] = "Line_Break",
        ["sc"] = "Script",
        ["ccc"] = "Canonical_Combining_Class",
        ["VO"] = "Vertical_Orientation",
        ["SB"] = "Sentence_Break",
        ["WB"] = "Word_Break",
        ["GCB"] = "Grapheme_Cluster_Break",
        ["InCB"] = "Indic_Conjunct_Break",
        ["jt"] = "Joining_Type",
        ["jg"] = "Joining_Group",
        ["Alpha"] = "Alphabetic",
        ["Lower"] = "Lowercase",
        ["Upper"] = "Uppercase",
        ["Cased"] = "Cased",
        ["CI"] = "Case_Ignorable",
        ["ID_Start"] = "ID_Start",
        ["ID_Continue"] = "ID_Continue",
        ["XID_Start"] = "XID_Start",
        ["XID_Continue"] = "XID_Continue"
    };

    private static readonly Dictionary<(string Property, string Value), string> s_valueAliases = new()
    {
        [("General_Category", "Lu")] = "Uppercase_Letter",
        [("General_Category", "Ll")] = "Lowercase_Letter",
        [("Bidi_Class", "L")] = "Left_To_Right",
        [("Bidi_Class", "R")] = "Right_To_Left",
        [("Bidi_Class", "ON")] = "Other_Neutral",
        [("Bidi_Class", "EN")] = "European_Number",
        [("East_Asian_Width", "W")] = "Wide",
        [("East_Asian_Width", "Na")] = "Narrow",
        [("East_Asian_Width", "N")] = "Neutral",
        [("Line_Break", "AL")] = "Alphabetic",
        [("Line_Break", "ID")] = "Ideographic",
        [("Line_Break", "BA")] = "Break_After",
        [("Line_Break", "SP")] = "Space"
    };

    /// <summary>Long property name for a short alias, or null when unknown.</summary>
    public static string? PropertyAlias(string? alias) =>
        alias is not null && s_propertyAliases.TryGetValue(alias, out string? name) ? name : null;

    /// <summary>Long value name for (property, short value), or null when absent.</summary>
    public static string? ValueAlias(string? property, string? shortValue) =>
        property is not null && shortValue is not null &&
        s_valueAliases.TryGetValue((property, shortValue), out string? name)
            ? name
            : null;

    /// <summary>True when <paramref name="cp"/> has a CaseFolding mapping.</summary>
    public static bool HasCasefold(uint cp) => Ucd.HasCasefold(cp);

    /// <summary>Block name for <paramref name="cp"/> (Blocks.txt via the generated tables).</summary>
    public static string? BlockName(uint cp) => Ucd.BlockName(Ucd.Block(cp));
}
