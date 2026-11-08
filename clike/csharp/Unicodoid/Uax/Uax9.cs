using System;
using System.Collections.Generic;

namespace Unicodoid;

/// <summary>
/// UAX #9 Unicode Bidirectional Algorithm (Unicode 18.0.0, Revision 52).
///
/// Paragraph direction (P2/P3), weak/neutral resolution, embedded levels
/// (X1..X10), visual reordering (L2) and mirroring (L3/L4).  The scalar
/// property data and the official BidiMirroring / BidiBrackets tables are
/// shared with the C reference implementation so the results agree.
/// </summary>
public static class Uax9
{
    /// <summary>Left-to-right paragraph base direction.</summary>
    public const int Ltr = 0;

    /// <summary>Right-to-left paragraph base direction.</summary>
    public const int Rtl = 1;

    /// <summary>First strong character, else LTR (P2/P3).</summary>
    public const int Auto = -1;

    private const int MaxDepth = 125;

    /// <summary>Bidirectional_Class of a scalar.</summary>
    public static byte BidiClass(uint cp) => Ucd.BidiClass(cp);

    private static bool IsStrongL(byte c) => c == Ucd.Bc.L;

    private static bool IsStrongR(byte c) => c is Ucd.Bc.R or Ucd.Bc.Al;

    private static bool IsStrong(byte c) => IsStrongL(c) || IsStrongR(c);

    private static bool IsExplicitOpen(byte c) =>
        c is Ucd.Bc.Lre or Ucd.Bc.Lro or Ucd.Bc.Rle or Ucd.Bc.Rlo;

    private static bool IsExplicitClose(byte c) => c == Ucd.Bc.Pdf;

    private static bool IsIsolate(byte c) =>
        c is Ucd.Bc.Fsi or Ucd.Bc.Lri or Ucd.Bc.Rli or Ucd.Bc.Pdi;

    private static bool IsTerm(byte c) => c is Ucd.Bc.B or Ucd.Bc.S;

    private static bool TryMirror(uint cp, out uint mirrored)
    {
        (uint Cp, uint Mirrored)[] table = Uax9Tables.Mirroring;
        int lo = 0;
        int hi = table.Length;
        while (lo < hi)
        {
            int mid = lo + (hi - lo) / 2;
            if (cp < table[mid].Cp)
            {
                hi = mid;
            }
            else if (cp > table[mid].Cp)
            {
                lo = mid + 1;
            }
            else
            {
                mirrored = table[mid].Mirrored;
                return true;
            }
        }
        mirrored = cp;
        return false;
    }

    private static bool TryBracket(uint cp, out uint counterpart, out char kind)
    {
        (uint Cp, uint Counterpart, char Kind)[] table = Uax9Tables.Brackets;
        int lo = 0;
        int hi = table.Length;
        while (lo < hi)
        {
            int mid = lo + (hi - lo) / 2;
            if (cp < table[mid].Cp)
            {
                hi = mid;
            }
            else if (cp > table[mid].Cp)
            {
                lo = mid + 1;
            }
            else
            {
                counterpart = table[mid].Counterpart;
                kind = table[mid].Kind;
                return true;
            }
        }
        counterpart = 0;
        kind = '\0';
        return false;
    }

    /// <summary>Paragraph embedding level (0 for LTR, 1 for RTL) per P2/P3.</summary>
    public static byte ParagraphLevel(ReadOnlySpan<uint> cps, int baseDir)
    {
        if (baseDir == Rtl) return 1;
        if (baseDir == Ltr) return 0;
        foreach (uint cp in cps)
        {
            byte c = Ucd.BidiClass(cp);
            if (IsStrongR(c)) return 1;
            if (IsStrongL(c)) return 0;
        }
        return 0;
    }

