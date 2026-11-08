# C++ - UTR (Unicode Technical Reports)

Part of the Unicodoid Unicode 18.0.0 implementation. Each module lives in
`clike/cpp/include/unicodoid/utr/` and `clike/cpp/src/utr/`. The full
cross-language matrix is in the top-level
[report coverage](../../index.md#report-coverage).

| Doc | Specification | C++ header |
| --- | --- | --- |
| - | UTR #17 Character Encoding Model | `utr/utr17_character_encoding_model.hpp` |
| - | UTR #23 Character Property Model | `utr/utr23_character_property_model.hpp` |
| - | UTR #25 Support for Mathematics | `utr/utr25_mathematics.hpp` |
| - | UTR #33 Conformance Model | `utr/utr33_conformance.hpp` |
| - | UTR #54 Mongolian 12.1 Baseline | `utr/utr54_mongolian.hpp` |
| - | UTR #56 Cuneiform Sign Lists | `utr/utr56_cuneiform_signlists.hpp` |
| [utr59.md](utr59.md) | UTR #59 East Asian Spacing | `utr/utr59_east_asian_spacing.hpp` |

**Status:** `test_cpp`, `test_cpp_new_reports`, and the East Asian spacing
conformance suite pass in CTest.

**Out of scope:** conditional East Asian Spacing resolution is policy driven;
see the spec note.

*Generated tables:* shared Unicode 18.0.0 layer (generator 1.2.0), linked via
the C `unicodoid` library.
