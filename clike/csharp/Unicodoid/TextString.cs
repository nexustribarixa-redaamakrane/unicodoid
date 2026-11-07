using System;

namespace Unicodoid;

/// <summary>
/// Length-bounded byte string parameterized by an encoding system.  Strings
/// never depend on null terminators and may contain embedded zero bytes.  A
/// string keeps its own codec state so that appending characters to a modal
/// encoding produces one coherent stream.
/// </summary>
public sealed class TextString : IEquatable<TextString>, IComparable<TextString>
{
    private byte[] _buffer;
    private int _length;
    private object? _state;

    public IEncodingSystem EncodingSystem { get; }

    /// <summary>Length in bytes.</summary>
    public int Length => _length;

    /// <summary>Raw bytes without copying.</summary>
    public ReadOnlySpan<byte> Bytes => _buffer.AsSpan(0, _length);

    public TextString(IEncodingSystem system, int initialCapacity = 16)
    {
        EncodingSystem = system ?? throw new ArgumentNullException(nameof(system));
        _buffer = new byte[Math.Max(1, initialCapacity)];
        _state = system.CreateState();
    }

    /// <summary>Create an owned string by encoding a sequence of character values.</summary>
    public static TextString FromChars(
        IEncodingSystem system,
        ReadOnlySpan<CharacterValue> chars)
    {
        UnicodoidStatus status = TryFromChars(system, chars, out TextString? result);
        return status == UnicodoidStatus.Ok
            ? result!
            : throw new UnicodoidException(status);
    }

    public static UnicodoidStatus TryFromChars(
        IEncodingSystem system,
        ReadOnlySpan<CharacterValue> chars,
        out TextString? result)
    {
        result = null;
        if (system is null) return UnicodoidStatus.InvalidParam;

        UnicodoidStatus status = system.GetByteCount(chars, out int required);
        if (status != UnicodoidStatus.Ok) return status;

        var str = new TextString(system, Math.Max(16, required));
        foreach (CharacterValue ch in chars)
        {
            status = str.TryAppendChar(ch);
            if (status != UnicodoidStatus.Ok) return status;
        }

        result = str;
        return UnicodoidStatus.Ok;
    }

    /// <summary>Create an owned string from an already-encoded byte sequence.</summary>
    public static TextString FromBytes(IEncodingSystem system, ReadOnlySpan<byte> bytes)
    {
        UnicodoidStatus status = TryFromBytes(system, bytes, out TextString? result);
        return status == UnicodoidStatus.Ok
            ? result!
            : throw new UnicodoidException(status);
    }

    public static UnicodoidStatus TryFromBytes(
        IEncodingSystem system,
        ReadOnlySpan<byte> bytes,
        out TextString? result)
    {
        result = null;
        if (system is null) return UnicodoidStatus.InvalidParam;

        object? state = system.CreateState();
        UnicodoidStatus status = system.Validate(state, bytes, out _);
        if (status != UnicodoidStatus.Ok) return status;

        var str = new TextString(system, Math.Max(16, bytes.Length));
        bytes.CopyTo(str._buffer);
        str._length = bytes.Length;
        // Keep the state reached by the validated byte stream so that a later
        // append continues the same modal stream.
        str._state = state;
        result = str;
        return UnicodoidStatus.Ok;
    }

    public UnicodoidStatus TryAppendChar(CharacterValue ch)
    {
        int maxBytes = Math.Max(1, EncodingSystem.MaxBytesPerChar);
        EnsureCapacity(maxBytes);

        Span<byte> destination = _buffer.AsSpan(_length);
        UnicodoidStatus status = EncodingSystem.EncodeChar(
            _state, ch, destination, out int written);
        if (status != UnicodoidStatus.Ok) return status;

        _length += written;
        return UnicodoidStatus.Ok;
    }

    public void AppendChar(CharacterValue ch)
    {
        UnicodoidStatus status = TryAppendChar(ch);
        if (status != UnicodoidStatus.Ok) throw new UnicodoidException(status);
    }

