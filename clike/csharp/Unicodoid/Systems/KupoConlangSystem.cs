using System;

namespace Unicodoid.Systems;

/// <summary>
/// KupoConlang-14: an independent 14-bit constructed-script encoding system.
/// It demonstrates a non-Unicode architecture: custom value space
/// (0x0000..0x3FFF), variable 1-to-2 byte representation, and no Unicode
/// profile at all.
/// </summary>
public sealed class KupoConlangSystem : EncodingSystemBase
{
    public static readonly KupoConlangSystem Instance = new();
    private KupoConlangSystem() { }

    public override string Id => "org.unicodoid.custom.kupoconlang";
    public override string Name => "KupoConlang-14";
    public override string Version => "1.0.0";
    public override string Author => "Unicodoid Conlang Working Group";
    public override string Description => "Independent 14-bit Variable-Width Conlang Script System";
    public override int CodeUnitBits => 8;
    public override int MinBytesPerChar => 1;
    public override int MaxBytesPerChar => 2;
    public override CharModel Model => CharModel.VariableWidth;
    public override Endianness Endianness => Endianness.None;
    public override CompatibilityLevel CompatibilityLevel => CompatibilityLevel.None;
    public override CharacterValue MaxCharValue => 0x3FFF;
    public override CharacterValue ReplacementChar => 0x003F;
    public override UnicodeProfile? UnicodeProfile => null;

    public override UnicodoidStatus EncodeChar(
        object? state,
        CharacterValue ch,
        Span<byte> destination,
        out int bytesWritten)
    {
        bytesWritten = 0;
        if (ch.Value > 0x3FFF) return UnicodoidStatus.UnmappableChar;

        if (ch.Value <= 0x7F)
        {
            if (destination.Length < 1) return UnicodoidStatus.BufferTooSmall;
            destination[0] = (byte)ch.Value;
            bytesWritten = 1;
        }
        else
        {
            if (destination.Length < 2) return UnicodoidStatus.BufferTooSmall;
            destination[0] = (byte)(0x80 | ((ch.Value >> 8) & 0x3F));
            destination[1] = (byte)(ch.Value & 0xFF);
            bytesWritten = 2;
        }
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

        byte b0 = source[0];
        if ((b0 & 0x80) == 0)
        {
            ch = b0;
            bytesRead = 1;
            return UnicodoidStatus.Ok;
        }
        if ((b0 & 0xC0) == 0x80)
        {
            if (source.Length < 2) return UnicodoidStatus.IncompleteSequence;
            uint value = (uint)(((b0 & 0x3F) << 8) | source[1]);
            if (value < 0x80) return UnicodoidStatus.InvalidCodeUnit;
            ch = value;
            bytesRead = 2;
            return UnicodoidStatus.Ok;
        }
        return UnicodoidStatus.InvalidCodeUnit;
    }

    public override UnicodoidStatus GetByteCount(
        ReadOnlySpan<CharacterValue> chars,
        out int byteCount)
    {
        byteCount = 0;
        int total = 0;
        foreach (CharacterValue ch in chars)
        {
            if (ch.Value <= 0x7F) total += 1;
            else if (ch.Value <= 0x3FFF) total += 2;
            else return UnicodoidStatus.UnmappableChar;
        }
        byteCount = total;
        return UnicodoidStatus.Ok;
    }
}
