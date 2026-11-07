using System;
using System.Collections.Generic;

namespace Unicodoid;

/// <summary>Boundary types supported by the segmenter.</summary>
public enum BoundaryType
{
    Grapheme = 0,
    Word = 1,
    Line = 2,
    Sentence = 3
}

/// <summary>One segment of a byte stream.</summary>
public readonly struct TextSegment
{
    public TextSegment(int byteOffset, int byteLength, int charCount, bool isBoundary)
    {
        ByteOffset = byteOffset;
        ByteLength = byteLength;
        CharCount = charCount;
        IsBoundary = isBoundary;
    }

    public int ByteOffset { get; }
    public int ByteLength { get; }
    public int CharCount { get; }
    public bool IsBoundary { get; }

    public override string ToString() =>
        $"[{ByteOffset}+{ByteLength}, {CharCount} chars]";
}

/// <summary>
/// Unicode text segmentation (UAX #29 and UAX #14).
///
/// Grapheme cluster, word, line, and sentence boundaries are computed from the
/// generated Unicode 18.0.0 property tables in
/// <see cref="Unicodoid.Generated.UcdData"/>.  The implementation follows the
/// default rule sets:
///
/// <list type="bullet">
/// <item>grapheme clusters: UAX #29 GB1–GB13 / GB999,</item>
/// <item>words: UAX #29 WB1–WB16 / WB999 (with WB4 folding),</item>
/// <item>sentences: UAX #29 SB1–SB11 / SB998 (with SB5 folding),</item>
/// <item>lines: UAX #14 LB1–LB31.</item>
/// </list>
/// </summary>
public sealed class Segmenter
{
    private readonly uint[] _codePoints;
    private readonly int[] _byteOffsets; // _codePoints.Length + 1 entries
    private int _index;

    public IEncodingSystem EncodingSystem { get; }
    public BoundaryType BoundaryType { get; }

    /// <summary>Number of decoded code points.</summary>
    public int Count => _codePoints.Length;

    private Segmenter(
        IEncodingSystem system,
        BoundaryType boundaryType,
        uint[] codePoints,
        int[] byteOffsets)
    {
        EncodingSystem = system;
        BoundaryType = boundaryType;
        _codePoints = codePoints;
        _byteOffsets = byteOffsets;
    }

    public static UnicodoidStatus TryCreate(
        IEncodingSystem system,
        ReadOnlySpan<byte> data,
        BoundaryType boundaryType,
        out Segmenter? segmenter)
    {
        segmenter = null;
        if (system is null) return UnicodoidStatus.InvalidParam;
        if (!system.HasUnicodeProfile) return UnicodoidStatus.Unsupported;

        var codePoints = new List<uint>();
        var offsets = new List<int>();
        var codec = new Codec(system);
        int offset = 0;

        while (offset < data.Length)
        {
            offsets.Add(offset);
            UnicodoidStatus status = codec.DecodeChar(
                data[offset..], out CharacterValue native, out int read);

            if (status == UnicodoidStatus.NoCharacter)
            {
                if (read == 0) return UnicodoidStatus.InternalError;
                offsets.RemoveAt(offsets.Count - 1);
                offset += read;
                continue;
            }
            if (status != UnicodoidStatus.Ok) return status;
            if (read == 0) return UnicodoidStatus.InternalError;

            if (!system.UnicodeProfile!.TryToUnicode(native, out uint scalar))
            {
                return UnicodoidStatus.UnmappableChar;
            }

            codePoints.Add(scalar);
            offset += read;
        }

        offsets.Add(data.Length);

        segmenter = new Segmenter(
            system, boundaryType, codePoints.ToArray(), offsets.ToArray());
        return UnicodoidStatus.Ok;
    }

    public static Segmenter Create(
        IEncodingSystem system,
        ReadOnlySpan<byte> data,
        BoundaryType boundaryType)
    {
        UnicodoidStatus status = TryCreate(system, data, boundaryType, out Segmenter? result);
        return status == UnicodoidStatus.Ok
            ? result!
            : throw new UnicodoidException(status);
    }

    /// <summary>Produce the next segment; false at end of input.</summary>
    public bool Next(out TextSegment segment)
    {
        if (_index >= _codePoints.Length)
        {
            segment = default;
            return false;
        }

        int start = _index;
        int end = BoundaryType switch
        {
            BoundaryType.Grapheme => NextGrapheme(start),
            BoundaryType.Word => NextWord(start),
            BoundaryType.Line => NextLine(start),
            BoundaryType.Sentence => NextSentence(start),
            _ => start + 1
        };

        if (end <= start) end = start + 1;
        if (end > _codePoints.Length) end = _codePoints.Length;

        int byteStart = _byteOffsets[start];
        int byteEnd = _byteOffsets[end];

        segment = new TextSegment(
            byteStart, byteEnd - byteStart, end - start, isBoundary: true);
        _index = end;
        return true;
    }

    public IEnumerable<TextSegment> Segments()
    {
        while (Next(out TextSegment segment))
        {
            yield return segment;
        }
    }

    /// <summary>Collect all segments into an array.</summary>
    public TextSegment[] ToArray()
    {
        var result = new List<TextSegment>();
        while (Next(out TextSegment segment))
        {
            result.Add(segment);
        }
        return result.ToArray();
    }

