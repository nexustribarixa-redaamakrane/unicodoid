# Rust - UTS (Unicode Technical Standards)

Part of the Unicodoid Unicode 18.0.0 implementation. Each module is a native
Rust module in `rust/src/uts/` over the generated Unicode 18.0.0 tables. The
full cross-language matrix is in the top-level
[report coverage](../../index.md#report-coverage).

| Doc | Specification | Rust module |
| --- | --- | --- |
| [uts10.md](uts10.md) | UTS #10 Unicode Collation Algorithm | `uts::uts10` |
| - | UTS #18 Regular Expressions | `uts::uts18` |
| - | UTS #35 CLDR Part 1: Core | `uts::cldr` |
| - | UTS #37 Ideographic Variation Database | `uts::uts37` |
| - | UTS #39 Security Mechanisms | `uts::uts39` |
| - | UTS #46 IDNA Compatibility | `uts::uts46` |
| [uts51.md](uts51.md) | UTS #51 Unicode Emoji | `uts::uts51` |
| - | UTS #55 Source Code Handling | `uts::uts55` |
| [uts58.md](uts58.md) | UTS #58 Link Detection and Formatting | `uts::uts58` |
| - | UTS #61 Set Notation | `uts::uts61` |

**Status:** the collation, emoji-sequence, link-detection, and formatting
suites plus `tests/new_reports.rs` pass with `cargo test`.

**Out of scope (by design):** CLDR XML resource bundles, URL initiation, and
UCA search; see the per-spec notes.

*Generated tables:* `rust/src/generated/` (Unicode 18.0.0, generator 1.2.0).
