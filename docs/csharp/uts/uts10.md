# UTS #10 — Unicode Collation Algorithm (C# implementation)

- **Specification:** UTS #10 Unicode Collation Algorithm (Unicode 18.0.0, Revision 55)
- **Module:** `clike/csharp/Unicodoid/Uts/Uts10.cs`
- **Generated data:** generated Unicode 18.0.0 collation tables (generator 1.2.0)

## Implementation status

The C# implementation builds three-level sort keys from the generated Unicode
18.0.0 collation weights and supports collation comparison with strength and
numeric ordering options.

## Conformance

- The C# collation conformance harness runs the official Unicode 18.0.0 UCA
  test vectors and passes.

## Out of scope

- CLDR tailorings and UCA §11 features are not claimed.