    public static UnicodoidStatus TrySegment(
        IEncodingSystem system,
        ReadOnlySpan<byte> data,
        BoundaryType boundaryType,
        out TextSegment[] segments)
    {
        segments = Array.Empty<TextSegment>();
        UnicodoidStatus status = TryCreate(system, data, boundaryType, out Segmenter? segmenter);
        if (status != UnicodoidStatus.Ok) return status;
        segments = segmenter!.ToArray();
        return UnicodoidStatus.Ok;
    }

    public static TextSegment[] Segment(
        IEncodingSystem system,
        ReadOnlySpan<byte> data,
        BoundaryType boundaryType)
    {
        UnicodoidStatus status = TrySegment(system, data, boundaryType, out TextSegment[] result);
        return status == UnicodoidStatus.Ok
            ? result
            : throw new UnicodoidException(status);
    }

    // ---- UAX #29 grapheme clusters --------------------------------------

    private static bool IsControl(uint codePoint)
    {
        byte g = Ucd.GraphemeBreak(codePoint);
        return g is Ucd.Gcb.Control or Ucd.Gcb.Cr or Ucd.Gcb.Lf;
    }

    private int NextGrapheme(int start) => NextGraphemeCluster(_codePoints, start);

    /// <summary>
    /// Scalar-level extended grapheme cluster primitive (UAX #29).  Returns
    /// the index one past the last scalar of the cluster beginning at
    /// <paramref name="start"/>, or <c>_codePoints.Length</c> when out of range.
    /// </summary>
    public static int NextGraphemeCluster(ReadOnlySpan<uint> cps, int start)
    {
        int i = start + 1;
        if (i > cps.Length) return cps.Length;

        uint prev = cps[start];
        byte prevG = Ucd.GraphemeBreak(prev);
        long prevRiPos = prevG == Ucd.Gcb.Ri ? 0 : -1;
        bool emojiContext = Ucd.ExtendedPictographic(prev);
        bool emojiZwjPending = false;
        // GB9c (Unicode 18.0.0): ConjunctLinker ConjunctExtender* x LinkingConsonant.
        bool inLinkerConjunct = Ucd.IndicConjunctBreak(prev) == Ucd.Incb.Linker;

        while (i < cps.Length)
        {
            uint cur = cps[i];
            byte curG = Ucd.GraphemeBreak(cur);
            byte curIncb = Ucd.IndicConjunctBreak(cur);
            bool boundary = true;

            if (prev == 0x000D && cur == 0x000A)
            {
                // GB3: CR x LF.
                boundary = false;
            }
            else if (IsControl(prev) || IsControl(cur))
            {
                // GB4 / GB5: break before and after controls.
                boundary = true;
            }
            else if (prevG == Ucd.Gcb.L &&
                     curG is Ucd.Gcb.L or Ucd.Gcb.V or Ucd.Gcb.Lv or Ucd.Gcb.Lvt)
            {
                // GB6: L x (L | V | LV | LVT).
                boundary = false;
            }
            else if (prevG is Ucd.Gcb.Lv or Ucd.Gcb.V &&
                     curG is Ucd.Gcb.V or Ucd.Gcb.T)
            {
                // GB7: (LV | V) x (V | T).
                boundary = false;
            }
            else if (prevG is Ucd.Gcb.Lvt or Ucd.Gcb.T && curG == Ucd.Gcb.T)
            {
                // GB8: (LVT | T) x T.
                boundary = false;
            }
            else if (curG is Ucd.Gcb.Extend or Ucd.Gcb.Zwj)
            {
                // GB9: x (Extend | ZWJ).
                boundary = false;
            }
            else if (curG == Ucd.Gcb.SpacingMark)
            {
                // GB9a: x SpacingMark.
                boundary = false;
            }
            else if (prevG == Ucd.Gcb.Prepend)
            {
                // GB9b: Prepend x.
                boundary = false;
            }
            else if (curIncb == Ucd.Incb.Consonant && inLinkerConjunct)
            {
                // GB9c: InCB=Linker InCB=Extend* x InCB=Consonant.
                boundary = false;
            }
            else if (emojiZwjPending && Ucd.ExtendedPictographic(cur))
            {
                // GB11: ExtPict Extend* ZWJ x ExtPict.
                boundary = false;
            }
            else if (curG == Ucd.Gcb.Ri)
            {
                // GB12 / GB13: regional indicator pairing.
                long curRiPos = prevG == Ucd.Gcb.Ri ? prevRiPos + 1 : 0;
                if (curRiPos % 2 == 1) boundary = false;
            }

            // GB999: Any / Any (default break).
            if (boundary) return i;

            // Consume the current character and update cluster context.
            if (curG == Ucd.Gcb.Ri)
            {
                prevRiPos = prevG == Ucd.Gcb.Ri ? prevRiPos + 1 : 0;
            }
            else
            {
                prevRiPos = -1;
            }

            if (cur == 0x200D)
            {
                if (emojiContext) emojiZwjPending = true;
            }
            else if (curG == Ucd.Gcb.Extend)
            {
                // Extends stay inside the current emoji context.
            }
            else
            {
                emojiContext = Ucd.ExtendedPictographic(cur);
                emojiZwjPending = false;
            }

            if (curIncb == Ucd.Incb.Linker)
            {
                inLinkerConjunct = true;
            }
            else if (curIncb == Ucd.Incb.Extend)
            {
                // ConjunctExtender* keeps the linker context alive.
            }
            else
            {
                inLinkerConjunct = false;
            }

            prev = cur;
            prevG = curG;
            ++i;
        }
        return cps.Length;
    }

