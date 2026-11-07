# UAX #29 — Unicode Text Segmentation (Rust implementation)

- **Specification:** UAX #29 Unicode Text Segmentation (Unicode 18.0.0, Revision 47)
- **Module:** `rust/src/uax/uax29.rs`
- **Generated data:** `rust/src/generated/ucd_data.rs` (Unicode 18.0.0, generator 1.2.0)

## Implementation status

The Rust implementation of UAX #29 implements grapheme cluster, word, line, and
sentence segmentation over the generated Unicode 18.0.0 property tables,
including the Unicode 18.0.0 GB9c conjunct-avoidance rule.

## Conformance

- The Rust segmentation conformance suite runs the official Unicode 18.0.0
  break vectors and passes.

## Out of scope

- Cross-implementation agreement with the C segmenter is asserted over the same
  generated data.
