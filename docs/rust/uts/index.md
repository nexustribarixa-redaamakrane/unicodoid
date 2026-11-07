# Rust — UTS (Unicode Technical Standards)

Part of the Unicodoid Unicode 18.0.0 implementation. Native Rust modules over
generated Unicode 18.0.0 data.

| Doc | Specification | What it covers |
| --- | --- | --- |
| [uts10.md](uts10.md) | UTS #10 Unicode Collation Algorithm | `rust/src/uts/uts10.rs` — sort keys, strengths, numeric ordering |
| [uts51.md](uts51.md) | UTS #51 Unicode Emoji | `rust/src/uts/uts51.rs` — RGI emoji sequence validation (3,972) |
| [uts58.md](uts58.md) | UTS #58 Link Detection and Formatting | `rust/src/uts/uts58.rs` — detection, escaping, email local part |

**Status:** `tests/collation.rs`, `tests/uts51.rs`, and `tests/uts58.rs`
validate the official Unicode 18.0.0 vectors and pass.

**Out of scope (by design):** URL initiation, CLDR tailorings, and UCA §11
search; see the per-spec notes.

*Generated tables:* `rust/src/generated/` (Unicode 18.0.0, generator 1.2.0).
