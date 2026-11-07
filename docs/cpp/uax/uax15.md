# UAX #15 — Unicode Normalization Forms (C++ implementation)

- **Specification:** UAX #15 Unicode Normalization Forms (Unicode 18.0.0, Revision 58)
- **Module:** `clike/cpp/src/uax/uax15_normalization.cpp`
- **Generated data:** generated Unicode 18.0.0 tables (generator 1.2.0)

## Implementation status

The C++ implementation of UAX #15 implements NFC, NFD, NFKC, and NFKD directly
over the generated Unicode 18.0.0 normalization tables, including recursive
decomposition, compatibility decomposition, Hangul algorithmic
decomposition/composition, canonical ordering, and canonical composition.

- Ill-formed scalar sequences (surrogates or out-of-range values) are passed
  through unchanged; callers validate input beforehand.

## Conformance

- The C++ normalization conformance suite (`test_normalization`) validates the
  full UAX #15 test vectors from the official Unicode 18.0.0 distribution.

## Out of scope

- Normalization exclusions and composition exclusions beyond the generated
  tables are not re-derived locally; the generated tables are authoritative.