    // ---- UAX #29 word breaking ------------------------------------------
    //
    // WB4 remaps every Extend / Format / ZWJ following a character that is not
    // CR, LF, or Newline to that base character and then ignores it.  This is
    // modelled by folding the base and its following Extend / Format / ZWJ run
    // into one unit.  All remaining rules are evaluated between units.

    private readonly struct WbUnit
    {
        public WbUnit(int start, int end, byte cls, byte lastCls, bool extPict)
        {
            Start = start;
            End = end;
            Cls = cls;
            LastCls = lastCls;
            EndsWithZwj = lastCls == Ucd.Wb.Zwj;
            ExtPict = extPict;
        }

        public int Start { get; }
        public int End { get; }
        public byte Cls { get; }
        public byte LastCls { get; }
        public bool EndsWithZwj { get; }
        public bool ExtPict { get; }
    }

    private static bool WbIsNewline(byte cls) =>
        cls is Ucd.Wb.Cr or Ucd.Wb.Lf or Ucd.Wb.Newline;

    private static bool WbIsAhLetter(byte cls) =>
        cls is Ucd.Wb.ALetter or Ucd.Wb.HebrewLetter;

    /// <summary>MidLetter | MidNumLetQ (MidNumLet | Single_Quote).</summary>
    private static bool WbIsMidLetterQ(byte cls) =>
        cls is Ucd.Wb.MidLetter or Ucd.Wb.MidNumLet or Ucd.Wb.SingleQuote;

    /// <summary>MidNum | MidNumLetQ (MidNumLet | Single_Quote).</summary>
    private static bool WbIsMidNumQ(byte cls) =>
        cls is Ucd.Wb.MidNum or Ucd.Wb.MidNumLet or Ucd.Wb.SingleQuote;

    private List<WbUnit> WbBuildUnits()
    {
        var units = new List<WbUnit>(_codePoints.Length);
        int i = 0;
        while (i < _codePoints.Length)
        {
            byte cls = Ucd.WordBreak(_codePoints[i]);
            byte lastCls = cls;
            bool extPict = Ucd.ExtendedPictographic(_codePoints[i]);

            int j = i + 1;
            if (!WbIsNewline(cls))
            {
                while (j < _codePoints.Length)
                {
                    byte c2 = Ucd.WordBreak(_codePoints[j]);
                    if (c2 is not (Ucd.Wb.Extend or Ucd.Wb.Format or Ucd.Wb.Zwj))
                    {
                        break;
                    }
                    lastCls = c2;
                    ++j;
                }
            }

            units.Add(new WbUnit(i, j, cls, lastCls, extPict));
            i = j;
        }
        return units;
    }

    private static bool WbBreakBefore(List<WbUnit> u, int i)
    {
        if (i == 0) return false;       // WB1 (leading sot break is implicit)
        if (i >= u.Count) return true;  // WB2: / eot

        WbUnit prev = u[i - 1];
        WbUnit cur = u[i];
        byte p = prev.Cls;
        byte c = cur.Cls;

        // WB3: CR x LF
        if (p == Ucd.Wb.Cr && c == Ucd.Wb.Lf) return false;
        // WB3.1: (Newline | CR | LF) /
        if (WbIsNewline(p)) return true;
        // WB3.2: / (Newline | CR | LF)
        if (WbIsNewline(c)) return true;
        // WB3.3: ZWJ x ExtPict
        if (prev.EndsWithZwj && cur.ExtPict) return false;
        // WB3.4: WSegSpace x WSegSpace (evaluated on raw characters, i.e.
        // before WB4 folds any following Extend / Format / ZWJ into the space).
        if (prev.LastCls == Ucd.Wb.WSegSpace && c == Ucd.Wb.WSegSpace) return false;
        // WB5: AHLetter x AHLetter
        if (WbIsAhLetter(p) && WbIsAhLetter(c)) return false;
        // WB6: AHLetter x (MidLetter | MidNumLetQ) AHLetter
        if (WbIsAhLetter(p) && WbIsMidLetterQ(c) &&
            i + 1 < u.Count && WbIsAhLetter(u[i + 1].Cls)) return false;
        // WB7: AHLetter (MidLetter | MidNumLetQ) x AHLetter
        if (WbIsMidLetterQ(p) && WbIsAhLetter(c) &&
            i >= 2 && WbIsAhLetter(u[i - 2].Cls)) return false;
        // WB7.1: Hebrew_Letter x Single_Quote
        if (p == Ucd.Wb.HebrewLetter && c == Ucd.Wb.SingleQuote) return false;
        // WB7.2: Hebrew_Letter x Double_Quote Hebrew_Letter
        if (p == Ucd.Wb.HebrewLetter && c == Ucd.Wb.DoubleQuote &&
            i + 1 < u.Count && u[i + 1].Cls == Ucd.Wb.HebrewLetter) return false;
        // WB7.3: Hebrew_Letter Double_Quote x Hebrew_Letter
        if (p == Ucd.Wb.DoubleQuote && c == Ucd.Wb.HebrewLetter &&
            i >= 2 && u[i - 2].Cls == Ucd.Wb.HebrewLetter) return false;
        // WB8: Numeric x Numeric
        if (p == Ucd.Wb.Numeric && c == Ucd.Wb.Numeric) return false;
        // WB9: AHLetter x Numeric
        if (WbIsAhLetter(p) && c == Ucd.Wb.Numeric) return false;
        // WB10: Numeric x AHLetter
        if (p == Ucd.Wb.Numeric && WbIsAhLetter(c)) return false;
        // WB11: Numeric (MidNum | MidNumLetQ) x Numeric
        if (WbIsMidNumQ(p) && c == Ucd.Wb.Numeric &&
            i >= 2 && u[i - 2].Cls == Ucd.Wb.Numeric) return false;
        // WB12: Numeric x (MidNum | MidNumLetQ) Numeric
        if (p == Ucd.Wb.Numeric && WbIsMidNumQ(c) &&
            i + 1 < u.Count && u[i + 1].Cls == Ucd.Wb.Numeric) return false;
        // WB13: Katakana x Katakana
        if (p == Ucd.Wb.Katakana && c == Ucd.Wb.Katakana) return false;
        // WB13.1: (AHLetter | Numeric | Katakana | ExtendNumLet) x ExtendNumLet
        if ((WbIsAhLetter(p) || p is Ucd.Wb.Numeric or Ucd.Wb.Katakana or Ucd.Wb.ExtendNumLet) &&
            c == Ucd.Wb.ExtendNumLet) return false;
        // WB13.2: ExtendNumLet x (AHLetter | Numeric | Katakana)
        if (p == Ucd.Wb.ExtendNumLet &&
            (WbIsAhLetter(c) || c is Ucd.Wb.Numeric or Ucd.Wb.Katakana)) return false;
        // WB15 / WB16: regional indicator pairs
        if (p == Ucd.Wb.Ri && c == Ucd.Wb.Ri)
        {
            int count = 0;
            int k = i;
            while (k > 0 && u[k - 1].Cls == Ucd.Wb.Ri)
            {
                ++count;
                --k;
            }
            if (count % 2 == 1) return false;
        }
        // WB999: Any / Any
        return true;
    }

