using System;
using Unicodoid.Generated;

namespace Unicodoid;

/// <summary>
/// Unicode Character Database accessors backed by the generated Unicode 18.0.0
/// tables.  The generated tables are shared with the C and Rust
/// implementations; this class is the idiomatic C# accessor layer.
/// </summary>
public static class Ucd
{
    /// <summary>The Unicode version described by the generated tables.</summary>
    public const string Version = UcdData.Version;

    /// <summary>General_Category values (UAX #44), in generated-table order.</summary>
    public static class Gc
    {
        public const byte Cn = 0, Cc = 1, Cf = 2, Co = 3, Cs = 4;
        public const byte Ll = 5, Lm = 6, Lo = 7, Lt = 8, Lu = 9;
        public const byte Mc = 10, Me = 11, Mn = 12;
        public const byte Nd = 13, Nl = 14, No = 15;
        public const byte Pc = 16, Pd = 17, Pe = 18, Pf = 19, Pi = 20, Po = 21, Ps = 22;
        public const byte Sc = 23, Sk = 24, Sm = 25, So = 26;
        public const byte Zl = 27, Zp = 28, Zs = 29;
    }

    /// <summary>East_Asian_Width values (UAX #11), in generated-table order.</summary>
    public static class Eaw
    {
        public const byte N = 0, A = 1, H = 2, F = 3, Na = 4, W = 5;
    }

    /// <summary>Grapheme_Cluster_Break values (UAX #29), in generated-table order.</summary>
    public static class Gcb
    {
        public const byte Other = 0, Cr = 1, Lf = 2, Control = 3, Extend = 4, Zwj = 5;
        public const byte Ri = 6, Prepend = 7, SpacingMark = 8;
        public const byte L = 9, V = 10, T = 11, Lv = 12, Lvt = 13;
    }

    /// <summary>Word_Break values (UAX #29), in generated-table order.</summary>
    public static class Wb
    {
        public const byte Other = 0, Cr = 1, Lf = 2, Newline = 3, Extend = 4, Zwj = 5;
        public const byte Ri = 6, Format = 7, Katakana = 8, HebrewLetter = 9, ALetter = 10;
        public const byte SingleQuote = 11, DoubleQuote = 12, MidNumLet = 13, MidLetter = 14;
        public const byte MidNum = 15, Numeric = 16, ExtendNumLet = 17, WSegSpace = 18;
    }

    /// <summary>Sentence_Break values (UAX #29), in generated-table order.</summary>
    public static class Sb
    {
        public const byte Other = 0, Cr = 1, Lf = 2, Extend = 3, Sep = 4, Format = 5;
        public const byte Sp = 6, Lower = 7, Upper = 8, OLetter = 9, Numeric = 10;
        public const byte ATerm = 11, SContinue = 12, STerm = 13, Close = 14;
    }

    /// <summary>Indic_Conjunct_Break values (UAX #29), in generated-table order.</summary>
    public static class Incb
    {
        public const byte None = 0, Linker = 1, Consonant = 2, Extend = 3;
    }

    /// <summary>Line_Break values (UAX #14), in generated-table order.</summary>
    public static class Lb
    {
        public const byte Xx = 0, Ai = 1, Ak = 2, Al = 3, Ap = 4, As = 5, B2 = 6, Ba = 7;
        public const byte Bb = 8, Bk = 9, Cb = 10, Cj = 11, Cl = 12, Cm = 13, Cp = 14;
        public const byte Cr = 15, Eb = 16, Em = 17, Ex = 18, Gl = 19, H2 = 20, H3 = 21;
        public const byte Hh = 22, Hl = 23, Hy = 24, Id = 25, In = 26, Is = 27, Jl = 28;
        public const byte Jt = 29, Jv = 30, Lf = 31, Nl = 32, Ns = 33, Nu = 34, Op = 35;
        public const byte Po = 36, Pr = 37, Qu = 38, Ri = 39, Sa = 40, Sg = 41, Sp = 42;
        public const byte Sy = 43, Vf = 44, Vi = 45, Wj = 46, Zw = 47, Zwj = 48;
    }

    private static ushort Lookup(UcdData.Range[] table, uint cp)
    {
        int lo = 0;
        int hi = table.Length;
        while (lo < hi)
        {
            int mid = lo + (hi - lo) / 2;
            UcdData.Range range = table[mid];
            if (cp < range.Start)
            {
                hi = mid;
            }
            else if (cp > range.End)
            {
                lo = mid + 1;
            }
            else
            {
                return range.Value;
            }
        }
        return 0;
    }

