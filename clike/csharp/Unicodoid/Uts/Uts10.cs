using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Unicodoid;

/// <summary>Collation strength levels.</summary>
public enum CollationStrength
{
    Primary = 1,
    Secondary = 2,
    Tertiary = 3,
    Quaternary = 4,
    Identical = 5
}

/// <summary>Options controlling collation.</summary>
public sealed class CollationOptions
{
    public CollationStrength Strength { get; set; } = CollationStrength.Tertiary;

    /// <summary>
    /// Compare a dedicated case weight after the secondary level.  This is a
    /// Unicodoid extension over plain UCA and is disabled by default.
    /// </summary>
    public bool CaseLevel { get; set; }

    /// <summary>
    /// Compare runs of ASCII digits numerically.  This is a Unicodoid
    /// extension over plain UCA and is disabled by default.
    /// </summary>
    public bool NumericOrdering { get; set; }

    public static CollationOptions Default => new();

    public CollationOptions Clone() => new()
    {
        Strength = Strength,
        CaseLevel = CaseLevel,
        NumericOrdering = NumericOrdering
    };
}

/// <summary>
/// Unicode Collation Algorithm (UTS #10) for Unicode 18.0.0.
///
/// This is a conformant implementation of the Default Unicode Collation
/// Element Table (DUCET) in its NON_IGNORABLE variable-weighting
/// configuration:
///
/// <list type="bullet">
/// <item>NFD normalization as the mandatory first step (S1.1),</item>
/// <item>collation element construction with longest contiguous matches and
/// discontiguous non-starter extension with blocking (S2.1-S2.5),</item>
/// <item>three-level sort key formation with 16-bit weights (S3.1-S3.9).</item>
/// </list>
///
/// The DUCET weights come from the generated Unicode 18.0.0 tables in
/// <see cref="Ucd"/>.  CLDR tailorings, script reordering, backward secondary
/// levels and matching/searching are outside this module.
/// </summary>
public static class Collation
{
    private const uint HangulSBase = 0xAC00;
    private const uint HangulLBase = 0x1100;
    private const uint HangulVBase = 0x1161;
    private const uint HangulTBase = 0x11A7;
    private const int HangulLCount = 19;
    private const int HangulVCount = 21;
    private const int HangulTCount = 28;
    private const int HangulNCount = HangulVCount * HangulTCount;
    private const int HangulSCount = HangulLCount * HangulNCount;

    private readonly struct Ce
    {
        public Ce(ushort primary, ushort secondary, ushort tertiary)
        {
            Primary = primary;
            Secondary = secondary;
            Tertiary = tertiary;
        }

        public ushort Primary { get; }
        public ushort Secondary { get; }
        public ushort Tertiary { get; }
    }

    /* --------------------------------------------------------------------- */
    /* NFD normalization (S1.1)                                               */
    /* --------------------------------------------------------------------- */

    private static void DecomposeInto(uint codePoint, List<uint> output)
    {
        if (codePoint >= HangulSBase && codePoint < HangulSBase + HangulSCount)
        {
            uint sIndex = codePoint - HangulSBase;
            output.Add(HangulLBase + sIndex / HangulNCount);
            output.Add(HangulVBase + (sIndex % HangulNCount) / HangulTCount);
            uint t = HangulTBase + sIndex % HangulTCount;
            if (t != HangulTBase) output.Add(t);
            return;
        }

        uint[]? parts = Ucd.Decomposition(codePoint, compatibility: false);
        if (parts is not null)
        {
            foreach (uint part in parts)
            {
                DecomposeInto(part, output);
            }
            return;
        }

        output.Add(codePoint);
    }

