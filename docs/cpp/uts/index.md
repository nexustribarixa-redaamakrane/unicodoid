# C++ - UTS (Unicode Technical Standards)

Part of the Unicodoid Unicode 18.0.0 implementation. Each module lives in
`clike/cpp/include/unicodoid/uts/` and `clike/cpp/src/uts/`. The full
cross-language matrix is in the top-level
[report coverage](../../index.md#report-coverage).

| Doc | Specification | C++ header |
| --- | --- | --- |
| [uts10.md](uts10.md) | UTS #10 Unicode Collation Algorithm | `uts/uts10_collation.hpp` |
| - | UTS #18 Regular Expressions | `uts/uts18_regex.hpp` |
| - | UTS #35 CLDR Part 1: Core | `uts/cldr.hpp` |
| - | UTS #37 Ideographic Variation Database | `uts/uts37_ivd.hpp` |
| - | UTS #39 Security Mechanisms | `uts/uts39_security.hpp` |
| - | UTS #46 IDNA Compatibility | `uts/uts46_idna_compat.hpp` |
| [uts51.md](uts51.md) | UTS #51 Unicode Emoji | `uts/uts51_emoji_sequences.hpp` |
| - | UTS #55 Source Code Handling | `uts/uts55_source_code.hpp` |
| [uts58.md](uts58.md) | UTS #58 Link Detection and Formatting | `uts/uts58_link_detection.hpp` |
| - | UTS #61 Set Notation | `uts/uts61_set_notation.hpp` |

**Status:** `test_cpp`, `test_cpp_new_reports`, and the emoji/linkification
conformance suites pass in CTest. UTS #18 / #35 / #39 / #46 / #61 forward to
the C reference implementation; the CLDR helpers are aggregated in
`unicodoid::uts35`.

**Out of scope (by design):** CLDR XML resource bundles, URL initiation, and
UCA search; see the per-spec notes.

*Generated tables:* shared Unicode 18.0.0 layer (generator 1.2.0), linked via
the C `unicodoid` library.
