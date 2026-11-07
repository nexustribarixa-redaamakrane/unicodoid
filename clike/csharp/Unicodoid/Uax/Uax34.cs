using System;

namespace Unicodoid;

/// <summary>
/// UAX #34 Unicode Named Character Sequences (Unicode 18.0.0, Revision 23).
///
/// Exposes the normative <c>NamedSequences.txt</c> data and validates scalar
/// sequences against it.  Names are compared exactly; callers that need loose
/// matching apply their own normalization first.
/// </summary>
public static class Uax34
{
    /// <summary>Number of registered named character sequences.</summary>
    public static int Count => Ucd.NamedSeqCount;

    /// <summary>Name of the sequence at <paramref name="index"/>, or null.</summary>
    public static string? Name(int index) => Ucd.NamedSeqName(index);

    /// <summary>Code points of the sequence at <paramref name="index"/>, or null.</summary>
    public static uint[]? Sequence(int index) => Ucd.NamedSeqCps(index);

    /// <summary>Look up a sequence by exact name.</summary>
    public static uint[]? Find(string name)
    {
        int index = Ucd.NamedSeqFind(name);
        return index < 0 ? null : Ucd.NamedSeqCps(index);
    }

    private static bool ValidScalar(uint cp) =>
        cp <= 0x10FFFF && cp is < 0xD800 or > 0xDFFF;

    /// <summary>True when <paramref name="cps"/> is exactly one registered sequence.</summary>
    public static bool IsRegistered(ReadOnlySpan<uint> cps)
    {
        if (cps.IsEmpty) return false;
        foreach (uint cp in cps)
        {
            if (!ValidScalar(cp)) return false;
        }

        for (int i = 0; i < Count; ++i)
        {
            uint[]? sequence = Sequence(i);
            if (sequence is not null && cps.SequenceEqual(sequence)) return true;
        }
        return false;
    }
}
