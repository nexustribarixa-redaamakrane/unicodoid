# UAX #44 — Unicode Character Database Properties (C# implementation)

- **Specification:** UAX #44 Unicode Character Database (Unicode 18.0.0)
- **Module:** `clike/csharp/Unicodoid/Uax/Uax44.cs`
- **Generated data:** `clike/csharp/Unicodoid/Generated/UcdData.cs` (Unicode 18.0.0, generator 1.2.0)

## Implementation status

The C# implementation exposes the UAX #44 property surface through typed
accessors over the generated Unicode 18.0.0 character database tables, including
general category, canonical combining class, and binary properties such as
Grapheme_Extend and Grapheme_Base.

## Conformance

- The C# property conformance checks cover representative script, combining
  mark, and grapheme extend values from the generated Unicode 18.0.0 data.

## Out of scope

- Property enumeration is delegated to the generated tables.
