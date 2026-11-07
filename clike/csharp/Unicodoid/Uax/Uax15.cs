using System;
using System.Collections.Generic;

namespace Unicodoid;

/// <summary>Normalization forms from UAX #15.</summary>
public enum NormalizationForm
{
    Nfc = 0,
    Nfd = 1,
    Nfkc = 2,
    Nfkd = 3,
    Custom = 4
}

/// <summary>
/// UAX #15 normalization for Unicode-compatible systems.
///
/// The module is opt-in: the system must expose a Unicode profile.  It
/// implements the full canonical and compatibility decomposition tables,
/// canonical pair composition, Hangul decomposition/composition and canonical
/// ordering of combining marks, all taken from the generated Unicode 18.0.0
/// tables (see <see cref="Ucd"/>).  It is exercised against the official
/// NormalizationTest.txt in the test suite.
/// </summary>
public static class Normalization
{
    private const uint HangulSBase = 0xAC00;
    private const uint HangulLBase = 0x1100;
    private const uint HangulVBase = 0x1161;
    private const uint HangulTBase = 0x11A7;
    private const int HangulLCount = 19;
    private const int HangulVCount = 21;
    private const int HangulTCount = 28;
    private const int HangulNCount = HangulVCount * HangulTCount;
    private const int HangulSCount = HangulLCount * HangulNCount;

    private static void Decompose(uint codePoint, bool compatibility, List<uint> output)
    {
        if (codePoint >= HangulSBase && codePoint < HangulSBase + HangulSCount)
        {
            uint sIndex = codePoint - HangulSBase;
            uint l = HangulLBase + sIndex / HangulNCount;
            uint v = HangulVBase + (sIndex % HangulNCount) / HangulTCount;
            uint t = HangulTBase + sIndex % HangulTCount;
            output.Add(l);
            output.Add(v);
            if (t != HangulTBase) output.Add(t);
            return;
        }

        uint[]? decomposition = Ucd.Decomposition(codePoint, compatibility);
        if (decomposition is not null)
        {
            foreach (uint part in decomposition)
            {
                Decompose(part, compatibility, output);
            }
            return;
        }

        output.Add(codePoint);
    }

    private static bool TryComposePair(uint starter, uint ch, out uint composed)
    {
        composed = 0;

        if (starter >= HangulLBase && starter < HangulLBase + HangulLCount &&
            ch >= HangulVBase && ch < HangulVBase + HangulVCount)
        {
            uint lIndex = starter - HangulLBase;
            uint vIndex = ch - HangulVBase;
            composed = HangulSBase + (lIndex * HangulVCount + vIndex) * HangulTCount;
            return true;
        }

        if (starter >= HangulSBase && starter < HangulSBase + HangulSCount &&
            (starter - HangulSBase) % HangulTCount == 0 &&
            ch > HangulTBase && ch < HangulTBase + HangulTCount)
        {
            composed = starter + (ch - HangulTBase);
            return true;
        }

        return Ucd.TryComposePair(starter, ch, out composed);
    }

    private static void CanonicalOrder(List<uint> sequence)
    {
        for (int i = 1; i < sequence.Count; ++i)
        {
            uint codePoint = sequence[i];
            int combiningClass = Ucd.CombiningClass(codePoint);
            if (combiningClass == 0) continue;

            int j = i;
            while (j > 0)
            {
                int previous = Ucd.CombiningClass(sequence[j - 1]);
                if (previous == 0 || previous <= combiningClass) break;
                sequence[j] = sequence[j - 1];
                --j;
            }
            sequence[j] = codePoint;
        }
    }

    private static void Compose(List<uint> sequence, List<uint> output)
    {
        if (sequence.Count == 0) return;

        output.Add(sequence[0]);
        int starterPosition = 0;
        uint starter = sequence[0];
        int lastCombiningClass = 0;

        for (int i = 1; i < sequence.Count; ++i)
        {
            uint codePoint = sequence[i];
            int combiningClass = Ucd.CombiningClass(codePoint);

            if ((lastCombiningClass < combiningClass || lastCombiningClass == 0) &&
                TryComposePair(starter, codePoint, out uint composed))
            {
                output[starterPosition] = composed;
                starter = composed;
                continue;
            }

            if (combiningClass == 0)
            {
                starterPosition = output.Count;
                starter = codePoint;
            }

            output.Add(codePoint);
            lastCombiningClass = combiningClass;
        }
    }

    private static UnicodoidStatus DecodeScalars(
        IEncodingSystem system,
        ReadOnlySpan<byte> input,
        List<uint> output)
    {
        var codec = new Codec(system);
        int offset = 0;

        while (offset < input.Length)
        {
            UnicodoidStatus status = codec.DecodeChar(
                input[offset..], out CharacterValue native, out int read);

            if (status == UnicodoidStatus.NoCharacter)
            {
                if (read == 0) return UnicodoidStatus.InternalError;
                offset += read;
                continue;
            }
            if (status != UnicodoidStatus.Ok) return status;
            if (read == 0) return UnicodoidStatus.InternalError;

            if (!system.UnicodeProfile!.TryToUnicode(native, out uint scalar))
            {
                return UnicodoidStatus.UnmappableChar;
            }

            output.Add(scalar);
            offset += read;
        }

        return UnicodoidStatus.Ok;
    }