    private int NextWord(int start)
    {
        if (_codePoints.Length == 0) return 0;

        List<WbUnit> units = WbBuildUnits();

        int first = units.Count;
        for (int k = 0; k < units.Count; ++k)
        {
            if (units[k].Start == start)
            {
                first = k;
                break;
            }
        }

        int end = _codePoints.Length;
        for (int k = first + 1; k <= units.Count; ++k)
        {
            if (WbBreakBefore(units, k))
            {
                end = k < units.Count ? units[k].Start : _codePoints.Length;
                break;
            }
        }

        if (end <= start) end = start + 1;
        return Math.Min(end, _codePoints.Length);
    }

    // ---- UAX #14 line breaking ------------------------------------------
    //
    // LB9 is applied by folding every base character together with the
    // following CM / ZWJ run into a single "unit".  Remaining CM / ZWJ
    // characters (LB10) become one-character AL units.  All line break rules
    // are then evaluated between units, which is exactly the "ignore affected
    // CM / ZWJ" requirement of LB9.

    private readonly struct LbUnit
    {
        public LbUnit(
            int start, int end, byte cls, byte gc, bool eastAsian,
            bool dotted, bool extPictCn, bool endsWithZwj)
        {
            Start = start;
            End = end;
            Cls = cls;
            Gc = gc;
            EastAsian = eastAsian;
            Dotted = dotted;
            ExtPictCn = extPictCn;
            EndsWithZwj = endsWithZwj;
        }

        public int Start { get; }
        public int End { get; }
        public byte Cls { get; }
        public byte Gc { get; }
        public bool EastAsian { get; }
        public bool Dotted { get; }
        public bool ExtPictCn { get; }
        public bool EndsWithZwj { get; }
    }

    /// <summary>LB1: resolve AI, SG, XX, SA, and CJ to their working classes.</summary>
    private static byte LbResolveClass(uint cp)
    {
        byte lb = Ucd.LineBreak(cp);
        byte gc = Ucd.GeneralCategory(cp);

        return lb switch
        {
            Ucd.Lb.Ai or Ucd.Lb.Sg or Ucd.Lb.Xx => Ucd.Lb.Al,
            Ucd.Lb.Sa => gc is Ucd.Gc.Mn or Ucd.Gc.Mc ? Ucd.Lb.Cm : Ucd.Lb.Al,
            Ucd.Lb.Cj => Ucd.Lb.Ns,
            _ => lb
        };
    }

    private static bool LbIsExcludedFromLb9(byte cls) =>
        cls is Ucd.Lb.Bk or Ucd.Lb.Cr or Ucd.Lb.Lf or Ucd.Lb.Nl or Ucd.Lb.Sp or Ucd.Lb.Zw;