    private static byte[] Resolve(ReadOnlySpan<uint> cps, int baseDir)
    {
        int length = cps.Length;
        var cls = new byte[length];
        var lvl = new byte[length];
        for (int i = 0; i < length; ++i) cls[i] = Ucd.BidiClass(cps[i]);

        byte para = ParagraphLevel(cps, baseDir);
        for (int i = 0; i < length; ++i) lvl[i] = para;

        // W1: NSM -> preceding strong, else ON.
        for (int i = 0; i < length; ++i)
        {
            if (cls[i] != Ucd.Bc.Nsm) continue;
            byte resolved = Ucd.Bc.On;
            int j = i;
            while (j > 0)
            {
                --j;
                if (cls[j] is Ucd.Bc.Nsm or Ucd.Bc.Bn) continue;
                resolved = cls[j];
                break;
            }
            if (resolved == Ucd.Bc.Bn) resolved = Ucd.Bc.On;
            cls[i] = resolved;
        }

        // W2: search backwards skipping EN/ES/ET/AN/CS for R/AL.
        for (int i = 0; i < length; ++i)
        {
            if (cls[i] != Ucd.Bc.En) continue;
            for (int j = i; j > 0; --j)
            {
                byte b = cls[j - 1];
                if (b is Ucd.Bc.En or Ucd.Bc.Es or Ucd.Bc.Et or Ucd.Bc.An or Ucd.Bc.Cs)
                {
                    continue;
                }
                if (b is Ucd.Bc.R or Ucd.Bc.Al) cls[i] = Ucd.Bc.An;
                break;
            }
        }

        // W3: AL -> R.
        for (int i = 0; i < length; ++i)
        {
            if (cls[i] == Ucd.Bc.Al) cls[i] = Ucd.Bc.R;
        }

        // W4: single ES between EN -> EN; single CS between N -> N.
        for (int i = 1; i + 1 < length; ++i)
        {
            if (cls[i] == Ucd.Bc.Es && cls[i - 1] == Ucd.Bc.En && cls[i + 1] == Ucd.Bc.En)
            {
                cls[i] = Ucd.Bc.En;
            }
            if (cls[i] == Ucd.Bc.Cs)
            {
                bool lp = cls[i - 1] is Ucd.Bc.En or Ucd.Bc.An;
                bool rp = cls[i + 1] is Ucd.Bc.En or Ucd.Bc.An;
                if (lp && rp) cls[i] = Ucd.Bc.En;
            }
        }

        // W5: ET adjacent to EN (search skipping ET) -> EN.
        for (int i = 0; i < length; ++i)
        {
            if (cls[i] != Ucd.Bc.Et) continue;
            bool nearEn = false;
            int j = i;
            while (j > 0)
            {
                --j;
                if (cls[j] == Ucd.Bc.Et) continue;
                nearEn = cls[j] == Ucd.Bc.En;
                break;
            }
            if (!nearEn)
            {
                j = i + 1;
                while (j < length)
                {
                    if (cls[j] == Ucd.Bc.Et)
                    {
                        ++j;
                        continue;
                    }
                    nearEn = cls[j] == Ucd.Bc.En;
                    break;
                }
            }
            if (nearEn) cls[i] = Ucd.Bc.En;
        }

        // W6: remaining ES/ET/CS -> ON.
        for (int i = 0; i < length; ++i)
        {
            if (cls[i] is Ucd.Bc.Es or Ucd.Bc.Et or Ucd.Bc.Cs) cls[i] = Ucd.Bc.On;
        }

        // W7: EN -> L when preceded by L (search skipping WS/ES/ET).
        for (int i = 0; i < length; ++i)
        {
            if (cls[i] != Ucd.Bc.En) continue;
            for (int j = i; j > 0; --j)
            {
                byte b = cls[j - 1];
                if (b is Ucd.Bc.Ws or Ucd.Bc.Es or Ucd.Bc.Et) continue;
                if (b == Ucd.Bc.L) cls[i] = Ucd.Bc.L;
                break;
            }
        }

        // N0: paired brackets.
        {
            var stack = new int[256];
            int sp = 0;
            for (int i = 0; i < length; ++i)
            {
                if (!TryBracket(cps[i], out _, out char kind)) continue;
                if (kind == 'o')
                {
                    if (sp < stack.Length) stack[sp++] = i;
                }
                else
                {
                    if (sp <= 0) continue;
                    int found = -1;
                    for (int k = sp - 1; k >= 0; --k)
                    {
                        if (TryBracket(cps[stack[k]], out uint openCounterpart, out _) &&
                            openCounterpart == cps[i])
                        {
                            found = k;
                            break;
                        }
                    }
                    if (found < 0) continue;
                    int openIndex = stack[found];
                    sp = found;

                    byte innerType = Ucd.Bc.On;
                    for (int k = openIndex + 1; k < i; ++k)
                    {
                        byte c = cls[k];
                        if (IsStrongR(c))
                        {
                            innerType = Ucd.Bc.R;
                            break;
                        }
                        if (IsStrongL(c))
                        {
                            innerType = Ucd.Bc.L;
                            break;
                        }
                    }
                    if (innerType != Ucd.Bc.On)
                    {
                        cls[openIndex] = innerType;
                        cls[i] = innerType;
                    }
                }
            }
        }

        // N1: neutrals between same strong (or EN/AN as L) take that type.
        for (int i = 0; i < length; ++i)
        {
            if (cls[i] != Ucd.Bc.On) continue;
            byte left = para != 0 ? Ucd.Bc.R : Ucd.Bc.L;
            for (int j = i; j > 0; --j)
            {
                byte b = cls[j - 1];
                if (b == Ucd.Bc.On) continue;
                if (b is Ucd.Bc.En or Ucd.Bc.An) left = Ucd.Bc.L;
                else if (b is Ucd.Bc.L or Ucd.Bc.R) left = b;
                break;
            }
            byte right = para != 0 ? Ucd.Bc.R : Ucd.Bc.L;
            for (int j = i + 1; j < length; ++j)
            {
                byte b = cls[j];
                if (b == Ucd.Bc.On) continue;
                if (b is Ucd.Bc.En or Ucd.Bc.An) right = Ucd.Bc.L;
                else if (b is Ucd.Bc.L or Ucd.Bc.R) right = b;
                break;
            }
            if (left == right) cls[i] = left;
        }

        // N2: remaining neutrals take paragraph type.
        for (int i = 0; i < length; ++i)
        {
            if (cls[i] == Ucd.Bc.On) cls[i] = para != 0 ? Ucd.Bc.R : Ucd.Bc.L;
        }

        // I1/I2 + X1..X10.
        {
            int delta = 0;
            var embStack = new byte[MaxDepth];
            int embSp = 0;
            for (int i = 0; i < length; ++i)
            {
                byte c = cls[i];
                int curBase = para + 2 * delta;
                if (curBase < 0) curBase = 0;
                if (curBase > MaxDepth) curBase = MaxDepth;

                if (c is Ucd.Bc.Lre or Ucd.Bc.Rle)
                {
                    if (embSp < MaxDepth) embStack[embSp++] = (byte)(para + 2 * delta);
                    ++delta;
                    lvl[i] = (byte)curBase;
                    continue;
                }
                if (c is Ucd.Bc.Lro or Ucd.Bc.Rlo)
                {
                    if (embSp < MaxDepth) embStack[embSp++] = (byte)(para + 2 * delta);
                    ++delta;
                    lvl[i] = (byte)curBase;
                    continue;
                }
                if (c == Ucd.Bc.Pdf)
                {
                    if (embSp > 0) --embSp;
                    if (delta > 0) --delta;
                    lvl[i] = (byte)curBase;
                    continue;
                }
                if (IsIsolate(c))
                {
                    lvl[i] = (byte)curBase;
                    continue;
                }
                if (IsTerm(c))
                {
                    lvl[i] = para;
                    continue;
                }
                if (c == Ucd.Bc.Bn)
                {
                    lvl[i] = (byte)curBase;
                    continue;
                }

                bool odd = (curBase % 2) == 1;
                bool isR = c is Ucd.Bc.R or Ucd.Bc.An;
                bool isL = c is Ucd.Bc.L or Ucd.Bc.En;
                if (isR) lvl[i] = (byte)(odd ? curBase : curBase + 1);
                else if (isL) lvl[i] = (byte)(odd ? curBase + 1 : curBase);
                else lvl[i] = (byte)curBase;
            }
        }

        return lvl;
    }

