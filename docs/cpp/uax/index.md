# C++ - UAX (Unicode Standard Annexes)

Part of the Unicodoid Unicode 18.0.0 implementation. Each module lives in
`clike/cpp/include/unicodoid/uax/` and `clike/cpp/src/uax/`. The full
cross-language matrix is in the top-level
[report coverage](../../index.md#report-coverage).

| Doc | Specification | C++ header |
| --- | --- | --- |
| - | UAX #9 Bidirectional Algorithm | `uax/uax9_bidi.hpp` |
| - | UAX #11 East Asian Width | `uax/uax11_eaw.hpp` |
| - | UAX #14 Line Breaking Algorithm | `uax/uax14_linebreak.hpp` |
| [uax15.md](uax15.md) | UAX #15 Normalization Forms | `uax/uax15_normalization.hpp` |
| - | UAX #24 Script Property | `uax/uax24_script_property.hpp` |
| [uax29.md](uax29.md) | UAX #29 Text Segmentation | `uax/uax29_segmentation.hpp` |
| - | UAX #31 Identifiers and Syntax | `uax/uax31_identifiers.hpp` |
| [uax34.md](uax34.md) | UAX #34 Named Character Sequences | `uax/uax34_named_sequences.hpp` |
| - | UAX #38 Han Database (Unihan) | `uax/uax38_unihan.hpp` |
| - | UAX #41 Common References | `uax/uax41_common_refs.hpp` |
| - | UAX #42 UCD in XML | `uax/uax42_ucd_xml.hpp` |
| [uax44.md](uax44.md) | UAX #44 Unicode Character Database | `uax/uax44_properties.hpp` |
| - | UAX #45 U-source Ideographs | `uax/uax45_usource_ideographs.hpp` |
| - | UAX #50 Vertical Text Layout | `uax/uax50_vtl.hpp` |
| - | UAX #53 Arabic Mark Rendering | `uax/uax53_arabic_mark_rendering.hpp` |
| - | UAX #57 Egyptian Hieroglyph Database | `uax/uax57_unikemet.hpp` |
| - | UAX #60 Data for East Asian Scripts | `uax/uax60_ea_script_data.hpp` |

**Status:** `test_cpp`, `test_cpp_conformance`, `test_cpp_specs`, and
`test_cpp_new_reports` pass in CTest. UAX #9 / #38 / #41 / #42 / #45 / #53 /
#57 forward to the C reference implementation so both paths agree exactly;
the remaining modules are native C++ over the generated tables.

*Generated tables:* shared Unicode 18.0.0 layer (generator 1.2.0), linked via
the C `unicodoid` library.
