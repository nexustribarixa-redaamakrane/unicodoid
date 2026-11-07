using System;

namespace Unicodoid.Systems;

/// <summary>Variable-width 8-bit Unicode Transformation Format.</summary>
public sealed class Utf8System : EncodingSystemBase
{
    public static readonly Utf8System Instance = new();

    private Utf8System() { }

    public override string Id => "org.unicode.utf8";
    public override string Name => "UTF-8";
    public override string Version => "1.0.0";
    public override string Author => "Unicode Consortium / Ken Thompson / Rob Pike";
    public override string Description => "Variable-width 8-bit Unicode Transformation Format";
    public override int CodeUnitBits => 8;
    public override int MinBytesPerChar => 1;
    public override int MaxBytesPerChar => 4;
    public override CharModel Model => CharModel.VariableWidth;
    public override Endianness Endianness => Endianness.None;
    public override CompatibilityLevel CompatibilityLevel => CompatibilityLevel.Full;
    public override CharacterValue MaxCharValue => UnicodeScalars.MaxScalar;
    public override CharacterValue ReplacementChar => UnicodeScalars.Replacement;

    public override UnicodeProfile UnicodeProfile { get; } = new(
        new Version(18, 0, 0),
        UaxFlags.Bidi9 | UaxFlags.EastAsianWidth11 | UaxFlags.LineBreak14 |
        UaxFlags.Normalization15 | UaxFlags.Script24 | UaxFlags.Segmentation29 |
        UaxFlags.IdSyntax31 | UaxFlags.Ucd44,
        UtsFlags.Uca10 | UtsFlags.Regex18 | UtsFlags.Ldml35 |
        UtsFlags.Security39 | UtsFlags.Idna46 | UtsFlags.Emoji51,
        UtrFlags.EncodingModel17 | UtrFlags.PropertyModel23 |
        UtrFlags.Math25 | UtrFlags.Conformance33,
        ToUnicode,
        FromUnicode);

    private static uint? ToUnicode(CharacterValue native) =>
        IsValidUnicodeScalar(native.Value) ? (uint)native.Value : null;

    private static CharacterValue? FromUnicode(uint scalar) =>
        UnicodeScalars.IsValid(scalar) ? new CharacterValue(scalar) : null;

    public override UnicodoidStatus EncodeChar(
        object? state,
        CharacterValue ch,
        Span<byte> destination,
        out int bytesWritten)
    {
        bytesWritten = 0;
        if (!IsValidUnicodeScalar(ch.Value))
        {
            return UnicodoidStatus.UnmappableChar;
        }

        ulong value = ch.Value;
        if (value <= 0x7F)
        {
            if (destination.Length < 1) return UnicodoidStatus.BufferTooSmall;
            destination[0] = (byte)value;
            bytesWritten = 1;
        }
        else if (value <= 0x7FF)
        {
            if (destination.Length < 2) return UnicodoidStatus.BufferTooSmall;
            destination[0] = (byte)(0xC0 | (value >> 6));
            destination[1] = (byte)(0x80 | (value & 0x3F));
            bytesWritten = 2;
        }
        else if (value <= 0xFFFF)
        {
            if (destination.Length < 3) return UnicodoidStatus.BufferTooSmall;
            destination[0] = (byte)(0xE0 | (value >> 12));
            destination[1] = (byte)(0x80 | ((value >> 6) & 0x3F));
            destination[2] = (byte)(0x80 | (value & 0x3F));
            bytesWritten = 3;
        }
        else
        {
            if (destination.Length < 4) return UnicodoidStatus.BufferTooSmall;
            destination[0] = (byte)(0xF0 | (value >> 18));
            destination[1] = (byte)(0x80 | ((value >> 12) & 0x3F));
            destination[2] = (byte)(0x80 | ((value >> 6) & 0x3F));
            destination[3] = (byte)(0x80 | (value & 0x3F));
            bytesWritten = 4;
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
        if (source.Length == 0) return UnicodoidStatus.IncompleteSequence;

        byte b0 = source[0];
        if ((b0 & 0x80) == 0x00)
        {
            ch = b0;
            bytesRead = 1;
            return UnicodoidStatus.Ok;
        }
        if ((b0 & 0xE0) == 0xC0)
        {
            if (source.Length < 2) return UnicodoidStatus.IncompleteSequence;
            byte b1 = source[1];
            if ((b1 & 0xC0) != 0x80) return UnicodoidStatus.InvalidCodeUnit;
            uint value = (uint)(((b0 & 0x1F) << 6) | (b1 & 0x3F));
            if (value < 0x80) return UnicodoidStatus.InvalidCodeUnit;
            ch = value;
            bytesRead = 2;
            return UnicodoidStatus.Ok;
        }
        if ((b0 & 0xF0) == 0xE0)
        {
            if (source.Length < 3) return UnicodoidStatus.IncompleteSequence;
            byte b1 = source[1];
            byte b2 = source[2];
            if ((b1 & 0xC0) != 0x80 || (b2 & 0xC0) != 0x80)
            {
                return UnicodoidStatus.InvalidCodeUnit;
            }
            uint value = (uint)(((b0 & 0x0F) << 12) | ((b1 & 0x3F) << 6) | (b2 & 0x3F));
            if (value < 0x800) return UnicodoidStatus.InvalidCodeUnit;
            if (value >= 0xD800 && value <= 0xDFFF) return UnicodoidStatus.InvalidCodeUnit;
            ch = value;
            bytesRead = 3;
            return UnicodoidStatus.Ok;
        }
        if ((b0 & 0xF8) == 0xF0)
        {
            if (source.Length < 4) return UnicodoidStatus.IncompleteSequence;
            byte b1 = source[1];
            byte b2 = source[2];
            byte b3 = source[3];
            if ((b1 & 0xC0) != 0x80 || (b2 & 0xC0) != 0x80 || (b3 & 0xC0) != 0x80)
            {
                return UnicodoidStatus.InvalidCodeUnit;
            }
            uint value = (uint)(((b0 & 0x07) << 18) | ((b1 & 0x3F) << 12) |
                                ((b2 & 0x3F) << 6) | (b3 & 0x3F));
            if (value < 0x10000 || value > UnicodeScalars.MaxScalar)
            {
                return UnicodoidStatus.InvalidCodeUnit;
            }
            ch = value;
            bytesRead = 4;
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
            if (!IsValidUnicodeScalar(ch.Value)) return UnicodoidStatus.UnmappableChar;
            total += ch.Value switch
            {
                <= 0x7F => 1,
                <= 0x7FF => 2,
                <= 0xFFFF => 3,
                _ => 4
            };
        }
        byteCount = total;
        return UnicodoidStatus.Ok;
    }
}
