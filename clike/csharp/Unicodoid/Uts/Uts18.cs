using System;

namespace Unicodoid;

/// <summary>
/// UTS #18 Unicode Regular Expressions (Revision 25, RL1.1 level).
///
/// Evaluates a single <c>\p{...}</c> / <c>\P{...}</c> property escape against
/// the generated UCD tables and reports syntax validity.  Full pattern
/// compilation and UnicodeSet algebra are out of scope.
/// </summary>
public static class Uts18
{
    /// <summary>The scalar is not a member of a known property.</summary>
    public const int None = 0;

    /// <summary>Matched a General_Category value.</summary>
    public const int GeneralCategory = 1;

    /// <summary>Matched a Script value.</summary>
    public const int Script = 2;

    /// <summary>Matched a binary property.</summary>
    public const int Binary = 3;

    /// <summary>Matched an East_Asian_Width value.</summary>
    public const int Eaw = 4;

    /// <summary>The property name is not known.</summary>
    public const int Invalid = 5;

    private static readonly string[] s_gcShorts =
    [
        "Cn", "Cc", "Cf", "Co", "Cs", "Ll", "Lm", "Lo", "Lt", "Lu", "Mc", "Me", "Mn",
        "Nd", "Nl", "No", "Pc", "Pd", "Pe", "Pf", "Pi", "Po", "Ps", "Sc", "Sk", "Sm",
        "So", "Zl", "Zp", "Zs"
    ];

    /// <summary>Width in chars of a <c>\p{...}</c> escape starting at <paramref name="pat"/>, or 0.</summary>
    public static int PropertyEscapeLength(string? pat)
    {
        if (pat is null) return 0;
        bool isEscape = pat.StartsWith("\\p{", StringComparison.Ordinal) ||
                        pat.StartsWith("\\P{", StringComparison.Ordinal);
        if (!isEscape) return 0;
        int close = pat.IndexOf('}');
        return close < 0 ? 0 : close + 1;
    }

    /// <summary>True when <paramref name="pat"/> is a syntactically valid property escape.</summary>
    public static bool IsValidEscape(string? pat) => PropertyEscapeLength(pat) != 0;

    private static int MatchProperty(string name, uint cp)
    {
        // Known names return their kind when the scalar is a member, None when
        // the property is known but the scalar is not a member, and Invalid
        // when the name is not a known property.
        if (name is "Alphabetic" or "Alpha")
        {
            return Ucd.HasBinaryProperty(cp, Ucd.BinaryProperty.Alphabetic) ? Binary : None;
        }
        if (name == "Uppercase")
        {
            return Ucd.HasBinaryProperty(cp, Ucd.BinaryProperty.Uppercase) ? Binary : None;
        }
        if (name == "Lowercase")
        {
            return Ucd.HasBinaryProperty(cp, Ucd.BinaryProperty.Lowercase) ? Binary : None;
        }
        if (name == "Emoji")
        {
            return Ucd.HasBinaryProperty(cp, Ucd.BinaryProperty.Emoji) ? Binary : None;
        }

        byte gc = Ucd.GeneralCategory(cp);
        string? gcName = Ucd.GeneralCategoryName(gc);
        if (string.Equals(name, gcName, StringComparison.Ordinal)) return GeneralCategory;

        if (name == "Letter")
        {
            bool isLetter = gc >= Ucd.Gc.Ll && gc <= Ucd.Gc.Lu;
            return isLetter ? GeneralCategory : None;
        }
        if (name is "Uppercase_Letter" or "Lu")
        {
            return gc == Ucd.Gc.Lu ? GeneralCategory : None;
        }
        if (name is "Lowercase_Letter" or "Ll")
        {
            return gc == Ucd.Gc.Ll ? GeneralCategory : None;
        }

        if (name.Length == 2 && Array.IndexOf(s_gcShorts, name) >= 0) return None;

        string? script = Ucd.ScriptName(Ucd.Script(cp));
        if (string.Equals(name, script, StringComparison.Ordinal)) return Script;
        for (int i = 0; i < Ucd.ScriptCount; ++i)
        {
            if (string.Equals(name, Ucd.ScriptName(i), StringComparison.Ordinal)) return None;
        }

        string? eaw = Ucd.EastAsianWidthName(Ucd.EastAsianWidth(cp));
        if (string.Equals(name, eaw, StringComparison.Ordinal)) return Eaw;
        if (name is "Wide" or "Narrow" or "Neutral" or "Ambiguous" or "Halfwidth" or "Fullwidth")
        {
            return None;
        }
        return Invalid;
    }

    /// <summary>
    /// Evaluate whether <paramref name="cp"/> matches the escape at
    /// <paramref name="pat"/> (negated when the escape starts with <c>\P</c>).
    /// Returns <see cref="None"/> when <paramref name="pat"/> is not a property escape.
    /// </summary>
    public static int PropertyMatch(string? pat, uint cp)
    {
        if (pat is null) return None;

        bool negated = false;
        bool isEscape;
        if (pat.StartsWith("\\p{", StringComparison.Ordinal))
        {
            isEscape = true;
        }
        else if (pat.StartsWith("\\P{", StringComparison.Ordinal))
        {
            isEscape = true;
            negated = true;
        }
        else
        {
            isEscape = false;
        }
        if (!isEscape) return None;

        const int nameStart = 3;
        int close = pat.IndexOf('}', nameStart);
        if (close <= nameStart) return Invalid;

        string name = pat.Substring(nameStart, close - nameStart);
        int match = MatchProperty(name, cp);
        if (match == Invalid) return Invalid;
        if (negated)
        {
            // Negated escape: membership in the negated set is reported with
            // Binary kind (the exact family kind is not retained).
            return match == None ? Binary : None;
        }
        return match;
    }
}