    private List<LbUnit> LbBuildUnits()
    {
        var units = new List<LbUnit>(_codePoints.Length);
        int i = 0;
        while (i < _codePoints.Length)
        {
            uint cp = _codePoints[i];
            byte cls = LbResolveClass(cp);
            byte gc = Ucd.GeneralCategory(cp);
            byte eaw = Ucd.EastAsianWidth(cp);
            bool eastAsian = eaw is Ucd.Eaw.H or Ucd.Eaw.F or Ucd.Eaw.W;

            int j = i + 1;
            byte resolved = cls;
            bool endsWithZwj = false;

            if (cls is Ucd.Lb.Cm or Ucd.Lb.Zwj)
            {
                // LB10: a remaining combining mark / ZWJ behaves like AL.
                // Following CM / ZWJ characters are also "remaining", so they
                // form their own AL units (the resulting break decisions are
                // identical because AL x AL is prohibited by LB28).
                resolved = Ucd.Lb.Al;
                endsWithZwj = cls == Ucd.Lb.Zwj;
            }
            else if (LbIsExcludedFromLb9(cls))
            {
                // LB9 does not absorb CM / ZWJ after these classes.
                resolved = cls;
            }
            else
            {
                // LB9: treat X (CM | ZWJ)* as X.
                resolved = cls;
                while (j < _codePoints.Length)
                {
                    byte c2cls = LbResolveClass(_codePoints[j]);
                    if (c2cls is not (Ucd.Lb.Cm or Ucd.Lb.Zwj)) break;
                    endsWithZwj = c2cls == Ucd.Lb.Zwj;
                    ++j;
                }
            }

            bool extPictCn = gc == Ucd.Gc.Cn && Ucd.ExtendedPictographic(cp);
            units.Add(new LbUnit(
                i, j, resolved, gc, eastAsian, cp == 0x25CC, extPictCn, endsWithZwj));
            i = j;
        }
        return units;
    }

    /// <summary>True when the units immediately before index k end with NU (SY | IS)*.</summary>
    private static bool LbNuSeqBefore(List<LbUnit> u, int k)
    {
        int j = k;
        while (j > 0 && u[j - 1].Cls is Ucd.Lb.Sy or Ucd.Lb.Is) --j;
        return j > 0 && u[j - 1].Cls == Ucd.Lb.Nu;
    }

