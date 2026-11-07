# C# — UAX (Unicode Standard Annexes)

Part of the Unicodoid Unicode 18.0.0 implementation. Native C# modules over
the generated Unicode 18.0.0 tables.

| Doc | Specification | What it covers |
| --- | --- | --- |
| [uax15.md](uax15.md) | UAX #15 Normalization Forms | `Uax15.cs` — decomposition, composition, Hangul handling |
| [uax29.md](uax29.md) | UAX #29 Text Segmentation | `Uax29.cs` — grapheme, word, line, sentence boundaries |
| [uax34.md](uax34.md) | UAX #34 Named Character Sequences | `Uax34.cs` — 461 named sequences |
| [uax44.md](uax44.md) | UAX #44 Unicode Character Database | `Ucd.cs` — property queries |

**Status:** the C# conformance harness runs the official Unicode 18.0.0
`NormalizationTest.txt` (20,171 cases / 403,420 checks), segmentation suites,
and named sequences, and passes (303 checks total in `Unicodoid.Tests`).

*Generated tables:* `clike/csharp/Unicodoid/Generated/UcdData.cs` (Unicode
18.0.0, generator 1.2.0).
