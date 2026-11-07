using System;
using System.Collections.Generic;

namespace Unicodoid;

/// <summary>
/// Stateful stream facade over an encoding system.  A codec owns one instance
/// of the system's per-stream state; stateless systems use <c>null</c>.
/// </summary>
public sealed class Codec
{
    private object? _state;

    public IEncodingSystem EncodingSystem { get; }

    public Codec(IEncodingSystem system)
    {
        EncodingSystem = system ?? throw new ArgumentNullException(nameof(system));
        _state = system.CreateState();
    }

    /// <summary>Reset the codec to its initial state.</summary>
    public void Reset() => EncodingSystem.ResetState(_state);

    public UnicodoidStatus EncodeChar(
        CharacterValue ch,
        Span<byte> destination,
        out int bytesWritten) =>
        EncodingSystem.EncodeChar(_state, ch, destination, out bytesWritten);

    public UnicodoidStatus DecodeChar(
        ReadOnlySpan<byte> source,
        out CharacterValue ch,
        out int bytesRead) =>
        EncodingSystem.DecodeChar(_state, source, out ch, out bytesRead);

    public UnicodoidStatus Validate(ReadOnlySpan<byte> source, out int invalidOffset) =>
        EncodingSystem.Validate(_state, source, out invalidOffset);

    public UnicodoidStatus CountChars(ReadOnlySpan<byte> source, out int count) =>
        EncodingSystem.CountChars(_state, source, out count);

    /// <summary>
    /// Encode a sequence as one stream.  On
    /// <see cref="UnicodoidStatus.BufferTooSmall"/>,
    /// <paramref name="bytesWritten"/> receives the number of bytes encoded
    /// for the successful prefix.
    /// </summary>
    public UnicodoidStatus Encode(
        ReadOnlySpan<CharacterValue> chars,
        Span<byte> destination,
        out int bytesWritten)
    {
        int offset = 0;
        foreach (CharacterValue ch in chars)
        {
            UnicodoidStatus status = EncodeChar(
                ch, destination[offset..], out int written);
            if (status != UnicodoidStatus.Ok)
            {
                bytesWritten = offset;
                return status;
            }
            offset += written;
        }
        bytesWritten = offset;
        return UnicodoidStatus.Ok;
    }

    /// <summary>
    /// Decode a complete byte stream into character values.  A modal
    /// control-only fragment advances the input but contributes no character.
    /// </summary>
    public UnicodoidStatus Decode(
        ReadOnlySpan<byte> source,
        Span<CharacterValue> destination,
        out int count)
    {
        int offset = 0;
        int total = 0;

        while (offset < source.Length)
        {
            UnicodoidStatus status = DecodeChar(
                source[offset..], out CharacterValue ch, out int read);

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
            if (total >= destination.Length)
            {
                count = total;
                return UnicodoidStatus.BufferTooSmall;
            }

            destination[total++] = ch;
            offset += read;
        }

        count = total;
        return UnicodoidStatus.Ok;
    }

    /// <summary>Decode an entire byte stream into a newly allocated array.</summary>
    public UnicodoidStatus DecodeAll(ReadOnlySpan<byte> source, out CharacterValue[] chars)
    {
        var result = new List<CharacterValue>();
        int offset = 0;

        while (offset < source.Length)
        {
            UnicodoidStatus status = DecodeChar(
                source[offset..], out CharacterValue ch, out int read);

            if (status == UnicodoidStatus.NoCharacter)
            {
                if (read == 0)
                {
                    chars = result.ToArray();
                    return UnicodoidStatus.InternalError;
                }
                offset += read;
                continue;
            }
            if (status != UnicodoidStatus.Ok)
            {
                chars = result.ToArray();
                return status;
            }
            if (read == 0)
            {
                chars = result.ToArray();
                return UnicodoidStatus.InternalError;
            }

            result.Add(ch);
            offset += read;
        }

        chars = result.ToArray();
        return UnicodoidStatus.Ok;
    }

    /// <summary>Encode an entire character sequence into a newly allocated array.</summary>
    public UnicodoidStatus EncodeAll(
        ReadOnlySpan<CharacterValue> chars,
        out byte[] bytes)
    {
        var result = new List<byte>();
        Span<byte> temp = stackalloc byte[16];

        foreach (CharacterValue ch in chars)
        {
            UnicodoidStatus status = EncodeChar(ch, temp, out int written);
            if (status != UnicodoidStatus.Ok)
            {
                bytes = result.ToArray();
                return status;
            }
            for (int i = 0; i < written; ++i)
            {
                result.Add(temp[i]);
            }
        }

        bytes = result.ToArray();
        return UnicodoidStatus.Ok;
    }
}