    /// <summary>Canonical NFD decomposition plus canonical ordering.</summary>
    private static List<uint> Nfd(ReadOnlySpan<uint> input)
    {
        var output = new List<uint>(input.Length);
        foreach (uint cp in input)
        {
            DecomposeInto(cp, output);
        }

        // Stable insertion sort by Canonical_Combining_Class; starters
        // (ccc 0) act as barriers.
        for (int i = 1; i < output.Count; ++i)
        {
            uint codePoint = output[i];
            int combiningClass = Ucd.CombiningClass(codePoint);
            if (combiningClass == 0) continue;

            int j = i;
            while (j > 0)
            {
                int previous = Ucd.CombiningClass(output[j - 1]);
                if (previous == 0 || previous <= combiningClass) break;
                output[j] = output[j - 1];
                --j;
            }
            output[j] = codePoint;
        }

        return output;
    }

    /* --------------------------------------------------------------------- */
    /* Collation elements (S2)                                                */
    /* --------------------------------------------------------------------- */

    private static void PushWeights(List<Ce> output, List<Ucd.CollationWeight> weights)
    {
        foreach (Ucd.CollationWeight weight in weights)
        {
            output.Add(new Ce(weight.Primary, weight.Secondary, weight.Tertiary));
        }
    }

    /// <summary>Produce the CE sequence for one code point with no explicit mapping.</summary>
    private static void CeFromImplicit(List<Ce> output, uint codePoint)
    {
        // Siniform ideographic scripts: base from @implicitweights.
        if (Ucd.ImplicitWeight(codePoint, out ushort basePrimary, out uint offset))
        {
            output.Add(new Ce(basePrimary, 0x0020, 0x0002));
            output.Add(new Ce((ushort)(offset | 0x8000u), 0, 0));
            return;
        }

        ushort aaa;
        if (Ucd.UnifiedIdeograph(codePoint))
        {
            string? block = Ucd.BlockName(Ucd.Block(codePoint));
            if (block == "CJK Unified Ideographs" ||
                block == "CJK Compatibility Ideographs")
            {
                aaa = (ushort)(0xFB40u + (codePoint >> 15));
            }
            else
            {
                aaa = (ushort)(0xFB80u + (codePoint >> 15));
            }
        }
        else
        {
            aaa = (ushort)(0xFBC0u + (codePoint >> 15));
        }

        ushort bbb = (ushort)((codePoint & 0x7FFFu) | 0x8000u);
        output.Add(new Ce(aaa, 0x0020, 0x0002));
        output.Add(new Ce(bbb, 0, 0));
    }

