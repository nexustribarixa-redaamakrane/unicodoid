# C — UTS (Unicode Technical Standards)

Part of the Unicodoid Unicode 18.0.0 implementation. Each note below describes
the C module, its conformance harness, and its limits.

| Doc | Specification | What it covers |
| --- | --- | --- |
| [uts10.md](uts10.md) | UTS #10 Unicode Collation Algorithm | DUCET-based sort keys, strengths, numeric ordering in `clike/src/unicodoid_collation.c` |
| [uts51.md](uts51.md) | UTS #51 Unicode Emoji | RGI emoji sequence validation (3,972 sequences) |
| [uts58.md](uts58.md) | UTS #58 Link Detection and Formatting | Detection termination, minimal escaping, email local part |

**Status:** CTest `collation`, `emoji_sequences`, and `linkification`
(`test_uts58`) pass against the official Unicode 18.0.0 vectors.

**Out of scope (by design):** URL initiation, CLDR tailorings, and UCA §11
search; see the per-spec notes for the exact boundary.

*Generated tables:* `clike/src/unicodoid_ucd_data.c` (Unicode 18.0.0,
generator 1.2.0).
