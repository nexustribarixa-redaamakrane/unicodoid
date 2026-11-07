using System;

namespace Unicodoid;

/// <summary>
/// General category groups from UAX #44.  The numeric values match the C SDK.
/// </summary>
public enum UnicodeCategory
{
    Other = 0,
    UppercaseLetter = 1,
    LowercaseLetter = 2,
    TitlecaseLetter = 3,
    ModifierLetter = 4,
    OtherLetter = 5,
    DecimalNumber = 6,
    LetterNumber = 7,
    OtherNumber = 8,
    SpaceSeparator = 9,
    LineSeparator = 10,
    ParagraphSeparator = 11,
    Control = 12,
    Format = 13,
    Surrogate = 14,
    PrivateUse = 15,
    Unassigned = 16,
    Punctuation = 17,
    Symbol = 18,
    Mark = 19
}

/// <summary>East Asian Width classes from UAX #11.</summary>
public enum EastAsianWidth
{
    Neutral = 0,
    Ambiguous = 1,
    Halfwidth = 2,
    Fullwidth = 3,
    Narrow = 4,
    Wide = 5
}

/// <summary>
/// Character properties for Unicode scalar values and for native values of an
/// encoding system.  The scalar queries are backed by the generated Unicode
/// 18.0.0 tables; every query is exact for the declared Unicode version.
/// </summary>
public static class UnicodeProperties
{
    // ---- Scalar utilities -------------------------------------------------

    public static UnicodeCategory GeneralCategory(uint codePoint)
    {
        return Ucd.GeneralCategory(codePoint) switch
        {
            Ucd.Gc.Lu => UnicodeCategory.UppercaseLetter,
            Ucd.Gc.Ll => UnicodeCategory.LowercaseLetter,
            Ucd.Gc.Lt => UnicodeCategory.TitlecaseLetter,
            Ucd.Gc.Lm => UnicodeCategory.ModifierLetter,
            Ucd.Gc.Lo => UnicodeCategory.OtherLetter,
            Ucd.Gc.Nd => UnicodeCategory.DecimalNumber,
            Ucd.Gc.Nl => UnicodeCategory.LetterNumber,
            Ucd.Gc.No => UnicodeCategory.OtherNumber,
            Ucd.Gc.Zs => UnicodeCategory.SpaceSeparator,
            Ucd.Gc.Zl => UnicodeCategory.LineSeparator,
            Ucd.Gc.Zp => UnicodeCategory.ParagraphSeparator,
            Ucd.Gc.Cc => UnicodeCategory.Control,
            Ucd.Gc.Cf => UnicodeCategory.Format,
            Ucd.Gc.Cs => UnicodeCategory.Surrogate,
            Ucd.Gc.Co => UnicodeCategory.PrivateUse,
            Ucd.Gc.Cn => UnicodeCategory.Unassigned,
            Ucd.Gc.Pc or Ucd.Gc.Pd or Ucd.Gc.Pe or Ucd.Gc.Pf or
            Ucd.Gc.Pi or Ucd.Gc.Po or Ucd.Gc.Ps => UnicodeCategory.Punctuation,
            Ucd.Gc.Sc or Ucd.Gc.Sk or Ucd.Gc.Sm or Ucd.Gc.So => UnicodeCategory.Symbol,
            Ucd.Gc.Mc or Ucd.Gc.Me or Ucd.Gc.Mn => UnicodeCategory.Mark,
            _ => UnicodeCategory.Other
        };
    }

    public static EastAsianWidth GetEastAsianWidth(uint codePoint)
    {
        return Ucd.EastAsianWidth(codePoint) switch
        {
            Ucd.Eaw.N => EastAsianWidth.Neutral,
            Ucd.Eaw.A => EastAsianWidth.Ambiguous,
            Ucd.Eaw.H => EastAsianWidth.Halfwidth,
            Ucd.Eaw.F => EastAsianWidth.Fullwidth,
            Ucd.Eaw.Na => EastAsianWidth.Narrow,
            Ucd.Eaw.W => EastAsianWidth.Wide,
            _ => EastAsianWidth.Neutral
        };
    }

    public static bool IsAlphabetic(uint codePoint) => Ucd.Alphabetic(codePoint);

    public static bool IsNumeric(uint codePoint) =>
        GeneralCategory(codePoint) is
            UnicodeCategory.DecimalNumber or
            UnicodeCategory.LetterNumber or
            UnicodeCategory.OtherNumber;

    public static bool IsWhitespace(uint codePoint) => Ucd.WhiteSpace(codePoint);

    public static bool IsControl(uint codePoint) =>
        GeneralCategory(codePoint) == UnicodeCategory.Control;

    public static bool IsPunctuation(uint codePoint) =>
        GeneralCategory(codePoint) == UnicodeCategory.Punctuation;

    public static bool IsMark(uint codePoint) =>
        GeneralCategory(codePoint) == UnicodeCategory.Mark;

    public static int CombiningClass(uint codePoint) => Ucd.CombiningClass(codePoint);

    public static uint ToUppercase(uint codePoint) => Ucd.SimpleUppercase(codePoint);

    public static uint ToLowercase(uint codePoint) => Ucd.SimpleLowercase(codePoint);

