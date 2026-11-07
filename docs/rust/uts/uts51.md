# UTS #51 — Unicode Emoji (Rust implementation)

- **Specification:** UTS #51 Unicode Emoji (Unicode 18.0.0, Revision 4)
- **Module:** `rust/src/uts/uts51.rs`
- **Generated data:** generated Unicode 18.0.0 emoji sequence tables (generator 1.2.0)

## Implementation status

The Rust implementation validates emoji sequences and RGI emoji membership from
the official Unicode 18.0.0 emoji data files, including Basic_Emoji range
expansion and emoji modifier acceptance.

## Conformance

- The Rust emoji suite validates the official Unicode 18.0.0 emoji sequence data
  and passes.

## Out of scope

- Presentation metadata is not claimed.