    /// <summary>Canonical combining class (UAX #44); zero means a starter.</summary>
    public static int CombiningClass(uint cp) => Lookup(UcdData.Ccc, cp);

    /// <summary>UTS #58 Link_Term values: 1 Include, 2 Hard, 3 Soft, 4 Close, 5 Open.  Unlisted code points have value 2 (Hard).</summary>
    public static int LinkTerm(uint cp) => Lookup(UcdData.Link_term_range, cp);

    /// <summary>UTS #58 Link_Bracket: maps a Close bracket code point to its opening counterpart.  Returns false when there is no mapping.</summary>
    public static bool TryLinkBracket(uint cp, out uint openCp)
    {
        int lo = 0;
        int hi = UcdData.LinkBracket.Length;
        while (lo < hi)
        {
            int mid = lo + (hi - lo) / 2;
            UcdData.Pair pair = UcdData.LinkBracket[mid];
            if (cp < pair.Cp)
            {
                hi = mid;
            }
            else if (cp > pair.Cp)
            {
                lo = mid + 1;
            }
            else
            {
                openCp = pair.Mapped;
                return true;
            }
        }
        openCp = 0;
        return false;
    }

    /// <summary>UTS #58 Link_Email binary property.</summary>
    public static bool LinkEmail(uint cp) => Lookup(UcdData.Link_email_range, cp) != 0;

    /// <summary>UTR #59 East_Asian_Spacing values: 1 W, 2 N, 3 O, 4 C.  Unlisted code points have value 3 (O).</summary>
    public static int EastAsianSpacing(uint cp) => Lookup(UcdData.East_asian_spacing_range, cp);

    /// <summary>UAX #34 named sequences.</summary>
    public static int NamedSeqCount => UcdData.NamedSeqNames.Length;

    public static string? NamedSeqName(int index) =>
        index >= 0 && index < UcdData.NamedSeqNames.Length ? UcdData.NamedSeqNames[index] : null;

    public static uint[]? NamedSeqCps(int index)
    {
        if (index < 0 || index >= UcdData.NamedSeqIndex.Length) return null;
        (uint offset, ushort length) = UcdData.NamedSeqIndex[index];
        var cps = new uint[length];
        for (int i = 0; i < length; ++i) cps[i] = UcdData.Pool[offset + i];
        return cps;
    }

    public static int NamedSeqFind(string name)
    {
        for (int i = 0; i < UcdData.NamedSeqNames.Length; ++i)
        {
            if (string.Equals(UcdData.NamedSeqNames[i], name, StringComparison.Ordinal)) return i;
        }
        return -1;
    }

    /// <summary>UTS #51 RGI emoji sequences (emoji-sequences.txt and emoji-zwj-sequences.txt).</summary>
    public static int EmojiSeqCount => UcdData.EmojiSeqTypes.Length;

    public static string? EmojiSeqType(int index) =>
        index >= 0 && index < UcdData.EmojiSeqTypes.Length ? UcdData.EmojiSeqTypes[index] : null;

    public static uint[]? EmojiSeqCps(int index)
    {
        if (index < 0 || index >= UcdData.EmojiSeqIndex.Length) return null;
        (uint offset, ushort length) = UcdData.EmojiSeqIndex[index];
        var cps = new uint[length];
        for (int i = 0; i < length; ++i) cps[i] = UcdData.Pool[offset + i];
        return cps;
    }

    /// <summary>Simple uppercase mapping for <paramref name="cp"/> (UAX #44), or the code point unchanged.</summary>
    public static uint SimpleUppercase(uint cp) => PairLookup(UcdData.Simple_upper, cp);

    /// <summary>Simple lowercase mapping for <paramref name="cp"/> (UAX #44), or the code point unchanged.</summary>
    public static uint SimpleLowercase(uint cp) => PairLookup(UcdData.Simple_lower, cp);

    /// <summary>Simple titlecase mapping for <paramref name="cp"/> (UAX #44), or the code point unchanged.</summary>
    public static uint SimpleTitlecase(uint cp) => PairLookup(UcdData.Simple_title, cp);

    /// <summary>True when <paramref name="cp"/> has the UAX #44 White_Space property.</summary>
    public static bool WhiteSpace(uint cp) => Lookup(UcdData.Prop_white_space, cp) != 0;