    /// <summary>
    /// Build the collation element array following UTS #10 §7.2 (S2.1-S2.5).
    ///
    /// The scan is driven by a <c>consumed</c> map: a code point absorbed into
    /// a discontiguous contraction is removed from the string (S2.1.3), while
    /// non-starters that could not be absorbed stay in place and are revisited
    /// by the main loop (S2.5 proceeds past S only).
    /// </summary>
    private static List<Ce> BuildCes(List<uint> nfd)
    {
        var output = new List<Ce>();
        var consumed = new bool[nfd.Count];
        int i = 0;

        while (i < nfd.Count)
        {
            if (consumed[i])
            {
                ++i;
                continue;
            }

            // Gather the contiguous run of remaining code points starting at i.
            var seq = new List<uint>(nfd.Count - i);
            var idx = new List<int>(nfd.Count - i);
            for (int k = i; k < nfd.Count; ++k)
            {
                if (consumed[k]) continue;
                idx.Add(k);
                seq.Add(nfd[k]);
            }

            // S2.1: longest contiguous match.  Contractions take precedence
            // over a single mapping; a single with no table entry falls back
            // to the implicit weights of S2.2.
            var weights = new List<Ucd.CollationWeight>();
            int matchedElements = Ucd.CollationContraction(
                CollectionsMarshal.AsSpan(seq), weights, out int matched);
            int matchLen;
            List<Ucd.CollationWeight> currentWeights;

            if (matched >= 2 && matchedElements > 0)
            {
                matchLen = matched;
                currentWeights = weights;
            }
            else
            {
                var single = new List<Ucd.CollationWeight>();
                int n = Ucd.CollationWeights(seq[0], single);
                if (n > 0)
                {
                    matchLen = 1;
                    currentWeights = single;
                }
                else
                {
                    matchLen = 0;
                    currentWeights = new List<Ucd.CollationWeight>();
                }
            }

            if (matchLen == 0)
            {
                CeFromImplicit(output, nfd[i]);
                consumed[i] = true;
                ++i;
                continue;
            }

            // S2.1.1-S2.1.3: discontiguous non-starter extension.  Only
            // unblocked non-starters are considered; the lookahead stops at
            // the first starter or the first blocked non-starter.  Absorbed
            // code points are removed; the rest remain for later iterations.
            var current = new List<uint>(matchLen + 1);
            for (int k = 0; k < matchLen; ++k) current.Add(seq[k]);

            int blocking = 0;
            int after = matchLen < seq.Count ? idx[matchLen] : nfd.Count;
            int j = after;

            while (j < nfd.Count)
            {
                if (consumed[j])
                {
                    ++j;
                    continue;
                }

                uint codePoint = nfd[j];
                int combiningClass = Ucd.CombiningClass(codePoint);
                if (combiningClass == 0) break; // starter terminates the lookahead
                if (combiningClass <= blocking) break; // blocked non-starter

                current.Add(codePoint);
                var trial = new List<Ucd.CollationWeight>();
                int trialElements = Ucd.CollationContraction(
                    CollectionsMarshal.AsSpan(current), trial, out int trialMatched);

                if (trialElements > 0 && trialMatched == current.Count)
                {
                    currentWeights = trial;
                    consumed[j] = true; // S2.1.3: remove C
                    ++j;
                    continue; // absorbed C does not block
                }

                current.RemoveAt(current.Count - 1);
                // Not an extension: keep it for a later iteration and let it
                // contribute to the blocking context.
                if (combiningClass > blocking) blocking = combiningClass;
                ++j;
            }

            PushWeights(output, currentWeights);
            for (int k = 0; k < matchLen; ++k) consumed[idx[k]] = true;
            i = after;
        }

        return output;
    }

    /* --------------------------------------------------------------------- */
    /* Sort key formation (S3)                                                */
    /* --------------------------------------------------------------------- */

    private static ushort CaseWeight(uint codePoint) => Ucd.GeneralCategory(codePoint) switch
    {
        Ucd.Gc.Lu => 2,
        Ucd.Gc.Ll => 1,
        _ => 0
    };

    /// <summary>
    /// Build the three-level UCA sort key as a sequence of 16-bit units.
    /// Each level is terminated by a zero unit.  The optional case level is
    /// inserted after the secondary level.
    /// </summary>
    public static ushort[] SortKeyUnits(
        ReadOnlySpan<uint> codePoints, CollationOptions? options = null)
    {
        CollationOptions opts = options ?? CollationOptions.Default;
        List<uint> nfd = Nfd(codePoints);
        List<Ce> ces = BuildCes(nfd);
        int strength = (int)opts.Strength;

        var units = new List<ushort>(ces.Count * 3 + 3);

        // Level 1.
        foreach (Ce ce in ces)
        {
            if (ce.Primary != 0) units.Add(ce.Primary);
        }
        units.Add(0);

        // Level 2.
        if (strength >= (int)CollationStrength.Secondary)
        {
            foreach (Ce ce in ces)
            {
                if (ce.Secondary != 0) units.Add(ce.Secondary);
            }
            units.Add(0);
        }

        // Optional dedicated case level.
        if (opts.CaseLevel && strength >= (int)CollationStrength.Secondary)
        {
            foreach (uint cp in nfd) units.Add(CaseWeight(cp));
            units.Add(0);
        }

        // Level 3.
        if (strength >= (int)CollationStrength.Tertiary)
        {
            foreach (Ce ce in ces)
            {
                if (ce.Tertiary != 0) units.Add(ce.Tertiary);
            }
            units.Add(0);
        }

        return units.ToArray();
    }

