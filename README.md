# Unicodoid

Unicodoid is a general-purpose SDK for designing, implementing, and managing
**independent Unicode-like encoding systems**. A character is a 64-bit integer
in the value space declared by its encoding system. That value may be a Unicode
scalar, a token identifier, or a composite reference.

Unicode compatibility is optional. A system may expose a Unicode profile, a
partial mapping, or no relationship with Unicode at all. The codec, string, and
transcoding layers never assume one.

The repository contains coherent implementations in four languages:

| Language | Location | What it contains |
| --- | --- | --- |
| C (C11) | `clike/include`, `clike/src` | Library, spec/conformance suites, `ConvertUTF.h` / `ConvertUTF.c` |
| C++ (C++17) | `clike/cpp` | Native C++ modules over generated data, `ConvertUTF.hpp` / `ConvertUTF.cpp` (native, not a wrapper) |
| C# (.NET 8) | `clike/csharp/Unicodoid` | Managed implementation, `ConvertUTF.cs`, `harness.cs` |
| Rust (edition 2024) | `rust` | Standalone crate, `convertutf.rs`, `tests/harness.rs` |

The ConvertUTF ports are the discontinued W3C/Unicode `ConvertUTF` library
re-issued at **Unicode 18.0.0** (maximum code point U+10FFFF, replacement
character U+FFFD). The C API is unchanged (`ConvertUTF8toUTF16` …), so
existing callers keep compiling; the harnesses exercise the same vectors in
all four languages.

## Design principles

1. **No assumed Unicode relationship.** Unicode is a profile, not a
   precondition.
2. **State-aware systems.** Every operation receives a per-stream state buffer.
   Stateless systems ignore it; modal systems use it.
3. **Explicit transcoding.** Two systems are bridged only when they are the
   same system, both declare usable Unicode profiles, or the caller supplies an
   explicit value bridge. There is no implicit identity mapping.
4. **Length-bounded strings.** Strings carry explicit byte lengths and may
   contain embedded zero bytes. They are never null-terminated.
5. **Shared contract, idiomatic implementations.** Status values, character
   semantics, and built-in system behaviour align across languages, while each
   API follows its language's conventions.
6. **Generated data, not hand-curated tables.** All Unicode property,
   normalization, collation, emoji, named-sequence, and link data is derived
   from the official Unicode 18.0.0 distribution by one reproducible generator.
7. **No hidden platform integration.** The SDK does not depend on a windowing
   system, website generator, network service, or external Unicode database.

## Layout

```text
unicodoid/
├── clike/
│   ├── include/unicodoid/      C public headers
│   │   ├── ConvertUTF.h        C ConvertUTF API (Unicode 18.0.0)
│   │   └── unicodoid*.h        SDK headers
│   ├── src/                    C implementation
│   │   ├── ConvertUTF.c          C ConvertUTF implementation
│   │   ├── uax/, uts/, utr/, systems/, ...
│   │   └── unicodoid_ucd_data.c  Generated Unicode 18.0.0 tables
│   ├── cpp/
│   │   ├── include/unicodoid/
│   │   │   ├── ConvertUTF.hpp    Native C++ ConvertUTF API
│   │   │   └── unicodoid.hpp     C++ wrapper
│   │   ├── src/
│   │   │   ├── ConvertUTF.cpp      Native C++ ConvertUTF implementation
│   │   │   └── uax/, uts/, utr/    Native spec modules
│   │   └── tests/
│   ├── csharp/
│   │   ├── Unicodoid/
│   │   │   ├── ConvertUTF.cs       C# ConvertUTF port
│   │   │   ├── Uax/, Uts/, Utr/    Spec modules
│   │   │   └── Systems/            Built-in systems
│   │   └── Unicodoid.Tests/
│   │       ├── Program.cs          Shared test driver
│   │       └── harness.cs          C# ConvertUTF harness
│   ├── tests/                      C suites + harness.c
│   └── CMakeLists.txt              Static libraries, tests, install rules
├── rust/
│   ├── src/
│   │   ├── convertutf.rs         Rust ConvertUTF port
│   │   ├── uax/, uts/, utr/, generated/, ...
│   │   └── lib.rs
│   ├── tests/
│   │   ├── harness.rs              Rust ConvertUTF harness
│   │   └── core, unicode, ...      Integration suites
│   └── Cargo.toml
├── docs/
│   ├── c/ cpp/ csharp/ rust/       # per-language uax/, uts/, utr/, utn/
│   │   └── <group>/index.md        # folder indexes (this rewrite)
│   └── unicode18/                  # report inventory + UTN #1 note
├── tools/gen_ucd.py                UCD generator (version 1.2.0)
└── data/
    ├── ucd/18.0.0/                 Official Unicode 18.0.0 data
    └── generated/ucd18_manifest.json
```

