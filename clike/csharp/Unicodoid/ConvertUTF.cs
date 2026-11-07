using System;

namespace Unicodoid;

/// <summary>Result of a conversion step, mirroring the ConvertUTF result codes.</summary>
public enum ConversionResult
{
    ConversionOk = 0,
    SourceExhausted = 1,
    TargetExhausted = 2,
    SourceIllegal = 3
}

/// <summary>
/// Strictness of a conversion.  When strict, irregular sequences and isolated
/// surrogates cause an error.  When lenient, both irregular sequences and
/// isolated surrogates are converted.
/// </summary>
public enum ConversionFlags
{
    StrictConversion = 0,
    LenientConversion = 1
}

/// <summary>
/// UTF-8, UTF-16 and UTF-32 conversion: the ConvertUTF port.
///
/// <para>
/// Ported from the discontinued W3C/Unicode ConvertUTF library and updated to
/// the Unicode 18.0.0 data set.  The routines convert between UTF-8, UTF-16 and
/// UTF-32 code unit sequences, report the first encountered problem through a
/// <see cref="ConversionResult"/>, and point the source and target cursors at
/// the end of the last successfully converted item.
/// </para>
/// <para>
/// Whether the strict or lenient flag is set, all illegal sequences cause an
/// error return: sequences such as <c>0xC0 0x80</c> in UTF-8, or values above
/// <see cref="UnicodeMaxLegalUtf32"/> in UTF-32.  Conformant callers must check
/// for illegal sequences.  When lenient, characters above the Unicode maximum
/// are converted to the replacement character; when strict they are an error.
/// </para>
/// </summary>
public static class ConvertUTF
{
    /// <summary>Official Unicode version targeted by this conversion module.</summary>
    public const string UnicodeVersion = "18.0.0";

    /// <summary>The Unicode replacement character (U+FFFD).</summary>
    public const uint UnicodeReplacementChar = 0x0000FFFD;

    /// <summary>The largest code point of the Basic Multilingual Plane (U+FFFF).</summary>
    public const uint UnicodeMaxBmp = 0x0000FFFF;

    /// <summary>The largest encoded code point (U+10FFFF).</summary>
    public const uint UnicodeMaxUtf16 = 0x0010FFFF;

    /// <summary>The largest 32-bit value addressable by the Unicode value space.</summary>
    public const uint UnicodeMaxUtf32 = 0x7FFFFFFF;

    /// <summary>The largest legal Unicode code point (U+10FFFF).</summary>
    public const uint UnicodeMaxLegalUtf32 = 0x0010FFFF;

    /// <summary>Start of the high surrogate range (U+D800).</summary>
    public const uint UniSurHighStart = 0x0000D800;

    /// <summary>End of the high surrogate range (U+DBFF).</summary>
    public const uint UniSurHighEnd = 0x0000DBFF;

    /// <summary>Start of the low surrogate range (U+DC00).</summary>
    public const uint UniSurLowStart = 0x0000DC00;

    /// <summary>End of the low surrogate range (U+DFFF).</summary>
    public const uint UniSurLowEnd = 0x0000DFFF;

    /// <summary>Shift used to split a code point into surrogate halves.</summary>
    public const int HalfShift = 10;

    /// <summary>Half-base offset for supplementary code points (U+10000).</summary>
    public const uint HalfBase = 0x0010000;

    /// <summary>Mask holding the low part of a supplementary code point.</summary>
    public const uint HalfMask = 0x003FF;

    /// <summary>
    /// Number of trailing bytes expected in a UTF-8 sequence, indexed by the
    /// first byte.  A sequence of N bytes has N - 1 trailing bytes.
    /// </summary>
    private static readonly byte[] TrailingBytesForUtf8 = BuildTrailingBytesForUtf8();

    /// <summary>Magic offsets subtracted from accumulated UTF-8 values.</summary>
    private static readonly uint[] OffsetsFromUtf8 =
    {
        0x00000000u, 0x00003080u, 0x000E2080u, 0x03C82080u, 0xFA082080u, 0x82082080u
    };

    /// <summary>First byte marks for UTF-8 sequences of one to six bytes.</summary>
    private static readonly byte[] FirstByteMark = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

