# UTR #59 — East Asian Spacing (Rust implementation)

- **Specification:** UTR #59 East Asian Spacing (Unicode 18.0.0, Revision 1 draft)
- **Module:** `rust/src/utr/utr59.rs`
- **Generated data:** generated Unicode 18.0.0 East Asian Spacing table (generator 1.2.0)

## Implementation status

The Rust implementation answers East Asian Spacing (`W`, `N`, `O`, `C`) queries and
computes cluster spacing offsets from the generated Unicode 18.0.0
East_Asian_Spacing data.

## Conformance

- The Rust East Asian Spacing suite validates the official Unicode 18.0.0 data
  and passes.

## Out of scope

- Conditional East Asian Spacing resolution is policy driven.