    /// <summary>Decide whether a line break is allowed immediately before unit i.</summary>
    private static bool LbBreakBefore(List<LbUnit> u, int i)
    {
        if (i == 0) return false;      // LB2: sot x
        if (i >= u.Count) return true; // LB3: ! eot

        LbUnit prev = u[i - 1];
        LbUnit cur = u[i];
        byte p = prev.Cls;
        byte c = cur.Cls;

        // LB4: BK !
        if (p == Ucd.Lb.Bk) return true;
        // LB5: CR x LF, then CR / LF / NL !
        if (p == Ucd.Lb.Cr && c == Ucd.Lb.Lf) return false;
        if (p is Ucd.Lb.Cr or Ucd.Lb.Lf or Ucd.Lb.Nl) return true;
        // LB6: x (BK | CR | LF | NL)
        if (c is Ucd.Lb.Bk or Ucd.Lb.Cr or Ucd.Lb.Lf or Ucd.Lb.Nl) return false;
        // LB7: x SP, x ZW
        if (c is Ucd.Lb.Sp or Ucd.Lb.Zw) return false;
        // LB8: ZW SP* !
        {
            int k = i;
            while (k > 0 && u[k - 1].Cls == Ucd.Lb.Sp) --k;
            if (k > 0 && u[k - 1].Cls == Ucd.Lb.Zw) return true;
        }
        // LB8a: ZWJ x
        if (prev.EndsWithZwj) return false;
        // LB11: x WJ, WJ x
        if (c == Ucd.Lb.Wj || p == Ucd.Lb.Wj) return false;
        // LB12: GL x
        if (p == Ucd.Lb.Gl) return false;
        // LB12a: [^SP HY HH] x GL
        if (c == Ucd.Lb.Gl && p is not (Ucd.Lb.Sp or Ucd.Lb.Hy or Ucd.Lb.Hh)) return false;
        // LB13: x CL, x CP, x EX, x SY
        if (c is Ucd.Lb.Cl or Ucd.Lb.Cp or Ucd.Lb.Ex or Ucd.Lb.Sy) return false;
        // LB14: OP SP* x
        {
            int k = i;
            while (k > 0 && u[k - 1].Cls == Ucd.Lb.Sp) --k;
            if (k > 0 && u[k - 1].Cls == Ucd.Lb.Op) return false;
        }
        // LB15a: (sot | BK | CR | LF | NL | OP | QU | GL | SP | ZW) QU_Pi SP* x
        {
            int k = i;
            while (k > 0 && u[k - 1].Cls == Ucd.Lb.Sp) --k;
            if (k > 0 && u[k - 1].Cls == Ucd.Lb.Qu && u[k - 1].Gc == Ucd.Gc.Pi)
            {
                if (k - 1 == 0) return false; // sot QU_Pi SP* x
                byte pc = u[k - 2].Cls;
                if (pc is Ucd.Lb.Bk or Ucd.Lb.Cr or Ucd.Lb.Lf or Ucd.Lb.Nl or
                    Ucd.Lb.Op or Ucd.Lb.Qu or Ucd.Lb.Gl or Ucd.Lb.Sp or Ucd.Lb.Zw)
                {
                    return false;
                }
            }
        }
        // LB15b: x QU_Pf (SP | GL | WJ | CL | QU | CP | EX | IS | SY | BK | CR | LF | NL | ZW | eot)
        if (c == Ucd.Lb.Qu && cur.Gc == Ucd.Gc.Pf)
        {
            if (i + 1 >= u.Count) return false;
            byte nc = u[i + 1].Cls;
            if (nc is Ucd.Lb.Sp or Ucd.Lb.Gl or Ucd.Lb.Wj or Ucd.Lb.Cl or Ucd.Lb.Qu or
                Ucd.Lb.Cp or Ucd.Lb.Ex or Ucd.Lb.Is or Ucd.Lb.Sy or Ucd.Lb.Bk or
                Ucd.Lb.Cr or Ucd.Lb.Lf or Ucd.Lb.Nl or Ucd.Lb.Zw)
            {
                return false;
            }
        }
        // LB15c: SP ! IS NU
        if (p == Ucd.Lb.Sp && c == Ucd.Lb.Is &&
            i + 1 < u.Count && u[i + 1].Cls == Ucd.Lb.Nu) return true;
        // LB15d: x IS
        if (c == Ucd.Lb.Is) return false;
        // LB16: (CL | CP) SP* x NS
        if (c == Ucd.Lb.Ns)
        {
            int k = i;
            while (k > 0 && u[k - 1].Cls == Ucd.Lb.Sp) --k;
            if (k > 0 && u[k - 1].Cls is Ucd.Lb.Cl or Ucd.Lb.Cp) return false;
        }
        // LB17: B2 SP* x B2
        if (c == Ucd.Lb.B2)
        {
            int k = i;
            while (k > 0 && u[k - 1].Cls == Ucd.Lb.Sp) --k;
            if (k > 0 && u[k - 1].Cls == Ucd.Lb.B2) return false;
        }
        // LB18: SP !
        if (p == Ucd.Lb.Sp) return true;
        // LB19: x QUmPi, QUmPf x
        if (c == Ucd.Lb.Qu && cur.Gc != Ucd.Gc.Pi) return false;
        if (p == Ucd.Lb.Qu && prev.Gc != Ucd.Gc.Pf) return false;
        // LB19a: East Asian quotation tailoring
        if (c == Ucd.Lb.Qu)
        {
            if (!prev.EastAsian) return false;             // [^EA] x QU
            if (i + 1 >= u.Count) return false;            // x QU eot
            if (!u[i + 1].EastAsian) return false;         // x QU [^EA]
        }
        if (p == Ucd.Lb.Qu && !cur.EastAsian) return false; // QU x [^EA]
        if (p == Ucd.Lb.Qu)                                 // (sot | [^EA]) QU x
        {
            if (i - 1 == 0) return false;
            if (!u[i - 2].EastAsian) return false;
        }
        // LB20: ! CB, CB !
        if (c == Ucd.Lb.Cb || p == Ucd.Lb.Cb) return true;
        // LB20a: (sot | BK | CR | LF | NL | SP | ZW | CB | GL) (HY | HH) x (AL | HL)
        if (p is Ucd.Lb.Hy or Ucd.Lb.Hh && c is Ucd.Lb.Al or Ucd.Lb.Hl)
        {
            if (i - 1 == 0) return false;
            byte pc = u[i - 2].Cls;
            if (pc is Ucd.Lb.Bk or Ucd.Lb.Cr or Ucd.Lb.Lf or Ucd.Lb.Nl or
                Ucd.Lb.Sp or Ucd.Lb.Zw or Ucd.Lb.Cb or Ucd.Lb.Gl) return false;
        }
        // LB21: x BA, x HH, x HY, x NS, BB x
        if (c is Ucd.Lb.Ba or Ucd.Lb.Hh or Ucd.Lb.Hy or Ucd.Lb.Ns) return false;
        if (p == Ucd.Lb.Bb) return false;
        // LB21a: HL (HY | HH) x [^HL]
        if (p is Ucd.Lb.Hy or Ucd.Lb.Hh && i >= 2 &&
            u[i - 2].Cls == Ucd.Lb.Hl && c != Ucd.Lb.Hl) return false;
        // LB21b: SY x HL
        if (p == Ucd.Lb.Sy && c == Ucd.Lb.Hl) return false;
        // LB22: x IN
        if (c == Ucd.Lb.In) return false;
        // LB23: (AL | HL) x NU, NU x (AL | HL)
        if (p is Ucd.Lb.Al or Ucd.Lb.Hl && c == Ucd.Lb.Nu) return false;
        if (p == Ucd.Lb.Nu && c is Ucd.Lb.Al or Ucd.Lb.Hl) return false;
        // LB23a: PR x (ID | EB | EM), (ID | EB | EM) x PO
        if (p == Ucd.Lb.Pr && c is Ucd.Lb.Id or Ucd.Lb.Eb or Ucd.Lb.Em) return false;
        if (p is Ucd.Lb.Id or Ucd.Lb.Eb or Ucd.Lb.Em && c == Ucd.Lb.Po) return false;
        // LB24: (PR | PO) x (AL | HL), (AL | HL) x (PR | PO)
        if (p is Ucd.Lb.Pr or Ucd.Lb.Po && c is Ucd.Lb.Al or Ucd.Lb.Hl) return false;
        if (p is Ucd.Lb.Al or Ucd.Lb.Hl && c is Ucd.Lb.Pr or Ucd.Lb.Po) return false;
        // LB25: numeric sequences
        if (c is Ucd.Lb.Po or Ucd.Lb.Pr)
        {
            if (p is Ucd.Lb.Cl or Ucd.Lb.Cp && LbNuSeqBefore(u, i - 1)) return false;
            if (LbNuSeqBefore(u, i)) return false;
        }
        if (p is Ucd.Lb.Po or Ucd.Lb.Pr)
        {
            if (c == Ucd.Lb.Op)
            {
                if (i + 1 < u.Count && u[i + 1].Cls == Ucd.Lb.Nu) return false;
                if (i + 2 < u.Count &&
                    u[i + 1].Cls == Ucd.Lb.Is && u[i + 2].Cls == Ucd.Lb.Nu) return false;
            }
            if (c == Ucd.Lb.Nu) return false;
        }
        if (p == Ucd.Lb.Hy && c == Ucd.Lb.Nu) return false;
        if (p == Ucd.Lb.Is && c == Ucd.Lb.Nu) return false;
        if (c == Ucd.Lb.Nu && LbNuSeqBefore(u, i)) return false;
        // LB26: Korean syllables
        if (p == Ucd.Lb.Jl && c is Ucd.Lb.Jl or Ucd.Lb.Jv or Ucd.Lb.H2 or Ucd.Lb.H3) return false;
        if (p is Ucd.Lb.Jv or Ucd.Lb.H2 && c is Ucd.Lb.Jv or Ucd.Lb.Jt) return false;
        if (p is Ucd.Lb.Jt or Ucd.Lb.H3 && c == Ucd.Lb.Jt) return false;
        // LB27: Korean syllable blocks behave like ID
        if (p is Ucd.Lb.Jl or Ucd.Lb.Jv or Ucd.Lb.Jt or Ucd.Lb.H2 or Ucd.Lb.H3 &&
            c == Ucd.Lb.Po) return false;
        if (p == Ucd.Lb.Pr &&
            c is Ucd.Lb.Jl or Ucd.Lb.Jv or Ucd.Lb.Jt or Ucd.Lb.H2 or Ucd.Lb.H3) return false;
        // LB28: (AL | HL) x (AL | HL)
        if (p is Ucd.Lb.Al or Ucd.Lb.Hl && c is Ucd.Lb.Al or Ucd.Lb.Hl) return false;
        // LB28a: Brahmic orthographic syllables
        {
            bool prevAk = p == Ucd.Lb.Ak || prev.Dotted || p == Ucd.Lb.As;
            bool curAk = c == Ucd.Lb.Ak || cur.Dotted || c == Ucd.Lb.As;
            if (p == Ucd.Lb.Ap && curAk) return false;
            if (prevAk && c is Ucd.Lb.Vf or Ucd.Lb.Vi) return false;
            if (p == Ucd.Lb.Vi && (c == Ucd.Lb.Ak || cur.Dotted) && i >= 2)
            {
                LbUnit p2 = u[i - 2];
                if (p2.Cls == Ucd.Lb.Ak || p2.Dotted || p2.Cls == Ucd.Lb.As) return false;
            }
            if (prevAk && curAk && i + 1 < u.Count && u[i + 1].Cls == Ucd.Lb.Vf) return false;
        }
        // LB29: IS x (AL | HL)
        if (p == Ucd.Lb.Is && c is Ucd.Lb.Al or Ucd.Lb.Hl) return false;
        // LB30: (AL | HL | NU) x OPmEastAsian, CPmEastAsian x (AL | HL | NU)
        if (p is Ucd.Lb.Al or Ucd.Lb.Hl or Ucd.Lb.Nu && c == Ucd.Lb.Op && !cur.EastAsian)
            return false;
        if (p == Ucd.Lb.Cp && !prev.EastAsian && c is Ucd.Lb.Al or Ucd.Lb.Hl or Ucd.Lb.Nu)
            return false;
        // LB30a: regional indicator pairs
        if (p == Ucd.Lb.Ri && c == Ucd.Lb.Ri)
        {
            int count = 0;
            int k = i;
            while (k > 0 && u[k - 1].Cls == Ucd.Lb.Ri)
            {
                ++count;
                --k;
            }
            if (count % 2 == 1) return false;
        }
        // LB30b: EB x EM, ExtPictUnassigned x EM
        if (c == Ucd.Lb.Em && (p == Ucd.Lb.Eb || prev.ExtPictCn)) return false;
        // LB31: break everywhere else.
        return true;
    }

