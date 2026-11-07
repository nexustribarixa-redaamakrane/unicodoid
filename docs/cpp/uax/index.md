# C++ — UAX (Unicode Standard Annexes)

Part of the Unicodoid Unicode 18.0.0 implementation. The C++ modules are
**native implementations** over the generated Unicode 18.0.0 tables — they do
not forward to the C segmenter or property code.

| Doc | Specification | What it covers |
| --- | --- | --- |
| [uax15.md](uax15.md) | UAX #15 Normalization Forms | NFC/NFD/NFKC/NFKD decomposition, composition, and Hangul handling |
| [uax29.md](uax29.md) | UAX #29 Text Segmentation | Grapheme, word, line, and sentence rules in `cpp/src/uax/uax29_segmentation.cpp` (incl. GB9c conjunct avoidance) |
| [uax34.md](uax34.md) | UAX #34 Named Character Sequences | Name lookup (461 sequences) |
| [uax44.md](uax44.md) | UAX #44 Unicode Character Database | Property lookups (`General_Category`, `Script`, `Bidi_Class`, …) |

**Status:** `test_cpp_conformance` (segmentation), `test_cpp_specs`
(version + spec tables), and `test_cpp` pass in CTest.

*Generated tables:* shared Unicode 18.0.0 layer (generator 1.2.0), linked via
the C `unicodoid` library.