    private static byte[] BuildTrailingBytesForUtf8()
    {
        var trailing = new byte[256];
        for (int i = 0; i < 256; ++i)
        {
            byte b = (byte)i;
            trailing[i] = b < 0x80 ? (byte)0
                : b < 0xC2 ? (byte)0
                : b < 0xE0 ? (byte)1
                : b < 0xF0 ? (byte)2
                : b <= 0xF4 ? (byte)3
                : (byte)4;
        }
        return trailing;
    }

    /// <summary>
    /// Convert a UTF-8 source buffer into a UTF-16 target buffer.  The source
    /// range is <c>source[sourceStart .. sourceEnd]</c> and the target range is
    /// <c>target[targetStart .. targetEnd]</c>; both cursors start at the start
    /// of their range and are updated to the end of the last successfully
    /// converted item.
    /// </summary>
    /// <returns>The conversion result; the source and target cursors are always
    /// updated before returning.</returns>
    public static ConversionResult ConvertUtf8ToUtf16(
        ReadOnlySpan<byte> source,
        int sourceStart,
        int sourceEnd,
        Span<ushort> target,
        int targetStart,
        int targetEnd,
        ConversionFlags flags,
        out int sourcePos,
        out int targetPos)
    {
        sourcePos = sourceStart;
        targetPos = targetStart;
        ConversionResult result = ConversionResult.ConversionOk;
        int sourceIdx = sourceStart;
        int targetIdx = targetStart;

        while (sourceIdx < sourceEnd)
        {
            if (targetIdx >= targetEnd)
            {
                result = ConversionResult.TargetExhausted;
                break;
            }

            int extraBytesToRead = TrailingBytesForUtf8[source[sourceIdx]];
            if (sourceIdx + extraBytesToRead >= sourceEnd)
            {
                result = ConversionResult.SourceExhausted;
                break;
            }

            if (!IsLegalUtf8(source, sourceIdx, extraBytesToRead + 1))
            {
                result = ConversionResult.SourceIllegal;
                break;
            }

            uint ch = 0;
            int bytesToRead = extraBytesToRead + 1;
            while (true)
            {
                ch += source[sourceIdx];
                sourceIdx++;
                bytesToRead--;
                if (bytesToRead > 0)
                {
                    ch <<= 6;
                }
                else
                {
                    break;
                }
            }
            ch -= OffsetsFromUtf8[extraBytesToRead];

            if (targetIdx >= targetEnd)
            {
                sourceIdx -= (extraBytesToRead + 1);
                result = ConversionResult.TargetExhausted;
                break;
            }

            if (ch <= UnicodeMaxBmp)
            {
                if (ch >= UniSurHighStart && ch <= UniSurLowEnd)
                {
                    if (flags == ConversionFlags.StrictConversion)
                    {
                        sourceIdx -= (extraBytesToRead + 1);
                        result = ConversionResult.SourceIllegal;
                        break;
                    }

                    target[targetIdx++] = (ushort)UnicodeReplacementChar;
                }
                else
                {
                    target[targetIdx++] = (ushort)ch;
                }
            }
            else if (ch > UnicodeMaxUtf16)
            {
                if (flags == ConversionFlags.StrictConversion)
                {
                    sourceIdx -= (extraBytesToRead + 1);
                    result = ConversionResult.SourceIllegal;
                    break;
                }

                target[targetIdx++] = (ushort)UnicodeReplacementChar;
            }
            else
            {
                if (targetIdx + 1 >= targetEnd)
                {
                    sourceIdx -= (extraBytesToRead + 1);
                    result = ConversionResult.TargetExhausted;
                    break;
                }

                ch -= HalfBase;
                target[targetIdx++] = (ushort)((ch >> HalfShift) + UniSurHighStart);
                target[targetIdx++] = (ushort)((ch & HalfMask) + UniSurLowStart);
            }
        }

        sourcePos = sourceIdx;
        targetPos = targetIdx;
        return result;
    }

