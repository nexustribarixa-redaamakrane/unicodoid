using System;

namespace Unicodoid;

/// <summary>
/// Convenience base class for encoding systems.  It supplies state defaults
/// and generic validate/count loops; concrete systems implement encoding,
/// decoding, and sizing.
/// </summary>
public abstract class EncodingSystemBase : IEncodingSystem
{
    public abstract string Id { get; }
    public abstract string Name { get; }
    public abstract string Version { get; }
    public abstract string Author { get; }
    public abstract string Description { get; }

    public abstract int CodeUnitBits { get; }
    public abstract int MinBytesPerChar { get; }
    public abstract int MaxBytesPerChar { get; }
    public abstract CharModel Model { get; }
    public abstract Endianness Endianness { get; }
    public abstract CompatibilityLevel CompatibilityLevel { get; }
    public abstract CharacterValue MaxCharValue { get; }
    public abstract CharacterValue ReplacementChar { get; }
    public abstract UnicodeProfile? UnicodeProfile { get; }

    public virtual int StateSize => 0;

    public bool IsStateful => StateSize > 0;

    public bool HasUnicodeProfile =>
        UnicodeProfile is not null &&
        CompatibilityLevel != CompatibilityLevel.None;

    public virtual object? CreateState() => null;

    public virtual void ResetState(object? state) { }

    public virtual object? CloneState(object? state) => state;

    public abstract UnicodoidStatus EncodeChar(
        object? state,
        CharacterValue ch,
        Span<byte> destination,
        out int bytesWritten);

    public abstract UnicodoidStatus DecodeChar(
        object? state,
        ReadOnlySpan<byte> source,
        out CharacterValue ch,
        out int bytesRead);

    public abstract UnicodoidStatus GetByteCount(
        ReadOnlySpan<CharacterValue> chars,
        out int byteCount);

    public virtual UnicodoidStatus Validate(
        object? state,
        ReadOnlySpan<byte> source,
        out int invalidOffset)
    {
        object? workingState = state;
        if (workingState is null && IsStateful)
        {
            workingState = CreateState();
        }

        int offset = 0;
        while (offset < source.Length)
        {
            UnicodoidStatus status = DecodeChar(
                workingState, source[offset..], out _, out int read);
            if (status == UnicodoidStatus.NoCharacter)
            {
                if (read == 0)
                {
                    invalidOffset = offset;
                    return UnicodoidStatus.InternalError;
                }
                offset += read;
                continue;
            }
            if (status != UnicodoidStatus.Ok)
            {
                invalidOffset = offset;
                return status;
            }
            if (read == 0)
            {
                invalidOffset = offset;
                return UnicodoidStatus.InternalError;
            }
            offset += read;
        }
        invalidOffset = 0;
        return UnicodoidStatus.Ok;
    }

    public virtual UnicodoidStatus CountChars(
        object? state,
        ReadOnlySpan<byte> source,
        out int count)
    {
        object? workingState = state;
        if (workingState is null && IsStateful)
        {
            workingState = CreateState();
        }

        int offset = 0;
        int total = 0;
        while (offset < source.Length)
        {
            UnicodoidStatus status = DecodeChar(
                workingState, source[offset..], out _, out int read);
            if (status == UnicodoidStatus.NoCharacter)
            {
                if (read == 0)
                {
                    count = total;
                    return UnicodoidStatus.InternalError;
                }
                offset += read;
                continue;
            }
            if (status != UnicodoidStatus.Ok)
            {
                count = total;
                return status;
            }
            if (read == 0)
            {
                count = total;
                return UnicodoidStatus.InternalError;
            }
            offset += read;
            ++total;
        }
        count = total;
        return UnicodoidStatus.Ok;
    }

    protected static bool IsValidUnicodeScalar(ulong value) =>
        value <= UnicodeScalars.MaxScalar &&
        (value < UnicodeScalars.SurrogateMin || value > UnicodeScalars.SurrogateMax);

    protected static void WriteU16(Span<byte> destination, ushort value, bool littleEndian)
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

    protected static ushort ReadU16(ReadOnlySpan<byte> source, bool littleEndian) =>
        littleEndian
            ? (ushort)(source[0] | (source[1] << 8))
            : (ushort)((source[0] << 8) | source[1]);

    protected static void WriteU32(Span<byte> destination, uint value, bool littleEndian)
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

    protected static uint ReadU32(ReadOnlySpan<byte> source, bool littleEndian) =>
        littleEndian
            ? (uint)(source[0] | (source[1] << 8) | (source[2] << 16) | (source[3] << 24))
            : (uint)((source[0] << 24) | (source[1] << 16) | (source[2] << 8) | source[3]);
}
