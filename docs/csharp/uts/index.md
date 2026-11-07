# C# — UTS (Unicode Technical Standards)

Part of the Unicodoid Unicode 18.0.0 implementation. Native C# modules over
generated Unicode 18.0.0 data.

| Doc | Specification | What it covers |
| --- | --- | --- |
| [uts10.md](uts10.md) | UTS #10 Unicode Collation Algorithm | `Uts10.cs` — sort keys, strengths, numeric ordering |
| [uts51.md](uts51.md) | UTS #51 Unicode Emoji | `Uts51.cs` — RGI emoji sequence validation (3,972) |
| [uts58.md](uts58.md) | UTS #58 Link Detection and Formatting | `Uts58.cs` — detection termination, escaping, email local part |

**Status:** the C# conformance harness validates collation (201,036 cases),
emoji, and link detection/formatting against the official Unicode 18.0.0
vectors and passes.

**Out of scope (by design):** URL initiation, CLDR tailorings, and UCA §11
search; see the per-spec notes.

*Generated tables:* `Generated/UcdData.cs` (Unicode 18.0.0, generator 1.2.0).
