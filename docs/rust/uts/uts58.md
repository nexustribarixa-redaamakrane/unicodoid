# UTS #58 — Unicode Link Detection and Formatting (Rust implementation)

- **Specification:** UTS #58 Unicode Link Detection and Formatting: URLs and
  Email Addresses (Unicode 18.0.0, Revision 3)
- **Module:** `rust/src/uts/uts58.rs`
- **Generated data:** generated Unicode 18.0.0 link data (generator 1.2.0)

## Implementation status

The Rust implementation provides native link detection termination, URL minimal
escaping, and email local part detection over the generated Unicode 18.0.0 link
property tables.

## Conformance

- The Rust link suite runs the official Unicode 18.0.0 link test vectors and
  passes.

## Out of scope

- URL initiation is delegated to the caller, as in the specification.