    /// <summary>True when <paramref name="cp"/> has the UAX #44 Alphabetic property.</summary>
    public static bool Alphabetic(uint cp) => Lookup(UcdData.Prop_alphabetic, cp) != 0;

    /// <summary>True when <paramref name="cp"/> has the UAX #44 Unified_Ideograph property.</summary>
    public static bool UnifiedIdeograph(uint cp) =>
        Lookup(UcdData.Prop_unified_ideograph, cp) != 0;

    /// <summary>Block index (UAX #44); the value indexes <see cref="BlockName"/>.</summary>
    public static int Block(uint cp) => Lookup(UcdData.Block_range, cp);

    /// <summary>Block name for an index returned by <see cref="Block"/>, or null.</summary>
    public static string? BlockName(int index) =>
        index >= 0 && index < UcdData.BlockNames.Length ? UcdData.BlockNames[index] : null;

    /// <summary>One DUCET collation element weight triple (UTS #10).</summary>
    public readonly record struct CollationWeight(
        ushort Primary, ushort Secondary, ushort Tertiary, bool Variable);

    private static CollationWeight ReadWeight(int index)
    {
        int b = index * 4;
        return new CollationWeight(
            (ushort)UcdData.CollationKeys[b],
            (ushort)UcdData.CollationKeys[b + 1],
            (ushort)UcdData.CollationKeys[b + 2],
            UcdData.CollationKeys[b + 3] != 0);
    }

    /// <summary>
    /// DUCET collation elements for a single code point.  Appends the elements
    /// to <paramref name="output"/> and returns how many were appended; zero
    /// means the code point has no explicit mapping (see UTS #10 S2.2).
    /// </summary>
    public static int CollationWeights(uint cp, List<CollationWeight> output)
    {
        UcdData.Seq[] table = UcdData.CollationSingle;
        int lo = 0;
        int hi = table.Length;
        while (lo < hi)
        {
            int mid = lo + (hi - lo) / 2;
            UcdData.Seq entry = table[mid];
            if (cp < entry.Cp)
            {
                hi = mid;
            }
            else if (cp > entry.Cp)
            {
                lo = mid + 1;
            }
            else
            {
                for (int i = 0; i < entry.Length; ++i)
                {
                    output.Add(ReadWeight((int)entry.Offset + i));
                }
                return entry.Length;
            }
        }
        return 0;
    }

    /// <summary>
    /// Longest DUCET contraction matching a prefix of <paramref name="cps"/>.
    /// <paramref name="matched"/> receives the number of code points matched;
    /// the return value is the number of collation elements appended to
    /// <paramref name="output"/> (zero when no contraction applies).
    /// </summary>
    public static int CollationContraction(
        ReadOnlySpan<uint> cps, List<CollationWeight> output, out int matched)
    {
        matched = 0;
        int best = 0;
        UcdData.Seq bestEntry = default;

        foreach (UcdData.Seq entry in UcdData.CollationMulti)
        {
            int n = (int)(entry.Cp >> 24);
            int off = (int)(entry.Cp & 0x00FF_FFFF);
            if (n == 0 || n > cps.Length || n <= best) continue;

            bool equal = true;
            for (int j = 0; j < n; ++j)
            {
                if (cps[j] != UcdData.Pool[off + j])
                {
                    equal = false;
                    break;
                }
            }
            if (equal)
            {
                best = n;
                bestEntry = entry;
            }
        }

        if (best == 0) return 0;
        matched = best;
        for (int i = 0; i < bestEntry.Length; ++i)
        {
            output.Add(ReadWeight((int)bestEntry.Offset + i));
        }
        return bestEntry.Length;
    }

    /// <summary>
    /// Siniform implicit-weight group for <paramref name="cp"/> (UTS #10 Table
    /// 16, from the DUCET <c>@implicitweights</c> lines).  Returns the primary
    /// base and the offset added to the second implicit weight.
    /// </summary>
    public static bool ImplicitWeight(uint cp, out ushort basePrimary, out uint offset)
    {
        foreach (UcdData.Implicit entry in UcdData.ImplicitWeights)
        {
            if (cp >= entry.Start && cp <= entry.End)
            {
                basePrimary = (ushort)entry.Base;
                offset = cp - entry.OffsetBase;
                return true;
            }
        }
        basePrimary = 0;
        offset = 0;
        return false;
    }

