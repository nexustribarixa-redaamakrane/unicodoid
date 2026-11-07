using System;

namespace Unicodoid.Systems;

internal static class Utf32Codec
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
        native.Value <= UnicodeScalars.MaxScalar && UnicodeScalars.IsValid((uint)native.Value)
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
        if (destination.Length < 4) return UnicodoidStatus.BufferTooSmall;
        WriteU32(destination, (uint)ch.Value, littleEndian);
        bytesWritten = 4;
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
        if (source.Length < 4) return UnicodoidStatus.IncompleteSequence;
        uint value = ReadU32(source, littleEndian);
        if (!UnicodeScalars.IsValid(value)) return UnicodoidStatus.InvalidCodeUnit;
        ch = value;
        bytesRead = 4;
        return UnicodoidStatus.Ok;
    }

    internal static UnicodoidStatus GetByteCount(
        ReadOnlySpan<CharacterValue> chars,
        out int byteCount)
    {
        byteCount = 0;
        foreach (CharacterValue ch in chars)
        {
            if (!IsScalar(ch)) return UnicodoidStatus.UnmappableChar;
        }
        byteCount = checked(chars.Length * 4);
        return UnicodoidStatus.Ok;
    }

    private static void WriteU32(Span<byte> destination, uint value, bool littleEndian)
    {
        if (littleEndian)
        {
            destination[0] = (byte)value;
            destination[1] = (byte)(value >> 8);
            destination[2] = (byte)(value >> 16);
            destination[3] = (byte)(value >> 24);
        }
        else
        {
            destination[0] = (byte)(value >> 24);
            destination[1] = (byte)(value >> 16);
            destination[2] = (byte)(value >> 8);
            destination[3] = (byte)value;
        }
    }

    private static uint ReadU32(ReadOnlySpan<byte> source, bool littleEndian) =>
        littleEndian
            ? (uint)(source[0] | (source[1] << 8) | (source[2] << 16) | (source[3] << 24))
            : (uint)((source[0] << 24) | (source[1] << 16) | (source[2] << 8) | source[3]);
}

/// <summary>Fixed-width 32-bit Unicode Transformation Format (Little Endian).</summary>
public sealed class Utf32LeSystem : EncodingSystemBase
{
    public static readonly Utf32LeSystem Instance = new();
    private Utf32LeSystem() { }

    public override string Id => "org.unicode.utf32le";
    public override string Name => "UTF-32LE";
    public override string Version => "1.0.0";
    public override string Author => "Unicode Consortium";
    public override string Description => "Fixed-width 32-bit Unicode Transformation Format (Little Endian)";
    public override int CodeUnitBits => 32;
    public override int MinBytesPerChar => 4;
    public override int MaxBytesPerChar => 4;
    public override CharModel Model => CharModel.FixedWidth;
    public override Endianness Endianness => Endianness.LittleEndian;
    public override CompatibilityLevel CompatibilityLevel => CompatibilityLevel.Full;
    public override CharacterValue MaxCharValue => UnicodeScalars.MaxScalar;
    public override CharacterValue ReplacementChar => UnicodeScalars.Replacement;
    public override UnicodeProfile UnicodeProfile => Utf32Codec.Profile;

    public override UnicodoidStatus EncodeChar(object? state, CharacterValue ch, Span<byte> destination, out int bytesWritten) =>
        Utf32Codec.Encode(ch, destination, out bytesWritten, true);

    public override UnicodoidStatus DecodeChar(object? state, ReadOnlySpan<byte> source, out CharacterValue ch, out int bytesRead) =>
        Utf32Codec.Decode(source, out ch, out bytesRead, true);

    public override UnicodoidStatus GetByteCount(ReadOnlySpan<CharacterValue> chars, out int byteCount) =>
        Utf32Codec.GetByteCount(chars, out byteCount);
}

/// <summary>Fixed-width 32-bit Unicode Transformation Format (Big Endian).</summary>
public sealed class Utf32BeSystem : EncodingSystemBase
{
    public static readonly Utf32BeSystem Instance = new();
    private Utf32BeSystem() { }

    public override string Id => "org.unicode.utf32be";
    public override string Name => "UTF-32BE";
    public override string Version => "1.0.0";
    public override string Author => "Unicode Consortium";
    public override string Description => "Fixed-width 32-bit Unicode Transformation Format (Big Endian)";
    public override int CodeUnitBits => 32;
    public override int MinBytesPerChar => 4;
    public override int MaxBytesPerChar => 4;
    public override CharModel Model => CharModel.FixedWidth;
    public override Endianness Endianness => Endianness.BigEndian;
    public override CompatibilityLevel CompatibilityLevel => CompatibilityLevel.Full;
    public override CharacterValue MaxCharValue => UnicodeScalars.MaxScalar;
    public override CharacterValue ReplacementChar => UnicodeScalars.Replacement;
    public override UnicodeProfile UnicodeProfile => Utf32Codec.Profile;

    public override UnicodoidStatus EncodeChar(object? state, CharacterValue ch, Span<byte> destination, out int bytesWritten) =>
        Utf32Codec.Encode(ch, destination, out bytesWritten, false);

    public override UnicodoidStatus DecodeChar(object? state, ReadOnlySpan<byte> source, out CharacterValue ch, out int bytesRead) =>
        Utf32Codec.Decode(source, out ch, out bytesRead, false);

    public override UnicodoidStatus GetByteCount(ReadOnlySpan<CharacterValue> chars, out int byteCount) =>
        Utf32Codec.GetByteCount(chars, out byteCount);
}