    /// <summary>
    /// Convert a UTF-16 source buffer into a UTF-8 target buffer.
    /// </summary>
    public static ConversionResult ConvertUtf16ToUtf8(
        ReadOnlySpan<ushort> source,
        int sourceStart,
        int sourceEnd,
        Span<byte> target,
        int targetStart,
        int targetEnd,
        ConversionFlags flags,
        out int sourcePos,
        out int targetPos)
    {
        sourcePos = sourceStart;
        targetPos = targetStart;
        ConversionResult result = ConversionResult.ConversionOk;
        int sourceIdx = sourceStart;
        int targetIdx = targetStart;

        while (sourceIdx < sourceEnd)
        {
            int oldSource = sourceIdx;
            uint ch = source[sourceIdx];
            sourceIdx++;

            if (ch >= UniSurHighStart && ch <= UniSurHighEnd)
            {
                if (sourceIdx < sourceEnd)
                {
                    uint ch2 = source[sourceIdx];
                    if (ch2 >= UniSurLowStart && ch2 <= UniSurLowEnd)
                    {
                        ch = ((ch - UniSurHighStart) << HalfShift) + (ch2 - UniSurLowStart) + HalfBase;
                        sourceIdx++;
                    }
                    else if (flags == ConversionFlags.StrictConversion)
                    {
                        sourceIdx--;
                        result = ConversionResult.SourceIllegal;
                        break;
                    }
                }
                else
                {
                    sourceIdx--;
                    result = ConversionResult.SourceExhausted;
                    break;
                }
            }
            else if (flags == ConversionFlags.StrictConversion)
            {
                if (ch >= UniSurLowStart && ch <= UniSurLowEnd)
                {
                    sourceIdx--;
                    result = ConversionResult.SourceIllegal;
                    break;
                }
            }

            uint bytesToWrite = 0;
            if (ch < 0x80)
            {
                bytesToWrite = 1;
            }
            else if (ch < 0x800)
            {
                bytesToWrite = 2;
            }
            else if (ch < 0x10000)
            {
                bytesToWrite = 3;
            }
            else if (ch < 0x110000)
            {
                bytesToWrite = 4;
            }
            else
            {
                bytesToWrite = 3;
                ch = UnicodeReplacementChar;
                result = ConversionResult.SourceIllegal;
            }

            if (targetIdx + bytesToWrite > targetEnd)
            {
                sourceIdx = oldSource;
                targetIdx -= (int)bytesToWrite;
                result = ConversionResult.TargetExhausted;
                break;
            }

            int n = (int)bytesToWrite;
            for (int j = 0; j < n; ++j)
            {
                int shift = 6 * (n - 1 - j);
                byte b = j == 0
                    ? (byte)((ch >> shift) | FirstByteMark[(int)bytesToWrite])
                    : (byte)(((ch >> shift) & 0x3F) | 0x80);
                target[targetIdx + j] = b;
            }
            targetIdx += n;
        }

        sourcePos = sourceIdx;
        targetPos = targetIdx;
        return result;
    }

    /// <summary>
    /// Convert a UTF-8 source buffer into a UTF-32 target buffer.
    /// </summary>
    public static ConversionResult ConvertUtf8ToUtf32(
        ReadOnlySpan<byte> source,
        int sourceStart,
        int sourceEnd,
        Span<uint> target,
        int targetStart,
        int targetEnd,
        ConversionFlags flags,
        out int sourcePos,
        out int targetPos)
    {
        sourcePos = sourceStart;
        targetPos = targetStart;
        ConversionResult result = ConversionResult.ConversionOk;
        int sourceIdx = sourceStart;
        int targetIdx = targetStart;

        while (sourceIdx < sourceEnd)
        {
            int extraBytesToRead = TrailingBytesForUtf8[source[sourceIdx]];
            if (sourceIdx + extraBytesToRead >= sourceEnd)
            {
                result = ConversionResult.SourceExhausted;
                break;
            }

            if (!IsLegalUtf8(source, sourceIdx, extraBytesToRead + 1))
            {
                result = ConversionResult.SourceIllegal;
                break;
            }

            uint ch = 0;
            int bytesToRead = extraBytesToRead + 1;
            while (true)
            {
                ch += source[sourceIdx];
                sourceIdx++;
                bytesToRead--;
                if (bytesToRead > 0)
                {
                    ch <<= 6;
                }
                else
                {
                    break;
                }
            }
            ch -= OffsetsFromUtf8[extraBytesToRead];

            if (targetIdx >= targetEnd)
            {
                sourceIdx -= (extraBytesToRead + 1);
                result = ConversionResult.TargetExhausted;
                break;
            }

            if (ch <= UnicodeMaxLegalUtf32)
            {
                if (ch >= UniSurHighStart && ch <= UniSurLowEnd)
                {
                    if (flags == ConversionFlags.StrictConversion)
                    {
                        sourceIdx -= (extraBytesToRead + 1);
                        result = ConversionResult.SourceIllegal;
                        break;
                    }

                    target[targetIdx++] = UnicodeReplacementChar;
                }
                else
                {
                    target[targetIdx++] = ch;
                }
            }
            else
            {
                result = ConversionResult.SourceIllegal;
                target[targetIdx++] = UnicodeReplacementChar;
            }
        }

        sourcePos = sourceIdx;
        targetPos = targetIdx;
        return result;
    }