    private static uint PairLookup(UcdData.Pair[] table, uint cp)
    {
        int lo = 0;
        int hi = table.Length;
        while (lo < hi)
        {
            int mid = lo + (hi - lo) / 2;
            UcdData.Pair pair = table[mid];
            if (cp < pair.Cp)
            {
                hi = mid;
            }
            else if (cp > pair.Cp)
            {
                lo = mid + 1;
            }
            else
            {
                return pair.Mapped;
            }
        }
        return cp;
    }

    /// <summary>
    /// Decomposition mapping for <paramref name="cp"/>, if any.  When
    /// <paramref name="compatibility"/> is false only canonical decompositions
    /// are returned.
    /// </summary>
    public static uint[]? Decomposition(uint cp, bool compatibility)
    {
        int lo = 0;
        int hi = UcdData.Decomp.Length;
        while (lo < hi)
        {
            int mid = lo + (hi - lo) / 2;
            UcdData.MapEntry entry = UcdData.Decomp[mid];
            if (cp < entry.Cp)
            {
                hi = mid;
            }
            else if (cp > entry.Cp)
            {
                lo = mid + 1;
            }
            else
            {
                if (!compatibility && entry.Kind == 1) return null;
                var parts = new uint[entry.Length];
                int offset = (int)entry.Offset;
                for (int i = 0; i < parts.Length; ++i) parts[i] = UcdData.Pool[offset + i];
                return parts;
            }
        }
        return null;
    }

    /// <summary>True when <paramref name="cp"/> has a decomposition mapping.</summary>
    public static bool HasDecomposition(uint cp, bool compatibility) =>
        Decomposition(cp, compatibility) is not null;

    /// <summary>Canonical composition for the pair (<paramref name="first"/>, <paramref name="second"/>), if any.</summary>
    public static bool TryComposePair(uint first, uint second, out uint composed)
    {
        int lo = 0;
        int hi = UcdData.Composition.Length;
        while (lo < hi)
        {
            int mid = lo + (hi - lo) / 2;
            UcdData.Comp entry = UcdData.Composition[mid];
            if (first < entry.First || (first == entry.First && second < entry.Second))
            {
                hi = mid;
            }
            else if (first > entry.First || (first == entry.First && second > entry.Second))
            {
                lo = mid + 1;
            }
            else
            {
                composed = entry.Composed;
                return true;
            }
        }
        composed = 0;
        return false;
    }

    public static byte GeneralCategory(uint cp) => (byte)Lookup(UcdData.Gc, cp);

    public static byte EastAsianWidth(uint cp) => (byte)Lookup(UcdData.Eaw, cp);

    public static byte GraphemeBreak(uint cp) => (byte)Lookup(UcdData.Grapheme, cp);

    public static byte WordBreak(uint cp) => (byte)Lookup(UcdData.Word, cp);

    public static byte SentenceBreak(uint cp) => (byte)Lookup(UcdData.Sentence, cp);

    public static byte LineBreak(uint cp) => (byte)Lookup(UcdData.Line_break_range, cp);

    public static byte IndicConjunctBreak(uint cp) => (byte)Lookup(UcdData.Incb, cp);

    public static bool ExtendedPictographic(uint cp) =>
        Lookup(UcdData.Prop_extended_pictographic, cp) != 0;

    /// <summary>UTS #51 Emoji binary property.</summary>
    public static bool Emoji(uint cp) => Lookup(UcdData.Prop_emoji, cp) != 0;

    /// <summary>UTS #51 Emoji_Modifier_Base binary property.</summary>
    public static bool EmojiModifierBase(uint cp) =>
        Lookup(UcdData.Prop_emoji_modifier_base, cp) != 0;

    /// <summary>UAX #50 Vertical_Orientation values.</summary>
    public static class Vo
    {
        public const byte U = 0;
        public const byte R = 1;
        public const byte Tu = 2;
        public const byte Tr = 3;
    }

    /// <summary>Vertical_Orientation (UAX #50); unlisted code points are U.</summary>
    public static byte VerticalOrientation(uint cp) => (byte)Lookup(UcdData.Vertical, cp);

    /// <summary>Bidirectional_Class values (UAX #9), in generated-table order.</summary>
    public static class Bc
    {
        public const byte L = 0, R = 1, En = 2, Es = 3, Et = 4, An = 5, Cs = 6;
        public const byte B = 7, S = 8, Ws = 9, On = 10, Lre = 11, Lro = 12;
        public const byte Al = 13, Rle = 14, Rlo = 15, Pdf = 16, Nsm = 17;
        public const byte Bn = 18, Fsi = 19, Lri = 20, Rli = 21, Pdi = 22;
    }