    private static int CompareUnits(ushort[] a, ushort[] b)
    {
        int n = Math.Min(a.Length, b.Length);
        for (int i = 0; i < n; ++i)
        {
            if (a[i] < b[i]) return -1;
            if (a[i] > b[i]) return 1;
        }
        if (a.Length < b.Length) return -1;
        if (a.Length > b.Length) return 1;
        return 0;
    }

    private static int UcaCompare(uint[] a, uint[] b, CollationOptions options) =>
        CompareUnits(SortKeyUnits(a, options), SortKeyUnits(b, options));

    private static bool IsAsciiDigit(uint cp) => cp >= '0' && cp <= '9';

    private static int CompareNumericRun(
        uint[] a, ref int aPosition,
        uint[] b, ref int bPosition)
    {
        int ai = aPosition;
        int bi = bPosition;

        while (ai < a.Length && IsAsciiDigit(a[ai])) ++ai;
        while (bi < b.Length && IsAsciiDigit(b[bi])) ++bi;

        int aStart = aPosition;
        int bStart = bPosition;
        while (aStart < ai - 1 && a[aStart] == '0') ++aStart;
        while (bStart < bi - 1 && b[bStart] == '0') ++bStart;

        int aDigits = ai - aStart;
        int bDigits = bi - bStart;
        int result = 0;

        if (aDigits != bDigits)
        {
            result = aDigits < bDigits ? -1 : 1;
        }
        else
        {
            for (int i = 0; i < aDigits; ++i)
            {
                if (a[aStart + i] != b[bStart + i])
                {
                    result = a[aStart + i] < b[bStart + i] ? -1 : 1;
                    break;
                }
            }
        }

        aPosition = ai;
        bPosition = bi;
        return result;
    }

    private static int NextNonDigitRunEnd(uint[] cps, int start)
    {
        int end = start;
        while (end < cps.Length && !IsAsciiDigit(cps[end])) ++end;
        return end;
    }

    private static int NumericCompare(uint[] a, uint[] b, CollationOptions options)
    {
        int ai = 0;
        int bi = 0;

        while (ai < a.Length && bi < b.Length)
        {
            if (IsAsciiDigit(a[ai]) && IsAsciiDigit(b[bi]))
            {
                int result = CompareNumericRun(a, ref ai, b, ref bi);
                if (result != 0) return result;
            }
            else
            {
                int nextA = NextNonDigitRunEnd(a, ai);
                int nextB = NextNonDigitRunEnd(b, bi);
                int result = UcaCompare(
                    a[ai..nextA], b[bi..nextB], options);
                ai = nextA;
                bi = nextB;
                if (result != 0) return result;
            }
        }

        if (ai < a.Length) return 1;
        if (bi < b.Length) return -1;
        return 0;
    }

    /// <summary>
    /// Compare two scalar sequences with the configured collation options.
    /// This is the primitive used by the conformance tests and by
    /// <see cref="Compare"/>.
    /// </summary>
    public static int CompareScalars(
        ReadOnlySpan<uint> a, ReadOnlySpan<uint> b, CollationOptions? options = null)
    {
        CollationOptions opts = options ?? CollationOptions.Default;
        uint[] aa = a.ToArray();
        uint[] bb = b.ToArray();
        return opts.NumericOrdering
            ? NumericCompare(aa, bb, opts)
            : UcaCompare(aa, bb, opts);
    }

    /* --------------------------------------------------------------------- */
    /* TextString-facing API                                                  */
    /* --------------------------------------------------------------------- */