    private int NextLine(int start)
    {
        if (_codePoints.Length == 0) return 0;

        List<LbUnit> units = LbBuildUnits();

        int first = units.Count;
        for (int k = 0; k < units.Count; ++k)
        {
            if (units[k].Start == start)
            {
                first = k;
                break;
            }
        }

        int end = _codePoints.Length;
        for (int k = first + 1; k <= units.Count; ++k)
        {
            if (LbBreakBefore(units, k))
            {
                end = k < units.Count ? units[k].Start : _codePoints.Length;
                break;
            }
        }

        if (end <= start) end = start + 1;
        return Math.Min(end, _codePoints.Length);
    }

    // ---- UAX #29 sentence breaking --------------------------------------
    //
    // SB5 ignores Extend / Format characters following a base character that
    // is not a paragraph separator.  Folding them into the base makes the SB
    // rules directly expressible between units.

    private readonly struct SbUnit
    {
        public SbUnit(int start, int end, byte cls)
        {
            Start = start;
            End = end;
            Cls = cls;
        }

        public int Start { get; }
        public int End { get; }
        public byte Cls { get; }
    }

    private static bool SbIsPara(byte cls) =>
        cls is Ucd.Sb.Sep or Ucd.Sb.Cr or Ucd.Sb.Lf;

