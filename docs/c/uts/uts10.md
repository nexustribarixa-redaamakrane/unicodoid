# UTS #10 — Unicode Collation Algorithm (C implementation)

- **Specification:** UTS #10 Unicode Collation Algorithm (Unicode 18.0.0, Revision 55)
- **Module:** `clike/src/uts/uts10_collation.c`
- **Generated data:** `clike/src/unicodoid_ucd_data.c` (Unicode 18.0.0, generator 1.2.0)

## Implementation status

The C implementation of UTS #10 builds the three-level sort key (primary,
secondary, tertiary) from the generated Unicode 18.0.0 collation weights and
supports strength selection, numeric ordering, and the `Non_Ignorable`
variable-weighting configuration used by the conformance suite.

- Default-ignorable and shifted conformance vectors are handled through the
  generated collation weights and implicit-weight tables.
- Comparison, sorting keys, and string ordering are derived from the generated
  weight data; no CLDR tailoring data is shipped.

## Conformance

- `clike/tests/test_collation.c` runs the official Unicode 18.0.0 UCA conformance
  suite (`data/ucd/18.0.0/uca/test/CollationTest/`) and passes.

## Out of scope

- CLDR tailorings, script reordering, semi-stability, and the §11 searching and
  matching algorithms are not implemented and are not claimed.
