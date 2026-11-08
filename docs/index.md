# Unicodoid Documentation Index

Unicodoid implements the current Unicode report set inventoried in
[`docs/unicode18/inventory.json`](unicode18/inventory.json) (61 reports,
34 implemented scope; withdrawn / suspended / superseded / stabilized
reports are excluded with recorded dispositions).

## Language trees

- [`docs/c/`](c/) — C (C11): core SDK, ConvertUTF, UAX/UTS/UTR scalar modules
- [`docs/cpp/`](cpp/) — C++ (C++17) over generated data
- [`docs/csharp/`](csharp/) — C# (.NET 8)
- [`docs/rust/`](rust/) — Rust (edition 2024)

Each tree has `uax/`, `uts/`, `utr/`, `utn/` indexes. The top-level
Unicode 18 inventory and UTN disposition notes live under
[`docs/unicode18/`](unicode18/).

## Report coverage

Every in-scope report has a module in all four language trees. The table
names the C header, the C++ header, the Rust module and the C# type.

| Report | Title | C | C++ | Rust | C# |
| --- | --- | --- | --- | --- | --- |
| UAX #9 | Bidirectional Algorithm | `uax/uax9_bidi.h` | `uax/uax9_bidi.hpp` | `uax/uax9.rs` | `Uax9` |
| UTS #10 | Collation Algorithm | `uts/uts10_collation.h` | `uts/uts10_collation.hpp` | `uts/uts10.rs` | `Uts10` |
| UAX #11 | East Asian Width | `uax/uax11_eaw.h` | `uax/uax11_eaw.hpp` | `uax/uax11.rs` | `Uax11` |
| UAX #14 | Line Breaking Algorithm | `uax/uax14_linebreak.h` | `uax/uax14_linebreak.hpp` | `uax/uax14.rs` | `Uax14` |
| UAX #15 | Normalization Forms | `uax/uax15_normalization.h` | `uax/uax15_normalization.hpp` | `uax/uax15.rs` | `Normalization` |
| UTR #17 | Character Encoding Model | `utr/utr17_character_encoding_model.h` | `utr/utr17_character_encoding_model.hpp` | `utr/utr17.rs` | `Utr17` |
| UTS #18 | Regular Expressions | `uts/uts18_regex.h` | `uts/uts18_regex.hpp` | `uts/uts18.rs` | `Uts18` |
| UTR #23 | Character Property Model | `utr/utr23_character_property_model.h` | `utr/utr23_character_property_model.hpp` | `utr/utr23.rs` | `Utr23` |
| UAX #24 | Script Property | `uax/uax24_script_property.h` | `uax/uax24_script_property.hpp` | `uax/uax24.rs` | `Uax24` |
| UTR #25 | Support for Mathematics | `utr/utr25_mathematics.h` | `utr/utr25_mathematics.hpp` | `utr/utr25.rs` | `Utr25` |
| UAX #29 | Text Segmentation | `uax/uax29_segmentation.h` | `uax/uax29_segmentation.hpp` | `uax/uax29.rs` | `Segmenter` |
| UAX #31 | Identifiers and Syntax | `uax/uax31_identifiers.h` | `uax/uax31_identifiers.hpp` | `uax/uax31.rs` | `Uax31` |
| UTR #33 | Conformance Model | `utr/utr33_conformance.h` | `utr/utr33_conformance.hpp` | `utr/utr33.rs` | `Utr33` |
| UAX #34 | Named Character Sequences | `uax/uax34_named_sequences.h` | `uax/uax34_named_sequences.hpp` | `uax/uax34.rs` | `Uax34` |
| UTS #35 | CLDR Part 1: Core | `uts/uts35_cldr/` (9 modules) | `uts/cldr.hpp` | `uts/cldr.rs` | `Cldr` |
| UTS #37 | Ideographic Variation Database | `uts/uts37_ivd.h` | `uts/uts37_ivd.hpp` | `uts/uts37.rs` | `Uts37` |
| UAX #38 | Han Database (Unihan) | `uax/uax38_unihan.h` | `uax/uax38_unihan.hpp` | `uax/uax38.rs` | `Uax38` |
| UTS #39 | Security Mechanisms | `uts/uts39_security.h` | `uts/uts39_security.hpp` | `uts/uts39.rs` | `Uts39` |
| UAX #41 | Common References | `uax/uax41_common_refs.h` (docs only) | `uax/uax41_common_refs.hpp` | `uax/uax41.rs` | `Uax41` |
| UAX #42 | UCD in XML | `uax/uax42_ucd_xml.h` | `uax/uax42_ucd_xml.hpp` | `uax/uax42.rs` | `Uax42` |
| UAX #44 | Character Database | `uax/uax44_ucd.h` | `uax/uax44_properties.hpp` | `uax/uax44.rs` | `Uax44`, `UnicodeProperties` |
| UAX #45 | U-source Ideographs | `uax/uax45_usource_ideographs.h` | `uax/uax45_usource_ideographs.hpp` | `uax/uax45.rs` | `Uax45` |
| UTS #46 | IDNA Compatibility | `uts/uts46_idna_compat.h` | `uts/uts46_idna_compat.hpp` | `uts/uts46.rs` | `Uts46` |
| UAX #50 | Vertical Text Layout | `uax/uax50_vtl.h` | `uax/uax50_vtl.hpp` | `uax/uax50.rs` | `Uax50` |
| UTS #51 | Emoji | `uts/uts51_emoji_sequences.h` | `uts/uts51_emoji_sequences.hpp` | `uts/uts51.rs` | `Uts51` |
| UAX #53 | Arabic Mark Rendering | `uax/uax53_arabic_mark_rendering.h` | `uax/uax53_arabic_mark_rendering.hpp` | `uax/uax53.rs` | `Uax53` |
| UTR #54 | Mongolian 12.1 Baseline | `utr/utr54_mongolian.h` | `utr/utr54_mongolian.hpp` | `utr/utr54.rs` | `Utr54` |
| UTS #55 | Source Code Handling | `uts/uts55_source_code.h` | `uts/uts55_source_code.hpp` | `uts/uts55.rs` | `Uts55` |
| UTR #56 | Cuneiform Sign Lists | `utr/utr56_cuneiform_signlists.h` | `utr/utr56_cuneiform_signlists.hpp` | `utr/utr56.rs` | `Utr56` |
| UAX #57 | Egyptian Hieroglyph Database | `uax/uax57_unikemet.h` | `uax/uax57_unikemet.hpp` | `uax/uax57.rs` | `Uax57` |
| UTS #58 | Link Detection and Formatting | `uts/uts58_link_detection.h` | `uts/uts58_link_detection.hpp` | `uts/uts58.rs` | `Uts58` |
| UTR #59 | East Asian Spacing | `utr/utr59_east_asian_spacing.h` | `utr/utr59_east_asian_spacing.hpp` | `utr/utr59.rs` | `Utr59` |
| UAX #60 | Data for East Asian Scripts | `uax/uax60_ea_script_data.h` | `uax/uax60_ea_script_data.hpp` | `uax/uax60.rs` | `Uax60` |
| UTS #61 | Set Notation | `uts/uts61_set_notation.h` | `uts/uts61_set_notation.hpp` | `uts/uts61.rs` | `Uts61` |

Excluded (withdrawn / suspended / superseded / stabilized) reports have
no module; see `inventory.json` for dispositions.