    private static bool SbIsSaterm(byte cls) =>
        cls is Ucd.Sb.STerm or Ucd.Sb.ATerm;

    private List<SbUnit> SbBuildUnits()
    {
        var units = new List<SbUnit>(_codePoints.Length);
        int i = 0;
        while (i < _codePoints.Length)
        {
            byte cls = Ucd.SentenceBreak(_codePoints[i]);

            int j = i + 1;
            if (!SbIsPara(cls))
            {
                // SB5: X (Extend | Format)* -> X.
                while (j < _codePoints.Length)
                {
                    byte c2 = Ucd.SentenceBreak(_codePoints[j]);
                    if (c2 is not (Ucd.Sb.Extend or Ucd.Sb.Format)) break;
                    ++j;
                }
            }

            units.Add(new SbUnit(i, j, cls));
            i = j;
        }
        return units;
    }

    /// <summary>
    /// Scan the left context ending immediately before unit i and test for
    /// "SATerm Close*" (allowSp false) or "SATerm Close* Sp*" (allowSp true).
    /// </summary>
    private static bool SbScanSaterm(List<SbUnit> u, int i, bool allowSp, bool atermOnly)
    {
        int k = i;
        if (allowSp)
        {
            while (k > 0 && u[k - 1].Cls == Ucd.Sb.Sp) --k;
        }
        while (k > 0 && u[k - 1].Cls == Ucd.Sb.Close) --k;
        if (k == 0) return false;
        byte t = u[k - 1].Cls;
        if (atermOnly) return t == Ucd.Sb.ATerm;
        return SbIsSaterm(t);
    }

    /// <summary>SB8 right-hand side: (not (OLetter | Upper | Lower | ParaSep | SATerm))* Lower.</summary>
    private static bool Sb8Lookahead(List<SbUnit> u, int i)
    {
        int k = i;
        while (k < u.Count)
        {
            byte c = u[k].Cls;
            if (c is Ucd.Sb.OLetter or Ucd.Sb.Upper or Ucd.Sb.Lower ||
                SbIsPara(c) || SbIsSaterm(c))
            {
                break;
            }
            ++k;
        }
        return k < u.Count && u[k].Cls == Ucd.Sb.Lower;
    }

    private static bool SbBreakBefore(List<SbUnit> u, int i)
    {
        if (i == 0) return false;      // SB1
        if (i >= u.Count) return true; // SB2: Any / eot

        SbUnit prev = u[i - 1];
        SbUnit cur = u[i];
        byte p = prev.Cls;
        byte c = cur.Cls;

        // SB3: CR x LF
        if (p == Ucd.Sb.Cr && c == Ucd.Sb.Lf) return false;
        // SB4: ParaSep /
        if (SbIsPara(p)) return true;
        // SB6: ATerm x Numeric
        if (p == Ucd.Sb.ATerm && c == Ucd.Sb.Numeric) return false;
        // SB7: (Upper | Lower) ATerm x Upper
        if (p == Ucd.Sb.ATerm && c == Ucd.Sb.Upper && i >= 2)
        {
            byte p2 = u[i - 2].Cls;
            if (p2 is Ucd.Sb.Upper or Ucd.Sb.Lower) return false;
        }
        // SB8: ATerm Close* Sp* x (not (OLetter | Upper | Lower | ParaSep | SATerm))* Lower
        if (SbScanSaterm(u, i, allowSp: true, atermOnly: true) && Sb8Lookahead(u, i))
            return false;
        // SB8a: SATerm Close* Sp* x (SContinue | SATerm)
        if ((c == Ucd.Sb.SContinue || SbIsSaterm(c)) &&
            SbScanSaterm(u, i, allowSp: true, atermOnly: false)) return false;
        // SB9: SATerm Close* x (Close | Sp | ParaSep)
        if ((c == Ucd.Sb.Close || c == Ucd.Sb.Sp || SbIsPara(c)) &&
            SbScanSaterm(u, i, allowSp: false, atermOnly: false)) return false;
        // SB10: SATerm Close* Sp* x (Sp | ParaSep)
        if ((c == Ucd.Sb.Sp || SbIsPara(c)) &&
            SbScanSaterm(u, i, allowSp: true, atermOnly: false)) return false;
        // SB11: SATerm Close* Sp* ParaSep? /
        {
            int k = i;
            if (k > 0 && SbIsPara(u[k - 1].Cls)) --k;
            while (k > 0 && u[k - 1].Cls == Ucd.Sb.Sp) --k;
            while (k > 0 && u[k - 1].Cls == Ucd.Sb.Close) --k;
            if (k > 0 && SbIsSaterm(u[k - 1].Cls)) return true;
        }
        // SB998: Any x Any
        return false;
    }

    private int NextSentence(int start)
    {
        if (_codePoints.Length == 0) return 0;

        List<SbUnit> units = SbBuildUnits();

        int first = units.Count;
        for (int k = 0; k < units.Count; ++k)
        {
            if (units[k].Start == start)
            {
                first = k;
                break;
            }
        }

        int end = _codePoints.Length;
        for (int k = first + 1; k <= units.Count; ++k)
        {
            if (SbBreakBefore(units, k))
            {
                end = k < units.Count ? units[k].Start : _codePoints.Length;
                break;
            }
        }

        if (end <= start) end = start + 1;
        return Math.Min(end, _codePoints.Length);
    }
}