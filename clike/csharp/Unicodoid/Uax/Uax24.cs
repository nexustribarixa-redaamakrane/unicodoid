using System;

namespace Unicodoid;

/// <summary>
/// UAX #24 Unicode Script Property (Unicode 18.0.0, Revision 41).
///
/// Script values are generated indices; use <see cref="ScriptName"/> for the
/// long Script name (for example "Latin").  Script_Extensions is not carried
/// by the generated tables and is out of scope.
/// </summary>
public static class Uax24
{
    /// <summary>Number of known Script values (including Unknown at index 0).</summary>
    public static int Count => Ucd.ScriptCount;

    /// <summary>Script name for <paramref name="value"/>, or null when out of range.</summary>
    public static string? ScriptName(int value) => Ucd.ScriptName(value);

    /// <summary>Script value of <paramref name="cp"/>.</summary>
    public static ushort Script(uint cp) => Ucd.Script(cp);

    /// <summary>Find a Script value by name; 0 (Unknown) when absent.</summary>
    public static ushort Find(string name) => Ucd.ScriptFind(name);

    /// <summary>True when <paramref name="cp"/> belongs to Script value <paramref name="script"/>.</summary>
    public static bool IsScript(uint cp, ushort script) => Ucd.Script(cp) == script;
}
