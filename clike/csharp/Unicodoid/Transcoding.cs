using System;

namespace Unicodoid;

/// <summary>Substitute-output callback for <see cref="TranscodePolicy.Callback"/>.</summary>
public delegate bool TranscodeCallback(
    IEncodingSystem sourceSystem,
    IEncodingSystem destinationSystem,
    CharacterValue unmappableChar,
    Span<byte> destination,
    out int bytesWritten);

/// <summary>
/// Explicit value bridge for systems that do not share a Unicode profile.
/// Implementations map one native source character value to one native
/// destination character value.
/// </summary>
public delegate UnicodoidStatus TranscodeBridge(
    IEncodingSystem sourceSystem,
    CharacterValue sourceChar,
    IEncodingSystem destinationSystem,
    out CharacterValue destinationChar);

/// <summary>Transcoding configuration.</summary>
public sealed class TranscodeConfig
{
    public TranscodePolicy Policy { get; set; } = TranscodePolicy.Strict;

    /// <summary>Custom replacement value; <c>null</c> uses the destination default.</summary>
    public CharacterValue? CustomReplacement { get; set; }

    public TranscodeCallback? Callback { get; set; }

    public TranscodeBridge? Bridge { get; set; }

    public TranscodeConfig Clone() => new()
    {
        Policy = Policy,
        CustomReplacement = CustomReplacement,
        Callback = Callback,
        Bridge = Bridge
    };
}

/// <summary>
/// Explicit transcoding between independent encoding systems.  Two systems are
/// bridged in exactly one of these ways:
/// <list type="bullet">
///   <item>they are the same system (validated byte copy);</item>
///   <item>both declare Unicode profiles (route through scalar values);</item>
///   <item>the caller supplies a bridge for non-Unicode value mapping.</item>
/// </list>
/// There is no implicit identity mapping between unrelated systems.
/// </summary>
public static class Transcoding
{
    private sealed class ByteBuffer
    {
        private byte[] _data = Array.Empty<byte>();
        private int _length;

        public int Count => _length;

        public void Append(ReadOnlySpan<byte> bytes)
        {
            Ensure(_length + bytes.Length);
            bytes.CopyTo(_data.AsSpan(_length));
            _length += bytes.Length;
        }

        public void CopyTo(Span<byte> destination) =>
            _data.AsSpan(0, _length).CopyTo(destination);

        public byte[] ToArray() => _data.AsSpan(0, _length).ToArray();

        private void Ensure(int needed)
        {
            if (needed <= _data.Length) return;
            int capacity = _data.Length == 0 ? 64 : _data.Length;
            while (capacity < needed)
            {
                if (capacity > int.MaxValue / 2)
                {
                    capacity = needed;
                    break;
                }
                capacity *= 2;
            }
            Array.Resize(ref _data, capacity);
        }
    }

    private static bool IsSameSystem(IEncodingSystem a, IEncodingSystem b) =>
        ReferenceEquals(a, b) ||
        string.Equals(a.Id, b.Id, StringComparison.Ordinal);

    /// <summary>
    /// Transcode one native character value into another system.  This performs
    /// mapping only; it does not encode bytes.
    /// </summary>
    public static UnicodoidStatus Char(
        IEncodingSystem sourceSystem,
        CharacterValue sourceChar,
        IEncodingSystem destinationSystem,
        out CharacterValue destinationChar,
        TranscodeConfig? config = null)
    {
        destinationChar = CharacterValue.Invalid;
        if (sourceSystem is null || destinationSystem is null)
        {
            return UnicodoidStatus.InvalidParam;
        }

        if (IsSameSystem(sourceSystem, destinationSystem))
        {
            destinationChar = sourceChar;
            return UnicodoidStatus.Ok;
        }

        if (sourceSystem.HasUnicodeProfile && destinationSystem.HasUnicodeProfile)
        {
            UnicodeProfile sourceProfile = sourceSystem.UnicodeProfile!;
            UnicodeProfile destinationProfile = destinationSystem.UnicodeProfile!;
            if (!sourceProfile.TryToUnicode(sourceChar, out uint scalar))
            {
                return UnicodoidStatus.UnmappableChar;
            }
            return destinationProfile.TryFromUnicode(scalar, out destinationChar)
                ? UnicodoidStatus.Ok
                : UnicodoidStatus.UnmappableChar;
        }

        if (config?.Bridge is not null)
        {
            return config.Bridge(sourceSystem, sourceChar, destinationSystem, out destinationChar);
        }

        // No implicit identity mapping between unrelated systems.
        return UnicodoidStatus.UnmappableChar;
    }

    /// <summary>
    /// Transcode a raw byte buffer.  On
    /// <see cref="UnicodoidStatus.BufferTooSmall"/>,
    /// <paramref name="bytesWritten"/> receives the required destination size.
    /// </summary>
    public static UnicodoidStatus Bytes(
        IEncodingSystem sourceSystem,
        ReadOnlySpan<byte> input,
        IEncodingSystem destinationSystem,
        Span<byte> output,
        out int bytesWritten,
        TranscodeConfig? config = null)
    {
        bytesWritten = 0;
        if (sourceSystem is null || destinationSystem is null)
        {
            return UnicodoidStatus.InvalidParam;
        }

        TranscodeConfig cfg = config ?? new TranscodeConfig();
        UnicodoidStatus status = ToBuffer(
            sourceSystem, input, destinationSystem, cfg, out byte[] result);
        bytesWritten = result.Length;

        if (status != UnicodoidStatus.Ok) return status;
        if (result.Length > output.Length) return UnicodoidStatus.BufferTooSmall;

        result.CopyTo(output);
        return UnicodoidStatus.Ok;
    }

