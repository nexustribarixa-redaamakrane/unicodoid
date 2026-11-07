# UTS #58 — Unicode Link Detection and Formatting (C# implementation)

- **Specification:** UTS #58 Unicode Link Detection and Formatting: URLs and
  Email Addresses (Unicode 18.0.0, Revision 3)
- **Module:** `clike/csharp/Unicodoid/Uts/Uts58.cs`
- **Generated data:** generated Unicode 18.0.0 link data (generator 1.2.0)

## Implementation status

The C# implementation provides native link detection termination, URL minimal
escaping, and email local part detection over the generated Unicode 18.0.0 link
property tables.

## Conformance

- The C# link conformance harness runs the official Unicode 18.0.0 link test
  vectors and passes.

## Out of scope

- URL initiation is delegated to the caller, as in the specification.