    /// <summary>Resolve one embedding level per scalar.</summary>
    public static byte[] ResolveLevels(ReadOnlySpan<uint> cps, int baseDir) =>
        cps.IsEmpty ? Array.Empty<byte>() : Resolve(cps, baseDir);

    /// <summary>Reorder a paragraph to visual order (L2), applying mirroring (L3/L4).</summary>
    public static uint[] VisualOrder(ReadOnlySpan<uint> cps, int baseDir)
    {
        int length = cps.Length;
        if (length == 0) return Array.Empty<uint>();

        byte[] lvl = Resolve(cps, baseDir);
        byte para = ParagraphLevel(cps, baseDir);
        for (int i = 0; i < length; ++i)
        {
            byte c = Ucd.BidiClass(cps[i]);
            if (c is Ucd.Bc.B or Ucd.Bc.S) lvl[i] = para;
        }

        var order = new int[length];
        for (int i = 0; i < length; ++i) order[i] = i;

        byte maxLevel = 0;
        byte minOdd = 0;
        bool hasOdd = false;
        for (int i = 0; i < length; ++i)
        {
            if (lvl[i] > maxLevel) maxLevel = lvl[i];
            if ((lvl[i] % 2) == 1)
            {
                hasOdd = true;
                if (minOdd == 0 || lvl[i] < minOdd) minOdd = lvl[i];
            }
        }

        if (hasOdd)
        {
            for (int k = maxLevel; k >= minOdd; --k)
            {
                int i = 0;
                while (i < length)
                {
                    if (lvl[order[i]] < k)
                    {
                        ++i;
                        continue;
                    }
                    int j = i;
                    while (j < length && lvl[order[j]] >= k) ++j;
                    int a = i;
                    int b = j;
                    while (a < b)
                    {
                        --b;
                        if (a >= b) break;
                        (order[a], order[b]) = (order[b], order[a]);
                        ++a;
                    }
                    i = j;
                }
            }
        }

        var output = new uint[length];
        for (int i = 0; i < length; ++i)
        {
            uint cp = cps[order[i]];
            if ((lvl[order[i]] % 2) == 1 && TryMirror(cp, out uint mirrored))
            {
                cp = mirrored;
            }
            output[i] = cp;
        }
        return output;
    }

    /// <summary>True when <paramref name="cp"/> has a mirrored counterpart per BidiMirroring.txt.</summary>
    public static bool IsMirrored(uint cp) => TryMirror(cp, out _);

    /// <summary>Mirrored counterpart of <paramref name="cp"/>, or <paramref name="cp"/> when there is none.</summary>
    public static uint MirroredGlyph(uint cp) => TryMirror(cp, out uint mirrored) ? mirrored : cp;

    /// <summary>True when <paramref name="cp"/> is a paired bracket per BidiBrackets.txt.</summary>
    public static bool IsPairedBracket(uint cp) => TryBracket(cp, out _, out _);
}
