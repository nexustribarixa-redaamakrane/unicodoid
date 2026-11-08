# C - UTS (Unicode Technical Standards)

Part of the Unicodoid Unicode 18.0.0 implementation. Each module is compiled
from `clike/src/uts/`. The full cross-language matrix is in the top-level
[report coverage](../../index.md#report-coverage).

| Doc | Specification | C module |
| --- | --- | --- |
| [uts10.md](uts10.md) | UTS #10 Unicode Collation Algorithm | `uts/uts10_collation.h` |
| - | UTS #18 Regular Expressions | `uts/uts18_regex.h` |
| - | UTS #35 CLDR Part 1: Core | `uts/uts35_cldr/` (9 modules) |
| - | UTS #37 Ideographic Variation Database | `uts/uts37_ivd.h` |
| - | UTS #39 Security Mechanisms | `uts/uts39_security.h` |
| - | UTS #46 IDNA Compatibility | `uts/uts46_idna_compat.h` |
| [uts51.md](uts51.md) | UTS #51 Unicode Emoji | `uts/uts51_emoji_sequences.h` |
| - | UTS #55 Source Code Handling | `uts/uts55_source_code.h` |
| [uts58.md](uts58.md) | UTS #58 Link Detection and Formatting | `uts/uts58_link_detection.h` |
| - | UTS #61 Set Notation | `uts/uts61_set_notation.h` |

**Status:** CTest `collation`, `emoji_sequences`, `linkification`, and
`new_reports` pass against the official Unicode 18.0.0 vectors.

**Out of scope (by design):** URL initiation, CLDR tailorings, and UCA
search; see the per-spec notes.

*Generated tables:* `clike/src/unicodoid_ucd_data.c` (Unicode 18.0.0,
generator 1.2.0).