    /// <summary>Transcode a string into a newly allocated destination string.</summary>
    public static UnicodoidStatus TryString(
        TextString source,
        IEncodingSystem destinationSystem,
        out TextString? result,
        TranscodeConfig? config = null)
    {
        result = null;
        if (source is null || destinationSystem is null)
        {
            return UnicodoidStatus.InvalidParam;
        }

        TranscodeConfig cfg = config ?? new TranscodeConfig();
        UnicodoidStatus status = ToBuffer(
            source.EncodingSystem, source.Bytes, destinationSystem, cfg, out byte[] bytes);
        if (status != UnicodoidStatus.Ok) return status;

        return TextString.TryFromBytes(destinationSystem, bytes, out result);
    }

    public static TextString String(
        TextString source,
        IEncodingSystem destinationSystem,
        TranscodeConfig? config = null)
    {
        UnicodoidStatus status = TryString(source, destinationSystem, out TextString? result, config);
        return status == UnicodoidStatus.Ok
            ? result!
            : throw new UnicodoidException(status);
    }

    private static UnicodoidStatus ToBuffer(
        IEncodingSystem sourceSystem,
        ReadOnlySpan<byte> input,
        IEncodingSystem destinationSystem,
        TranscodeConfig config,
        out byte[] result)
    {
        result = Array.Empty<byte>();
        if (input.Length == 0) return UnicodoidStatus.Ok;

        // Same system: validate once, then copy the byte stream unchanged.
        if (IsSameSystem(sourceSystem, destinationSystem))
        {
            var check = new Codec(sourceSystem);
            UnicodoidStatus checkStatus = check.Validate(input, out _);
            if (checkStatus != UnicodoidStatus.Ok) return checkStatus;
            result = input.ToArray();
            return UnicodoidStatus.Ok;
        }

        var builder = new ByteBuffer();
        UnicodoidStatus status = TranscodeCore(
            sourceSystem, input, destinationSystem, config, builder);
        result = builder.ToArray();
        return status;
    }

    private static UnicodoidStatus TranscodeCore(
        IEncodingSystem sourceSystem,
        ReadOnlySpan<byte> input,
        IEncodingSystem destinationSystem,
        TranscodeConfig config,
        ByteBuffer output)
    {
        var sourceCodec = new Codec(sourceSystem);
        var destinationCodec = new Codec(destinationSystem);
        byte[] encodeBuffer = new byte[Math.Max(16, destinationSystem.MaxBytesPerChar)];
        byte[] callbackBuffer = new byte[256];

        int inputOffset = 0;

        while (inputOffset < input.Length)
        {
            UnicodoidStatus status = sourceCodec.DecodeChar(
                input[inputOffset..], out CharacterValue sourceChar, out int read);

            if (status == UnicodoidStatus.NoCharacter)
            {
                if (read == 0) return UnicodoidStatus.InternalError;
                inputOffset += read;
                continue;
            }

            if (status != UnicodoidStatus.Ok)
            {
                if (config.Policy == TranscodePolicy.Strict) return status;

                int advance = read > 0 ? read : 1;
                if (advance > input.Length - inputOffset)
                {
                    advance = input.Length - inputOffset;
                }
                inputOffset += advance;

                if (config.Policy == TranscodePolicy.Skip) continue;

                if (config.Policy == TranscodePolicy.Callback && config.Callback is not null)
                {
                    bool handled = config.Callback(
                        sourceSystem, destinationSystem, CharacterValue.Invalid,
                        callbackBuffer, out int substituteLength);
                    if (!handled) return UnicodoidStatus.UnmappableChar;
                    if (substituteLength < 0 || substituteLength > callbackBuffer.Length)
                    {
                        return UnicodoidStatus.InternalError;
                    }
                    output.Append(callbackBuffer.AsSpan(0, substituteLength));
                    continue;
                }

                CharacterValue replacement =
                    config.CustomReplacement ?? destinationSystem.ReplacementChar;
                status = EncodeInto(destinationCodec, replacement, encodeBuffer, output);
                if (status != UnicodoidStatus.Ok) return status;
                continue;
            }

            inputOffset += read;

            status = Char(sourceSystem, sourceChar, destinationSystem, out CharacterValue destinationChar, config);
            if (status != UnicodoidStatus.Ok)
            {
                if (config.Policy == TranscodePolicy.Strict) return status;
                if (config.Policy == TranscodePolicy.Skip) continue;

                if (config.Policy == TranscodePolicy.Callback && config.Callback is not null)
                {
                    bool handled = config.Callback(
                        sourceSystem, destinationSystem, sourceChar,
                        callbackBuffer, out int substituteLength);
                    if (!handled) return UnicodoidStatus.UnmappableChar;
                    if (substituteLength < 0 || substituteLength > callbackBuffer.Length)
                    {
                        return UnicodoidStatus.InternalError;
                    }
                    output.Append(callbackBuffer.AsSpan(0, substituteLength));
                    continue;
                }

                destinationChar =
                    config.CustomReplacement ?? destinationSystem.ReplacementChar;
            }

            status = EncodeInto(destinationCodec, destinationChar, encodeBuffer, output);
            if (status != UnicodoidStatus.Ok) return status;
        }

        return UnicodoidStatus.Ok;
    }

    private static UnicodoidStatus EncodeInto(
        Codec codec,
        CharacterValue ch,
        byte[] buffer,
        ByteBuffer output)
    {
        UnicodoidStatus status = codec.EncodeChar(ch, buffer, out int written);
        if (status != UnicodoidStatus.Ok) return status;
        output.Append(buffer.AsSpan(0, written));
        return UnicodoidStatus.Ok;
    }
}
