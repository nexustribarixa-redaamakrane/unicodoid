# C# - UAX (Unicode Standard Annexes)

Part of the Unicodoid Unicode 18.0.0 implementation. Each module is a static
class in `clike/csharp/Unicodoid/Uax/` backed by the generated Unicode 18.0.0
tables. The full cross-language matrix is in the top-level
[report coverage](../../index.md#report-coverage).

| Doc | Specification | C# type |
| --- | --- | --- |
| - | UAX #9 Bidirectional Algorithm | `Uax9` (`Uax9.cs`, `Uax9Tables.cs`) |
| - | UAX #11 East Asian Width | `Uax11` |
| - | UAX #14 Line Breaking Algorithm | `Uax14` (plus `Segmenter.LineBoundaries`) |
| [uax15.md](uax15.md) | UAX #15 Normalization Forms | `Normalization` |
| - | UAX #24 Script Property | `Uax24` |
| [uax29.md](uax29.md) | UAX #29 Text Segmentation | `Segmenter` |
| - | UAX #31 Identifiers and Syntax | `Uax31` |
| [uax34.md](uax34.md) | UAX #34 Named Character Sequences | `Uax34` |
| - | UAX #38 Han Database (Unihan) | `Uax38` |
| - | UAX #41 Common References | `Uax41` |
| - | UAX #42 UCD in XML | `Uax42` |
| [uax44.md](uax44.md) | UAX #44 Unicode Character Database | `Uax44`, `UnicodeProperties` |
| - | UAX #45 U-source Ideographs | `Uax45` |
| - | UAX #50 Vertical Text Layout | `Uax50` |
| - | UAX #53 Arabic Mark Rendering | `Uax53` |
| - | UAX #57 Egyptian Hieroglyph Database | `Uax57` |
| - | UAX #60 Data for East Asian Scripts | `Uax60` |

**Status:** the C# harness runs the official Unicode 18.0.0
`NormalizationTest.txt` (20,171 cases / 403,420 checks), the segmentation
suites, named sequences, and the new-report parity checks; `Unicodoid.Tests`
reports 362 checks passed.

*Generated tables:* `clike/csharp/Unicodoid/Generated/UcdData.cs` (Unicode
18.0.0, generator 1.2.0).