    /// <summary>
    /// Append already-validated bytes without interpreting them.  The bytes are
    /// validated against a copy of the current state and the state is committed
    /// only on success.
    /// </summary>
    public UnicodoidStatus TryAppendBytes(ReadOnlySpan<byte> bytes)
    {
        if (bytes.Length == 0) return UnicodoidStatus.Ok;

        object? trial = EncodingSystem.CloneState(_state);
        UnicodoidStatus status = EncodingSystem.Validate(trial, bytes, out _);
        if (status != UnicodoidStatus.Ok) return status;

        EnsureCapacity(bytes.Length);
        bytes.CopyTo(_buffer.AsSpan(_length));
        _length += bytes.Length;
        _state = trial;
        return UnicodoidStatus.Ok;
    }

    public void AppendBytes(ReadOnlySpan<byte> bytes)
    {
        UnicodoidStatus status = TryAppendBytes(bytes);
        if (status != UnicodoidStatus.Ok) throw new UnicodoidException(status);
    }

    /// <summary>Count characters by decoding from the initial state.</summary>
    public UnicodoidStatus TryGetCharLength(out int charCount)
    {
        if (_length == 0)
        {
            charCount = 0;
            return UnicodoidStatus.Ok;
        }

        var codec = new Codec(EncodingSystem);
        return codec.CountChars(Bytes, out charCount);
    }

    public int CharLength
    {
        get
        {
            UnicodoidStatus status = TryGetCharLength(out int count);
            return status == UnicodoidStatus.Ok
                ? count
                : throw new UnicodoidException(status);
        }
    }

    /// <summary>Retrieve a character by zero-based character index.</summary>
    public UnicodoidStatus TryGetCharAt(int charIndex, out CharacterValue ch)
    {
        ch = CharacterValue.Invalid;
        if (charIndex < 0) return UnicodoidStatus.InvalidParam;

        var codec = new Codec(EncodingSystem);
        int index = 0;
        int offset = 0;

        while (offset < _length)
        {
            UnicodoidStatus status = codec.DecodeChar(
                Bytes[offset..], out CharacterValue decoded, out int read);

            if (status == UnicodoidStatus.NoCharacter)
            {
                if (read == 0) return UnicodoidStatus.InternalError;
                offset += read;
                continue;
            }
            if (status != UnicodoidStatus.Ok) return status;
            if (read == 0) return UnicodoidStatus.InternalError;

            if (index == charIndex)
            {
                ch = decoded;
                return UnicodoidStatus.Ok;
            }

            offset += read;
            ++index;
        }

        return UnicodoidStatus.NotFound;
    }

    public CharacterValue GetCharAt(int charIndex)
    {
        UnicodoidStatus status = TryGetCharAt(charIndex, out CharacterValue ch);
        return status == UnicodoidStatus.Ok
            ? ch
            : throw new UnicodoidException(status);
    }

    /// <summary>Reset the string's persistent codec state to its initial value.</summary>
    public void ResetState() => EncodingSystem.ResetState(_state);

    public TextString Clone()
    {
        var clone = new TextString(EncodingSystem, Math.Max(16, _length));
        Bytes.CopyTo(clone._buffer);
        clone._length = _length;
        clone._state = EncodingSystem.CloneState(_state);
        return clone;
    }

    public byte[] ToArray() => Bytes.ToArray();

    public bool ContentEquals(TextString? other) =>
        other is not null && Bytes.SequenceEqual(other.Bytes);

    public bool Equals(TextString? other) => ContentEquals(other);

    public override bool Equals(object? obj) => obj is TextString other && ContentEquals(other);

    public override int GetHashCode()
    {
        var hash = new HashCode();
        hash.AddBytes(Bytes);
        return hash.ToHashCode();
    }

    public int CompareTo(TextString? other)
    {
        if (other is null) return 1;
        return Bytes.SequenceCompareTo(other.Bytes);
    }

    private void EnsureCapacity(int extra)
    {
        int needed = _length + extra;
        if (needed <= _buffer.Length) return;

        int capacity = _buffer.Length * 2;
        if (capacity < needed) capacity = needed + 16;
        Array.Resize(ref _buffer, capacity);
    }
}
