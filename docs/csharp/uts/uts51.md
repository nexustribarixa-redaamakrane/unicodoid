# UTS #51 — Unicode Emoji (C# implementation)

- **Specification:** UTS #51 Unicode Emoji (Unicode 18.0.0, Revision 4)
- **Module:** `clike/csharp/Unicodoid/Uts/Uts51.cs`
- **Generated data:** generated Unicode 18.0.0 emoji sequence tables (generator 1.2.0)

## Implementation status

The C# implementation validates emoji sequences and RGI emoji membership from
the official Unicode 18.0.0 emoji data files, including Basic_Emoji range
expansion and emoji modifier acceptance.

## Conformance

- The C# emoji conformance harness validates the official Unicode 18.0.0 emoji
  sequence data and passes.

## Out of scope

- Presentation metadata is not claimed.