    private static UnicodoidStatus EncodeScalars(
        IEncodingSystem system,
        List<uint> scalars,
        List<byte> output)
    {
        var codec = new Codec(system);
        byte[] buffer = new byte[Math.Max(8, system.MaxBytesPerChar)];

        foreach (uint scalar in scalars)
        {
            if (!system.UnicodeProfile!.TryFromUnicode(scalar, out CharacterValue native))
            {
                return UnicodoidStatus.UnmappableChar;
            }

            UnicodoidStatus status = codec.EncodeChar(native, buffer, out int written);
            if (status != UnicodoidStatus.Ok) return status;

            for (int i = 0; i < written; ++i)
            {
                output.Add(buffer[i]);
            }
        }

        return UnicodoidStatus.Ok;
    }

    /// <summary>
    /// Normalize a byte buffer.  On
    /// <see cref="UnicodoidStatus.BufferTooSmall"/>, <paramref name="bytesWritten"/>
    /// receives the required destination size.
    /// </summary>
    public static UnicodoidStatus Normalize(
        IEncodingSystem system,
        ReadOnlySpan<byte> input,
        NormalizationForm form,
        Span<byte> destination,
        out int bytesWritten)
    {
        bytesWritten = 0;
        if (system is null) return UnicodoidStatus.InvalidParam;
        if (form == NormalizationForm.Custom) return UnicodoidStatus.Unsupported;
        if (!system.HasUnicodeProfile) return UnicodoidStatus.Unsupported;
        if (input.Length == 0) return UnicodoidStatus.Ok;

        bool compatibility =
            form is NormalizationForm.Nfkc or NormalizationForm.Nfkd;
        bool compose =
            form is NormalizationForm.Nfc or NormalizationForm.Nfkc;

        var decoded = new List<uint>();
        UnicodoidStatus status = DecodeScalars(system, input, decoded);
        if (status != UnicodoidStatus.Ok) return status;

        var decomposed = new List<uint>(decoded.Count);
        foreach (uint scalar in decoded)
        {
            Decompose(scalar, compatibility, decomposed);
        }
        CanonicalOrder(decomposed);

        List<uint> normalized;
        if (compose)
        {
            normalized = new List<uint>(decomposed.Count);
            Compose(decomposed, normalized);
        }
        else
        {
            normalized = decomposed;
        }

        var encoded = new List<byte>(input.Length);
        status = EncodeScalars(system, normalized, encoded);
        if (status != UnicodoidStatus.Ok) return status;

        bytesWritten = encoded.Count;
        if (encoded.Count > destination.Length) return UnicodoidStatus.BufferTooSmall;

        for (int i = 0; i < encoded.Count; ++i)
        {
            destination[i] = encoded[i];
        }

        return UnicodoidStatus.Ok;
    }

    /// <summary>Normalize into a newly allocated byte array.</summary>
    public static UnicodoidStatus TryNormalize(
        IEncodingSystem system,
        ReadOnlySpan<byte> input,
        NormalizationForm form,
        out byte[] result)
    {
        result = Array.Empty<byte>();
        if (system is null) return UnicodoidStatus.InvalidParam;

        UnicodoidStatus status = Normalize(
            system, input, form, Span<byte>.Empty, out int required);
        if (status == UnicodoidStatus.Ok && required == 0) return UnicodoidStatus.Ok;
        if (status != UnicodoidStatus.BufferTooSmall) return status;

        byte[] buffer = new byte[required];
        status = Normalize(system, input, form, buffer, out int written);
        if (status != UnicodoidStatus.Ok) return status;

        if (written != buffer.Length)
        {
            Array.Resize(ref buffer, written);
        }
        result = buffer;
        return UnicodoidStatus.Ok;
    }

    public static byte[] Normalize(
        IEncodingSystem system,
        ReadOnlySpan<byte> input,
        NormalizationForm form)
    {
        UnicodoidStatus status = TryNormalize(system, input, form, out byte[] result);
        return status == UnicodoidStatus.Ok
            ? result
            : throw new UnicodoidException(status);
    }

    /// <summary>Normalize a string, preserving its encoding system.</summary>
    public static UnicodoidStatus TryNormalize(
        TextString source,
        NormalizationForm form,
        out TextString? result)
    {
        result = null;
        if (source is null) return UnicodoidStatus.InvalidParam;

        UnicodoidStatus status = TryNormalize(
            source.EncodingSystem, source.Bytes, form, out byte[] bytes);
        if (status != UnicodoidStatus.Ok) return status;

        return TextString.TryFromBytes(source.EncodingSystem, bytes, out result);
    }

    public static TextString Normalize(TextString source, NormalizationForm form)
    {
        UnicodoidStatus status = TryNormalize(source, form, out TextString? result);
        return status == UnicodoidStatus.Ok
            ? result!
            : throw new UnicodoidException(status);
    }

    /// <summary>True when the byte buffer is already in the requested form.</summary>
    public static bool IsNormalized(
        IEncodingSystem system,
        ReadOnlySpan<byte> input,
        NormalizationForm form)
    {
        if (system is null) return false;
        if (input.Length == 0) return true;

        UnicodoidStatus status = TryNormalize(system, input, form, out byte[] normalized);
        return status == UnicodoidStatus.Ok && normalized.AsSpan().SequenceEqual(input);
    }
}
