path = r"C:\Users\KARIMABENDA\Documents\unicodoid\clike\csharp\Unicodoid\Properties.cs"
src = open(path, encoding="utf-8").read()

src = src.replace("""/// <summary>
/// Character properties for Unicode scalar values and for native values of an
/// encoding system.  The scalar tables are a compact, hand-maintained subset of
/// the UCD; they support the SDK's opt-in algorithms but are not a generated
/// property database.
/// </summary>""",
"""/// <summary>
/// Character properties for Unicode scalar values and for native values of an
/// encoding system.  The scalar queries are backed by the generated Unicode
/// 18.0.0 tables; every query is exact for the declared Unicode version.
/// </summary>""")

start = src.index("    public static UnicodeCategory GeneralCategory(uint codePoint)")
end = src.index("    // ---- System-aware queries")

new_mid = """    public static UnicodeCategory GeneralCategory(uint codePoint)
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

"""
src = src[:start] + new_mid + src[end:]
open(path, "w", encoding="utf-8").write(src)
print("done")
