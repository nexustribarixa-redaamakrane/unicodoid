using System;

namespace Unicodoid.Systems;

/// <summary>Standard 7-bit American Standard Code for Information Interchange.</summary>
public sealed class AsciiSystem : EncodingSystemBase
{
    public static readonly AsciiSystem Instance = new();
    private AsciiSystem() { }

    public override string Id => "org.unicodoid.ascii";
    public override string Name => "ASCII-7";
    public override string Version => "1.0.0";
    public override string Author => "ANSI / ASA";
    public override string Description => "Standard 7-bit American Standard Code for Information Interchange";
    public override int CodeUnitBits => 8;
    public override int MinBytesPerChar => 1;
    public override int MaxBytesPerChar => 1;
    public override CharModel Model => CharModel.FixedWidth;
    public override Endianness Endianness => Endianness.None;
    public override CompatibilityLevel CompatibilityLevel => CompatibilityLevel.Partial;
    public override CharacterValue MaxCharValue => 0x7F;
    public override CharacterValue ReplacementChar => (CharacterValue)'?';

    public override UnicodeProfile UnicodeProfile { get; } = new(
        new Version(18, 0, 0),
        UaxFlags.LineBreak14 | UaxFlags.Segmentation29 | UaxFlags.Ucd44,
        UtsFlags.Uca10,
        UtrFlags.EncodingModel17,
        ToUnicode,
        FromUnicode);

    private static uint? ToUnicode(CharacterValue native) =>
        native.Value <= 0x7F ? (uint)native.Value : null;

    private static CharacterValue? FromUnicode(uint scalar) =>
        scalar <= 0x7F ? new CharacterValue(scalar) : null;

    public override UnicodoidStatus EncodeChar(
        object? state,
        CharacterValue ch,
        Span<byte> destination,
        out int bytesWritten)
    {
        bytesWritten = 0;
        if (ch.Value > 0x7F) return UnicodoidStatus.UnmappableChar;
        if (destination.Length < 1) return UnicodoidStatus.BufferTooSmall;
        destination[0] = (byte)ch.Value;
        bytesWritten = 1;
        return UnicodoidStatus.Ok;
    }

    public override UnicodoidStatus DecodeChar(
        object? state,
        ReadOnlySpan<byte> source,
        out CharacterValue ch,
        out int bytesRead)
    {
        ch = CharacterValue.Invalid;
        bytesRead = 0;
        if (source.Length < 1) return UnicodoidStatus.IncompleteSequence;
        byte value = source[0];
        if (value > 0x7F) return UnicodoidStatus.InvalidCodeUnit;
        ch = value;
        bytesRead = 1;
        return UnicodoidStatus.Ok;
    }

    public override UnicodoidStatus GetByteCount(
        ReadOnlySpan<CharacterValue> chars,
        out int byteCount)
    {
        byteCount = 0;
        foreach (CharacterValue ch in chars)
        {
            if (ch.Value > 0x7F) return UnicodoidStatus.UnmappableChar;
        }
        byteCount = chars.Length;
        return UnicodoidStatus.Ok;
    }
}