## Core model

### Character values and status codes

All languages use a 64-bit character value (`unicodoid_char64_t`,
`CharacterValue`, or `CharValue = u64`). Status values are part of the
cross-language contract:

```text
   0  Ok
 -1  InvalidParam          -8  Unsupported
 -2  BufferTooSmall        -9  CapacityExceeded
 -3  InvalidCodeUnit      -10  AllocationFailed
 -4  UnmappableChar       -11  Internal
 -5  IncompleteSequence   -12  State
 -6  NotFound             -13  Overflow
 -7  AlreadyExists        -14  NoCharacter
```

`NoCharacter` is not a failure. Modal systems return it when input was consumed
but produced no character value, such as a shift control. Streaming loops must
advance by the reported byte count and continue.

### Encoding systems

A system declares metadata (identifier, name, version, author, description,
code-unit width, byte widths, model, endianness, compatibility level, maximum
value, replacement value) and implements:

- encode one character, updating state;
- decode one character, updating state;
- compute the exact byte count for a character sequence;
- optionally validate or count a complete byte stream;
- optionally create and reset per-stream state;
- optionally expose a Unicode profile.

The built-in systems are:

| System | ID | Unicode profile | Model |
| --- | --- | --- | --- |
| UTF-8 | `org.unicode.utf8` | Full | Variable width |
| UTF-16LE | `org.unicode.utf16le` | Full | Variable width |
| UTF-16BE | `org.unicode.utf16be` | Full | Variable width |
| UTF-32LE | `org.unicode.utf32le` | Full | Fixed width |
| UTF-32BE | `org.unicode.utf32be` | Full | Fixed width |
| ASCII-7 | `org.unicodoid.ascii` | Partial | Fixed width |
| KupoConlang-14 | `org.unicodoid.custom.kupoconlang` | None | Variable width |
| ShiftModal-16 | `org.unicodoid.shiftmodal16` | None | Modal, stateful |

KupoConlang-14 is a 14-bit constructed-script example. ShiftModal-16 is a
deliberately non-Unicode modal example: `SO` enters shifted mode, `SI` returns
to base mode, and `ESC` introduces an escaped value. Both exist to prove that
the architecture does not require Unicode.

### Strings, codecs, and transcoding

A codec owns one state instance for a stream. A string owns both its bytes and
its persistent codec state, so appending to a modal stream continues the same
mode. Strings can be built from characters, from validated bytes, cloned, reset,
and indexed by character.

Transcoding supports four policies:

- `Strict`: fail with the original mapping or decoding status;
- `Replace`: emit the destination replacement character;
- `Skip`: omit the character;
- `Callback`: delegate substitution to the caller.

## ConvertUTF

`ConvertUTF` converts between UTF-8, UTF-16, and UTF-32 code unit sequences.
Each routine reports the first encountered problem as a conversion result
(`ConversionOk`, `SourceExhausted`, `TargetExhausted`, `SourceIllegal`) and
updates the source and target cursors to the end of the last successfully
converted item. The `strictConversion` flag rejects irregular sequences and
isolated surrogates; `lenientConversion` replaces them with U+FFFD. Illegal
sequences — overlong forms such as `C0 80`, encoded surrogates such as
`ED A0 80`, and values above U+10FFFF — are errors in both modes.

| Port | Files | API |
| --- | --- | --- |
| C | `clike/include/unicodoid/ConvertUTF.h`, `clike/src/ConvertUTF.c` | `ConvertUTF8toUTF16`, `ConvertUTF16toUTF8`, `ConvertUTF8toUTF32`, `ConvertUTF32toUTF8`, `ConvertUTF16toUTF32`, `ConvertUTF32toUTF16`, `isLegalUTF8Sequence` |
| C++ | `clike/cpp/include/unicodoid/ConvertUTF.hpp`, `clike/cpp/src/ConvertUTF.cpp` | `unicodoid::convertutf::utf8ToUtf16`, `utf16ToUtf8`, `utf8ToUtf32`, `utf32ToUtf8`, `utf16ToUtf32`, `utf32ToUtf16`, `isLegalUtf8` over `std::vector` (native, not a wrapper) |
| C# | `clike/csharp/Unicodoid/ConvertUTF.cs` | `ConvertUTF.ConvertUtf8ToUtf16` (…), `ConvertUTF.IsLegalUtf8Sequence` with span/cursor out parameters |
| Rust | `rust/src/convertutf.rs` | `utf8_to_utf16`, `utf16_to_utf8`, `utf8_to_utf32`, `utf32_to_utf8`, `utf16_to_utf32`, `utf32_to_utf16`, `is_legal_utf8` returning `Result<Vec<_>, ConversionError>` |

