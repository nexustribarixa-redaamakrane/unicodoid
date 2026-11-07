# UAX #34 — Unicode Named Character Sequences (C++ implementation)

- **Specification:** UAX #34 Unicode Named Character Sequences (Unicode 18.0.0, Revision 23)
- **Module:** `clike/cpp/src/uax/uax34_named_sequences.cpp`
- **Generated data:** generated Unicode 18.0.0 tables (generator 1.2.0)

## Implementation status

The C++ implementation resolves named character sequences through the generated
Unicode 18.0.0 named sequence tables, mirroring the C data layer accessors with a
C++-typed API.

## Conformance

- `test_cpp_specs` validates every entry of the official Unicode 18.0.0
  `NamedSequences.txt` through the C++ named sequence module and passes.

## Out of scope

- Only the Unicode 18.0.0 data revision covered by the generated tables is
  claimed.
