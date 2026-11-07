using System;

namespace Unicodoid;

/// <summary>
/// Status and error codes returned across the Unicodoid SDK.  The numeric
/// values are part of the cross-language contract and match the C SDK.
/// </summary>
public enum UnicodoidStatus
{
    Ok = 0,
    InvalidParam = -1,
    BufferTooSmall = -2,
    InvalidCodeUnit = -3,
    UnmappableChar = -4,
    IncompleteSequence = -5,
    NotFound = -6,
    AlreadyExists = -7,
    Unsupported = -8,
    CapacityExceeded = -9,
    AllocationFailed = -10,
    InternalError = -11,
    State = -12,
    Overflow = -13,
    NoCharacter = -14
}

/// <summary>
/// Character modeling architecture declared by an encoding system.  This is
/// descriptive metadata only; behaviour always comes from the system's
/// operations.
/// </summary>
public enum CharModel
{
    FixedWidth = 0,
    VariableWidth = 1,
    ModalStateful = 2,
    TokenBased = 3,
    Custom = 4
}

/// <summary>Byte order expectation for multi-byte code units.</summary>
public enum Endianness
{
    None = 0,
    LittleEndian = 1,
    BigEndian = 2
}

/// <summary>
/// Level of optional Unicode compatibility declared by an encoding system.
/// <see cref="None"/> is a first-class state: a system need not have any
/// relationship with Unicode.
/// </summary>
public enum CompatibilityLevel
{
    None = 0,
    Partial = 1,
    Full = 2,
    Profile = 3
}

/// <summary>Strategy applied during transcoding when a character cannot be mapped.</summary>
public enum TranscodePolicy
{
    Strict = 0,
    Replace = 1,
    Skip = 2,
    Callback = 3
}

/// <summary>
/// A character value in an encoding system.  The value may be a scalar, a
/// token identifier, or a composite reference; Unicodoid does not assume it is
/// a Unicode scalar value.
/// </summary>
public readonly struct CharacterValue : IEquatable<CharacterValue>, IComparable<CharacterValue>
{
    /// <summary>Sentinel used where no character value exists.</summary>
    public static readonly CharacterValue Invalid = new(ulong.MaxValue);

    public readonly ulong Value;

    public CharacterValue(ulong value) => Value = value;
    public CharacterValue(long value) => Value = unchecked((ulong)value);
    public CharacterValue(uint value) => Value = value;
    public CharacterValue(int value) => Value = unchecked((uint)value);
    public CharacterValue(ushort value) => Value = value;
    public CharacterValue(short value) => Value = unchecked((uint)value);
    public CharacterValue(byte value) => Value = value;
    public CharacterValue(sbyte value) => Value = unchecked((uint)value);
    public CharacterValue(char value) => Value = value;

    public bool IsInvalid => Value == ulong.MaxValue;

    public static implicit operator CharacterValue(ulong value) => new(value);
    public static implicit operator CharacterValue(uint value) => new(value);
    public static implicit operator CharacterValue(int value) => new(value);
    public static implicit operator CharacterValue(ushort value) => new(value);
    public static implicit operator CharacterValue(short value) => new(value);
    public static implicit operator CharacterValue(byte value) => new(value);
    public static implicit operator CharacterValue(sbyte value) => new(value);
    public static implicit operator CharacterValue(char value) => new(value);
    public static explicit operator ulong(CharacterValue ch) => ch.Value;
    public static explicit operator uint(CharacterValue ch) => (uint)ch.Value;

    public bool Equals(CharacterValue other) => Value == other.Value;
    public override bool Equals(object? obj) => obj is CharacterValue other && Equals(other);
    public override int GetHashCode() => Value.GetHashCode();
    public int CompareTo(CharacterValue other) => Value.CompareTo(other.Value);

    public static bool operator ==(CharacterValue left, CharacterValue right) => left.Equals(right);
    public static bool operator !=(CharacterValue left, CharacterValue right) => !left.Equals(right);
    public static bool operator <(CharacterValue left, CharacterValue right) => left.Value < right.Value;
    public static bool operator >(CharacterValue left, CharacterValue right) => left.Value > right.Value;
    public static bool operator <=(CharacterValue left, CharacterValue right) => left.Value <= right.Value;
    public static bool operator >=(CharacterValue left, CharacterValue right) => left.Value >= right.Value;

    public override string ToString() => $"0x{Value:X4}";
}

/// <summary>Unicode scalar value constants and helpers.</summary>
public static class UnicodeScalars
{
    public const uint MaxScalar = 0x0010FFFF;
    public const uint SurrogateMin = 0x0000D800;
    public const uint SurrogateMax = 0x0000DFFF;
    public const uint Replacement = 0x0000FFFD;

    public static bool IsValid(uint codePoint) =>
        codePoint <= MaxScalar &&
        (codePoint < SurrogateMin || codePoint > SurrogateMax);
}

/// <summary>Exception wrapper used by convenience APIs that do not expose status codes.</summary>
public sealed class UnicodoidException : Exception
{
    public UnicodoidStatus Status { get; }

    public UnicodoidException(UnicodoidStatus status)
        : base(status.ToMessage()) => Status = status;

    public UnicodoidException(UnicodoidStatus status, string message)
        : base(message) => Status = status;
}

/// <summary>Human-readable descriptions for status codes.</summary>
public static class StatusExtensions
{
    public static string ToMessage(this UnicodoidStatus status) => status switch
    {
        UnicodoidStatus.Ok => "ok",
        UnicodoidStatus.InvalidParam => "invalid parameter",
        UnicodoidStatus.BufferTooSmall => "buffer too small",
        UnicodoidStatus.InvalidCodeUnit => "invalid code unit",
        UnicodoidStatus.UnmappableChar => "unmappable character",
        UnicodoidStatus.IncompleteSequence => "incomplete sequence",
        UnicodoidStatus.NotFound => "not found",
        UnicodoidStatus.AlreadyExists => "already exists",
        UnicodoidStatus.Unsupported => "unsupported",
        UnicodoidStatus.CapacityExceeded => "capacity exceeded",
        UnicodoidStatus.AllocationFailed => "allocation failed",
        UnicodoidStatus.InternalError => "internal error",
        UnicodoidStatus.State => "invalid codec state",
        UnicodoidStatus.Overflow => "numeric overflow",
        UnicodoidStatus.NoCharacter => "no character produced",
        _ => "unknown status"
    };

    public static bool IsError(this UnicodoidStatus status) => status < 0;
}