    /// <summary>
    /// Convert a UTF-32 source buffer into a UTF-8 target buffer.
    /// </summary>
    public static ConversionResult ConvertUtf32ToUtf8(
        ReadOnlySpan<uint> source,
        int sourceStart,
        int sourceEnd,
        Span<byte> target,
        int targetStart,
        int targetEnd,
        ConversionFlags flags,
        out int sourcePos,
        out int targetPos)
    {
        sourcePos = sourceStart;
        targetPos = targetStart;
        ConversionResult result = ConversionResult.ConversionOk;
        int sourceIdx = sourceStart;
        int targetIdx = targetStart;

        while (sourceIdx < sourceEnd)
        {
            uint ch = source[sourceIdx];
            sourceIdx++;

            if (flags == ConversionFlags.StrictConversion)
            {
                if (ch >= UniSurHighStart && ch <= UniSurLowEnd)
                {
                    sourceIdx--;
                    result = ConversionResult.SourceIllegal;
                    break;
                }
            }

            uint bytesToWrite = 0;
            if (ch < 0x80)
            {
                bytesToWrite = 1;
            }
            else if (ch < 0x800)
            {
                bytesToWrite = 2;
            }
            else if (ch < 0x10000)
            {
                bytesToWrite = 3;
            }
            else if (ch <= UnicodeMaxLegalUtf32)
            {
                bytesToWrite = 4;
            }
            else
            {
                bytesToWrite = 3;
                ch = UnicodeReplacementChar;
                result = ConversionResult.SourceIllegal;
            }

            if (targetIdx + bytesToWrite > targetEnd)
            {
                sourceIdx--;
                targetIdx -= (int)bytesToWrite;
                result = ConversionResult.TargetExhausted;
                break;
            }

            int n = (int)bytesToWrite;
            for (int j = 0; j < n; ++j)
            {
                int shift = 6 * (n - 1 - j);
                byte b = j == 0
                    ? (byte)((ch >> shift) | FirstByteMark[(int)bytesToWrite])
                    : (byte)(((ch >> shift) & 0x3F) | 0x80);
                target[targetIdx + j] = b;
            }
            targetIdx += n;
        }

        sourcePos = sourceIdx;
        targetPos = targetIdx;
        return result;
    }

    /// <summary>
    /// Convert a UTF-16 source buffer into a UTF-32 target buffer.
    /// </summary>
    public static ConversionResult ConvertUtf16ToUtf32(
        ReadOnlySpan<ushort> source,
        int sourceStart,
        int sourceEnd,
        Span<uint> target,
        int targetStart,
        int targetEnd,
        ConversionFlags flags,
        out int sourcePos,
        out int targetPos)
    {
        sourcePos = sourceStart;
        targetPos = targetStart;
        ConversionResult result = ConversionResult.ConversionOk;
        int sourceIdx = sourceStart;
        int targetIdx = targetStart;

        while (sourceIdx < sourceEnd)
        {
            int oldSource = sourceIdx;
            uint ch = source[sourceIdx];
            sourceIdx++;

            if (ch >= UniSurHighStart && ch <= UniSurHighEnd)
            {
                if (sourceIdx < sourceEnd)
                {
                    uint ch2 = source[sourceIdx];
                    if (ch2 >= UniSurLowStart && ch2 <= UniSurLowEnd)
                    {
                        ch = ((ch - UniSurHighStart) << HalfShift) + (ch2 - UniSurLowStart) + HalfBase;
                        sourceIdx++;
                    }
                    else if (flags == ConversionFlags.StrictConversion)
                    {
                        sourceIdx--;
                        result = ConversionResult.SourceIllegal;
                        break;
                    }
                }
                else
                {
                    sourceIdx--;
                    result = ConversionResult.SourceExhausted;
                    break;
                }
            }
            else if (flags == ConversionFlags.StrictConversion)
            {
                if (ch >= UniSurLowStart && ch <= UniSurLowEnd)
                {
                    sourceIdx--;
                    result = ConversionResult.SourceIllegal;
                    break;
                }
            }

            if (targetIdx >= targetEnd)
            {
                sourceIdx = oldSource;
                result = ConversionResult.TargetExhausted;
                break;
            }

            target[targetIdx++] = ch;
        }

        sourcePos = sourceIdx;
        targetPos = targetIdx;
        return result;
    }

