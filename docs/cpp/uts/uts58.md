# UTS #58 — Unicode Link Detection and Formatting (C++ implementation)

- **Specification:** UTS #58 Unicode Link Detection and Formatting: URLs and
  Email Addresses (Unicode 18.0.0, Revision 3)
- **Module:** `clike/cpp/src/uts/uts58_link_detection.cpp`
- **Generated data:** generated Unicode 18.0.0 link data (generator 1.2.0)

## Implementation status

The C++ implementation provides native link detection termination, URL minimal
escaping, and email local part detection over the generated Unicode 18.0.0 link
property tables.

## Conformance

- `test_cpp_specs` validates the unit algorithms and the official
  `LinkDetectionTest.txt` conformance vectors and passes.

## Out of scope

- URL initiation is delegated to the caller, as in the specification.
