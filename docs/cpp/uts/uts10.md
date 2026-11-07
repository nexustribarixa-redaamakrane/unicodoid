# UTS #10 — Unicode Collation Algorithm (C++ implementation)

- **Specification:** UTS #10 Unicode Collation Algorithm (Unicode 18.0.0, Revision 55)
- **Module:** `clike/cpp/src/uts/uts10_collation.cpp`
- **Generated data:** generated Unicode 18.0.0 collation tables (generator 1.2.0)

## Implementation status

The C++ implementation builds three-level sort keys (primary, secondary,
tertiary) from the generated Unicode 18.0.0 collation weight data and exposes
collation comparison with strength and numeric-ordering options.

## Conformance

- `test_cpp_specs` validates the sort-key surface and comparison behaviour
  against the generated Unicode 18.0.0 tables and passes.

## Out of scope

- DUCET replacement, CLDR tailorings, and UCA §11 search algorithms are not
  implemented and are not claimed.