    /// <summary>Joining_Type values (UAX #9 / UAX #53), in generated-table order.</summary>
    public static class Jt
    {
        public const byte U = 0, C = 1, D = 2, L = 3, R = 4, T = 5;
    }

    /// <summary>
    /// Binary properties exposed by the generated data layer (UAX #44).  The
    /// numeric values match the C <c>unicodoid_binary_property</c> enum.
    /// </summary>
    public enum BinaryProperty
    {
        Alphabetic = 0,
        Lowercase = 1,
        Uppercase = 2,
        Cased = 3,
        CaseIgnorable = 4,
        Math = 5,
        IdStart = 6,
        IdContinue = 7,
        XidStart = 8,
        XidContinue = 9,
        DefaultIgnorableCodePoint = 10,
        GraphemeExtend = 11,
        GraphemeBase = 12,
        GraphemeLink = 13,
        WhiteSpace = 14,
        PatternSyntax = 15,
        PatternWhiteSpace = 16,
        Dash = 17,
        Hyphen = 18,
        QuotationMark = 19,
        TerminalPunctuation = 20,
        Diacritic = 21,
        Extender = 22,
        SoftDotted = 23,
        JoinControl = 24,
        BidiControl = 25,
        NoncharacterCodePoint = 26,
        VariationSelector = 27,
        Emoji = 28,
        EmojiPresentation = 29,
        EmojiModifier = 30,
        EmojiModifierBase = 31,
        EmojiComponent = 32,
        ExtendedPictographic = 33,
        UnifiedIdeograph = 34,
        Radical = 35,
        Deprecated = 36,
        FullCompositionExclusion = 37,
        SentenceTerminal = 38,
        RegionalIndicator = 39,
        ModifierCombiningMark = 40
    }

    /// <summary>Bidirectional_Class of <paramref name="cp"/> (UAX #9).</summary>
    public static byte BidiClass(uint cp) => (byte)Lookup(UcdData.Bidi, cp);

    /// <summary>Script value of <paramref name="cp"/> (UAX #24); indexes <see cref="ScriptName"/>.</summary>
    public static ushort Script(uint cp) => Lookup(UcdData.Script_range, cp);

    /// <summary>Number of known Script values including Unknown at index 0.</summary>
    public static int ScriptCount => UcdData.ScriptNames.Length;

    /// <summary>Script name for a value returned by <see cref="Script"/>, or null.</summary>
    public static string? ScriptName(int index) =>
        index >= 0 && index < UcdData.ScriptNames.Length ? UcdData.ScriptNames[index] : null;

    /// <summary>Find a Script value by long name; 0 (Unknown) when absent.</summary>
    public static ushort ScriptFind(string name)
    {
        for (int i = 0; i < UcdData.ScriptNames.Length; ++i)
        {
            if (string.Equals(UcdData.ScriptNames[i], name, StringComparison.Ordinal))
            {
                return (ushort)i;
            }
        }
        return 0;
    }

    /// <summary>Joining_Type of <paramref name="cp"/> (UAX #9 / UAX #53).</summary>
    public static byte JoiningType(uint cp) => (byte)Lookup(UcdData.Joining, cp);

    /// <summary>Age value of <paramref name="cp"/>; indexes <see cref="AgeName"/>.</summary>
    public static byte Age(uint cp) => (byte)Lookup(UcdData.Age_range, cp);

    /// <summary>Age name for a value returned by <see cref="Age"/>, or null.</summary>
    public static string? AgeName(int index) =>
        index >= 0 && index < UcdData.AgeNames.Length ? UcdData.AgeNames[index] : null;

    /// <summary>General_Category name for a value returned by <see cref="GeneralCategory"/>.</summary>
    public static string? GeneralCategoryName(int value) =>
        value >= 0 && value < UcdData.GcNames.Length ? UcdData.GcNames[value] : null;

    /// <summary>Bidirectional_Class name for a value returned by <see cref="BidiClass"/>.</summary>
    public static string? BidiClassName(int value) =>
        value >= 0 && value < UcdData.BidiNames.Length ? UcdData.BidiNames[value] : null;

