# Rust — UAX (Unicode Standard Annexes)

Part of the Unicodoid Unicode 18.0.0 implementation. Native Rust modules over
the generated Unicode 18.0.0 tables.

| Doc | Specification | What it covers |
| --- | --- | --- |
| [uax29.md](uax29.md) | UAX #29 Text Segmentation | `rust/src/segmentation.rs` — grapheme, word, line, sentence boundaries |
| [uax34.md](uax34.md) | UAX #34 Named Character Sequences | `rust/src/uax/uax34.rs` — 461 named sequences |

**Status:** `tests/conformance.rs` and `tests/uax34.rs` pass against the
official Unicode 18.0.0 vectors.

**Not in Rust's per-spec notes, but present in C++/C#:** UAX #15 normalization
and UAX #44 properties — see `docs/cpp/uax/index.md` and
`docs/csharp/uax/index.md`. Rust still ships `normalization.rs` and
`properties.rs` code paths verified by the conformance suites.

*Generated tables:* `rust/src/generated/` (Unicode 18.0.0, generator 1.2.0).
