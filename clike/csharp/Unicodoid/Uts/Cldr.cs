using System;

namespace Unicodoid;

/// <summary>
/// UTS #35 Unicode Locale Data Markup Language (CLDR Part 1, CLDR 49).
///
/// Aggregated helpers for the CLDR surfaces carried by Unicodoid: locale
/// identifiers, number symbols, dates, collation settings, currency/person
/// names, keyboards, MessageFormat 2.0 and supplemental data.  Full CLDR
/// XML resource bundles are an external distribution.
/// </summary>
public static class Cldr
{
    /// <summary>CLDR version string.</summary>
    public const string Version = "49";

    /// <summary>Parsed BCP 47 locale identifier.</summary>
    public readonly record struct Locale(
        string Language, string Script, string Region, string Variant);

    /// <summary>Collation strength/alternate settings.</summary>
    public readonly record struct CollationSettings(int Strength, int Alternate);

    /// <summary>Number formatting symbols for a region.</summary>
    public readonly record struct NumberSymbols(
        string Group, string Decimal, string Minus, string Percent);

    /// <summary>Plural categories.</summary>
    public static class Plural
    {
        public const int Other = 0, One = 1, Two = 2, Few = 3, Many = 4, Zero = 5;
    }

    /// <summary>Person-name orders.</summary>
    public static class NameOrder
    {
        public const int FamilyFirst = 0, GivenFirst = 1;
    }

    // ---- Part 1: Core ----------------------------------------------------

    private static bool IsLowerAlpha(string s)
    {
        foreach (char c in s)
        {
            if (c < 'a' || c > 'z') return false;
        }
        return true;
    }

