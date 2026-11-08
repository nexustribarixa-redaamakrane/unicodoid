# Rust - UTR (Unicode Technical Reports)

Part of the Unicodoid Unicode 18.0.0 implementation. Each module is a native
Rust module in `rust/src/utr/` over the generated Unicode 18.0.0 tables. The
full cross-language matrix is in the top-level
[report coverage](../../index.md#report-coverage).

| Doc | Specification | Rust module |
| --- | --- | --- |
| - | UTR #17 Character Encoding Model | `utr::utr17` |
| - | UTR #23 Character Property Model | `utr::utr23` |
| - | UTR #25 Support for Mathematics | `utr::utr25` |
| - | UTR #33 Conformance Model | `utr::utr33` |
| - | UTR #54 Mongolian 12.1 Baseline | `utr::utr54` |
| - | UTR #56 Cuneiform Sign Lists | `utr::utr56` |
| [utr59.md](utr59.md) | UTR #59 East Asian Spacing | `utr::utr59` |

**Status:** the East Asian spacing suite and `tests/new_reports.rs` pass with
`cargo test`.

**Out of scope:** conditional East Asian Spacing resolution is policy driven;
see the spec note.

*Generated tables:* `rust/src/generated/` (Unicode 18.0.0, generator 1.2.0).