    // ---- System-aware queries (require a Unicode profile) -----------------

    private static UnicodoidStatus ToScalar(
        IEncodingSystem system,
        CharacterValue ch,
        out uint scalar)
    {
        scalar = 0;
        if (system is null) return UnicodoidStatus.InvalidParam;
        if (!system.HasUnicodeProfile) return UnicodoidStatus.Unsupported;
        return system.UnicodeProfile!.TryToUnicode(ch, out scalar)
            ? UnicodoidStatus.Ok
            : UnicodoidStatus.UnmappableChar;
    }

    private static UnicodoidStatus FromScalar(
        IEncodingSystem system,
        uint scalar,
        out CharacterValue ch)
    {
        ch = CharacterValue.Invalid;
        if (system is null) return UnicodoidStatus.InvalidParam;
        if (!system.HasUnicodeProfile) return UnicodoidStatus.Unsupported;
        return system.UnicodeProfile!.TryFromUnicode(scalar, out ch)
            ? UnicodoidStatus.Ok
            : UnicodoidStatus.UnmappableChar;
    }

    public static UnicodoidStatus TryGetGeneralCategory(
        IEncodingSystem system,
        CharacterValue ch,
        out UnicodeCategory category)
    {
        category = UnicodeCategory.Other;
        UnicodoidStatus status = ToScalar(system, ch, out uint scalar);
        if (status != UnicodoidStatus.Ok) return status;
        category = GeneralCategory(scalar);
        return UnicodoidStatus.Ok;
    }

    public static UnicodoidStatus TryGetEastAsianWidth(
        IEncodingSystem system,
        CharacterValue ch,
        out EastAsianWidth width)
    {
        width = EastAsianWidth.Neutral;
        UnicodoidStatus status = ToScalar(system, ch, out uint scalar);
        if (status != UnicodoidStatus.Ok) return status;
        width = GetEastAsianWidth(scalar);
        return UnicodoidStatus.Ok;
    }

    public static UnicodoidStatus TryIsAlphabetic(
        IEncodingSystem system,
        CharacterValue ch,
        out bool value)
    {
        value = false;
        UnicodoidStatus status = ToScalar(system, ch, out uint scalar);
        if (status != UnicodoidStatus.Ok) return status;
        value = IsAlphabetic(scalar);
        return UnicodoidStatus.Ok;
    }

    public static UnicodoidStatus TryIsNumeric(
        IEncodingSystem system,
        CharacterValue ch,
        out bool value)
    {
        value = false;
        UnicodoidStatus status = ToScalar(system, ch, out uint scalar);
        if (status != UnicodoidStatus.Ok) return status;
        value = IsNumeric(scalar);
        return UnicodoidStatus.Ok;
    }

    public static UnicodoidStatus TryIsWhitespace(
        IEncodingSystem system,
        CharacterValue ch,
        out bool value)
    {
        value = false;
        UnicodoidStatus status = ToScalar(system, ch, out uint scalar);
        if (status != UnicodoidStatus.Ok) return status;
        value = IsWhitespace(scalar);
        return UnicodoidStatus.Ok;
    }

    public static UnicodoidStatus TryToUppercase(
        IEncodingSystem system,
        CharacterValue ch,
        out CharacterValue result)
    {
        result = CharacterValue.Invalid;
        UnicodoidStatus status = ToScalar(system, ch, out uint scalar);
        if (status != UnicodoidStatus.Ok) return status;
        return FromScalar(system, ToUppercase(scalar), out result);
    }

    public static UnicodoidStatus TryToLowercase(
        IEncodingSystem system,
        CharacterValue ch,
        out CharacterValue result)
    {
        result = CharacterValue.Invalid;
        UnicodoidStatus status = ToScalar(system, ch, out uint scalar);
        if (status != UnicodoidStatus.Ok) return status;
        return FromScalar(system, ToLowercase(scalar), out result);
    }

    // ---- Throwing conveniences -------------------------------------------

    public static UnicodeCategory GetGeneralCategory(IEncodingSystem system, CharacterValue ch)
    {
        UnicodoidStatus status = TryGetGeneralCategory(system, ch, out UnicodeCategory category);
        return status == UnicodoidStatus.Ok
            ? category
            : throw new UnicodoidException(status);
    }

    public static EastAsianWidth GetEastAsianWidth(IEncodingSystem system, CharacterValue ch)
    {
        UnicodoidStatus status = TryGetEastAsianWidth(system, ch, out EastAsianWidth width);
        return status == UnicodoidStatus.Ok
            ? width
            : throw new UnicodoidException(status);
    }

    public static CharacterValue ToUppercase(IEncodingSystem system, CharacterValue ch)
    {
        UnicodoidStatus status = TryToUppercase(system, ch, out CharacterValue result);
        return status == UnicodoidStatus.Ok
            ? result
            : throw new UnicodoidException(status);
    }

    public static CharacterValue ToLowercase(IEncodingSystem system, CharacterValue ch)
    {
        UnicodoidStatus status = TryToLowercase(system, ch, out CharacterValue result);
        return status == UnicodoidStatus.Ok
            ? result
            : throw new UnicodoidException(status);
    }
}