Harnesses (same vectors in every language):

- `clike/tests/harness.c` → CTest `convertutf` (39 checks)
- `clike/cpp/tests/harness.cpp` → CTest `convertutf_cpp` (44 checks)
- `clike/csharp/Unicodoid.Tests/harness.cs` → `TestConvertUtf()` (part of 303 C# checks)
- `rust/tests/harness.rs` → `cargo test --test harness` (5 tests)

```c
/* C */
const UTF8* s = (const UTF8*)"\xF0\x9F\x98\x80";
const UTF8* send = s + 4;
UTF16* t = dst;
UTF16* tend = dst + 2;
ConversionResult r = ConvertUTF8toUTF16(&s, send, &t, tend, strictConversion);
```

```rust
use unicodoid::convertutf::{utf8_to_utf16, ConversionFlags};

let utf16 = utf8_to_utf16(&[0xF0, 0x9F, 0x98, 0x80], ConversionFlags::StrictConversion)?;
```

## Optional Unicode support

A Unicode profile maps native values to Unicode scalar values and back, and
declares supported UAX/UTS/UTR features plus a Unicode version. Built-in Unicode
profiles declare version **18.0.0**.

The specification catalog contains checked entries for the Unicode 18.0.0
technical report set (61 reports inventoried in `docs/unicode18/inventory.json`):

- **UAX** #9, #11, #14, #15, #24, #29, #31, #34, #38, #41, #42, #44, #45, #50, #53,
  #57, #60
- **UTS** #10, #18, #35, #37, #39, #46, #51, #55, #58, #61
- **UTR** #17, #23, #25, #33, #54, #56, #59
- **UTN** #1

The implemented algorithms are opt-in and require a usable Unicode profile:

- character properties and case conversion (UAX #44);
- canonical/compatibility decomposition, composition, Hangul handling, and
  canonical ordering (UAX #15);
- grapheme, word, line, and sentence segmentation (UAX #29);
- named character sequences (UAX #34);
- multi-level collation, numeric ordering, and sort keys (UTS #10);
- emoji RGI sequence validation (UTS #51);
- link detection, minimal escaping, and email local-part detection (UTS #58);
- East Asian Spacing (UTR #59);
- UTF-8/UTF-16/UTF-32 conversion (ConvertUTF, this section).

UTN #1 is documentation only: it is catalogued but no runtime behaviour is
derived from it (see `docs/unicode18/utn/utn1.md`).

## Generated Unicode data

The Unicode 18.0.0 baseline is backed by generated data:

- Official UCD 18.0.0 data is downloaded into `data/ucd/18.0.0/`.
- `tools/gen_ucd.py` (generator version `1.2.0`) derives the shared property,
  normalization, collation, emoji, named-sequence, link, spacing, and slicing
  tables.
- `data/generated/ucd18_manifest.json` records the Unicode version, generator
  version, source checksum, and table counts for the generated data.

The generated-data version is asserted by the per-language suites (for example
`test_cpp_specs` checks generated version `18.0.0` and generator version
`1.2.0`, `tests/unicode.rs` checks `crate::unicode::UnicodeVersion::new(18, 0, 0)`
and the C# suite checks `Ucd.Version == "18.0.0"`).

## Documentation

Implementation notes live under the per-language docs trees, each with an
`index.md` entry point:

```text
docs/
├── c/ cpp/ csharp/ rust/
│   ├── uax/index.md    uax15 / uax29 / uax34 / uax44 notes
│   ├── uts/index.md    uts10 / uts51 / uts58 notes
│   ├── utr/index.md    utr59 note
│   └── utn/index.md    utn1 disposition
└── unicode18/
    ├── inventory.json  All 61 reports with dispositions (34 implemented scope)
    └── utn/utn1.md
```

## Quick start

### C

```c
#include "unicodoid/unicodoid.h"

unicodoid_codec_t codec;
unicodoid_codec_init(&codec, unicodoid_system_get_utf8());

uint8_t buffer[4];
size_t written = 0;
unicodoid_codec_encode_char(&codec, 0x1F600, buffer, sizeof buffer, &written);
unicodoid_codec_free(&codec);
```

### Rust

```rust
use unicodoid::{Codec, utf8_system};

let mut codec = Codec::new(utf8_system());
let mut buffer = [0u8; 4];
let written = codec.encode_char(0x1F600, &mut buffer).unwrap();
```

## Build and test

### C and C++ (CMake)

```sh
cmake -S clike -B build/cmake_c -DCMAKE_BUILD_TYPE=Release
cmake --build build/cmake_c -j2
ctest --test-dir build/cmake_c -C Debug
```

Suites: `core`, `unicode`, `stateful`, `conformance`, `linkification`,
`collation`, `named_sequences`, `emoji_sequences`, `east_asian_spacing`,
`normalization`, `convertutf`, `cpp`, `cpp_conformance`, `cpp_specs`,
`convertutf_cpp` (15 total).

### C#

```sh
dotnet build clike/csharp/Unicodoid.Tests/Unicodoid.Tests.csproj
clike/csharp/Unicodoid.Tests/bin/Debug/net8.0/Unicodoid.Tests.exe
```

The test program exits non-zero on any failure. Current result: **303 checks
passed** (including `TestConvertUtf()` from `harness.cs`).

### Rust

```sh
cargo test --manifest-path rust/Cargo.toml
```

The crate has no external dependencies. Integration targets include `core`,
`unicode`, `stateful`, `conformance`, `collation`, `uax34`, `uts51`, `uts58`,
`harness`, `utr59`.

> **Edition note:** the requested `edition = "2026"` is not accepted by the
> available toolchain (`rustc`/`cargo` 1.99.1, which accepts 2015, 2018, 2021,
> 2024, and `future`). The crate therefore uses `edition = "2024"`, the newest
> stable edition supported by that compiler.

## Verification status

- **C/C++:** all 15 CTest suites pass, including `convertutf` (39 checks) and
  `convertutf_cpp` (44 checks). The C++ spec modules (`uax15`, `uax29`, `uax34`,
  `uax44`, `uts10`, `uts51`, `uts58`, `utr59`) are native C++ over generated
  data, not wrappers of the C implementations.
- **C#:** 303 checks pass, including normalization (20,171 cases / 403,420
  checks), collation, named sequences (461), emoji (3,972), link detection /
  formatting, East Asian spacing (1,299 ranges), and `TestConvertUtf()`.
- **Rust:** all integration suites pass, including `harness` (5 ConvertUTF
  tests: vectors, legality/version, round-trips).
- **Version correctness** is asserted everywhere the generated tables are
  exposed (Unicode `18.0.0`, generator `1.2.0`).

## Known limitations

These are deliberate scope boundaries, not hidden stubs:

- **Conformance claims are data-bound.** Algorithms are exercised against the
  official Unicode 18.0.0 conformance vectors; they are not claimed merely
  because the generated tables compile.
- **Collation scope.** The UCA implementation covers the generated DUCET-based
  weights and the conformance suite; CLDR tailorings and UCA §11 search features
  are not implemented.
- **The Unicode catalog is a subset.** Only entries checked against the official
  reports index are listed; stabilized/superseded/withdrawn/suspended reports
  are excluded with recorded dispositions in `inventory.json`.
- **No optional Unicode algorithms beyond those listed.** Bidi, regular
  expressions, IDNA, and security algorithms are not implemented.
- **C memory management uses `malloc`/`realloc`.** Freestanding or
  allocation-free operation is not claimed.
- **Profile version is metadata.** Algorithms do not validate their tables
  against the declared Unicode version at runtime.
- **Rust uses edition 2024.** See the edition note above.

## Independence and attribution

Unicodoid is standalone. It contains no references to, or dependencies on, any
other internal prototype, encoding architecture, windowing system, or website
generator. The C and C++ libraries have no third-party dependencies; the C#
projects use only the .NET base class library; the Rust crate has no
dependencies.

The ConvertUTF ports retain the original Unicode, Inc. copyright and
redistribution notice in each source header. Unicode, the Unicode logo, and
the Unicode Standard are trademarks of Unicode, Inc. This project is not
affiliated with or endorsed by the Unicode Consortium. Specification titles,
numbers, and URLs are cited for identification and interoperability.
