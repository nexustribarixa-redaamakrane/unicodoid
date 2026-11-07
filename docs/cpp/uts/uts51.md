# UTS #51 — Unicode Emoji (C++ implementation)

- **Specification:** UTS #51 Unicode Emoji (Unicode 18.0.0, Revision 4)
- **Module:** `clike/cpp/src/uts/uts51_emoji_sequences.cpp`
- **Generated data:** generated Unicode 18.0.0 emoji sequence tables (generator 1.2.0)

## Implementation status

The C++ implementation validates emoji sequences and RGI emoji membership from
the official Unicode 18.0.0 emoji data files, including Basic_Emoji range
expansion to individual code points and emoji modifier acceptance.

## Conformance

- `test_cpp_specs` validates the RGI set over the official Unicode 18.0.0 emoji
  sequence data and passes.

## Out of scope

- Presentation metadata and UTS #51 guidance outside sequence validation are not
  claimed.
