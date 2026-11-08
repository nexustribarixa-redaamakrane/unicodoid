# C# - UTR (Unicode Technical Reports)

Part of the Unicodoid Unicode 18.0.0 implementation. Each module is a static
class in `clike/csharp/Unicodoid/Utr/` over the generated Unicode 18.0.0
tables. The full cross-language matrix is in the top-level
[report coverage](../../index.md#report-coverage).

| Doc | Specification | C# type |
| --- | --- | --- |
| - | UTR #17 Character Encoding Model | `Utr17` |
| - | UTR #23 Character Property Model | `Utr23` |
| - | UTR #25 Support for Mathematics | `Utr25` |
| - | UTR #33 Conformance Model | `Utr33` |
| - | UTR #54 Mongolian 12.1 Baseline | `Utr54` |
| - | UTR #56 Cuneiform Sign Lists | `Utr56` |
| [utr59.md](utr59.md) | UTR #59 East Asian Spacing | `Utr59` |

**Status:** the East Asian spacing conformance suite and the new-report
parity checks pass in `Unicodoid.Tests`.

**Out of scope:** conditional East Asian Spacing resolution is policy driven;
see the spec note.

*Generated tables:* `clike/csharp/Unicodoid/Generated/UcdData.cs` (Unicode
18.0.0, generator 1.2.0).
