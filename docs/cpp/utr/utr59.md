# UTR #59 — East Asian Spacing (C++ implementation)

- **Specification:** UTR #59 East Asian Spacing (Unicode 18.0.0, Revision 1 draft)
- **Module:** `clike/cpp/src/utr/utr59_east_asian_spacing.cpp`
- **Generated data:** generated Unicode 18.0.0 East Asian Spacing table (generator 1.2.0)

## Implementation status

The C++ implementation answers East Asian Spacing (`W`, `N`, `O`, `C`) queries
and computes cluster spacing offsets from the generated Unicode 18.0.0
East_Asian_Spacing table and the conditional data referenced by UTR #59.

## Conformance

- `test_cpp_specs` validates the property over the official Unicode 18.0.0
  `east-asian-spacing.txt` data and passes.

## Out of scope

- Conditional East Asian Spacing resolution is policy driven and not part of the
  property conformance claim.
