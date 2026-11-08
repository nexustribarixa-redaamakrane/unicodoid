#!/usr/bin/env python3
"""Generate Unicode property tables for every Unicodoid language.

The generator reads only the official files downloaded by fetch_ucd.py.  Every
emitted file carries the Unicode version, generator version, and a checksum of
the source data so that generated tables can be reproduced and audited.

Usage:
    python tools/gen_ucd.py [--unicode-version 18.0.0]

The Unicode version defaults to 18.0.0 (the current baseline).  When
compatibility backports (e.g. U17/U16 data) are added, pass e.g.
``--unicode-version 17.0.0``; the generator then reads from
``data/ucd/<version>/`` and writes ``data/generated/ucd<major>_manifest.json``.
Without the flag the behaviour is unchanged.

Outputs (version-dependent manifest name):
  clike/include/unicodoid/unicodoid_ucd_data.h
  clike/src/unicodoid_ucd_data.c
  clike/csharp/Unicodoid/Generated/UcdData.cs
  rust/src/generated/ucd_data.rs
  data/generated/ucd18_manifest.json  (for Unicode 18.0.0)
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DATA = ROOT / "data" / "ucd" / "18.0.0"
OUT_MANIFEST = ROOT / "data" / "generated" / "ucd18_manifest.json"
GENERATOR_VERSION = "1.2.0"
UNICODE_VERSION = "18.0.0"
DEFAULT_UNICODE_VERSION = "18.0.0"

MAX_CP = 0x110000

# General category values, in the order used by the generated tables.
GC_VALUES = [
    "Cn", "Cc", "Cf", "Co", "Cs", "Ll", "Lm", "Lo", "Lt", "Lu", "Mc", "Me",
    "Mn", "Nd", "Nl", "No", "Pc", "Pd", "Pe", "Pf", "Pi", "Po", "Ps", "Sc",
    "Sk", "Sm", "So", "Zl", "Zp", "Zs",
]
GC_INDEX = {v: i for i, v in enumerate(GC_VALUES)}

BIDI_VALUES = [
    "L", "R", "EN", "ES", "ET", "AN", "CS", "B", "S", "WS", "ON", "LRE",
    "LRO", "AL", "RLE", "RLO", "PDF", "NSM", "BN", "FSI", "LRI", "RLI",
    "PDI", "BN", "L", "R", "AL", "L", "R", "AL", "ET", "ON", "L", "R",
    "AL", "L", "R", "AL", "ON", "L", "R", "AL", "L", "R", "AL", "L", "R",
]
# The list above is only used to keep indices stable; the actual class names
# are read from DerivedBidiClass.txt.
BIDI_ORDER = [
    "L", "R", "EN", "ES", "ET", "AN", "CS", "B", "S", "WS", "ON", "LRE",
    "LRO", "AL", "RLE", "RLO", "PDF", "NSM", "BN", "FSI", "LRI", "RLI",
    "PDI",
]
BIDI_INDEX = {v: i for i, v in enumerate(BIDI_ORDER)}

EAW_ORDER = ["N", "A", "H", "F", "Na", "W"]
EAW_INDEX = {v: i for i, v in enumerate(EAW_ORDER)}

VO_ORDER = ["U", "R", "Tu", "Tr"]
VO_INDEX = {v: i for i, v in enumerate(VO_ORDER)}

JOINING_ORDER = ["U", "C", "D", "L", "R", "T"]
JOINING_INDEX = {v: i for i, v in enumerate(JOINING_ORDER)}

LINE_BREAK_ORDER = [
    "XX", "AI", "AK", "AL", "AP", "AS", "B2", "BA", "BB", "BK", "CB",
    "CJ", "CL", "CM", "CP", "CR", "EB", "EM", "EX", "GL", "H2", "H3",
    "HH", "HL", "HY", "ID", "IN", "IS", "JL", "JT", "JV", "LF", "NL",
    "NS", "NU", "OP", "PO", "PR", "QU", "RI", "SA", "SG", "SP", "SY",
    "VF", "VI", "WJ", "ZW", "ZWJ",
]
LINE_BREAK_INDEX = {v: i for i, v in enumerate(LINE_BREAK_ORDER)}

GRAPHEME_ORDER = [
    "Other", "CR", "LF", "Control", "Extend", "ZWJ", "Regional_Indicator",
    "Prepend", "SpacingMark", "L", "V", "T", "LV", "LVT",
]
GRAPHEME_INDEX = {v: i for i, v in enumerate(GRAPHEME_ORDER)}

WORD_ORDER = [
    "Other", "CR", "LF", "Newline", "Extend", "ZWJ", "Regional_Indicator",
    "Format", "Katakana", "Hebrew_Letter", "ALetter", "Single_Quote",
    "Double_Quote", "MidNumLet", "MidLetter", "MidNum", "Numeric",
    "ExtendNumLet", "WSegSpace",
]
WORD_INDEX = {v: i for i, v in enumerate(WORD_ORDER)}

SENTENCE_ORDER = [
    "Other", "CR", "LF", "Extend", "Sep", "Format", "Sp", "Lower", "Upper",
    "OLetter", "Numeric", "ATerm", "SContinue", "STerm", "Close",
]
SENTENCE_INDEX = {v: i for i, v in enumerate(SENTENCE_ORDER)}

INCB_ORDER = ["None", "Linker", "Consonant", "Extend"]
INCB_INDEX = {v: i for i, v in enumerate(INCB_ORDER)}


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="replace")


def data_lines(path: Path):
    for raw in read(path).splitlines():
        line = raw.split("#", 1)[0].strip()
        if not line:
            continue
        yield line


def parse_range(text: str) -> tuple[int, int]:
    text = text.strip()
    if ".." in text:
        a, b = text.split("..", 1)
        return int(a, 16), int(b, 16)
    v = int(text, 16)
    return v, v


def parse_value_ranges(path: Path, mapping: dict[str, int] | None = None):
    out = []
    for line in data_lines(path):
        fields = [f.strip() for f in line.split(";")]
        if len(fields) < 2:
            continue
        a, b = parse_range(fields[0])
        value = fields[1]
        if mapping is not None:
            value = mapping.get(value, 0)
        out.append((a, b, value))
    return out


def ranges_from_bytearray(arr: bytearray):
    out = []
    start = 0
    cur = arr[0]
    for cp in range(1, MAX_CP):
        v = arr[cp]
        if v != cur:
            out.append((start, cp - 1, cur))
            start = cp
            cur = v
    out.append((start, MAX_CP - 1, cur))
    return out


def ranges_from_u16array(arr):
    out = []
    start = 0
    cur = arr[0]
    for cp in range(1, MAX_CP):
        v = arr[cp]
        if v != cur:
            out.append((start, cp - 1, cur))
            start = cp
            cur = v
    out.append((start, MAX_CP - 1, cur))
    return out


def parse_unicode_data():
    gc = bytearray(GC_INDEX["Cn"] for _ in range(MAX_CP))
    ccc = bytearray(MAX_CP)
    bidi = bytearray(BIDI_INDEX["L"] for _ in range(MAX_CP))
    decomp: dict[int, tuple[bool, list[int]]] = {}
    simple_upper: dict[int, int] = {}
    simple_lower: dict[int, int] = {}
    simple_title: dict[int, int] = {}
    numeric: dict[int, tuple[int, int, int]] = {}
    names: dict[int, str] = {}

    pending_first = None
    for raw in read(DATA / "ucd" / "UnicodeData.txt").splitlines():
        if not raw.strip():
            continue
        f = raw.split(";")
        cp = int(f[0], 16)
        name = f[1]
        if name.endswith(", First>"):
            pending_first = (cp, f)
            continue
        if name.endswith(", Last>"):
            assert pending_first is not None
            start, ff = pending_first
            for x in range(start, cp + 1):
                gc[x] = GC_INDEX.get(ff[2], GC_INDEX["Cn"])
                ccc[x] = int(ff[3])
                bidi[x] = BIDI_INDEX.get(ff[4], BIDI_INDEX["L"])
            pending_first = None
            continue

        gc[cp] = GC_INDEX.get(f[2], GC_INDEX["Cn"])
        ccc[cp] = int(f[3])
        bidi[cp] = BIDI_INDEX.get(f[4], BIDI_INDEX["L"])
        if f[5]:
            parts = f[5].split()
            compat = parts[0].startswith("<")
            if compat:
                parts = parts[1:]
            decomp[cp] = (compat, [int(p, 16) for p in parts])
        if f[6] or f[7] or f[8]:
            num = f[8] if f[8] else (f[7] if f[7] else "")
            if num:
                if "/" in num:
                    n, d = num.split("/")
                    numeric[cp] = (int(n), int(d), 0)
                else:
                    numeric[cp] = (int(num), 1, int(num) if num.isdigit() else 0)
        if f[12]:
            simple_upper[cp] = int(f[12], 16)
        if f[13]:
            simple_lower[cp] = int(f[13], 16)
        if f[14]:
            simple_title[cp] = int(f[14], 16)
        names[cp] = name

    return gc, ccc, bidi, decomp, simple_upper, simple_lower, simple_title, numeric, names


def parse_binary_properties():
    """Return a dict of property name -> bytearray(0/1)."""
    props: dict[str, bytearray] = {}

    def ensure(name):
        if name not in props:
            props[name] = bytearray(MAX_CP)
        return props[name]

    # DerivedCoreProperties and PropList are range/value files.
    for path in (DATA / "ucd" / "DerivedCoreProperties.txt", DATA / "ucd" / "PropList.txt"):
        for line in data_lines(path):
            fields = [f.strip() for f in line.split(";")]
            if len(fields) < 2:
                continue
            name = fields[1]
            a, b = parse_range(fields[0])
            if name == "InCB":
                continue
            arr = ensure(name)
            for cp in range(a, b + 1):
                arr[cp] = 1

    # Emoji properties.
    for line in data_lines(DATA / "ucd" / "emoji" / "emoji-data.txt"):
        fields = [f.strip() for f in line.split(";")]
        if len(fields) < 2:
            continue
        name = fields[1]
        a, b = parse_range(fields[0])
        arr = ensure(name)
        for cp in range(a, b + 1):
            arr[cp] = 1

    # Binary properties derived from UCD files.
    for line in data_lines(DATA / "ucd" / "DerivedNormalizationProps.txt"):
        fields = [f.strip() for f in line.split(";")]
        if len(fields) < 2:
            continue
        name = fields[1]
        if name in ("Full_Composition_Exclusion", "Composition_Exclusion"):
            a, b = parse_range(fields[0])
            arr = ensure(name)
            for cp in range(a, b + 1):
                arr[cp] = 1

    return props


def parse_incb():
    arr = bytearray(MAX_CP)
    for line in data_lines(DATA / "ucd" / "DerivedCoreProperties.txt"):
        fields = [f.strip() for f in line.split(";")]
        if len(fields) < 3 or fields[1] != "InCB":
            continue
        a, b = parse_range(fields[0])
        value = INCB_INDEX.get(fields[2], 0)
        for cp in range(a, b + 1):
            arr[cp] = value
    return arr


def parse_scripts():
    scripts: dict[str, int] = {}
    arr = bytearray(MAX_CP)
    for line in data_lines(DATA / "ucd" / "Scripts.txt"):
        fields = [f.strip() for f in line.split(";")]
        if len(fields) < 2:
            continue
        name = fields[1]
        if name not in scripts:
            scripts[name] = len(scripts) + 1
        a, b = parse_range(fields[0])
        value = scripts[name]
        for cp in range(a, b + 1):
            arr[cp] = value
    return arr, [None] + list(scripts.keys())


def parse_blocks():
    blocks: dict[str, int] = {}
    arr = [0] * MAX_CP
    for line in data_lines(DATA / "ucd" / "Blocks.txt"):
        fields = [f.strip() for f in line.split(";")]
        if len(fields) < 2:
            continue
        name = fields[1]
        if name not in blocks:
            blocks[name] = len(blocks) + 1
        a, b = parse_range(fields[0])
        value = blocks[name]
        for cp in range(a, b + 1):
            arr[cp] = value
    return arr, [None] + list(blocks.keys())


def parse_age():
    ages: dict[str, int] = {}
    arr = bytearray(MAX_CP)
    for line in data_lines(DATA / "ucd" / "DerivedAge.txt"):
        fields = [f.strip() for f in line.split(";")]
        if len(fields) < 2:
            continue
        name = fields[1]
        if name not in ages:
            ages[name] = len(ages) + 1
        a, b = parse_range(fields[0])
        value = ages[name]
        for cp in range(a, b + 1):
            arr[cp] = value
    ordered = [None] + sorted(ages, key=lambda s: tuple(int(x) for x in s.split(".")))
    remap = {name: i + 1 for i, name in enumerate(ordered[1:])}
    out = bytearray(MAX_CP)
    for cp in range(MAX_CP):
        if arr[cp]:
            out[cp] = remap[ordered[arr[cp]]]
    return out, ordered


def parse_eaw():
    arr = bytearray(MAX_CP)
    for line in data_lines(DATA / "ucd" / "EastAsianWidth.txt"):
        fields = [f.strip() for f in line.split(";")]
        if len(fields) < 2:
            continue
        a, b = parse_range(fields[0])
        value = EAW_INDEX.get(fields[1], 0)
        for cp in range(a, b + 1):
            arr[cp] = value
    return arr


def parse_line_break():
    arr = bytearray(MAX_CP)
    for line in data_lines(DATA / "ucd" / "LineBreak.txt"):
        fields = [f.strip() for f in line.split(";")]
        if len(fields) < 2:
            continue
        a, b = parse_range(fields[0])
        value = LINE_BREAK_INDEX.get(fields[1], 0)
        for cp in range(a, b + 1):
            arr[cp] = value
    # Defaults documented in the file header.
    defaults = [
        (0x3400, 0x4DBF, "ID"), (0x4E00, 0x9FFF, "ID"),
        (0xF900, 0xFAFF, "ID"), (0x20000, 0x2FFFD, "ID"),
        (0x30000, 0x3FFFD, "ID"),
        (0x1FC00, 0x1FFFD, "ID"), (0x20A0, 0x20CF, "PR"),
    ]
    for a, b, value in defaults:
        v = LINE_BREAK_INDEX[value]
        for cp in range(a, b + 1):
            if arr[cp] == 0:
                arr[cp] = v
    return arr


def parse_bidi_classes():
    arr = bytearray(MAX_CP)
    for line in data_lines(DATA / "ucd" / "extracted" / "DerivedBidiClass.txt"):
        fields = [f.strip() for f in line.split(";")]
        if len(fields) < 2:
            continue
        a, b = parse_range(fields[0])
        value = BIDI_INDEX.get(fields[1], 0)
        for cp in range(a, b + 1):
            arr[cp] = value
    return arr



def parse_break_property(path: Path, order: list[str]):
    index = {v: i for i, v in enumerate(order)}
    arr = bytearray(MAX_CP)
    for line in data_lines(path):
        fields = [f.strip() for f in line.split(";")]
        if len(fields) < 2:
            continue
        a, b = parse_range(fields[0])
        value = index.get(fields[1], 0)
        for cp in range(a, b + 1):
            arr[cp] = value
    return arr


def parse_vertical():
    arr = bytearray(MAX_CP)
    for line in data_lines(DATA / "ucd" / "VerticalOrientation.txt"):
        fields = [f.strip() for f in line.split(";")]
        if len(fields) < 2:
            continue
        a, b = parse_range(fields[0])
        value = VO_INDEX.get(fields[1], 0)
        for cp in range(a, b + 1):
            arr[cp] = value
    return arr


def parse_joining():
    arr = bytearray(MAX_CP)
    for line in data_lines(DATA / "ucd" / "extracted" / "DerivedJoiningType.txt"):
        fields = [f.strip() for f in line.split(";")]
        if len(fields) < 2:
            continue
        a, b = parse_range(fields[0])
        value = JOINING_INDEX.get(fields[1], 0)
        for cp in range(a, b + 1):
            arr[cp] = value
    return arr


def parse_case_folding():
    full: dict[int, list[int]] = {}
    simple: dict[int, int] = {}
    for line in data_lines(DATA / "ucd" / "CaseFolding.txt"):
        fields = [f.strip() for f in line.split(";")]
        if len(fields) < 3:
            continue
        cp = int(fields[0], 16)
        status = fields[1]
        mapping = [int(x, 16) for x in fields[2].split()]
        if status in ("C", "F"):
            full[cp] = mapping
        if status in ("C", "S"):
            simple[cp] = mapping[0]
    return full, simple


def parse_special_casing():
    lower: dict[int, list[int]] = {}
    title: dict[int, list[int]] = {}
    upper: dict[int, list[int]] = {}
    for line in data_lines(DATA / "ucd" / "SpecialCasing.txt"):
        fields = [f.strip() for f in line.split(";")]
        if len(fields) < 5:
            continue
        if fields[4]:
            continue  # conditional mapping; handled separately by the algorithm
        cp = int(fields[0], 16)
        lower[cp] = [int(x, 16) for x in fields[1].split()] if fields[1] else []
        title[cp] = [int(x, 16) for x in fields[2].split()] if fields[2] else []
        upper[cp] = [int(x, 16) for x in fields[3].split()] if fields[3] else []
    return lower, title, upper


def parse_named_sequences():
    entries = []
    for line in data_lines(DATA / "ucd" / "NamedSequences.txt"):
        fields = [f.strip() for f in line.split(";")]
        if len(fields) < 2:
            continue
        entries.append((fields[0], [int(x, 16) for x in fields[1].split()]))
    return entries


def parse_confusables():
    entries = []
    for line in data_lines(DATA / "security" / "confusables.txt"):
        fields = [f.strip() for f in line.split(";")]
        if len(fields) < 2:
            continue
        entries.append((int(fields[0], 16), [int(x, 16) for x in fields[1].split()]))
    return entries


def parse_idna():
    entries = []
    for line in data_lines(DATA / "idna" / "IdnaMappingTable.txt"):
        fields = [f.strip() for f in line.split(";")]
        if len(fields) < 2:
            continue
        a, b = parse_range(fields[0])
        status = fields[1]
        mapping = [int(x, 16) for x in fields[2].split()] if len(fields) > 2 and fields[2] else []
        flags = fields[3] if len(fields) > 3 else ""
        entries.append((a, b, status, mapping, flags))
    return entries



LINK_TERM_VALUES = {"Include": 1, "Hard": 2, "Soft": 3, "Close": 4, "Open": 5}
EAST_ASIAN_SPACING_VALUES = {"W": 1, "N": 2, "O": 3, "C": 4}


def parse_link_term():
    arr = bytearray(2 for _ in range(MAX_CP))
    for line in data_lines(DATA / "linkification" / "LinkTerm.txt"):
        fields = [f.strip() for f in line.split(";")]
        if len(fields) < 2:
            continue
        a, b = parse_range(fields[0])
        v = LINK_TERM_VALUES.get(fields[1])
        if v is None:
            continue
        for cp in range(a, b + 1):
            arr[cp] = v
    return arr


def parse_link_bracket():
    entries = []
    for line in data_lines(DATA / "linkification" / "LinkBracket.txt"):
        fields = [f.strip() for f in line.split(";")]
        if len(fields) < 2:
            continue
        a = int(fields[0], 16)
        b = int(fields[1], 16)
        entries.append((a, b))
    entries.sort(key=lambda e: e[0])
    return entries


def parse_link_email():
    arr = bytearray(MAX_CP)
    for line in data_lines(DATA / "linkification" / "LinkEmail.txt"):
        fields = line.split(";")
        a, b = parse_range(fields[0])
        for cp in range(a, b + 1):
            arr[cp] = 1
    return arr


def parse_east_asian_spacing():
    arr = bytearray(3 for _ in range(MAX_CP))  # missing values default to O (3)
    for line in data_lines(DATA / "tr59" / "east-asian-spacing.txt"):
        fields = [f.strip() for f in line.split(";")]
        if len(fields) < 2:
            continue
        a, b = parse_range(fields[0])
        v = EAST_ASIAN_SPACING_VALUES.get(fields[1])
        if v is None:
            continue
        for cp in range(a, b + 1):
            arr[cp] = v
    return arr


# UTS #10 Table 16 gives each siniform @implicitweights group a fixed offset
# origin (not the start of the individual range).  Tangut Supplement and
# Tangut Components Supplement continue the numbering of their parent blocks.
IMPLICIT_OFFSET_BASE = {
    0xFB00: 0x17000,  # Tangut / Tangut Supplement
    0xFB01: 0x18800,  # Tangut Components / Tangut Components Supplement
    0xFB02: 0x1B170,  # Nushu
    0xFB03: 0x18B00,  # Khitan Small Script
    0xFB04: 0x18E00,  # Jurchen / Jurchen Radicals
    0xFB05: 0x3D000,  # Seal
}


def parse_collation():
    singles = {}
    multis = {}
    implicit = []
    for raw in read(DATA / "uca" / "allkeys.txt").splitlines():
        line = raw.split("#", 1)[0].strip()
        if not line:
            continue
        if line.startswith("@implicitweights"):
            m = re.match(r"@implicitweights\s+([0-9A-Fa-f]+)\.\.([0-9A-Fa-f]+)\s*;\s*([0-9A-Fa-f]+)", line)
            if m:
                start = int(m.group(1), 16)
                end = int(m.group(2), 16)
                base = int(m.group(3), 16)
                offset_base = IMPLICIT_OFFSET_BASE.get(base, start)
                implicit.append((start, end, base, offset_base))
            continue
        if line.startswith("@"):
            continue
        if ";" not in line:
            continue
        left, right = line.split(";", 1)
        seq = [int(x, 16) for x in left.split()]
        weights = []
        for m in re.finditer(r"\[([.*])([0-9A-Fa-f]{4})\.([0-9A-Fa-f]{4})\.([0-9A-Fa-f]{4})\]", right):
            variable = 1 if m.group(1) == "*" else 0
            weights.append((int(m.group(2), 16), int(m.group(3), 16), int(m.group(4), 16), variable))
        if not weights:
            continue
        if len(seq) == 1:
            singles[seq[0]] = weights
        else:
            multis[tuple(seq)] = weights
    return singles, multis, implicit


def parse_emoji_sequences():
    entries = []
    for name in ("emoji-sequences.txt", "emoji-zwj-sequences.txt"):
        path = DATA / "emoji" / name
        if not path.exists():
            continue
        for line in data_lines(path):
            fields = [f.strip() for f in line.split(";")]
            if len(fields) < 2:
                continue
            parts = fields[0].split()
            # The only ranges in the RGI files are single-code-point
            # Basic_Emoji ranges; expand them so every RGI code point is
            # represented in the generated set.
            if len(parts) == 1 and ".." in parts[0]:
                a, b = parts[0].split("..", 1)
                for cp in range(int(a, 16), int(b, 16) + 1):
                    entries.append(([cp], fields[1]))
                continue
            seq = []
            ok = True
            for part in parts:
                if ".." in part:
                    ok = False
                    break
                seq.append(int(part, 16))
            if ok and seq:
                entries.append((seq, fields[1]))
    return entries


def pool_add(pool: list[int], seq: list[int]) -> int:
    if not seq:
        return 0
    # Simple linear intern; the number of distinct sequences is modest.
    key = tuple(seq)
    try:
        return pool_index[key]
    except KeyError:
        off = len(pool)
        pool.extend(seq)
        pool_index[key] = off
        return off


pool_index: dict[tuple[int, ...], int] = {}


def emit_c(path: Path, src_path: Path, data: dict):
    lines = []
    w = lines.append
    w("/* Generated by tools/gen_ucd.py -- do not edit. */")
    w(f"/* Unicode {UNICODE_VERSION}; generator {GENERATOR_VERSION}; sha256 {data['checksum']} */")
    w("#ifndef UNICODOID_UCD_DATA_H")
    w("#define UNICODOID_UCD_DATA_H")
    w("#include <stdint.h>")
    w("#include <stddef.h>")
    w("")
    w("#ifdef __cplusplus\nextern \"C\" {\n#endif")
    w("")
    w("typedef struct { uint32_t start; uint32_t end; uint16_t value; } unicodoid_ucd_range_t;")
    w("typedef struct { uint32_t cp; uint32_t offset; uint16_t length; uint8_t kind; } unicodoid_ucd_map_t;")
    w("typedef struct { uint32_t cp; uint32_t mapped; } unicodoid_ucd_pair_t;")
    w("typedef struct { uint32_t first; uint32_t second; uint32_t composed; } unicodoid_ucd_comp_t;")
    w("typedef struct { uint32_t cp; uint32_t offset; uint16_t length; } unicodoid_ucd_seq_t;")
    w("typedef struct { uint32_t cp; int64_t numerator; int64_t denominator; int64_t digit; } unicodoid_ucd_numeric_t;")
    w("typedef struct { uint32_t offset; uint16_t length; } unicodoid_ucd_named_seq_t;")
    w("typedef struct { uint32_t primary; uint32_t secondary; uint32_t tertiary; uint8_t variable; } unicodoid_ucd_weight_t;")
    w("typedef struct { uint32_t start; uint32_t end; uint32_t base; uint32_t offset_base; } unicodoid_ucd_implicit_t;")
    w("")
    w(f"#define UNICODOID_UCD_VERSION \"{UNICODE_VERSION}\"")
    w(f"#define UNICODOID_UCD_GENERATOR \"{GENERATOR_VERSION}\"")
    w(f"#define UNICODOID_UCD_CHECKSUM \"{data['checksum']}\"")
    w("")

    for name, arr in data["ranges"].items():
        w(f"extern const unicodoid_ucd_range_t unicodoid_ucd_{name}[];")
        w(f"extern const size_t unicodoid_ucd_{name}_count;")
    w("extern const unicodoid_ucd_pair_t unicodoid_ucd_link_bracket_map[];")
    w("extern const size_t unicodoid_ucd_link_bracket_map_count;")
    for name in ("decomp", "upper_full", "lower_full", "title_full", "fold_full", "confusables", "idna_mappings"):
        w(f"extern const unicodoid_ucd_map_t unicodoid_ucd_{name}[];")
        w(f"extern const size_t unicodoid_ucd_{name}_count;")
    w("extern const char* const unicodoid_ucd_named_seq_names[];")
    w("extern const size_t unicodoid_ucd_named_seq_count;")
    w("extern const unicodoid_ucd_named_seq_t unicodoid_ucd_named_seq_index[];")
    w("extern const char* const unicodoid_ucd_emoji_seq_types[];")
    w("extern const size_t unicodoid_ucd_emoji_seq_count;")
    w("extern const unicodoid_ucd_named_seq_t unicodoid_ucd_emoji_seq_index[];")
    w("extern const uint32_t unicodoid_ucd_pool[];")
    w("extern const size_t unicodoid_ucd_pool_count;")
    for name in ("simple_upper", "simple_lower", "simple_title", "simple_fold"):
        w(f"extern const unicodoid_ucd_pair_t unicodoid_ucd_{name}[];")
        w(f"extern const size_t unicodoid_ucd_{name}_count;")
    w("extern const unicodoid_ucd_comp_t unicodoid_ucd_composition[];")
    w("extern const size_t unicodoid_ucd_composition_count;")
    w("extern const unicodoid_ucd_seq_t unicodoid_ucd_collation_single[];")
    w("extern const size_t unicodoid_ucd_collation_single_count;")
    w("extern const unicodoid_ucd_seq_t unicodoid_ucd_collation_multi[];")
    w("extern const size_t unicodoid_ucd_collation_multi_count;")
    w("extern const uint32_t unicodoid_ucd_collation_keys[];")
    w("extern const size_t unicodoid_ucd_collation_keys_count;")
    w("extern const unicodoid_ucd_implicit_t unicodoid_ucd_implicit[];")
    w("extern const size_t unicodoid_ucd_implicit_count;")
    w("extern const unicodoid_ucd_numeric_t unicodoid_ucd_numeric[];")
    w("extern const size_t unicodoid_ucd_numeric_count;")
    w("extern const char* const unicodoid_ucd_script_names[];")
    w("extern const size_t unicodoid_ucd_script_names_count;")
    w("extern const char* const unicodoid_ucd_block_names[];")
    w("extern const size_t unicodoid_ucd_block_names_count;")
    w("extern const char* const unicodoid_ucd_age_names[];")
    w("extern const size_t unicodoid_ucd_age_names_count;")
    w("extern const char* const unicodoid_ucd_gc_names[];")
    w("extern const size_t unicodoid_ucd_gc_names_count;")
    w("extern const char* const unicodoid_ucd_bidi_names[];")
    w("extern const size_t unicodoid_ucd_bidi_names_count;")
    w("extern const char* const unicodoid_ucd_eaw_names[];")
    w("extern const size_t unicodoid_ucd_eaw_names_count;")
    w("extern const char* const unicodoid_ucd_line_break_names[];")
    w("extern const size_t unicodoid_ucd_line_break_names_count;")
    w("extern const char* const unicodoid_ucd_grapheme_names[];")
    w("extern const size_t unicodoid_ucd_grapheme_names_count;")
    w("extern const char* const unicodoid_ucd_word_names[];")
    w("extern const size_t unicodoid_ucd_word_names_count;")
    w("extern const char* const unicodoid_ucd_sentence_names[];")
    w("extern const size_t unicodoid_ucd_sentence_names_count;")
    w("extern const char* const unicodoid_ucd_vertical_names[];")
    w("extern const size_t unicodoid_ucd_vertical_names_count;")
    w("extern const char* const unicodoid_ucd_joining_names[];")
    w("extern const size_t unicodoid_ucd_joining_names_count;")
    w("extern const char* const unicodoid_ucd_incb_names[];")
    w("extern const size_t unicodoid_ucd_incb_names_count;")
    w("")
    w("#ifdef __cplusplus\n}\n#endif")
    w("#endif")
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")

    src = []
    s = src.append
    s("/* Generated by tools/gen_ucd.py -- do not edit. */")
    s(f"/* Unicode {UNICODE_VERSION}; generator {GENERATOR_VERSION}; sha256 {data['checksum']} */")
    s('#include "unicodoid/unicodoid_ucd_data.h"')
    s("")

    def emit_ranges(name, ranges):
        s(f"const unicodoid_ucd_range_t unicodoid_ucd_{name}[] = {{")
        for a, b, v in ranges:
            s(f"{{0x{a:X},0x{b:X},{v}}},")
        s("};")
        s(f"const size_t unicodoid_ucd_{name}_count = sizeof(unicodoid_ucd_{name})/sizeof(unicodoid_ucd_{name}[0]);")
        s("")

    for name, arr in data["ranges"].items():
        emit_ranges(name, ranges_from_bytearray(arr) if isinstance(arr, bytearray) else ranges_from_u16array(arr))

    s("const uint32_t unicodoid_ucd_pool[] = {")
    for i in range(0, len(data["pool"]), 12):
        s(",".join(f"0x{x:X}" for x in data["pool"][i:i+12]) + ",")
    s("};")
    s(f"const size_t unicodoid_ucd_pool_count = {len(data['pool'])};")
    s("")

    for name in ("decomp", "upper_full", "lower_full", "title_full", "fold_full", "confusables", "idna_mappings"):
        s(f"const unicodoid_ucd_map_t unicodoid_ucd_{name}[] = {{")
        for cp, off, length, kind in data["maps"][name]:
            s(f"{{0x{cp:X},0x{off:X},{length},{kind}}},")
        s("};")
        s(f"const size_t unicodoid_ucd_{name}_count = {len(data['maps'][name])};")
        s("")

    for name in ("simple_upper", "simple_lower", "simple_title", "simple_fold"):
        s(f"const unicodoid_ucd_pair_t unicodoid_ucd_{name}[] = {{")
        for cp, mapped in data[name]:
            s(f"{{0x{cp:X},0x{mapped:X}}},")
        s("};")
        s(f"const size_t unicodoid_ucd_{name}_count = {len(data[name])};")
        s("")

    s("const unicodoid_ucd_comp_t unicodoid_ucd_composition[] = {")
    for first, second, composed in data["composition"]:
        s(f"{{0x{first:X},0x{second:X},0x{composed:X}}},")
    s("};")
    s(f"const size_t unicodoid_ucd_composition_count = {len(data['composition'])};")
    s("")

    s("const unicodoid_ucd_pair_t unicodoid_ucd_link_bracket_map[] = {")
    for a, b in data["link_bracket"]:
        s(f"{{0x{a:X},0x{b:X}}},")
    s("};")
    s(f"const size_t unicodoid_ucd_link_bracket_map_count = {len(data['link_bracket'])};")
    s("")

    def c_str(text):
        assert chr(92) not in text, text
        assert '"' not in text, text
        return text

    s("const char* const unicodoid_ucd_named_seq_names[] = {")
    for name, _off, _len in data["named_seq_entries"]:
        s(f'"{c_str(name)}",')
    s("};")
    s(f"const size_t unicodoid_ucd_named_seq_count = {len(data['named_seq_entries'])};")
    s("")
    s("const unicodoid_ucd_named_seq_t unicodoid_ucd_named_seq_index[] = {")
    for _name, off, length in data["named_seq_entries"]:
        s(f"{{0x{off:X},{length}}},")
    s("};")
    s("")
    s("const char* const unicodoid_ucd_emoji_seq_types[] = {")
    for typ, _off, _len in data["emoji_seq_entries"]:
        s(f'"{c_str(typ)}",')
    s("};")
    s(f"const size_t unicodoid_ucd_emoji_seq_count = {len(data['emoji_seq_entries'])};")
    s("")
    s("const unicodoid_ucd_named_seq_t unicodoid_ucd_emoji_seq_index[] = {")
    for _typ, off, length in data["emoji_seq_entries"]:
        s(f"{{0x{off:X},{length}}},")
    s("};")
    s("")

    s("const unicodoid_ucd_seq_t unicodoid_ucd_collation_single[] = {")
    for cp, off, length in data["collation_single"]:
        s(f"{{0x{cp:X},0x{off:X},{length}}},")
    s("};")
    s(f"const size_t unicodoid_ucd_collation_single_count = {len(data['collation_single'])};")
    s("")

    s("const unicodoid_ucd_seq_t unicodoid_ucd_collation_multi[] = {")
    for cp, off, length in data["collation_multi"]:
        s(f"{{0x{cp:X},0x{off:X},{length}}},")
    s("};")
    s(f"const size_t unicodoid_ucd_collation_multi_count = {len(data['collation_multi'])};")
    s("")

    s("const uint32_t unicodoid_ucd_collation_keys[] = {")
    for i in range(0, len(data["collation_keys"]), 12):
        s(",".join(f"0x{x:X}" for x in data["collation_keys"][i:i+12]) + ",")
    s("};")
    s(f"const size_t unicodoid_ucd_collation_keys_count = {len(data['collation_keys'])};")
    s("")

    s("const unicodoid_ucd_implicit_t unicodoid_ucd_implicit[] = {")
    for a, b, base, offset_base in data["implicit"]:
        s(f"{{0x{a:X},0x{b:X},0x{base:X},0x{offset_base:X}}},")
    s("};")
    s(f"const size_t unicodoid_ucd_implicit_count = {len(data['implicit'])};")
    s("")

    s("const unicodoid_ucd_numeric_t unicodoid_ucd_numeric[] = {")
    for cp, num, den, digit in data["numeric"]:
        s(f"{{0x{cp:X},{num}LL,{den}LL,{digit}LL}},")
    s("};")
    s(f"const size_t unicodoid_ucd_numeric_count = {len(data['numeric'])};")
    s("")

    def emit_names(name, values):
        s(f"const char* const unicodoid_ucd_{name}[] = {{")
        for v in values:
            s(f'"{v}",')
        s("};")
        s(f"const size_t unicodoid_ucd_{name}_count = {len(values)};")
        s("")

    emit_names("script_names", data["script_names"])
    emit_names("block_names", data["block_names"])
    emit_names("age_names", data["age_names"])
    emit_names("gc_names", GC_VALUES)
    emit_names("bidi_names", BIDI_ORDER)
    emit_names("eaw_names", EAW_ORDER)
    emit_names("line_break_names", LINE_BREAK_ORDER)
    emit_names("grapheme_names", GRAPHEME_ORDER)
    emit_names("word_names", WORD_ORDER)
    emit_names("sentence_names", SENTENCE_ORDER)
    emit_names("vertical_names", VO_ORDER)
    emit_names("joining_names", JOINING_ORDER)
    emit_names("incb_names", INCB_ORDER)

    src_path.parent.mkdir(parents=True, exist_ok=True)
    src_path.write_text("\n".join(src) + "\n", encoding="utf-8")


def emit_rust(path: Path, data: dict):
    lines = []
    w = lines.append
    w("// Generated by tools/gen_ucd.py -- do not edit.")
    w(f"// Unicode {UNICODE_VERSION}; generator {GENERATOR_VERSION}; sha256 {data['checksum']}")
    w("#![allow(clippy::unreadable_literal)]")
    w("")
    w(f'pub const UCD_VERSION: &str = "{UNICODE_VERSION}";')
    w(f'pub const UCD_GENERATOR: &str = "{GENERATOR_VERSION}";')
    w(f'pub const UCD_CHECKSUM: &str = "{data["checksum"]}";')
    w("")
    w("pub type Range = (u32, u32, u16);")
    w("pub type MapEntry = (u32, u32, u16, u8);")
    w("pub type Pair = (u32, u32);")
    w("pub type Comp = (u32, u32, u32);")
    w("pub type Seq = (u32, u32, u16);")
    w("pub type Weight = (u16, u16, u16, u8);")
    w("pub type Implicit = (u32, u32, u32, u32);")
    w("")

    def emit_ranges(name, ranges):
        w(f"pub static {name}: &[Range] = &[")
        for a, b, v in ranges:
            w(f"({a}, {b}, {v}),")
        w("];")
        w("")

    for name, arr in data["ranges"].items():
        emit_ranges(name.upper(), ranges_from_bytearray(arr) if isinstance(arr, bytearray) else ranges_from_u16array(arr))

    w("pub static POOL: &[u32] = &[")
    for i in range(0, len(data["pool"]), 16):
        w(",".join(str(x) for x in data["pool"][i:i+16]) + ",")
    w("];")
    w("")

    for name in ("decomp", "upper_full", "lower_full", "title_full", "fold_full", "confusables", "idna_mappings"):
        w(f"pub static {name.upper()}: &[MapEntry] = &[")
        for cp, off, length, kind in data["maps"][name]:
            w(f"({cp}, {off}, {length}, {kind}),")
        w("];")
        w("")

    for name in ("simple_upper", "simple_lower", "simple_title", "simple_fold"):
        w(f"pub static {name.upper()}: &[Pair] = &[")
        for cp, mapped in data[name]:
            w(f"({cp}, {mapped}),")
        w("];")
        w("")

    w('pub static LINK_BRACKET: &[Pair] = &[')
    for a, b in data["link_bracket"]:
        w(f"({a}, {b}),")
    w("];")
    w("")
    w('pub static NAMED_SEQ_NAMES: &[&str] = &[')
    for name, _off, _len in data["named_seq_entries"]:
        w(f'"{name}",')
    w("];")
    w('pub static NAMED_SEQ_INDEX: &[(u32, u16)] = &[')
    for _name, off, length in data["named_seq_entries"]:
        w(f"({off}, {length}),")
    w("];")
    w("")
    w('pub static EMOJI_SEQ_TYPES: &[&str] = &[')
    for typ, _off, _len in data["emoji_seq_entries"]:
        w(f'"{typ}",')
    w("];")
    w('pub static EMOJI_SEQ_INDEX: &[(u32, u16)] = &[')
    for _typ, off, length in data["emoji_seq_entries"]:
        w(f"({off}, {length}),")
    w("];")
    w("")
    w("pub static COMPOSITION: &[Comp] = &[")
    for first, second, composed in data["composition"]:
        w(f"({first}, {second}, {composed}),")
    w("];")
    w("")

    w("pub static COLLATION_SINGLE: &[Seq] = &[")
    for cp, off, length in data["collation_single"]:
        w(f"({cp}, {off}, {length}),")
    w("];")
    w("")

    w("pub static COLLATION_MULTI: &[Seq] = &[")
    for cp, off, length in data["collation_multi"]:
        w(f"({cp}, {off}, {length}),")
    w("];")
    w("")

    w("pub static COLLATION_KEYS: &[u32] = &[")
    for i in range(0, len(data["collation_keys"]), 16):
        w(",".join(str(x) for x in data["collation_keys"][i:i+16]) + ",")
    w("];")
    w("")

    w("pub static IMPLICIT: &[Implicit] = &[")
    for a, b, base, offset_base in data["implicit"]:
        w(f"({a}, {b}, {base}, {offset_base}),")
    w("];")
    w("")

    w("pub static NUMERIC: &[(u32, i64, i64, i64)] = &[")
    for cp, num, den, digit in data["numeric"]:
        w(f"({cp}, {num}, {den}, {digit}),")
    w("];")
    w("")

    def emit_names(name, values):
        w(f"pub static {name.upper()}: &[&str] = &[")
        for v in values:
            w(f'"{v}",')
        w("];")
        w("")

    emit_names("script_names", data["script_names"])
    emit_names("block_names", data["block_names"])
    emit_names("age_names", data["age_names"])
    emit_names("gc_names", GC_VALUES)
    emit_names("bidi_names", BIDI_ORDER)
    emit_names("eaw_names", EAW_ORDER)
    emit_names("line_break_names", LINE_BREAK_ORDER)
    emit_names("grapheme_names", GRAPHEME_ORDER)
    emit_names("word_names", WORD_ORDER)
    emit_names("sentence_names", SENTENCE_ORDER)
    emit_names("vertical_names", VO_ORDER)
    emit_names("joining_names", JOINING_ORDER)
    emit_names("incb_names", INCB_ORDER)

    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def emit_csharp(path: Path, data: dict):
    lines = []
    w = lines.append
    w("// Generated by tools/gen_ucd.py -- do not edit.")
    w(f"// Unicode {UNICODE_VERSION}; generator {GENERATOR_VERSION}; sha256 {data['checksum']}")
    w("using System.Collections.Generic;")
    w("")
    w("namespace Unicodoid.Generated;")
    w("")
    w("internal static class UcdData")
    w("{")
    w(f'    public const string Version = "{UNICODE_VERSION}";')
    w(f'    public const string Generator = "{GENERATOR_VERSION}";')
    w(f'    public const string Checksum = "{data["checksum"]}";')
    w("    public readonly record struct Range(uint Start, uint End, ushort Value);")
    w("    public readonly record struct MapEntry(uint Cp, uint Offset, ushort Length, byte Kind);")
    w("    public readonly record struct Pair(uint Cp, uint Mapped);")
    w("    public readonly record struct Comp(uint First, uint Second, uint Composed);")
    w("    public readonly record struct Seq(uint Cp, uint Offset, ushort Length);")
    w("    public readonly record struct Implicit(uint Start, uint End, uint Base, uint OffsetBase);")
    w("    public readonly record struct Numeric(uint Cp, long Numerator, long Denominator, long Digit);")
    w("")

    # A single static constructor containing every table would exceed the JIT's
    # method-size limits (stack overflow on first use).  Emit one lazily
    # initialised property per table, with the elements split across small
    # chunk methods.
    CHUNK = 2048

    def emit_table(type_name, name, entries, formatter):
        backing = "s_" + name
        create = "Create" + name
        chunks = [entries[i:i + CHUNK] for i in range(0, len(entries), CHUNK)]
        w(f"    private static {type_name}[]? {backing};")
        w(f"    public static {type_name}[] {name} => {backing} ??= {create}();")
        w(f"    private static {type_name}[] {create}()")
        w("    {")
        w(f"        var result = new List<{type_name}>({len(entries)});")
        for index in range(len(chunks)):
            w(f"        Add{name}{index}(result);")
        w("        return result.ToArray();")
        w("    }")
        w("")
        for index, chunk in enumerate(chunks):
            w(f"    private static void Add{name}{index}(List<{type_name}> result)")
            w("    {")
            for entry in chunk:
                w(f"        result.Add({formatter(entry)});")
            w("    }")
            w("")

    for name, arr in data["ranges"].items():
        ranges = ranges_from_bytearray(arr) if isinstance(arr, bytearray) else ranges_from_u16array(arr)
        emit_table(
            "Range",
            name[0].upper() + name[1:],
            ranges,
            lambda e: f"new({e[0]}u, {e[1]}u, {e[2]})",
        )

    emit_table("uint", "Pool", data["pool"], lambda x: f"{x}u")

    for name in ("decomp", "upper_full", "lower_full", "title_full", "fold_full", "confusables", "idna_mappings"):
        emit_table(
            "MapEntry",
            name[0].upper() + name[1:],
            data["maps"][name],
            lambda e: f"new({e[0]}u, {e[1]}u, {e[2]}, {e[3]})",
        )

    for name in ("simple_upper", "simple_lower", "simple_title", "simple_fold"):
        emit_table(
            "Pair",
            name[0].upper() + name[1:],
            data[name],
            lambda e: f"new({e[0]}u, {e[1]}u)",
        )

    emit_table(
        "Comp",
        "Composition",
        data["composition"],
        lambda e: f"new({e[0]}u, {e[1]}u, {e[2]}u)",
    )

    emit_table(
        "Pair",
        "LinkBracket",
        data["link_bracket"],
        lambda e: f"new({e[0]}u, {e[1]}u)",
    )

    def csharp_str(text):
        assert chr(92) not in text, text
        assert '"' not in text, text
        return text

    w("    public static readonly string[] NamedSeqNames = new string[]")
    w("    {")
    for name, _off, _len in data["named_seq_entries"]:
        w(f'        "{csharp_str(name)}",')
    w("    };")
    w("")
    w("    public static readonly (uint, ushort)[] NamedSeqIndex = new (uint, ushort)[]")
    w("    {")
    for _name, off, length in data["named_seq_entries"]:
        w(f"        ({off}u, {length}),")
    w("    };")
    w("")
    w("    public static readonly string[] EmojiSeqTypes = new string[]")
    w("    {")
    for typ, _off, _len in data["emoji_seq_entries"]:
        w(f'        "{csharp_str(typ)}",')
    w("    };")
    w("")
    w("    public static readonly (uint, ushort)[] EmojiSeqIndex = new (uint, ushort)[]")
    w("    {")
    for _typ, off, length in data["emoji_seq_entries"]:
        w(f"        ({off}u, {length}),")
    w("    };")
    w("")

    emit_table(
        "Seq",
        "CollationSingle",
        data["collation_single"],
        lambda e: f"new({e[0]}u, {e[1]}u, {e[2]})",
    )

    emit_table(
        "Seq",
        "CollationMulti",
        data["collation_multi"],
        lambda e: f"new({e[0]}u, {e[1]}u, {e[2]})",
    )

    emit_table("uint", "CollationKeys", data["collation_keys"], lambda x: f"{x}u")

    emit_table(
        "Implicit",
        "ImplicitWeights",
        data["implicit"],
        lambda e: f"new({e[0]}u, {e[1]}u, {e[2]}u, {e[3]}u)",
    )

    emit_table(
        "Numeric",
        "NumericValues",
        data["numeric"],
        lambda e: f"new({e[0]}u, {e[1]}, {e[2]}, {e[3]})",
    )

    def emit_names(name, values):
        w(f"    public static readonly string[] {name[0].upper() + name[1:]} =")
        w("    [")
        for v in values:
            w(f'        "{v}",')
        w("    ];")
        w("")

    emit_names("scriptNames", data["script_names"])
    emit_names("blockNames", data["block_names"])
    emit_names("ageNames", data["age_names"])
    emit_names("gcNames", GC_VALUES)
    emit_names("bidiNames", BIDI_ORDER)
    emit_names("eawNames", EAW_ORDER)
    emit_names("lineBreakNames", LINE_BREAK_ORDER)
    emit_names("graphemeNames", GRAPHEME_ORDER)
    emit_names("wordNames", WORD_ORDER)
    emit_names("sentenceNames", SENTENCE_ORDER)
    emit_names("verticalNames", VO_ORDER)
    emit_names("joiningNames", JOINING_ORDER)
    emit_names("incbNames", INCB_ORDER)

    w("}")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def _parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Generate Unicode property tables from official UCD data.",
        epilog=(
            "Without arguments this generates the current 18.0.0 baseline. "
            "Pass e.g. --unicode-version 17.0.0 for a compatibility backport; "
            "the generator then reads data/ucd/<version>/ and writes "
            "data/generated/ucd<major>_manifest.json."
        ),
    )
    parser.add_argument(
        "--unicode-version",
        default=DEFAULT_UNICODE_VERSION,
        help="Unicode version to generate from (default: %(default)s)",
    )
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    global pool_index, DATA, OUT_MANIFEST, UNICODE_VERSION
    args = _parse_args(argv)

    version = args.unicode_version.strip()
    if not re.fullmatch(r"\d+\.\d+\.\d+", version):
        print(
            f"invalid --unicode-version {version!r}: expected X.Y.Z (e.g. 18.0.0)",
            file=sys.stderr,
        )
        return 2
    UNICODE_VERSION = version
    DATA = ROOT / "data" / "ucd" / version
    major = version.split(".", 1)[0]
    OUT_MANIFEST = ROOT / "data" / "generated" / f"ucd{major}_manifest.json"

    if not DATA.is_dir():
        print(
            f"data directory not found: {DATA} "
            f"(fetch it first, e.g. 'python tools/fetch_ucd.py --unicode-version {version}')",
            file=sys.stderr,
        )
        return 2

    print(f"generating from Unicode {version} data in {DATA}")
    pool_index = {}

    print("parsing UnicodeData.txt")
    (gc, ccc, bidi, decomp, simple_upper, simple_lower, simple_title,
     numeric, names) = parse_unicode_data()

    print("parsing binary properties")
    binary = parse_binary_properties()
    incb = parse_incb()
    bidi = parse_bidi_classes()
    scripts, script_names = parse_scripts()
    blocks, block_names = parse_blocks()
    age, age_names = parse_age()
    eaw = parse_eaw()
    line_break = parse_line_break()
    grapheme = parse_break_property(DATA / "ucd" / "auxiliary" / "GraphemeBreakProperty.txt", GRAPHEME_ORDER)
    word = parse_break_property(DATA / "ucd" / "auxiliary" / "WordBreakProperty.txt", WORD_ORDER)
    sentence = parse_break_property(DATA / "ucd" / "auxiliary" / "SentenceBreakProperty.txt", SENTENCE_ORDER)
    vertical = parse_vertical()
    joining = parse_joining()
    fold_full, simple_fold = parse_case_folding()
    special_lower, special_title, special_upper = parse_special_casing()
    confusables = parse_confusables()
    idna = parse_idna()
    collation_singles, collation_multis, implicit = parse_collation()
    named_sequences = parse_named_sequences()
    emoji_sequences = parse_emoji_sequences()
    link_term = parse_link_term()
    link_email = parse_link_email()
    east_asian_spacing = parse_east_asian_spacing()
    link_bracket = parse_link_bracket()

    ranges = {
        "gc": gc,
        "ccc": ccc,
        "bidi": bidi,
        "eaw": eaw,
        "script_range": scripts,
        "block_range": blocks,
        "age_range": age,
        "line_break_range": line_break,
        "grapheme": grapheme,
        "word": word,
        "sentence": sentence,
        "vertical": vertical,
        "joining": joining,
        "incb": incb,
    }
    idna_status = bytearray(MAX_CP)
    idna_nv8 = bytearray(MAX_CP)
    status_index = {"valid": 0, "mapped": 1, "deviation": 2, "ignored": 3, "disallowed": 4}
    for a, b, status, mapping, flags in idna:
        v = status_index.get(status, 4)
        nv8 = 1 if "NV8" in flags else 0
        for cp in range(a, b + 1):
            idna_status[cp] = v
            idna_nv8[cp] = nv8
    ranges["idna_status_range"] = idna_status
    ranges["idna_nv8_range"] = idna_nv8
    ranges["link_term_range"] = link_term
    ranges["link_email_range"] = link_email
    ranges["east_asian_spacing_range"] = east_asian_spacing
    for name, arr in binary.items():
        ranges["prop_" + name.lower()] = arr

    pool: list[int] = []
    maps = {k: [] for k in ("decomp", "upper_full", "lower_full", "title_full", "fold_full", "confusables", "idna_mappings")}

    # Decompositions.
    for cp in sorted(decomp):
        compat, seq = decomp[cp]
        off = pool_add(pool, seq)
        maps["decomp"].append((cp, off, len(seq), 1 if compat else 0))

    # Full case mappings.  SpecialCasing overrides the simple mapping.
    def add_full(name, source):
        for cp in sorted(source):
            seq = source[cp]
            if not seq:
                continue
            off = pool_add(pool, seq)
            maps[name].append((cp, off, len(seq), 0))

    add_full("upper_full", special_upper)
    add_full("lower_full", special_lower)
    add_full("title_full", special_title)
    add_full("fold_full", fold_full)

    # Confusables.
    for cp, seq in confusables:
        off = pool_add(pool, seq)
        maps["confusables"].append((cp, off, len(seq), 0))

    # IDNA mapping sequences.
    for a, b, status, mapping, flags in idna:
        if status in ("mapped", "deviation") and mapping:
            off = pool_add(pool, mapping)
            for cp in range(a, b + 1):
                maps["idna_mappings"].append((cp, off, len(mapping), 0))

    # Composition pairs: canonical two-element decompositions not excluded.
    exclusion = binary.get("Full_Composition_Exclusion")
    composition = []
    for cp in sorted(decomp):
        compat, seq = decomp[cp]
        if compat or len(seq) != 2:
            continue
        if exclusion is not None and exclusion[cp]:
            continue
        composition.append((seq[0], seq[1], cp))
    # Binary search over pairs requires ordering by (first, second).
    composition.sort(key=lambda e: (e[0], e[1]))

    # Collation tables.
    collation_single = []
    collation_multi = []
    collation_keys = []
    for cp in sorted(collation_singles):
        weights = collation_singles[cp]
        off = len(collation_keys) // 4
        for p, s, t, v in weights:
            collation_keys.extend((p, s, t, v))
        collation_single.append((cp, off, len(weights)))
    for seq in sorted(collation_multis):
        weights = collation_multis[seq]
        off = len(collation_keys) // 4
        for p, s, t, v in weights:
            collation_keys.extend((p, s, t, v))
        # Encode the contraction sequence into the pool.  The high byte of
        # the sequence offset stores the contraction length.
        off_seq = pool_add(pool, list(seq))
        collation_multi.append(((len(seq) << 24) | off_seq, off, len(weights)))

    named_seq_entries = []
    for name, seq in named_sequences:
        off = pool_add(pool, list(seq))
        named_seq_entries.append((name, off, len(seq)))

    emoji_seq_entries = []
    for seq, typ in emoji_sequences:
        off = pool_add(pool, list(seq))
        emoji_seq_entries.append((typ, off, len(seq)))

    checksum = hashlib.sha256()
    for path in sorted(DATA.rglob("*")):
        if path.is_file():
            checksum.update(str(path.relative_to(DATA)).encode())
            checksum.update(path.read_bytes())
    digest = checksum.hexdigest()

    data = {
        "checksum": digest,
        "ranges": ranges,
        "pool": pool,
        "maps": maps,
        "simple_upper": sorted(simple_upper.items()),
        "simple_lower": sorted(simple_lower.items()),
        "simple_title": sorted(simple_title.items()),
        "simple_fold": sorted(simple_fold.items()),
        "composition": composition,
        "collation_single": collation_single,
        "collation_multi": collation_multi,
        "collation_keys": collation_keys,
        "implicit": implicit,
        "link_bracket": link_bracket,
        "named_seq_entries": named_seq_entries,
        "emoji_seq_entries": emoji_seq_entries,
        "numeric": sorted((cp, num, den, digit) for cp, (num, den, digit) in numeric.items()),
        "script_names": script_names,
        "block_names": block_names,
        "age_names": age_names,
    }

    print("emitting C tables")
    emit_c(
        ROOT / "clike" / "include" / "unicodoid" / "unicodoid_ucd_data.h",
        ROOT / "clike" / "src" / "unicodoid_ucd_data.c",
        data,
    )
    print("emitting Rust tables")
    emit_rust(ROOT / "rust" / "src" / "generated" / "ucd_data.rs", data)
    print("emitting C# tables")
    emit_csharp(ROOT / "clike" / "csharp" / "Unicodoid" / "Generated" / "UcdData.cs", data)

    OUT_MANIFEST.parent.mkdir(parents=True, exist_ok=True)
    manifest = {
        "unicode_version": UNICODE_VERSION,
        "generator_version": GENERATOR_VERSION,
        "source_checksum": digest,
        "counts": {
            "ranges": {k: len(v) for k, v in ranges.items()},
            "pool": len(pool),
            "decompositions": len(maps["decomp"]),
            "compositions": len(composition),
            "case_folding": len(maps["fold_full"]),
            "confusables": len(maps["confusables"]),
            "idna_mappings": len(maps["idna_mappings"]),
            "collation_single": len(collation_single),
            "collation_multi": len(collation_multi),
            "collation_keys": len(collation_keys),
            "named_sequences": len(named_sequences),
            "emoji_sequences": len(emoji_sequences),
        },
        "named_sequences": [
            {"name": name, "sequence": seq} for name, seq in named_sequences
        ],
        "emoji_sequences": [
            {"sequence": seq, "type": typ} for seq, typ in emoji_sequences
        ],
    }
    OUT_MANIFEST.write_text(json.dumps(manifest, indent=2, ensure_ascii=False), encoding="utf-8")
    print(f"wrote {OUT_MANIFEST}")
    print(f"source checksum {digest}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
