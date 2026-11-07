using System;

namespace Unicodoid.Systems;

internal static class Utf16Codec
{
    internal static readonly UnicodeProfile Profile = new(
        new Version(18, 0, 0),
        UaxFlags.Bidi9 | UaxFlags.EastAsianWidth11 | UaxFlags.LineBreak14 |
        UaxFlags.Normalization15 | UaxFlags.Segmentation29 | UaxFlags.Ucd44,
        UtsFlags.Uca10 | UtsFlags.Regex18 | UtsFlags.Ldml35 | UtsFlags.Security39,
        UtrFlags.EncodingModel17,
        ToUnicode,
        FromUnicode);

    private static uint? ToUnicode(CharacterValue native) =>
        native.Value <= UnicodeScalars.MaxScalar &&
        (native.Value < UnicodeScalars.SurrogateMin || native.Value > UnicodeScalars.SurrogateMax)
            ? (uint)native.Value
            : null;

    private static CharacterValue? FromUnicode(uint scalar) =>
        UnicodeScalars.IsValid(scalar) ? new CharacterValue(scalar) : null;

    private static bool IsScalar(CharacterValue ch) =>
        ch.Value <= UnicodeScalars.MaxScalar && UnicodeScalars.IsValid((uint)ch.Value);

    internal static UnicodoidStatus Encode(
        CharacterValue ch,
        Span<byte> destination,
        out int bytesWritten,
        bool littleEndian)
    {
        bytesWritten = 0;
        if (!IsScalar(ch)) return UnicodoidStatus.UnmappableChar;

        if (ch.Value <= 0xFFFF)
        {
            if (destination.Length < 2) return UnicodoidStatus.BufferTooSmall;
            WriteU16(destination, (ushort)ch.Value, littleEndian);
            bytesWritten = 2;
        }
        else
        {
            if (destination.Length < 4) return UnicodoidStatus.BufferTooSmall;
            uint value = (uint)(ch.Value - 0x10000);
            ushort high = (ushort)(0xD800 + (value >> 10));
            ushort low = (ushort)(0xDC00 + (value & 0x3FF));
            WriteU16(destination, high, littleEndian);
            WriteU16(destination[2..], low, littleEndian);
            bytesWritten = 4;
        }
        return UnicodoidStatus.Ok;
    }

    internal static UnicodoidStatus Decode(
        ReadOnlySpan<byte> source,
        out CharacterValue ch,
        out int bytesRead,
        bool littleEndian)
    {
        ch = CharacterValue.Invalid;
        bytesRead = 0;
        if (source.Length < 2) return UnicodoidStatus.IncompleteSequence;

        ushort w1 = ReadU16(source, littleEndian);
        if (w1 >= 0xD800 && w1 <= 0xDBFF)
        {
            if (source.Length < 4) return UnicodoidStatus.IncompleteSequence;
            ushort w2 = ReadU16(source[2..], littleEndian);
            if (w2 < 0xDC00 || w2 > 0xDFFF) return UnicodoidStatus.InvalidCodeUnit;
            uint value = (((uint)(w1 - 0xD800) << 10) | (uint)(w2 - 0xDC00)) + 0x10000;
            ch = value;
            bytesRead = 4;
            return UnicodoidStatus.Ok;
        }
        if (w1 >= 0xDC00 && w1 <= 0xDFFF) return UnicodoidStatus.InvalidCodeUnit;

        ch = w1;
        bytesRead = 2;
        return UnicodoidStatus.Ok;
    }

    internal static UnicodoidStatus GetByteCount(
        ReadOnlySpan<CharacterValue> chars,
        out int byteCount)
    {
        byteCount = 0;
        int total = 0;
        foreach (CharacterValue ch in chars)
        {
            if (!IsScalar(ch)) return UnicodoidStatus.UnmappableChar;
            total += ch.Value <= 0xFFFF ? 2 : 4;
        }
        byteCount = total;
        return UnicodoidStatus.Ok;
    }

