# Rust - UAX (Unicode Standard Annexes)

Part of the Unicodoid Unicode 18.0.0 implementation. Each module is a native
Rust module in `rust/src/uax/` over the generated Unicode 18.0.0 tables. The
full cross-language matrix is in the top-level
[report coverage](../../index.md#report-coverage).

| Doc | Specification | Rust module |
| --- | --- | --- |
| - | UAX #9 Bidirectional Algorithm | `uax::uax9` |
| - | UAX #11 East Asian Width | `uax::uax11` |
| - | UAX #14 Line Breaking Algorithm | `uax::uax14` |
| - | UAX #15 Normalization Forms | `uax::uax15` |
| - | UAX #24 Script Property | `uax::uax24` |
| [uax29.md](uax29.md) | UAX #29 Text Segmentation | `segmentation` |
| - | UAX #31 Identifiers and Syntax | `uax::uax31` |
| [uax34.md](uax34.md) | UAX #34 Named Character Sequences | `uax::uax34` |
| - | UAX #38 Han Database (Unihan) | `uax::uax38` |
| - | UAX #41 Common References | `uax::uax41` |
| - | UAX #42 UCD in XML | `uax::uax42` |
| - | UAX #44 Unicode Character Database | `uax::uax44` |
| - | UAX #45 U-source Ideographs | `uax::uax45` |
| - | UAX #50 Vertical Text Layout | `uax::uax50` |
| - | UAX #53 Arabic Mark Rendering | `uax::uax53` |
| - | UAX #57 Egyptian Hieroglyph Database | `uax::uax57` |
| - | UAX #60 Data for East Asian Scripts | `uax::uax60` |

**Status:** `tests/conformance.rs`, `tests/uax34.rs`, and `tests/new_reports.rs`
pass against the official Unicode 18.0.0 vectors.

*Generated tables:* `rust/src/generated/` (Unicode 18.0.0, generator 1.2.0).
