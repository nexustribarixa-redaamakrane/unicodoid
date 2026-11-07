# UAX #44 — Unicode Character Database Properties (C++ implementation)

- **Specification:** UAX #44 Unicode Character Database (Unicode 18.0.0)
- **Module:** `clike/cpp/src/uax/uax44_properties.cpp`
- **Generated data:** generated Unicode 18.0.0 tables (generator 1.2.0)

## Implementation status

The C++ implementation exposes the UAX #44 property surface as typed accessors
over the generated Unicode 18.0.0 character database tables: general category,
canonical combining class, and binary properties including Grapheme_Extend,
Grapheme_Base, and Grapheme_Link.

## Conformance

- `test_cpp_specs` checks representative property values (Latin, combining marks,
  Devanagari) against the generated Unicode 18.0.0 data and passes.

## Out of scope

- Full property enumeration is delegated to the generated tables; the C++
  module performs no algorithm of its own beyond typed access.
