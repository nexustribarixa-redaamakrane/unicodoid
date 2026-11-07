# UAX #34 — Unicode Named Character Sequences (Rust implementation)

- **Specification:** UAX #34 Unicode Named Character Sequences (Unicode 18.0.0, Revision 23)
- **Module:** `rust/src/uax/uax34.rs`
- **Generated data:** `rust/src/generated/ucd_data.rs` (Unicode 18.0.0, generator 1.2.0)

## Implementation status

The Rust implementation resolves named character sequences through the generated
Unicode 18.0.0 named sequence tables.

## Conformance

- The Rust named sequence suite validates every official Unicode 18.0.0
  `NamedSequences.txt` entry and passes.

## Out of scope

- Conformance is limited to the Unicode 18.0.0 data revision shipped in the
  generated tables.
