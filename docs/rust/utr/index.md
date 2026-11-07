# Rust — UTR (Unicode Technical Reports)

Part of the Unicodoid Unicode 18.0.0 implementation.

| Doc | Specification | What it covers |
| --- | --- | --- |
| [utr59.md](utr59.md) | UTR #59 East Asian Spacing | `rust/src/utr/utr59.rs` — `W`/`N`/`O`/`C` spacing queries and cluster offsets |

**Status:** `tests/utr59.rs` validates the official `east-asian-spacing.txt`
data (1,299 ranges) and passes.

**Out of scope:** conditional East Asian Spacing resolution is policy driven.

*Generated tables:* `rust/src/generated/` (Unicode 18.0.0, generator 1.2.0).
