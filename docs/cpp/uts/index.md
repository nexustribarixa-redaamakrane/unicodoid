# C++ — UTS (Unicode Technical Standards)

Part of the Unicodoid Unicode 18.0.0 implementation. Native C++ modules over
generated Unicode 18.0.0 data.

| Doc | Specification | What it covers |
| --- | --- | --- |
| [uts10.md](uts10.md) | UTS #10 Unicode Collation Algorithm | Three-level sort keys, strengths, numeric ordering in `cpp/src/uts/uts10_collation.cpp` |
| [uts51.md](uts51.md) | UTS #51 Unicode Emoji | RGI emoji sequence validation |
| [uts58.md](uts58.md) | UTS #58 Link Detection and Formatting | Detection, escaping, email local part |

**Status:** `test_cpp_specs` validates the sort-key and link surfaces against
the generated tables and passes.

**Out of scope (by design):** URL initiation, CLDR tailorings, and UCA §11
search; see the per-spec notes.

*Generated tables:* shared Unicode 18.0.0 layer (generator 1.2.0).