    private static UnicodoidStatus DecodeString(TextString str, out uint[] codePoints)
    {
        codePoints = Array.Empty<uint>();
        if (str is null) return UnicodoidStatus.InvalidParam;
        if (!str.EncodingSystem.HasUnicodeProfile) return UnicodoidStatus.Unsupported;

        var result = new List<uint>(str.Length);
        var codec = new Codec(str.EncodingSystem);
        ReadOnlySpan<byte> data = str.Bytes;
        int offset = 0;

        while (offset < data.Length)
        {
            UnicodoidStatus status = codec.DecodeChar(
                data[offset..], out CharacterValue native, out int read);

            if (status == UnicodoidStatus.NoCharacter)
            {
                if (read == 0) return UnicodoidStatus.InternalError;
                offset += read;
                continue;
            }
            if (status != UnicodoidStatus.Ok) return status;
            if (read == 0) return UnicodoidStatus.InternalError;

            if (!str.EncodingSystem.UnicodeProfile!.TryToUnicode(native, out uint scalar))
            {
                return UnicodoidStatus.UnmappableChar;
            }

            result.Add(scalar);
            offset += read;
        }

        codePoints = result.ToArray();
        return UnicodoidStatus.Ok;
    }

    /// <summary>Compare two strings.  Throws when a system lacks a Unicode profile.</summary>
    public static int Compare(
        TextString a,
        TextString b,
        CollationOptions? options = null)
    {
        UnicodoidStatus status = TryCompare(a, b, options, out int result);
        return status == UnicodoidStatus.Ok
            ? result
            : throw new UnicodoidException(status);
    }

    public static UnicodoidStatus TryCompare(
        TextString a,
        TextString b,
        CollationOptions? options,
        out int result)
    {
        result = 0;
        if (a is null || b is null) return UnicodoidStatus.InvalidParam;
        if (ReferenceEquals(a, b)) return UnicodoidStatus.Ok;

        CollationOptions opts = options ?? CollationOptions.Default;

        UnicodoidStatus status = DecodeString(a, out uint[] ac);
        if (status != UnicodoidStatus.Ok) return status;
        status = DecodeString(b, out uint[] bc);
        if (status != UnicodoidStatus.Ok) return status;

        result = CompareScalars(ac, bc, opts);
        return UnicodoidStatus.Ok;
    }

    /// <summary>
    /// Generate a binary sort key.  On
    /// <see cref="UnicodoidStatus.BufferTooSmall"/>, <paramref name="bytesWritten"/>
    /// receives the required size.
    /// </summary>
    public static UnicodoidStatus TryGetSortKey(
        TextString str,
        CollationOptions? options,
        Span<byte> key,
        out int bytesWritten)
    {
        bytesWritten = 0;
        if (str is null) return UnicodoidStatus.InvalidParam;

        CollationOptions opts = options ?? CollationOptions.Default;
        UnicodoidStatus status = DecodeString(str, out uint[] codePoints);
        if (status != UnicodoidStatus.Ok) return status;

        ushort[] units = SortKeyUnits(codePoints, opts);
        int required = units.Length * 2;
        bool rawLevel = opts.Strength >= CollationStrength.Quaternary;
        if (rawLevel) required += str.Length + 2;

        if (required > key.Length)
        {
            bytesWritten = required;
            return UnicodoidStatus.BufferTooSmall;
        }

        int offset = 0;
        foreach (ushort unit in units)
        {
            key[offset++] = (byte)(unit >> 8);
            key[offset++] = (byte)unit;
        }

        if (rawLevel)
        {
            str.Bytes.CopyTo(key[offset..]);
            offset += str.Length;
            key[offset++] = 0;
            key[offset++] = 0;
        }

        bytesWritten = offset;
        return UnicodoidStatus.Ok;
    }

    public static byte[] GetSortKey(TextString str, CollationOptions? options = null)
    {
        UnicodoidStatus status = TryGetSortKey(
            str, options, Span<byte>.Empty, out int required);
        if (status == UnicodoidStatus.Ok && required == 0)
        {
            return Array.Empty<byte>();
        }
        if (status != UnicodoidStatus.BufferTooSmall)
        {
            throw new UnicodoidException(status);
        }

        byte[] buffer = new byte[required];
        status = TryGetSortKey(str, options, buffer, out int written);
        if (status != UnicodoidStatus.Ok)
        {
            throw new UnicodoidException(status);
        }

        if (written != buffer.Length)
        {
            Array.Resize(ref buffer, written);
        }
        return buffer;
    }
}
