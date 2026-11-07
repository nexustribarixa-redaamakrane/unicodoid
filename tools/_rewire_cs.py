path = r"C:\Users\KARIMABENDA\Documents\unicodoid\clike\csharp\Unicodoid\Normalization.cs"
src = open(path, encoding="utf-8").read()

src = src.replace("""/// <summary>
/// Partial UAX #15 normalization for Unicode-compatible systems.
///
/// The module is opt-in: the system must expose a Unicode profile.  It covers
/// canonical decomposition and composition, compatibility mappings for a
/// documented subset, Hangul decomposition/composition, and canonical ordering
/// of combining marks.  It does not claim full UAX #15 conformance against the
/// complete UCD.
/// </summary>""",
"""/// <summary>
/// UAX #15 normalization for Unicode-compatible systems.
///
/// The module is opt-in: the system must expose a Unicode profile.  It
/// implements the full canonical and compatibility decomposition tables,
/// canonical pair composition, Hangul decomposition/composition and canonical
/// ordering of combining marks, all taken from the generated Unicode 18.0.0
/// tables (see <see cref="Ucd"/>).  It is exercised against the official
/// NormalizationTest.txt in the test suite.
/// </summary>""")

start = src.index("    private readonly struct Decomposition")
end = src.index("    private const uint HangulSBase")
src = src[:start] + src[end:]

old_decompose = """        if (s_decompositions.TryGetValue(codePoint, out Decomposition decomposition) &&
            (!decomposition.Compatibility || compatibility))
        {
            foreach (uint part in decomposition.Parts)
            {
                Decompose(part, compatibility, output);
            }
            return;
        }"""
new_decompose = """        uint[]? decomposition = Ucd.Decomposition(codePoint, compatibility);
        if (decomposition is not null)
        {
            foreach (uint part in decomposition)
            {
                Decompose(part, compatibility, output);
            }
            return;
        }"""
assert old_decompose in src
src = src.replace(old_decompose, new_decompose)

old_pair = """        foreach (KeyValuePair<uint, Decomposition> pair in s_decompositions)
        {
            Decomposition entry = pair.Value;
            if (entry.Compatibility || entry.Parts.Length != 2) continue;
            if (entry.Parts[0] == starter && entry.Parts[1] == ch)
            {
                composed = pair.Key;
                return true;
            }
        }

        return false;
    }"""
new_pair = """        return Ucd.TryComposePair(starter, ch, out composed);
    }"""
assert old_pair in src
src = src.replace(old_pair, new_pair)

src = src.replace("UnicodeProperties.CombiningClass(", "Ucd.CombiningClass(")

open(path, "w", encoding="utf-8").write(src)
print("done")
