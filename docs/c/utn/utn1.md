# UTN #1 — Issues in Indic Language Collation (documentation disposition)

- **Unicode Technical Note:** UTN #1, "Issues in Indic Language Collation"
- **Reference:** https://www.unicode.org/notes/tn1/
- **Status:** Technical Note (not a standard; not an UAX/UTS/UTR/TR)

## Disposition

UTN #1 is an independent author work and is not part of the Unicode standards
process. The Unicodoid SDK treats it as documentation only:

- It is listed in the specification catalog with category `UTN`, number `1`.
- No runtime behaviour is asserted from the note; no algorithm in the SDK is
  derived from it.
- The SDK does not implement collation behaviour described there as a separate
  conformance claim; UTS #10 conformance is evaluated against the UCA data only.

## Documentation

This directory holds the per-language disposition of UTN #1 for the SDK's
implemented language surfaces. The note does not introduce a generated data
table and therefore has no implementation module under `src/`.
