using System;
using System.Collections.Generic;

namespace Unicodoid;

/// <summary>Official Unicode publication categories.</summary>
public enum UnicodeDocCategory
{
    Uax = 1,
    Uts = 2,
    Utr = 3,
    Utn = 4
}

/// <summary>Bitmask of Unicode Standard Annexes understood by this SDK.</summary>
[Flags]
public enum UaxFlags : uint
{
    None = 0,
    Bidi9 = 1 << 0,
    EastAsianWidth11 = 1 << 1,
    LineBreak14 = 1 << 2,
    Normalization15 = 1 << 3,
    Script24 = 1 << 4,
    Segmentation29 = 1 << 5,
    IdSyntax31 = 1 << 6,
    Ucd44 = 1 << 7,
    NamedSequences34 = 1 << 8,
    Unihan38 = 1 << 9,
    References41 = 1 << 10,
    UcdXml42 = 1 << 11,
    USource45 = 1 << 12,
    VerticalText50 = 1 << 13,
    ArabicMark53 = 1 << 14,
    Unikemet57 = 1 << 15,
    EastAsianData60 = 1 << 16
}

/// <summary>Bitmask of Unicode Technical Standards understood by this SDK.</summary>
[Flags]
public enum UtsFlags : uint
{
    None = 0,
    Uca10 = 1 << 0,
    Regex18 = 1 << 1,
    Ldml35 = 1 << 2,
    Security39 = 1 << 3,
    Idna46 = 1 << 4,
    Emoji51 = 1 << 5,
    Ivd37 = 1 << 6,
    SourceCode55 = 1 << 7,
    Links58 = 1 << 8,
    SetNotation61 = 1 << 9
}

/// <summary>Bitmask of Unicode Technical Reports referenced by this SDK.</summary>
[Flags]
public enum UtrFlags : uint
{
    None = 0,
    EncodingModel17 = 1 << 0,
    PropertyModel23 = 1 << 1,
    Math25 = 1 << 2,
    Conformance33 = 1 << 3,
    Mongolian54 = 1 << 4,
    Cuneiform56 = 1 << 5,
    EastAsianSpacing59 = 1 << 6
}

/// <summary>Metadata for one Unicode specification.</summary>
public sealed record UnicodeSpecRef(
    UnicodeDocCategory Category,
    int Number,
    string Title,
    string Url,
    string Status)
{
    public override string ToString() => $"{Category} #{Number}: {Title}";
}

/// <summary>
/// Optional Unicode compatibility profile attached to an encoding system.
/// A profile owns the mapping between native character values and Unicode
/// scalar values.  It is never required: systems may declare
/// <see cref="CompatibilityLevel.None"/> and expose no profile at all.
/// </summary>
public sealed class UnicodeProfile
{
    private readonly Func<CharacterValue, uint?> _toUnicode;
    private readonly Func<uint, CharacterValue?> _fromUnicode;

    public Version UnicodeVersion { get; }
    public UaxFlags SupportedUax { get; }
    public UtsFlags SupportedUts { get; }
    public UtrFlags SupportedUtr { get; }

    public UnicodeProfile(
        Version version,
        UaxFlags uax,
        UtsFlags uts,
        UtrFlags utr,
        Func<CharacterValue, uint?> toUnicode,
        Func<uint, CharacterValue?> fromUnicode)
    {
        UnicodeVersion = version ?? throw new ArgumentNullException(nameof(version));
        SupportedUax = uax;
        SupportedUts = uts;
        SupportedUtr = utr;
        _toUnicode = toUnicode ?? throw new ArgumentNullException(nameof(toUnicode));
        _fromUnicode = fromUnicode ?? throw new ArgumentNullException(nameof(fromUnicode));
    }

    /// <summary>Attempt to map a native character value to a Unicode scalar value.</summary>
    public bool TryToUnicode(CharacterValue native, out uint scalar)
    {
        uint? mapped = _toUnicode(native);
        if (mapped.HasValue && UnicodeScalars.IsValid(mapped.Value))
        {
            scalar = mapped.Value;
            return true;
        }
        scalar = 0;
        return false;
    }

    /// <summary>Attempt to map a Unicode scalar value to a native character value.</summary>
    public bool TryFromUnicode(uint scalar, out CharacterValue native)
    {
        if (!UnicodeScalars.IsValid(scalar))
        {
            native = CharacterValue.Invalid;
            return false;
        }
        CharacterValue? mapped = _fromUnicode(scalar);
        if (mapped.HasValue && !mapped.Value.IsInvalid)
        {
            native = mapped.Value;
            return true;
        }
        native = CharacterValue.Invalid;
        return false;
    }