    private static void WriteU16(Span<byte> destination, ushort value, bool littleEndian)
    {
        if (littleEndian)
        {
            destination[0] = (byte)value;
            destination[1] = (byte)(value >> 8);
        }
        else
        {
            destination[0] = (byte)(value >> 8);
            destination[1] = (byte)value;
        }
    }

    private static ushort ReadU16(ReadOnlySpan<byte> source, bool littleEndian) =>
        littleEndian
            ? (ushort)(source[0] | (source[1] << 8))
            : (ushort)((source[0] << 8) | source[1]);
}

/// <summary>UTF-16 Little Endian.</summary>
public sealed class Utf16LeSystem : EncodingSystemBase
{
    public static readonly Utf16LeSystem Instance = new();
    private Utf16LeSystem() { }

    public override string Id => "org.unicode.utf16le";
    public override string Name => "UTF-16LE";
    public override string Version => "1.0.0";
    public override string Author => "Unicode Consortium";
    public override string Description => "UTF-16 Little Endian";
    public override int CodeUnitBits => 16;
    public override int MinBytesPerChar => 2;
    public override int MaxBytesPerChar => 4;
    public override CharModel Model => CharModel.VariableWidth;
    public override Endianness Endianness => Endianness.LittleEndian;
    public override CompatibilityLevel CompatibilityLevel => CompatibilityLevel.Full;
    public override CharacterValue MaxCharValue => UnicodeScalars.MaxScalar;
    public override CharacterValue ReplacementChar => UnicodeScalars.Replacement;
    public override UnicodeProfile UnicodeProfile => Utf16Codec.Profile;

    public override UnicodoidStatus EncodeChar(object? state, CharacterValue ch, Span<byte> destination, out int bytesWritten) =>
        Utf16Codec.Encode(ch, destination, out bytesWritten, true);

    public override UnicodoidStatus DecodeChar(object? state, ReadOnlySpan<byte> source, out CharacterValue ch, out int bytesRead) =>
        Utf16Codec.Decode(source, out ch, out bytesRead, true);

    public override UnicodoidStatus GetByteCount(ReadOnlySpan<CharacterValue> chars, out int byteCount) =>
        Utf16Codec.GetByteCount(chars, out byteCount);
}

/// <summary>UTF-16 Big Endian.</summary>
public sealed class Utf16BeSystem : EncodingSystemBase
{
    public static readonly Utf16BeSystem Instance = new();
    private Utf16BeSystem() { }

    public override string Id => "org.unicode.utf16be";
    public override string Name => "UTF-16BE";
    public override string Version => "1.0.0";
    public override string Author => "Unicode Consortium";
    public override string Description => "UTF-16 Big Endian";
    public override int CodeUnitBits => 16;
    public override int MinBytesPerChar => 2;
    public override int MaxBytesPerChar => 4;
    public override CharModel Model => CharModel.VariableWidth;
    public override Endianness Endianness => Endianness.BigEndian;
    public override CompatibilityLevel CompatibilityLevel => CompatibilityLevel.Full;
    public override CharacterValue MaxCharValue => UnicodeScalars.MaxScalar;
    public override CharacterValue ReplacementChar => UnicodeScalars.Replacement;
    public override UnicodeProfile UnicodeProfile => Utf16Codec.Profile;

    public override UnicodoidStatus EncodeChar(object? state, CharacterValue ch, Span<byte> destination, out int bytesWritten) =>
        Utf16Codec.Encode(ch, destination, out bytesWritten, false);

    public override UnicodoidStatus DecodeChar(object? state, ReadOnlySpan<byte> source, out CharacterValue ch, out int bytesRead) =>
        Utf16Codec.Decode(source, out ch, out bytesRead, false);

    public override UnicodoidStatus GetByteCount(ReadOnlySpan<CharacterValue> chars, out int byteCount) =>
        Utf16Codec.GetByteCount(chars, out byteCount);
}
