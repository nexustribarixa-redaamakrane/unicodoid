# UTS #10 — Unicode Collation Algorithm (Rust implementation)

- **Specification:** UTS #10 Unicode Collation Algorithm (Unicode 18.0.0, Revision 55)
- **Module:** `rust/src/uts/uts10.rs`
- **Generated data:** generated Unicode 18.0.0 collation tables (generator 1.2.0)

## Implementation status

The Rust implementation builds three-level sort keys from the generated Unicode
18.0.0 collation weights and supports collation comparison with strength and
numeric ordering options.

## Conformance

- The Rust collation suite runs the official Unicode 18.0.0 UCA vectors and
  passes.

## Out of scope

- CLDR tailorings and UCA §11 features are not claimed.
