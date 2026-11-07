# C — UTR (Unicode Technical Reports)

Part of the Unicodoid Unicode 18.0.0 implementation.

| Doc | Specification | What it covers |
| --- | --- | --- |
| [utr59.md](utr59.md) | UTR #59 East Asian Spacing | `W`/`N`/`O`/`C` spacing queries and cluster offsets over the generated 1,299-range table |

**Status:** CTest `east_asian_spacing` (`test_utr59`) validates the official
`east-asian-spacing.txt` data and passes.

**Out of scope:** conditional East Asian Spacing resolution is policy driven;
see the spec note.

*Generated tables:* `clike/src/unicodoid_ucd_data.c` (Unicode 18.0.0,
generator 1.2.0).