    /// <summary>East_Asian_Width name for a value returned by <see cref="EastAsianWidth"/>.</summary>
    public static string? EastAsianWidthName(int value) =>
        value >= 0 && value < UcdData.EawNames.Length ? UcdData.EawNames[value] : null;

    /// <summary>Line_Break name for a value returned by <see cref="LineBreak"/>.</summary>
    public static string? LineBreakName(int value) =>
        value >= 0 && value < UcdData.LineBreakNames.Length ? UcdData.LineBreakNames[value] : null;

    /// <summary>Joining_Type name for a value returned by <see cref="JoiningType"/>.</summary>
    public static string? JoiningTypeName(int value) =>
        value >= 0 && value < UcdData.JoiningNames.Length ? UcdData.JoiningNames[value] : null;

    private static UcdData.Range[] BinaryTable(BinaryProperty property) =>
        property switch
        {
            BinaryProperty.Alphabetic => UcdData.Prop_alphabetic,
            BinaryProperty.Lowercase => UcdData.Prop_lowercase,
            BinaryProperty.Uppercase => UcdData.Prop_uppercase,
            BinaryProperty.Cased => UcdData.Prop_cased,
            BinaryProperty.CaseIgnorable => UcdData.Prop_case_ignorable,
            BinaryProperty.Math => UcdData.Prop_math,
            BinaryProperty.IdStart => UcdData.Prop_id_start,
            BinaryProperty.IdContinue => UcdData.Prop_id_continue,
            BinaryProperty.XidStart => UcdData.Prop_xid_start,
            BinaryProperty.XidContinue => UcdData.Prop_xid_continue,
            BinaryProperty.DefaultIgnorableCodePoint =>
                UcdData.Prop_default_ignorable_code_point,
            BinaryProperty.GraphemeExtend => UcdData.Prop_grapheme_extend,
            BinaryProperty.GraphemeBase => UcdData.Prop_grapheme_base,
            BinaryProperty.GraphemeLink => UcdData.Prop_grapheme_link,
            BinaryProperty.WhiteSpace => UcdData.Prop_white_space,
            BinaryProperty.PatternSyntax => UcdData.Prop_pattern_syntax,
            BinaryProperty.PatternWhiteSpace => UcdData.Prop_pattern_white_space,
            BinaryProperty.Dash => UcdData.Prop_dash,
            BinaryProperty.Hyphen => UcdData.Prop_hyphen,
            BinaryProperty.QuotationMark => UcdData.Prop_quotation_mark,
            BinaryProperty.TerminalPunctuation => UcdData.Prop_terminal_punctuation,
            BinaryProperty.Diacritic => UcdData.Prop_diacritic,
            BinaryProperty.Extender => UcdData.Prop_extender,
            BinaryProperty.SoftDotted => UcdData.Prop_soft_dotted,
            BinaryProperty.JoinControl => UcdData.Prop_join_control,
            BinaryProperty.BidiControl => UcdData.Prop_bidi_control,
            BinaryProperty.NoncharacterCodePoint => UcdData.Prop_noncharacter_code_point,
            BinaryProperty.VariationSelector => UcdData.Prop_variation_selector,
            BinaryProperty.Emoji => UcdData.Prop_emoji,
            BinaryProperty.EmojiPresentation => UcdData.Prop_emoji_presentation,
            BinaryProperty.EmojiModifier => UcdData.Prop_emoji_modifier,
            BinaryProperty.EmojiModifierBase => UcdData.Prop_emoji_modifier_base,
            BinaryProperty.EmojiComponent => UcdData.Prop_emoji_component,
            BinaryProperty.ExtendedPictographic => UcdData.Prop_extended_pictographic,
            BinaryProperty.UnifiedIdeograph => UcdData.Prop_unified_ideograph,
            BinaryProperty.Radical => UcdData.Prop_radical,
            BinaryProperty.Deprecated => UcdData.Prop_deprecated,
            BinaryProperty.FullCompositionExclusion => UcdData.Prop_full_composition_exclusion,
            BinaryProperty.SentenceTerminal => UcdData.Prop_sentence_terminal,
            BinaryProperty.RegionalIndicator => UcdData.Prop_regional_indicator,
            BinaryProperty.ModifierCombiningMark => UcdData.Prop_modifier_combining_mark,
            _ => UcdData.Prop_alphabetic
        };

    /// <summary>True when <paramref name="cp"/> has the binary property (UAX #44).</summary>
    public static bool HasBinaryProperty(uint cp, BinaryProperty property) =>
        Lookup(BinaryTable(property), cp) != 0;