    private static bool IsAlpha(string s)
    {
        foreach (char c in s)
        {
            if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) return false;
        }
        return true;
    }

    private static bool IsAlnum(string s)
    {
        foreach (char c in s)
        {
            if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                  (c >= '0' && c <= '9')))
            {
                return false;
            }
        }
        return true;
    }

    /// <summary>Parse a BCP 47 tag; false on an invalid tag.</summary>
    public static bool TryParseLocale(string? tag, out Locale locale)
    {
        locale = new Locale("und", "", "", "");
        if (tag is null || tag.Length >= 64) return false;

        string[] all = tag.Split('-', StringSplitOptions.RemoveEmptyEntries);
        if (all.Length < 1) return false;
        if (all.Length > 6) Array.Resize(ref all, 6);

        string language = all[0];
        if (language.Length < 2 || language.Length > 3 || !IsLowerAlpha(language))
        {
            return false;
        }

        string script = "";
        string region = "";
        string variant = "";
        int index = 1;

        if (index < all.Length && all[index].Length == 4 && IsAlpha(all[index]))
        {
            script = all[index];
            ++index;
        }

        if (index < all.Length &&
            ((all[index].Length == 2 && IsAlpha(all[index])) ||
             (all[index].Length == 3 && IsAlnum(all[index]))))
        {
            region = all[index];
            ++index;
        }

        if (index < all.Length)
        {
            if (all[index].Length > 8 || !IsAlnum(all[index])) return false;
            variant = all[index];
        }

        locale = new Locale(language, script, region, variant);
        return true;
    }

    /// <summary>True when <paramref name="tag"/> is a well-formed CLDR language tag.</summary>
    public static bool IsValidLocale(string? tag) => TryParseLocale(tag, out _);

    /// <summary>Default locale for a language (CLDR fallback), or "root".</summary>
    public static string DefaultLocale(string? language) => language switch
    {
        "en" => "en_US",
        "de" => "de_DE",
        "zh" => "zh_Hans_CN",
        _ => "root"
    };

    // ---- Collation -------------------------------------------------------

    /// <summary>True when <paramref name="locale"/> needs the shifted alternate (e.g. zh, sv).</summary>
    public static bool NeedsShifted(string? locale)
    {
        if (locale is null) return false;
        return locale is "zh" or "sv" ||
               locale.Contains("_zh", StringComparison.Ordinal) ||
               locale.Contains("_sv", StringComparison.Ordinal);
    }

    /// <summary>Default collation settings for <paramref name="locale"/>.</summary>
    public static CollationSettings GetCollation(string? locale) =>
        new(3, NeedsShifted(locale) ? 1 : 0);

    // ---- Dates -----------------------------------------------------------

    /// <summary>Calendar name for <paramref name="locale"/> (defaults to gregory).</summary>
    public static string Calendar(string? locale)
    {
        if (locale is not null &&
            (locale.StartsWith("th", StringComparison.Ordinal) ||
             locale.StartsWith("TH", StringComparison.Ordinal)))
        {
            return "buddhist";
        }
        if (locale is not null &&
            (locale.StartsWith("zh", StringComparison.Ordinal) ||
             locale.StartsWith("ZH", StringComparison.Ordinal)))
        {
            return "chinese";
        }
        return "gregory";
    }

    /// <summary>First day of week (0=Sunday..6=Saturday) for <paramref name="locale"/>.</summary>
    public static int FirstWeekday(string? locale) =>
        string.Equals(locale, "US", StringComparison.Ordinal) ? 0 : 1;

    /// <summary>True when <paramref name="pattern"/> is a CLDR date pattern.</summary>
    public static bool IsDatePattern(string? pattern)
    {
        if (string.IsNullOrEmpty(pattern)) return false;
        foreach (char c in pattern)
        {
            bool ok = c is 'y' or 'M' or 'd' or 'H' or 'm' or 's' or 'E' or 'a' or
                'B' or 'Z' or 'z' or 'G' or 'u' or 'L' or 'c' or 'e' or 'F' or
                'g' or 'j' or 'J' or 'k' or 'K' or 'n' or 'O' or 'Q' or 'r' or
                'S' or 't' or 'U' or 'V' or 'v' or 'w' or 'W' or 'x' or 'X' or
                'Y' or '/' or '-' or ':' or ' ' or '\'' or '.' or ',';
            if (!ok) return false;
        }
        return true;
    }

    // ---- General ---------------------------------------------------------

    private static readonly string[] s_weekdays =
    [
        "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
    ];

    /// <summary>True when <paramref name="text"/> is a wide English weekday name.</summary>
    public static bool IsWeekday(string? locale, string? text)
    {
        _ = locale;
        if (text is null) return false;
        return Array.IndexOf(s_weekdays, text) >= 0;
    }

    /// <summary>Month count (always 12 for Gregorian CLDR).</summary>
    public static int MonthCount(string? locale)
    {
        _ = locale;
        return 12;
    }

    /// <summary>Day count in a month (Feb = 29 in a leap year).</summary>
    public static int DayCount(string? locale, int month, bool leap)
    {
        _ = locale;
        int[] days = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];
        if (month < 1 || month > 12) return 0;
        if (month == 2 && leap) return 29;
        return days[month - 1];
    }

    // ---- Keyboards -------------------------------------------------------

    /// <summary>True when <paramref name="id"/> is a CLDR keyboard id.</summary>
    public static bool IsKeyboardId(string? id)
    {
        if (string.IsNullOrEmpty(id)) return false;
        foreach (char c in id)
        {
            if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                  (c >= '0' && c <= '9') || c == '_' || c == '-'))
            {
                return false;
            }
        }
        return true;
    }

    /// <summary>True when the layout is transparent (QWERTY-family default).</summary>
    public static bool IsTransparent(string? id)
    {
        if (id is null) return true;
        return !id.Contains("dvorak", StringComparison.Ordinal) &&
               !id.Contains("colemak", StringComparison.Ordinal);
    }

    // ---- MessageFormat 2.0 ----------------------------------------------

    /// <summary>Plural category of <paramref name="n"/> for <paramref name="locale"/>.</summary>
    public static int GetPlural(string? locale, double n)
    {
        _ = locale;
        if (n == 1.0) return Plural.One;
        if (n == 0.0) return Plural.Zero;
        return Plural.Other;
    }

    /// <summary>True when <paramref name="pattern"/> has balanced braces and no stray '}'.</summary>
    public static bool IsMessagePatternValid(string? pattern)
    {
        if (pattern is null) return false;
        int depth = 0;
        foreach (char c in pattern)
        {
            if (c == '{')
            {
                ++depth;
            }
            else if (c == '}')
            {
                --depth;
                if (depth < 0) return false;
            }
        }
        return depth == 0;
    }

    // ---- Numbers ---------------------------------------------------------

    /// <summary>Symbols for a region (ISO 3166); root defaults when absent.</summary>
    public static NumberSymbols GetNumberSymbols(string? region)
    {
        string group;
        string dec;
        if (region == "DE")
        {
            group = ".";
            dec = ",";
        }
        else if (region == "FR")
        {
            group = " ";
            dec = ",";
        }
        else
        {
            group = ",";
            dec = ".";
        }
        return new NumberSymbols(group, dec, "-", "%");
    }

    /// <summary>True when <paramref name="text"/> uses only ASCII digits and at most one '.'.</summary>
    public static bool IsDecimal(string? text)
    {
        if (string.IsNullOrEmpty(text)) return false;
        int dots = 0;
        foreach (char c in text)
        {
            if (c >= '0' && c <= '9') continue;
            if (c == '.' && dots == 0)
            {
                dots = 1;
                continue;
            }
            return false;
        }
        return true;
    }

    // ---- Person names ----------------------------------------------------

    /// <summary>Name order for <paramref name="locale"/> (family-first for CJK/Korean/Hungarian).</summary>
    public static int GetNameOrder(string? locale)
    {
        if (locale is null) return NameOrder.GivenFirst;
        if (locale.Length >= 2 &&
            (locale.StartsWith("zh", StringComparison.Ordinal) ||
             locale.StartsWith("ko", StringComparison.Ordinal) ||
             locale.StartsWith("ja", StringComparison.Ordinal) ||
             locale.StartsWith("hu", StringComparison.Ordinal)))
        {
            return NameOrder.FamilyFirst;
        }
        return NameOrder.GivenFirst;
    }

    /// <summary>True when <paramref name="name"/> looks like a personal name.</summary>
    public static bool IsPersonName(string? locale, string? name)
    {
        _ = locale;
        if (string.IsNullOrEmpty(name)) return false;
        foreach (char c in name)
        {
            if (c == ' ' || c == '-' || c == '\'' || c == '.' || c >= 0x80) continue;
            if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))) return false;
        }
        return true;
    }

    // ---- Supplemental ----------------------------------------------------

    /// <summary>True when <paramref name="zone"/> is a CLDR zone ID (IANA-style).</summary>
    public static bool IsZoneId(string? zone)
    {
        if (string.IsNullOrEmpty(zone)) return false;
        return zone.Contains('/', StringComparison.Ordinal) ||
               string.Equals(zone, "UTC", StringComparison.Ordinal);
    }

    /// <summary>Day-period name ("AM"/"PM") for an hour (0..23), or "at night".</summary>
    public static string DayPeriod(int hour)
    {
        if (hour < 0 || hour > 23) return "at night";
        return hour < 12 ? "AM" : "PM";
    }

    /// <summary>Number of day periods.</summary>
    public const int DayPeriodCount = 4;
}
