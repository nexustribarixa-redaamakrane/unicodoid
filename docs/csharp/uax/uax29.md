# UAX #29 — Unicode Text Segmentation (C# implementation)

- **Specification:** UAX #29 Unicode Text Segmentation (Unicode 18.0.0, Revision 47)
- **Module:** `clike/csharp/Unicodoid/Uax/Uax29.cs`
- **Generated data:** generated Unicode 18.0.0 property tables (generator 1.2.0)

## Implementation status

The C# implementation of UAX #29 implements grapheme cluster, word, line, and
sentence segmentation over the generated Unicode 18.0.0 property tables,
including the Unicode 18.0.0 GB9c conjunct-avoidance rule.

## Conformance

- The C# segmentation conformance harness runs the official Unicode 18.0.0
  `BreakTest.txt` vectors and passes.

## Out of scope

- UAX #14 default-rule conformance is asserted through the same generated break
  tables; language-specific tailoring is not claimed.
