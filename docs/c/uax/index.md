# C - UAX (Unicode Standard Annexes)

Part of the Unicodoid [Unicode 18.0.0](../../../docs/unicode18/inventory.json)
implementation. Each module is compiled from `clike/src/uax/`. The full
cross-language matrix is in the top-level [report coverage](../../index.md#report-coverage).

| Doc | Specification | C module |
| --- | --- | --- |
| - | UAX #9 Bidirectional Algorithm | `uax/uax9_bidi.h` |
| - | UAX #11 East Asian Width | `uax/uax11_eaw.h` |
| - | UAX #14 Line Breaking Algorithm | `uax/uax14_linebreak.h` |
| - | UAX #15 Normalization Forms | `uax/uax15_normalization.h` |
| - | UAX #24 Script Property | `uax/uax24_script_property.h` |
| [uax29.md](uax29.md) | UAX #29 Text Segmentation | `uax/uax29_segmentation.h` |
| - | UAX #31 Identifiers and Syntax | `uax/uax31_identifiers.h` |
| [uax34.md](uax34.md) | UAX #34 Named Character Sequences | `uax/uax34_named_sequences.h` |
| - | UAX #38 Han Database (Unihan) | `uax/uax38_unihan.h` |
| - | UAX #41 Common References | `uax/uax41_common_refs.h` (docs only) |
| - | UAX #42 UCD in XML | `uax/uax42_ucd_xml.h` |
| - | UAX #44 Unicode Character Database | `uax/uax44_ucd.h` |
| - | UAX #45 U-source Ideographs | `uax/uax45_usource_ideographs.h` |
| - | UAX #50 Vertical Text Layout | `uax/uax50_vtl.h` |
| - | UAX #53 Arabic Mark Rendering | `uax/uax53_arabic_mark_rendering.h` |
| - | UAX #57 Egyptian Hieroglyph Database | `uax/uax57_unikemet.h` |
| - | UAX #60 Data for East Asian Scripts | `uax/uax60_ea_script_data.h` |

**Status:** CTest `conformance`, `new_reports`, `normalization`, and the
`uax34` suite pass against the official Unicode 18.0.0 vectors.

*Generated tables:* `clike/src/unicodoid_ucd_data.c` (Unicode 18.0.0,
generator 1.2.0).
