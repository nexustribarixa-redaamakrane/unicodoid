# C — UAX (Unicode Standard Annexes)

Part of the Unicodoid [Unicode 18.0.0](../../../docs/unicode18/inventory.json)
implementation. Each note below describes the C module, its conformance
harness, and its limits.

| Doc | Specification | What it covers |
| --- | --- | --- |
| [uax29.md](uax29.md) | UAX #29 Text Segmentation | Grapheme, word, line, and sentence segmentation in `clike/src/unicodoid_segmentation.c` |
| [uax34.md](uax34.md) | UAX #34 Named Character Sequences | Name lookup and membership over `NamedSequences.txt` |

**Status:** `test_core`, `test_conformance` (UCD 18.0.0), and `test_uax34`
(461 named sequences) pass in CTest.

**Not implemented in C, but present in C++/C#:** UAX #15 normalization and
UAX #44 properties — see `docs/cpp/uax/index.md` and
`docs/csharp/uax/index.md`. The C SDK still ships normalization and property
code paths; their detailed spec notes live with the native C++ and C# ports
that carry the spec-specific harnesses.

*Generated tables:* `clike/src/unicodoid_ucd_data.c` (Unicode 18.0.0,
generator 1.2.0).
