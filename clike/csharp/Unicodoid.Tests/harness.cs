// harness.cs -- C# harness for the ConvertUTF port.
// Covers well-formedness, strict/lenient behavior, exhaustion, and
// cross-encoding equivalence for the Unicode 18.0.0 ConvertUTF port.
using System;
using Unicodoid;

namespace Unicodoid.Tests;

internal static partial class Program
{
    private static void TestConvertUtf()
    {
        int sourcePos;
        int targetPos;

        // UTF-8 -> UTF-16
        Span<byte> utf8A = new byte[] { 0x41 };
        Span<ushort> utf16A = new ushort[1];
        ConversionResult result = ConvertUTF.ConvertUtf8ToUtf16(
            utf8A, 0, utf8A.Length, utf16A, 0, utf16A.Length, ConversionFlags.StrictConversion,
            out sourcePos, out targetPos);
        Check(result == ConversionResult.ConversionOk, "utf8 to utf16 simple: ok");
        Check(sourcePos == 1 && targetPos == 1, "utf8 to utf16 simple: positions");
        Check(utf16A[0] == 0x0041, "utf8 to utf16 simple: value");

        Span<byte> utf8Accent = new byte[] { 0xC3, 0xA9 };
        Span<ushort> utf16Accent = new ushort[1];
        result = ConvertUTF.ConvertUtf8ToUtf16(
            utf8Accent, 0, utf8Accent.Length, utf16Accent, 0, utf16Accent.Length,
            ConversionFlags.StrictConversion, out sourcePos, out targetPos);
        Check(result == ConversionResult.ConversionOk, "utf8 to utf16 e-acute: ok");
        Check(utf16Accent[0] == 0x00E9, "utf8 to utf16 e-acute: value");

        Span<byte> utf8Han = new byte[] { 0xE6, 0xBC, 0xA2 };
        Span<ushort> utf16Han = new ushort[1];
        result = ConvertUTF.ConvertUtf8ToUtf16(
            utf8Han, 0, utf8Han.Length, utf16Han, 0, utf16Han.Length,
            ConversionFlags.StrictConversion, out sourcePos, out targetPos);
        Check(result == ConversionResult.ConversionOk, "utf8 to utf16 han: ok");
        Check(utf16Han[0] == 0x6F22, "utf8 to utf16 han: value");

        Span<byte> utf8Emoji = new byte[] { 0xF0, 0x9F, 0x98, 0x80 };
        Span<ushort> utf16Emoji = new ushort[2];
        result = ConvertUTF.ConvertUtf8ToUtf16(
            utf8Emoji, 0, utf8Emoji.Length, utf16Emoji, 0, utf16Emoji.Length,
            ConversionFlags.StrictConversion, out sourcePos, out targetPos);
        Check(result == ConversionResult.ConversionOk, "utf8 to utf16 emoji: ok");
        Check(utf16Emoji[0] == 0xD83D && utf16Emoji[1] == 0xDE00, "utf8 to utf16 emoji: surrogate pair");

        Span<byte> utf8Empty = Array.Empty<byte>();
        Span<ushort> utf16Empty = Array.Empty<ushort>();
        result = ConvertUTF.ConvertUtf8ToUtf16(
            utf8Empty, 0, utf8Empty.Length, utf16Empty, 0, utf16Empty.Length,
            ConversionFlags.StrictConversion, out sourcePos, out targetPos);
        Check(result == ConversionResult.ConversionOk, "utf8 to utf16 empty: ok");
        Check(sourcePos == 0 && targetPos == 0, "utf8 to utf16 empty: positions");

        // Invalid UTF-8 sequences.
        Span<byte> utf8Overlong = new byte[] { 0xC0, 0x80 };
        Span<ushort> utf16Overlong = new ushort[1];
        result = ConvertUTF.ConvertUtf8ToUtf16(
            utf8Overlong, 0, utf8Overlong.Length, utf16Overlong, 0, utf16Overlong.Length,
            ConversionFlags.StrictConversion, out sourcePos, out targetPos);
        Check(result == ConversionResult.SourceIllegal, "utf8 to utf16 overlong: source illegal");
        Check(sourcePos == 0, "utf8 to utf16 overlong: source rewound");

        Span<byte> utf8Surrogate = new byte[] { 0xED, 0xA0, 0x80 };
        Span<ushort> utf16Surrogate = new ushort[1];
        result = ConvertUTF.ConvertUtf8ToUtf16(
            utf8Surrogate, 0, utf8Surrogate.Length, utf16Surrogate, 0, utf16Surrogate.Length,
            ConversionFlags.StrictConversion, out sourcePos, out targetPos);
        Check(result == ConversionResult.SourceIllegal, "utf8 to utf16 encoded surrogate: source illegal");

        Span<byte> utf8Truncated = new byte[] { 0xE2, 0x82 };
        Span<ushort> utf16Truncated = new ushort[1];
        result = ConvertUTF.ConvertUtf8ToUtf16(
            utf8Truncated, 0, utf8Truncated.Length, utf16Truncated, 0, utf16Truncated.Length,
            ConversionFlags.StrictConversion, out sourcePos, out targetPos);
        Check(result == ConversionResult.SourceExhausted, "utf8 to utf16 truncated: source exhausted");

        Span<byte> utf8TooHigh = new byte[] { 0xF4, 0x90, 0x80, 0x80 };
        Span<ushort> utf16TooHigh = new ushort[1];
        result = ConvertUTF.ConvertUtf8ToUtf16(
            utf8TooHigh, 0, utf8TooHigh.Length, utf16TooHigh, 0, utf16TooHigh.Length,
            ConversionFlags.StrictConversion, out sourcePos, out targetPos);
        Check(result == ConversionResult.SourceIllegal, "utf8 to utf16 above u10ffff: source illegal");

        result = ConvertUTF.ConvertUtf8ToUtf16(
            utf8TooHigh, 0, utf8TooHigh.Length, utf16TooHigh, 0, utf16TooHigh.Length,
            ConversionFlags.LenientConversion, out sourcePos, out targetPos);
        Check(result == ConversionResult.SourceIllegal, "utf8 to utf16 above u10ffff: source illegal even when lenient");

        // UTF-16 -> UTF-8
        Span<ushort> utf16A2 = new ushort[] { 0x0041 };
        Span<byte> utf8A2 = new byte[1];
        result = ConvertUTF.ConvertUtf16ToUtf8(
            utf16A2, 0, utf16A2.Length, utf8A2, 0, utf8A2.Length, ConversionFlags.StrictConversion,
            out sourcePos, out targetPos);
        Check(result == ConversionResult.ConversionOk, "utf16 to utf8 simple: ok");
        Check(utf8A2[0] == 0x41, "utf16 to utf8 simple: value");

        Span<ushort> utf16EmojiPair = new ushort[] { 0xD83D, 0xDE00 };
        Span<byte> utf8EmojiUtf16 = new byte[4];
        result = ConvertUTF.ConvertUtf16ToUtf8(
            utf16EmojiPair, 0, utf16EmojiPair.Length, utf8EmojiUtf16, 0, utf8EmojiUtf16.Length,
            ConversionFlags.StrictConversion, out sourcePos, out targetPos);
        Check(result == ConversionResult.ConversionOk, "utf16 to utf8 emoji: ok");
        Check(utf8EmojiUtf16[0] == 0xF0 && utf8EmojiUtf16[1] == 0x9F && utf8EmojiUtf16[2] == 0x98 && utf8EmojiUtf16[3] == 0x80,
            "utf16 to utf8 emoji: bytes");

        Span<ushort> utf16LoneHigh = new ushort[] { 0xD800 };
        Span<byte> utf8LoneHigh = new byte[3];
        result = ConvertUTF.ConvertUtf16ToUtf8(
            utf16LoneHigh, 0, utf16LoneHigh.Length, utf8LoneHigh, 0, utf8LoneHigh.Length,
            ConversionFlags.StrictConversion, out sourcePos, out targetPos);
        Check(result == ConversionResult.SourceExhausted, "utf16 to utf8 lone high: source exhausted");

        result = ConvertUTF.ConvertUtf16ToUtf8(
            utf16LoneHigh, 0, utf16LoneHigh.Length, utf8LoneHigh, 0, utf8LoneHigh.Length,
            ConversionFlags.LenientConversion, out sourcePos, out targetPos);
        Check(result == ConversionResult.SourceExhausted, "utf16 to utf8 lone high: lenient source exhausted");

        // UTF-8 -> UTF-32
        Span<uint> utf32Emoji = new uint[] { 0x1F600 };
        Span<byte> utf8From32 = new byte[4];
        result = ConvertUTF.ConvertUtf32ToUtf8(
            utf32Emoji, 0, utf32Emoji.Length, utf8From32, 0, utf8From32.Length,
            ConversionFlags.StrictConversion, out sourcePos, out targetPos);
        Check(result == ConversionResult.ConversionOk, "utf32 to utf8 emoji: ok");
        Check(utf8From32[0] == 0xF0 && utf8From32[1] == 0x9F && utf8From32[2] == 0x98 && utf8From32[3] == 0x80,
            "utf32 to utf8 emoji: bytes");

        Span<uint> utf32Max = new uint[] { 0x10FFFF };
        Span<byte> utf8Max = new byte[4];
        result = ConvertUTF.ConvertUtf32ToUtf8(
            utf32Max, 0, utf32Max.Length, utf8Max, 0, utf8Max.Length,
            ConversionFlags.StrictConversion, out sourcePos, out targetPos);
        Check(result == ConversionResult.ConversionOk, "utf32 to utf8 max: ok");
        Check(utf8Max[0] == 0xF4 && utf8Max[1] == 0x8F && utf8Max[2] == 0xBF && utf8Max[3] == 0xBF,
            "utf32 to utf8 max: bytes");

        Span<uint> utf32Surrogate = new uint[] { 0xD800 };
        Span<byte> utf8SurrogateFrom32 = new byte[3];
        result = ConvertUTF.ConvertUtf32ToUtf8(
            utf32Surrogate, 0, utf32Surrogate.Length, utf8SurrogateFrom32, 0, utf8SurrogateFrom32.Length,
            ConversionFlags.StrictConversion, out sourcePos, out targetPos);
        Check(result == ConversionResult.SourceIllegal, "utf32 to utf8 surrogate: source illegal");

        // UTF-8 -> UTF-32
        Span<byte> utf8Emoji2 = new byte[] { 0xF0, 0x9F, 0x98, 0x80 };
        Span<uint> utf32FromUtf8 = new uint[1];
        result = ConvertUTF.ConvertUtf8ToUtf32(
            utf8Emoji2, 0, utf8Emoji2.Length, utf32FromUtf8, 0, utf32FromUtf8.Length,
            ConversionFlags.StrictConversion, out sourcePos, out targetPos);
        Check(result == ConversionResult.ConversionOk, "utf8 to utf32 emoji: ok");
        Check(utf32FromUtf8[0] == 0x1F600, "utf8 to utf32 emoji: value");

        // UTF-16 -> UTF-32
        Span<uint> utf32FromUtf16 = new uint[1];
        result = ConvertUTF.ConvertUtf16ToUtf32(
            utf16Emoji, 0, utf16Emoji.Length, utf32FromUtf16, 0, utf32FromUtf16.Length,
            ConversionFlags.StrictConversion, out sourcePos, out targetPos);
        Check(result == ConversionResult.ConversionOk, "utf16 to utf32 emoji: ok");
        Check(utf32FromUtf16[0] == 0x1F600, "utf16 to utf32 emoji: value");

        Span<ushort> utf16LoneHigh2 = new ushort[] { 0xD800, 0x0041 };
        Span<uint> utf32FromUtf16b = new uint[2];
        result = ConvertUTF.ConvertUtf16ToUtf32(
            utf16LoneHigh2, 0, utf16LoneHigh2.Length, utf32FromUtf16b, 0, utf32FromUtf16b.Length,
            ConversionFlags.StrictConversion, out sourcePos, out targetPos);
        Check(result == ConversionResult.SourceIllegal, "utf16 to utf32 lone high: source illegal");

        // UTF-32 -> UTF-16
        Span<ushort> utf16FromUtf32 = new ushort[2];
        result = ConvertUTF.ConvertUtf32ToUtf16(
            utf32Emoji, 0, utf32Emoji.Length, utf16FromUtf32, 0, utf16FromUtf32.Length,
            ConversionFlags.StrictConversion, out sourcePos, out targetPos);
        Check(result == ConversionResult.ConversionOk, "utf32 to utf16 emoji: ok");
        Check(utf16FromUtf32[0] == 0xD83D && utf16FromUtf32[1] == 0xDE00, "utf32 to utf16 emoji: surrogate pair");

        Span<ushort> utf16FromSurrogate = new ushort[1];
        result = ConvertUTF.ConvertUtf32ToUtf16(
            utf32Surrogate, 0, utf32Surrogate.Length, utf16FromSurrogate, 0, utf16FromSurrogate.Length,
            ConversionFlags.StrictConversion, out sourcePos, out targetPos);
        Check(result == ConversionResult.SourceIllegal, "utf32 to utf16 surrogate: source illegal");

        // UTF-8 legality checks.
        Check(ConvertUTF.IsLegalUtf8Sequence(new byte[] { 0x41 }, 0, 1), "isLegal utf8 ascii");
        Check(!ConvertUTF.IsLegalUtf8Sequence(new byte[] { 0xC0, 0x80 }, 0, 2), "isLegal utf8 overlong");
        Check(!ConvertUTF.IsLegalUtf8Sequence(new byte[] { 0xE2, 0x82 }, 0, 2), "isLegal utf8 truncated");
        Check(ConvertUTF.IsLegalUtf8Sequence(new byte[] { 0xC3, 0xA9 }, 0, 2), "isLegal utf8 e-acute");
        Check(ConvertUTF.IsLegalUtf8Sequence(new byte[] { 0xF0, 0x9F, 0x98, 0x80 }, 0, 4), "isLegal utf8 emoji");
        Check(!ConvertUTF.IsLegalUtf8Sequence(new byte[] { 0xF4, 0x90, 0x80, 0x80 }, 0, 4), "isLegal utf8 above u10ffff");
    }
}