    /// <summary>
    /// Convert a UTF-32 source buffer into a UTF-16 target buffer.
    /// </summary>
    public static ConversionResult ConvertUtf32ToUtf16(
        ReadOnlySpan<uint> source,
        int sourceStart,
        int sourceEnd,
        Span<ushort> target,
        int targetStart,
        int targetEnd,
        ConversionFlags flags,
        out int sourcePos,
        out int targetPos)
    {
        sourcePos = sourceStart;
        targetPos = targetStart;
        ConversionResult result = ConversionResult.ConversionOk;
        int sourceIdx = sourceStart;
        int targetIdx = targetStart;

        while (sourceIdx < sourceEnd)
        {
            uint ch = source[sourceIdx];
            sourceIdx++;

            if (flags == ConversionFlags.StrictConversion)
            {
                if (ch >= UniSurHighStart && ch <= UniSurLowEnd)
                {
                    sourceIdx--;
                    result = ConversionResult.SourceIllegal;
                    break;
                }
            }

            if (ch <= UnicodeMaxBmp)
            {
                if (ch >= UniSurHighStart && ch <= UniSurLowEnd)
                {
                    if (flags == ConversionFlags.StrictConversion)
                    {
                        sourceIdx--;
                        result = ConversionResult.SourceIllegal;
                        break;
                    }

                    target[targetIdx++] = (ushort)UnicodeReplacementChar;
                }
                else
                {
                    target[targetIdx++] = (ushort)ch;
                }
            }
            else if (ch > UnicodeMaxUtf16)
            {
                if (flags == ConversionFlags.StrictConversion)
                {
                    sourceIdx--;
                    result = ConversionResult.SourceIllegal;
                    break;
                }

                target[targetIdx++] = (ushort)UnicodeReplacementChar;
            }
            else
            {
                if (targetIdx + 1 >= targetEnd)
                {
                    sourceIdx--;
                    result = ConversionResult.TargetExhausted;
                    break;
                }

                ch -= HalfBase;
                target[targetIdx++] = (ushort)((ch >> HalfShift) + UniSurHighStart);
                target[targetIdx++] = (ushort)((ch & HalfMask) + UniSurLowStart);
            }
        }

        sourcePos = sourceIdx;
        targetPos = targetIdx;
        return result;
    }

    /// <summary>
    /// Returns whether the UTF-8 sequence starting at
    /// <c>source[sourceStart .. sourceEnd)</c> is legal.
    /// </summary>
    public static bool IsLegalUtf8Sequence(ReadOnlySpan<byte> source, int sourceStart, int sourceEnd)
    {
        if (sourceStart >= sourceEnd)
        {
            return false;
        }

        int length = TrailingBytesForUtf8[source[sourceStart]] + 1;
        if (sourceStart + length > sourceEnd)
        {
            return false;
        }

        return IsLegalUtf8(source, sourceStart, length);
    }

    private static bool IsLegalUtf8(ReadOnlySpan<byte> source, int sourceStart, int length)
    {
        if (length < 1 || length > 4)
        {
            return false;
        }

        uint first = source[sourceStart];
        if (length == 1)
        {
            if (first >= 0x80 && first < 0xC2) { return false; }
            if (first > 0xF4) { return false; }
            return true;
        }

        for (int i = sourceStart + 1; i < sourceStart + length; ++i)
        {
            if (source[i] < 0x80 || source[i] > 0xBF) { return false; }
        }

        uint last = source[sourceStart + length - 1];
        uint second = source[sourceStart + 1];
        bool legal = true;
        switch (first)
        {
            case 0xE0u: legal = second >= 0xA0; break;
            case 0xEDu: legal = second <= 0x9F; break;
            case 0xF0u: legal = second >= 0x90; break;
            case 0xF4u: legal = second <= 0x8F; break;
            default: legal = second >= 0x80; break;
        }

        if (!legal) { return false; }
        return first <= 0xF4;
    }
}