    /// <summary>Map to a Unicode scalar value, throwing when the value is unmappable.</summary>
    public uint ToUnicode(CharacterValue native) =>
        TryToUnicode(native, out uint scalar)
            ? scalar
            : throw new UnicodoidException(UnicodoidStatus.UnmappableChar);

    /// <summary>Map from a Unicode scalar value, throwing when the value is unmappable.</summary>
    public CharacterValue FromUnicode(uint scalar) =>
        TryFromUnicode(scalar, out CharacterValue native)
            ? native
            : throw new UnicodoidException(UnicodoidStatus.UnmappableChar);
}

/// <summary>
/// Generated catalog of the current Unicode publications relevant to this SDK,
/// plus the one Unicode Technical Note it references.  Titles, URLs and
/// statuses are taken verbatim from docs/unicode18/inventory.json (generated
/// from the official Unicode reports index and report pages) and the official
/// Unicode Technical Notes index.  Stabilized, superseded, withdrawn,
/// suspended or otherwise non-current reports are intentionally not listed;
/// their dispositions are documented under docs/unicode18/.
/// </summary>
public static class UnicodeCatalog
{
    private static readonly UnicodeSpecRef[] s_specs =
    {
        // UAX: Unicode Standard Annexes (current in Unicode 18.0.0)
        new(UnicodeDocCategory.Uax, 9, "Unicode Bidirectional Algorithm", "https://www.unicode.org/reports/tr9/tr9-52.html", "Current"),
        new(UnicodeDocCategory.Uax, 11, "East Asian Width", "https://www.unicode.org/reports/tr11/tr11-46.html", "Current"),
        new(UnicodeDocCategory.Uax, 14, "Unicode Line Breaking Algorithm", "https://www.unicode.org/reports/tr14/tr14-57.html", "Current"),
        new(UnicodeDocCategory.Uax, 15, "Unicode Normalization Forms", "https://www.unicode.org/reports/tr15/tr15-58.html", "Current"),
        new(UnicodeDocCategory.Uax, 24, "Unicode Script Property", "https://www.unicode.org/reports/tr24/tr24-41.html", "Current"),
        new(UnicodeDocCategory.Uax, 29, "Unicode Text Segmentation", "https://www.unicode.org/reports/tr29/tr29-49.html", "Current"),
        new(UnicodeDocCategory.Uax, 31, "Unicode Identifiers and Syntax", "https://www.unicode.org/reports/tr31/tr31-45.html", "Current"),
        new(UnicodeDocCategory.Uax, 34, "Unicode Named Character Sequences", "https://www.unicode.org/reports/tr34/tr34-33.html", "Current"),
        new(UnicodeDocCategory.Uax, 38, "Unicode Han Database (Unihan)", "https://www.unicode.org/reports/tr38/tr38-41.html", "Current"),
        new(UnicodeDocCategory.Uax, 41, "Common References for UAXs", "https://www.unicode.org/reports/tr41/tr41-38.html", "Current"),
        new(UnicodeDocCategory.Uax, 42, "Unicode Character Database in XML", "https://www.unicode.org/reports/tr42/tr42-40.html", "Current"),
        new(UnicodeDocCategory.Uax, 44, "Unicode Character Database", "https://www.unicode.org/reports/tr44/tr44-38.html", "Current"),
        new(UnicodeDocCategory.Uax, 45, "U-source Ideographs", "https://www.unicode.org/reports/tr45/tr45-34.html", "Current"),
        new(UnicodeDocCategory.Uax, 50, "Unicode Vertical Text Layout", "https://www.unicode.org/reports/tr50/tr50-35.html", "Current"),
        new(UnicodeDocCategory.Uax, 53, "Unicode Arabic Mark Rendering", "https://www.unicode.org/reports/tr53/tr53-13.html", "Current"),
        new(UnicodeDocCategory.Uax, 57, "Unicode Egyptian Hieroglyph Database", "https://www.unicode.org/reports/tr57/tr57-7.html", "Current"),
        new(UnicodeDocCategory.Uax, 60, "Data for East Asian Scripts", "https://www.unicode.org/reports/tr60/tr60-3.html", "Current"),

        // UTS: Unicode Technical Standards (current in Unicode 18.0.0)
        new(UnicodeDocCategory.Uts, 10, "Unicode Collation Algorithm", "https://www.unicode.org/reports/tr10/tr10-55.html", "Current"),
        new(UnicodeDocCategory.Uts, 18, "Unicode Regular Expressions", "https://www.unicode.org/reports/tr18/tr18-25.html", "Current"),
        new(UnicodeDocCategory.Uts, 35, "Unicode Common Locale Data & Rules (CLDR) Part 1: Core", "https://www.unicode.org/reports/tr35/", "Current"),
        new(UnicodeDocCategory.Uts, 37, "Unicode Ideographic Variation Database", "https://www.unicode.org/reports/tr37/tr37-16.html", "Current"),
        new(UnicodeDocCategory.Uts, 39, "Unicode Security Mechanisms", "https://www.unicode.org/reports/tr39/tr39-34.html", "Current"),
        new(UnicodeDocCategory.Uts, 46, "Unicode IDNA Compatibility Processing", "https://www.unicode.org/reports/tr46/tr46-36.html", "Current"),
        new(UnicodeDocCategory.Uts, 51, "Unicode Emoji", "https://www.unicode.org/reports/tr51/tr51-31.html", "Current"),
        new(UnicodeDocCategory.Uts, 55, "Unicode Source Code Handling", "https://www.unicode.org/reports/tr55/tr55-5.html", "Current"),
        new(UnicodeDocCategory.Uts, 58, "Unicode Link Detection and Formatting: URLs and Email Addresses", "https://www.unicode.org/reports/tr58/tr58-3.html", "Current"),
        new(UnicodeDocCategory.Uts, 61, "Unicode Set Notation", "https://www.unicode.org/reports/tr61/tr61-2.html", "Draft"),

        // UTR: Unicode Technical Reports (current in Unicode 18.0.0)
        new(UnicodeDocCategory.Utr, 17, "Unicode Character Encoding Model", "https://www.unicode.org/reports/tr17/tr17-9.html", "Current"),
        new(UnicodeDocCategory.Utr, 23, "The Unicode Character Property Model", "https://www.unicode.org/reports/tr23/tr23-15.html", "Current"),
        new(UnicodeDocCategory.Utr, 25, "Unicode Support for Mathematics", "https://www.unicode.org/reports/tr25/tr25-16.html", "Proposed Update"),
        new(UnicodeDocCategory.Utr, 33, "Conformance Model", "https://www.unicode.org/reports/tr33/tr33-7.html", "Current"),
        new(UnicodeDocCategory.Utr, 54, "Unicode Mongolian 12.1 Baseline", "https://www.unicode.org/reports/tr54/tr54-1.html", "Current"),
        new(UnicodeDocCategory.Utr, 56, "Unicode Cuneiform Sign Lists", "https://www.unicode.org/reports/tr56/tr56-5.html", "Current"),
        new(UnicodeDocCategory.Utr, 59, "East Asian Spacing", "https://www.unicode.org/reports/tr59/tr59-1.html", "Current"),

        // UTN: the one Unicode Technical Note referenced by this SDK (docs only).
        // UTNs are independent author works, not specifications; see docs/unicode18/utn/.
        new(UnicodeDocCategory.Utn, 1, "Issues in Indic Language Collation", "https://www.unicode.org/notes/tn1/", "Technical Note")
    };

    public static int Count => s_specs.Length;

    public static IReadOnlyList<UnicodeSpecRef> All => s_specs;

    public static bool TryGet(UnicodeDocCategory category, int number, out UnicodeSpecRef spec)
    {
        foreach (UnicodeSpecRef candidate in s_specs)
        {
            if (candidate.Category == category && candidate.Number == number)
            {
                spec = candidate;
                return true;
            }
        }
        spec = null!;
        return false;
    }

    public static UnicodeSpecRef Get(UnicodeDocCategory category, int number) =>
        TryGet(category, number, out UnicodeSpecRef spec)
            ? spec
            : throw new UnicodoidException(UnicodoidStatus.NotFound);

    public static bool TryAt(int index, out UnicodeSpecRef spec)
    {
        if (index < 0 || index >= s_specs.Length)
        {
            spec = null!;
            return false;
        }
        spec = s_specs[index];
        return true;
    }

    public static UnicodeSpecRef At(int index) =>
        TryAt(index, out UnicodeSpecRef spec)
            ? spec
            : throw new UnicodoidException(UnicodoidStatus.NotFound);
}
