# UAX #29 — Unicode Text Segmentation (C implementation)

- **Specification:** UAX #29 Unicode Text Segmentation (Unicode 18.0.0, Revision 47)
- **Module:** `clike/src/uax/uax29_segmentation.c`
- **Generated data:** `clike/src/unicodoid_ucd_data.c` (Unicode 18.0.0, generator 1.2.0)

## Implementation status

The C implementation of UAX #29 covers grapheme cluster, word, line, and
sentence segmentation on top of the generated Unicode 18.0.0 property tables
(`Grapheme_Cluster_Break`, `Word_Break`, `Line_Break`, and
`Sentence_Break`).

- Grapheme clusters implement GB1-GB10 plus the Unicode 18.0.0 GB9c
  conjunct-avoidance rule from the generated Indic_Conjunct_Break data.
- Word, line, and sentence rules implement the UAX #14 rule machinery over the
  generated break property tables, with an implicit start-of-text boundary.
- Break tests are evaluated against the official Unicode 18.0.0 conformance
  vectors shipped in `data/ucd/18.0.0/ucd/auxiliary/BreakTest.txt`.

## Conformance

- `clike/tests/test_ucd18_conformance.c` runs the official Unicode 18.0.0
  segmentation conformance suite and reports per-rule and per-vector results.
- The suite passes with the Unicode 18.0.0 data baseline.

## Out of scope

- UAX #14 language-specific tailoring beyond the default rule machinery is not
  claimed; UAX #14 conformance is limited to the default configuration.
