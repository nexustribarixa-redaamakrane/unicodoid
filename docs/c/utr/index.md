# C - UTR (Unicode Technical Reports)

Part of the Unicodoid Unicode 18.0.0 implementation. Each module is compiled
from `clike/src/utr/`. The full cross-language matrix is in the top-level
[report coverage](../../index.md#report-coverage).

| Doc | Specification | C module |
| --- | --- | --- |
| - | UTR #17 Character Encoding Model | `utr/utr17_character_encoding_model.h` |
| - | UTR #23 Character Property Model | `utr/utr23_character_property_model.h` |
| - | UTR #25 Support for Mathematics | `utr/utr25_mathematics.h` |
| - | UTR #33 Conformance Model | `utr/utr33_conformance.h` |
| - | UTR #54 Mongolian 12.1 Baseline | `utr/utr54_mongolian.h` |
| - | UTR #56 Cuneiform Sign Lists | `utr/utr56_cuneiform_signlists.h` |
| [utr59.md](utr59.md) | UTR #59 East Asian Spacing | `utr/utr59_east_asian_spacing.h` |

**Status:** CTest `east_asian_spacing` and `new_reports` pass against the
official Unicode 18.0.0 data.

**Out of scope:** conditional East Asian Spacing resolution is policy driven;
see the spec note.

*Generated tables:* `clike/src/unicodoid_ucd_data.c` (Unicode 18.0.0,
generator 1.2.0).
