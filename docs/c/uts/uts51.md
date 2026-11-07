# UTS #51 — Unicode Emoji (C implementation)

- **Specification:** UTS #51 Unicode Emoji (Unicode 18.0.0, Revision 4)
- **Module:** `clike/src/uts/uts51_emoji_sequences.c`
- **Generated data:** `clike/src/unicodoid_ucd_data.c` (Unicode 18.0.0, generator 1.2.0)

## Implementation status

The C implementation validates emoji sequences from the official Unicode 18.0.0
emoji sequence data (`emoji/emoji-sequences.txt` and
`emoji/emoji-zwj-sequences.txt`) and the RGI emoji set:

- `Basic_Emoji` entries are expanded to the individual code points they cover;
  emoji modifiers (including emoji modifiers accepted as Basic_Emoji) are
  recognized.
- Well-formedness validation (ZWJ sequences, flag sequences, keycaps, skin-tone
  modifiers) is independent of the RGI table membership.

## Conformance

- `clike/tests/test_uts51.c` validates every RGI sequence of the official
  Unicode 18.0.0 emoji data files and passes.

## Out of scope

- The emoji presentation and text presentation default tables beyond the RGI
  set, and UTS #51 §7 search/sorting guidance, are not claimed.
