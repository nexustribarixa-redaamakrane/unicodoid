# UTR #59 — East Asian Spacing (C implementation)

- **Specification:** UTR #59 East Asian Spacing (Unicode 18.0.0, Revision 1 draft)
- **Module:** `clike/src/utr/utr59_east_asian_spacing.c`
- **Generated data:** `clike/src/unicodoid_ucd_data.c` (Unicode 18.0.0, generator 1.2.0)

## Implementation status

The C implementation answers East Asian Spacing (`W`, `N`, `O`, `C`) queries from
the official Unicode 18.0.0 `tr59/east-asian-spacing.txt` data through the
generated East_Asian_Spacing table, and computes per-cluster spacing offsets for
the UAX #15 / UAX #11 pair-adjacency behavior.

## Conformance

- `clike/tests/test_utr59.c` validates the East Asian Spacing property over the
  official Unicode 18.0.0 data file and passes.

## Out of scope

- The conditional (C) East Asian Spacing values are resolved through the
  conditional data referenced by the official UTR #59 file; resolution policy is
  implementation defined and not part of the conformance claim.
