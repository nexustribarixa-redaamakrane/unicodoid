# UAX #15 — Unicode Normalization Forms (C# implementation)

- **Specification:** UAX #15 Unicode Normalization Forms (Unicode 18.0.0, Revision 58)
- **Module:** `clike/csharp/Unicodoid/Uax/Uax15.cs`
- **Generated data:** `clike/csharp/Unicodoid/Generated/UcdData.cs` (Unicode 18.0.0, generator 1.2.0)

## Implementation status

The C# implementation of UAX #15 implements NFC, NFD, NFKC, and NFKD over the
generated Unicode 18.0.0 normalization tables, including recursive
decomposition, compatibility decomposition, Hangul algorithmic
decomposition/composition, canonical ordering, and canonical composition.

## Conformance

- The C# normalization conformance suite validates the official Unicode 18.0.0
  normalization vectors and passes.

## Out of scope

- Generated tables are authoritative; no local re-derivation is performed.
