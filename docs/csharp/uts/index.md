# C# - UTS (Unicode Technical Standards)

Part of the Unicodoid Unicode 18.0.0 implementation. Each module is a static
class in `clike/csharp/Unicodoid/Uts/` over the generated Unicode 18.0.0
tables. The full cross-language matrix is in the top-level
[report coverage](../../index.md#report-coverage).

| Doc | Specification | C# type |
| --- | --- | --- |
| [uts10.md](uts10.md) | UTS #10 Unicode Collation Algorithm | `Uts10` |
| - | UTS #18 Regular Expressions | `Uts18` |
| - | UTS #35 CLDR Part 1: Core | `Cldr` |
| - | UTS #37 Ideographic Variation Database | `Uts37` |
| - | UTS #39 Security Mechanisms | `Uts39` |
| - | UTS #46 IDNA Compatibility | `Uts46` |
| [uts51.md](uts51.md) | UTS #51 Unicode Emoji | `Uts51` |
| - | UTS #55 Source Code Handling | `Uts55` |
| [uts58.md](uts58.md) | UTS #58 Link Detection and Formatting | `Uts58` |
| - | UTS #61 Set Notation | `Uts61` |

**Status:** the C# harness runs the emoji-sequence, link-detection, and
link-formatting conformance suites plus the new-report parity checks;
`Unicodoid.Tests` reports 362 checks passed.

**Out of scope (by design):** CLDR XML resource bundles, URL initiation, and
UCA search; see the per-spec notes.

*Generated tables:* `clike/csharp/Unicodoid/Generated/UcdData.cs` (Unicode
18.0.0, generator 1.2.0).
