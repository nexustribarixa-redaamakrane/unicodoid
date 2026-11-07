# UTR #59 — East Asian Spacing (C# implementation)

- **Specification:** UTR #59 East Asian Spacing (Unicode 18.0.0, Revision 1 draft)
- **Module:** `clike/csharp/Unicodoid/Utr/Utr59.cs`
- **Generated data:** generated Unicode 18.0.0 East Asian Spacing table (generator 1.2.0)

## Implementation status

The C# implementation answers East Asian Spacing (`W`, `N`, `O`, `C`) queries and
computes cluster spacing offsets from the generated Unicode 18.0.0
East_Asian_Spacing data.

## Conformance

- The C# East Asian Spacing conformance harness validates the official Unicode
  18.0.0 data and passes.

## Out of scope

- Conditional East Asian Spacing resolution is policy driven.
