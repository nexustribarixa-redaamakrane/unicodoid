using System;

namespace Unicodoid.Systems;

/// <summary>
/// ShiftModal-16: a deliberately non-Unicode, modal, stateful example system.
///
/// Base mode:
///   0x00..0x7F  -> values 0x0000..0x007F, except the three control bytes below.
///   0x1B b      -> value 0x0100 | b   (escape; consumes the following byte)
///   0x0E        -> switch to shifted mode (emits no character)
///   0x0F        -> switch to base mode    (emits no character)
///
/// Shifted mode:
///   0x0F        -> switch to base mode    (emits no character)
///   0x1B b      -> value 0x0100 | b      (escape; mode is unchanged)
///   b           -> value 0x4000 | b      (0x0E is ordinary data here)
///
/// The system intentionally has no Unicode profile: it exists to prove that
/// the codec, string, and transcoding layers do not assume a Unicode
/// relationship.
/// </summary>
public sealed class ShiftModal16System : EncodingSystemBase
{
    public static readonly ShiftModal16System Instance = new();

    private const byte Esc = 0x1B;
    private const byte So = 0x0E;
    private const byte Si = 0x0F;

    private sealed class ShiftModalState
    {
        public bool Shifted;
    }

    private ShiftModal16System() { }

    public override string Id => "org.unicodoid.shiftmodal16";
    public override string Name => "ShiftModal-16";
    public override string Version => "1.0.0";
    public override string Author => "Unicodoid Reference Systems";
    public override string Description => "Modal, stateful 16-bit example system with no Unicode profile";
    public override int CodeUnitBits => 8;
    public override int MinBytesPerChar => 1;
    public override int MaxBytesPerChar => 2;
    public override CharModel Model => CharModel.ModalStateful;
    public override Endianness Endianness => Endianness.None;
    public override CompatibilityLevel CompatibilityLevel => CompatibilityLevel.None;
    public override CharacterValue MaxCharValue => 0x40FF;
    public override CharacterValue ReplacementChar => 0x003F;
    public override UnicodeProfile? UnicodeProfile => null;

    public override int StateSize => 1;

    public override object? CreateState() => new ShiftModalState();

    public override void ResetState(object? state)
    {
        if (state is ShiftModalState modal)
        {
            modal.Shifted = false;
        }
    }

    public override object? CloneState(object? state) =>
        state is ShiftModalState modal
            ? new ShiftModalState { Shifted = modal.Shifted }
            : null;

    private static ShiftModalState GetState(object? state) =>
        state as ShiftModalState ?? new ShiftModalState();

    private static bool IsBaseReserved(byte value) =>
        value is Esc or So or Si;

    public override UnicodoidStatus EncodeChar(
        object? state,
        CharacterValue ch,
        Span<byte> destination,
        out int bytesWritten)
    {
        bytesWritten = 0;
        ShiftModalState modal = GetState(state);

        if (ch.Value <= 0x7F)
        {
            byte value = (byte)ch.Value;
            if (IsBaseReserved(value)) return UnicodoidStatus.UnmappableChar;

            if (modal.Shifted)
            {
                if (destination.Length < 2) return UnicodoidStatus.BufferTooSmall;
                destination[0] = Si;
                destination[1] = value;
                modal.Shifted = false;
                bytesWritten = 2;
            }
            else
            {
                if (destination.Length < 1) return UnicodoidStatus.BufferTooSmall;
                destination[0] = value;
                bytesWritten = 1;
            }
            return UnicodoidStatus.Ok;
        }

        if (ch.Value is >= 0x0100 and <= 0x01FF)
        {
            if (destination.Length < 2) return UnicodoidStatus.BufferTooSmall;
            destination[0] = Esc;
            destination[1] = (byte)(ch.Value & 0xFF);
            bytesWritten = 2;
            return UnicodoidStatus.Ok;
        }

        if (ch.Value is >= 0x4000 and <= 0x40FF)
        {
            byte value = (byte)(ch.Value & 0xFF);
            // In shifted mode SI and ESC are controls, not data.
            if (value is Si or Esc) return UnicodoidStatus.UnmappableChar;

            if (!modal.Shifted)
            {
                if (destination.Length < 2) return UnicodoidStatus.BufferTooSmall;
                destination[0] = So;
                destination[1] = value;
                modal.Shifted = true;
                bytesWritten = 2;
            }
            else
            {
                if (destination.Length < 1) return UnicodoidStatus.BufferTooSmall;
                destination[0] = value;
                bytesWritten = 1;
            }
            return UnicodoidStatus.Ok;
        }

        return UnicodoidStatus.UnmappableChar;
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

        ShiftModalState modal = GetState(state);
        bool saved = modal.Shifted;
        int consumed = 0;

        while (consumed < source.Length)
        {
            byte value = source[consumed];

            if (value == Esc)
            {
                if (consumed + 1 >= source.Length)
                {
                    modal.Shifted = saved;
                    return UnicodoidStatus.IncompleteSequence;
                }
                ch = 0x0100u | source[consumed + 1];
                bytesRead = consumed + 2;
                return UnicodoidStatus.Ok;
            }

            if (value == Si)
            {
                modal.Shifted = false;
                ++consumed;
                continue;
            }

            if (modal.Shifted)
            {
                ch = 0x4000u | value;
                bytesRead = consumed + 1;
                return UnicodoidStatus.Ok;
            }

            if (value == So)
            {
                modal.Shifted = true;
                ++consumed;
                continue;
            }

            ch = value;
            bytesRead = consumed + 1;
            return UnicodoidStatus.Ok;
        }

        // Only shift controls were present: the state change is committed.
        bytesRead = consumed;
        return UnicodoidStatus.NoCharacter;
    }

    public override UnicodoidStatus GetByteCount(
        ReadOnlySpan<CharacterValue> chars,
        out int byteCount)
    {
        byteCount = 0;
        bool shifted = false;
        int total = 0;

        foreach (CharacterValue ch in chars)
        {
            if (ch.Value <= 0x7F)
            {
                if (IsBaseReserved((byte)ch.Value)) return UnicodoidStatus.UnmappableChar;
                if (shifted)
                {
                    total += 2;
                    shifted = false;
                }
                else
                {
                    total += 1;
                }
            }
            else if (ch.Value is >= 0x0100 and <= 0x01FF)
            {
                total += 2;
            }
            else if (ch.Value is >= 0x4000 and <= 0x40FF)
            {
                byte value = (byte)(ch.Value & 0xFF);
                if (value is Si or Esc) return UnicodoidStatus.UnmappableChar;
                if (!shifted)
                {
                    total += 2;
                    shifted = true;
                }
                else
                {
                    total += 1;
                }
            }
            else
            {
                return UnicodoidStatus.UnmappableChar;
            }
        }

        byteCount = total;
        return UnicodoidStatus.Ok;
    }
}
