using System;

namespace Unicodoid;

/// <summary>
/// Contract implemented by all independent encoding systems in the Unicodoid
/// SDK.  Operations are state-aware: stateless systems ignore the state
/// argument, while modal systems receive a per-stream state object created by
/// <see cref="CreateState"/>.
/// </summary>
public interface IEncodingSystem
{
    string Id { get; }
    string Name { get; }
    string Version { get; }
    string Author { get; }
    string Description { get; }

    int CodeUnitBits { get; }
    int MinBytesPerChar { get; }
    int MaxBytesPerChar { get; }
    CharModel Model { get; }
    Endianness Endianness { get; }
    CompatibilityLevel CompatibilityLevel { get; }
    CharacterValue MaxCharValue { get; }
    CharacterValue ReplacementChar { get; }

    /// <summary>Optional Unicode profile; <c>null</c> means no Unicode relationship.</summary>
    UnicodeProfile? UnicodeProfile { get; }

    /// <summary>Size in bytes of one codec state object; zero for stateless systems.</summary>
    int StateSize => 0;

    /// <summary>True when the system requires per-stream codec state.</summary>
    bool IsStateful => StateSize > 0;

    /// <summary>True when the system declares and provides a usable Unicode profile.</summary>
    bool HasUnicodeProfile =>
        UnicodeProfile is not null &&
        CompatibilityLevel != CompatibilityLevel.None;

    /// <summary>Create a fresh state object, or <c>null</c> for stateless systems.</summary>
    object? CreateState() => null;

    /// <summary>Reset a state object to its initial value.</summary>
    void ResetState(object? state) { }

    /// <summary>Clone a state object so that trial operations can be rolled back.</summary>
    object? CloneState(object? state) => state;

    /// <summary>Encode one character value, updating state.</summary>
    UnicodoidStatus EncodeChar(
        object? state,
        CharacterValue ch,
        Span<byte> destination,
        out int bytesWritten);

    /// <summary>
    /// Decode one character value, updating state.  Returns
    /// <see cref="UnicodoidStatus.NoCharacter"/> when input was consumed but
    /// produced no character (modal shift controls).
    /// </summary>
    UnicodoidStatus DecodeChar(
        object? state,
        ReadOnlySpan<byte> source,
        out CharacterValue ch,
        out int bytesRead);

    /// <summary>Validate a complete byte buffer with the supplied state.</summary>
    UnicodoidStatus Validate(
        object? state,
        ReadOnlySpan<byte> source,
        out int invalidOffset);

    /// <summary>Count characters in a complete byte buffer with the supplied state.</summary>
    UnicodoidStatus CountChars(
        object? state,
        ReadOnlySpan<byte> source,
        out int count);

    /// <summary>Exact number of bytes needed to encode a sequence of characters.</summary>
    UnicodoidStatus GetByteCount(
        ReadOnlySpan<CharacterValue> chars,
        out int byteCount);
}
