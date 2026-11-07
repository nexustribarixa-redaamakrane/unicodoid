# UTS #58 — Unicode Link Detection and Formatting (C implementation)

- **Specification:** UTS #58 Unicode Link Detection and Formatting: URLs and
  Email Addresses (Unicode 18.0.0, Revision 3)
- **Module:** `clike/src/uts/uts58_link_detection.c`
- **Generated data:** `clike/src/unicodoid_ucd_data.c` (Unicode 18.0.0, generator 1.2.0)

## Implementation status

The C implementation provides the post-host machinery of UTS #58:

- Link detection termination (§3.5.1) with soft, hard, open, and close handling.
- URL minimal escaping (§4.1), including final-soft-character escaping.
- Email local part detection (§5.2).

Terminator, link-term, and email-character classifications come from the
generated Unicode 18.0.0 link data.

## Conformance

- `clike/tests/test_uts58.c` runs the official `LinkDetectionTest.txt` and
  `LinkFormattingTest.txt` conformance vectors and passes.

## Out of scope

- URL initiation (§3.2) and the `mailto:` scheme handling are outside the
  implemented scope; the specification itself leaves initiation to the caller.
