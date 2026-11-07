# UAX #29 — Unicode Text Segmentation (C++ implementation)

- **Specification:** UAX #29 Unicode Text Segmentation (Unicode 18.0.0, Revision 47)
- **Module:** `clike/cpp/src/uax/uax29_segmentation.cpp`
- **Generated data:** generated Unicode 18.0.0 tables (generator 1.2.0)

## Implementation status

The C++ implementation of UAX #29 is a native implementation over the generated
Unicode 18.0.0 property tables. It deliberately does not forward to the C
segmenter; the grapheme, word, line, and sentence rules are implemented in C++
against the generated `Grapheme_Cluster_Break`, `Word_Break`, `Line_Break`, and
`Sentence_Break` data.

- Includes the Unicode 18.0.0 GB9c conjunct-avoidance rule from generated Indic
  Conjunct_Break data.
- Boundary lists start at 0 and end at the input length.

## Conformance

- `test_cpp_conformance` runs the official Unicode 18.0.0 segmentation vectors
  through the native C++ segmenter and passes.

## Out of scope

- Cross-implementation equivalence with the C segmenter is asserted by test
  vectors; both must agree on the same generated data.
