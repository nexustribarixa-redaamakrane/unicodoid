# UAX #34 — Unicode Named Character Sequences (C implementation)

- **Specification:** UAX #34 Unicode Named Character Sequences (Unicode 18.0.0, Revision 23)
- **Module:** `clike/src/uax/uax34_named_sequences.c`
- **Generated data:** `clike/src/unicodoid_ucd_data.c` (Unicode 18.0.0, generator 1.2.0)

## Implementation status

The C implementation resolves named character sequences through the generated
Unicode 18.0.0 named sequence tables (`NamedSequences.txt`, revision 2 of the
Unicode 18.0.0 data set) exposed via `unicodoid_ucd_named_seq_*`.

- Sequence lookup, membership testing, and sequence enumeration are implemented
  over the generated sequence index.
- Unknown names and malformed scalar sequences are rejected rather than guessed.

## Conformance

- `clike/tests/test_uax34.c` validates every entry of the official Unicode 18.0.0
  `NamedSequences.txt` plus the generated lookup surface.

## Out of scope

- The Unicode 18.0.0 named sequence registry extensions introduced after
  Revision 23 of UAX #34 are not claimed; conformance is asserted only for the
  data revision the implementation was generated from.