    private static uint[]? MapLookup(UcdData.MapEntry[] table, uint cp)
    {
        int lo = 0;
        int hi = table.Length;
        while (lo < hi)
        {
            int mid = lo + (hi - lo) / 2;
            UcdData.MapEntry entry = table[mid];
            if (cp < entry.Cp)
            {
                hi = mid;
            }
            else if (cp > entry.Cp)
            {
                lo = mid + 1;
            }
            else
            {
                var parts = new uint[entry.Length];
                int offset = (int)entry.Offset;
                for (int i = 0; i < parts.Length; ++i) parts[i] = UcdData.Pool[offset + i];
                return parts;
            }
        }
        return null;
    }

    /// <summary>Confusable mapping (UTS #39), or null when <paramref name="cp"/> has none.</summary>
    public static uint[]? Confusable(uint cp) => MapLookup(UcdData.Confusables, cp);

    /// <summary>IDNA disposition values returned by <see cref="IdnaStatus"/>.</summary>
    public static class IdnaDisposition
    {
        public const int Valid = 0, Mapped = 1, Deviation = 2, Ignored = 3, Disallowed = 4;
    }

    /// <summary>
    /// IDNA disposition (UTS #46) and the mapping for mapped/deviation code
    /// points.  <paramref name="mapping"/> is null for other dispositions.
    /// </summary>
    public static int IdnaStatus(uint cp, out uint[]? mapping)
    {
        int status = (int)Lookup(UcdData.Idna_status_range, cp);
        mapping = status is IdnaDisposition.Mapped or IdnaDisposition.Deviation
            ? MapLookup(UcdData.Idna_mappings, cp)
            : null;
        return status;
    }

    /// <summary>True when <paramref name="cp"/> is disallowed in transitional processing (UTS #46).</summary>
    public static bool IdnaNv8(uint cp) => Lookup(UcdData.Idna_nv8_range, cp) != 0;

    private static uint[] FullMapping(UcdData.MapEntry[] table, uint cp)
    {
        uint[]? mapping = MapLookup(table, cp);
        return mapping ?? new[] { cp };
    }

    /// <summary>Full case folding (UTS #18 / UAX #44) for <paramref name="cp"/>.</summary>
    public static uint[] ToCasefold(uint cp)
    {
        uint[] mapping = FullMapping(UcdData.Fold_full, cp);
        if (mapping.Length == 1 && mapping[0] == cp)
        {
            mapping[0] = PairLookup(UcdData.Simple_fold, cp);
        }
        return mapping;
    }

    /// <summary>True when <paramref name="cp"/> has a case folding different from itself.</summary>
    public static bool HasCasefold(uint cp)
    {
        uint[] mapping = ToCasefold(cp);
        return mapping.Length > 0 && (mapping.Length != 1 || mapping[0] != cp);
    }

    /// <summary>Full uppercase mapping for <paramref name="cp"/> (UAX #44).</summary>
    public static uint[] ToUppercase(uint cp) => FullMapping(UcdData.Upper_full, cp);

    /// <summary>Full lowercase mapping for <paramref name="cp"/> (UAX #44).</summary>
    public static uint[] ToLowercase(uint cp) => FullMapping(UcdData.Lower_full, cp);

    /// <summary>Full titlecase mapping for <paramref name="cp"/> (UAX #44).</summary>
    public static uint[] ToTitlecase(uint cp) => FullMapping(UcdData.Title_full, cp);

    /// <summary>Numeric_Value as an exact fraction, when present (UAX #44).</summary>
    public static bool TryNumericValue(uint cp, out long numerator, out long denominator)
    {
        foreach (UcdData.Numeric entry in UcdData.NumericValues)
        {
            if (entry.Cp == cp)
            {
                numerator = entry.Numerator;
                denominator = entry.Denominator;
                return true;
            }
        }
        numerator = 0;
        denominator = 0;
        return false;
    }

    /// <summary>Decimal_Digit_Value, or -1 when <paramref name="cp"/> is not a decimal digit.</summary>
    public static int DecimalDigit(uint cp)
    {
        foreach (UcdData.Numeric entry in UcdData.NumericValues)
        {
            if (entry.Cp != cp) continue;
            return entry.Digit >= 0 ? (int)entry.Digit : -1;
        }
        return -1;
    }
}
