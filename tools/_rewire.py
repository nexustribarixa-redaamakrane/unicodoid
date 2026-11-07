path = r"C:\Users\KARIMABENDA\Documents\unicodoid\rust\src\normalization.rs"
src = open(path, encoding="utf-8").read()

src = src.replace("""//! Partial UAX #15 normalization for Unicode-compatible systems.
//!
//! The module is opt-in: the system must expose a Unicode profile.  It covers
//! canonical decomposition and composition, compatibility mappings for a
//! documented subset, Hangul decomposition/composition, and canonical ordering
//! of combining marks.  It does **not** claim full UAX #15 conformance against
//! the complete Unicode Character Database.""",
"""//! UAX #15 normalization for Unicode-compatible systems.
//!
//! The module is opt-in: the system must expose a Unicode profile.  It
//! implements the full canonical and compatibility decomposition tables,
//! canonical pair composition, Hangul decomposition/composition and canonical
//! ordering of combining marks, all taken from the generated Unicode 18.0.0
//! tables (see `ucd_data`).  It is exercised against the official
//! `NormalizationTest.txt` in `tests/normalization.rs`.""")

start = src.index("struct Decomposition {")
end = src.index("const HANGUL_SBASE")
src = src[:start] + src[end:]

start = src.index("fn find_decomposition(")
end = src.index("fn decompose_scalar(")
src = src[:start] + src[end:]

src = src.replace("""    if let Some(entry) = find_decomposition(cp) {
        if !entry.compatibility || compatibility {
            for &part in entry.parts {
                decompose_scalar(part, compatibility, out);
            }
            return;
        }
    }

    out.push(cp);""",
"""    if let Some(parts) = crate::ucd::decomposition(cp, compatibility) {
        for part in parts {
            decompose_scalar(part, compatibility, out);
        }
        return;
    }

    out.push(cp);""")

src = src.replace("""    // Canonical pair composition from the table.
    for entry in DECOMPOSITIONS {
        if entry.compatibility || entry.parts.len() != 2 {
            continue;
        }
        if entry.parts[0] == starter && entry.parts[1] == ch {
            return Some(entry.cp);
        }
    }

    None
}""",
"""    // Canonical pair composition from the generated table.
    crate::ucd::compose_pair(starter, ch)
}""")

src = src.replace("use crate::properties::combining_class;\n", "")
src = src.replace("let class = combining_class(cp);", "let class = crate::ucd::combining_class(cp);")
src = src.replace("let previous = combining_class(sequence[j - 1]);", "let previous = crate::ucd::combining_class(sequence[j - 1]);")

open(path, "w", encoding="utf-8").write(src)
print("done")
