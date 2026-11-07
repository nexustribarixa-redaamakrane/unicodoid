#!/usr/bin/env python3
"""Summarize conformance failures from test_conformance output."""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

TEST_FILES = {
    0: "GraphemeBreakTest.txt",
    1: "WordBreakTest.txt",
    2: "LineBreakTest.txt",
    3: "SentenceBreakTest.txt",
}


def utf8_len(cp):
    return 1 if cp < 0x80 else 2 if cp < 0x800 else 3 if cp < 0x10000 else 4


def parse_cases(path):
    cases = []
    for raw in path.read_text(encoding="utf-8", errors="ignore").splitlines():
        s = raw.strip()
        if not s or s.startswith("#"):
            continue
        body = s.split("#", 1)[0]
        tokens = body.split()
        cps, breaks = [], []
        pending = True
        for t in tokens:
            if t == "\u00f7":
                pending = True
            elif t == "\u00d7":
                pending = False
            else:
                cps.append(int(t, 16))
                breaks.append(pending)
                pending = False
        ends, off = [], 0
        for i, (cp, brk) in enumerate(zip(cps, breaks)):
            if brk and i > 0:
                ends.append(off)
            off += utf8_len(cp)
        ends.append(off)
        if not cps:
            ends = []
        cases.append((cps, ends, breaks))
    return cases


def main():
    text = Path(sys.argv[1] if len(sys.argv) > 1 else ROOT / "conf_out.txt").read_text(
        encoding="utf-8", errors="ignore"
    )
    lines = text.splitlines()
    fails = []
    pending_actual = None
    for line in lines:
        if line.startswith("ACTUAL "):
            am = re.search(r"ACTUAL bt=(\d+) count=(\d+) ends=([\d,]*)", line)
            if am:
                pending_actual = [int(x) for x in am.group(3).split(",") if x]
            continue
        m = re.search(
            r"FAIL conformance bt=(\d+) case (\d+) rc=(-?\d+) byte_len=(\d+) end_count=(\d+) ends=([\d,]*) bytes=([0-9A-F]*)",
            line,
        )
        if m:
            fails.append(
                {
                    "bt": int(m.group(1)),
                    "idx": int(m.group(2)),
                    "rc": int(m.group(3)),
                    "byte_len": int(m.group(4)),
                    "end_count": int(m.group(5)),
                    "ends": [int(x) for x in m.group(6).split(",") if x],
                    "bytes": m.group(7),
                    "actual": pending_actual,
                }
            )
            pending_actual = None

    by_bt = {}
    for f in fails:
        by_bt.setdefault(f["bt"], []).append(f)
    for bt in sorted(by_bt):
        print(f"bt={bt}: {len(by_bt[bt])} failures")
    if len(sys.argv) <= 2:
        return

    want_bt = int(sys.argv[2])
    limit = int(sys.argv[3]) if len(sys.argv) > 3 else 40
    fname = TEST_FILES[want_bt]
    cases = parse_cases(ROOT / "data" / "ucd" / "18.0.0" / "ucd" / "auxiliary" / fname)
    for f in by_bt.get(want_bt, [])[:limit]:
        cps, ends, breaks = cases[f["idx"]]
        raw = bytes.fromhex(f["bytes"])
        got_cps = []
        i = 0
        while i < len(raw):
            b = raw[i]
            if b < 0x80:
                n = 1
            elif b < 0xE0:
                n = 2
            elif b < 0xF0:
                n = 3
            else:
                n = 4
            got_cps.append(int.from_bytes(raw[i : i + n], "big") if n == 1 else None)
            i += n
        # Decode properly
        got_cps = list(raw.decode("utf-8", errors="replace"))
        got_cps = [ord(ch) for ch in got_cps]
        print(f"\n=== bt={want_bt} case {f['idx']} rc={f['rc']} ===")
        print("cps   :", " ".join(f"U+{c:04X}" for c in cps))
        print("want  :", f["ends"], "count", f["end_count"])
        print("actual:", f["actual"], "count", len(f["actual"]) if f["actual"] else "?")
        # map byte offsets to cp indices
        offs = [0]
        for c in cps:
            offs.append(offs[-1] + utf8_len(c))
        def to_idx(ends):
            out = []
            for e in ends:
                if e in offs:
                    out.append(offs.index(e))
                else:
                    out.append("?")
            return out
        print("want idx  :", to_idx(f["ends"]))
        print("actual idx:", to_idx(f["actual"] or []))
        # show break flags around mismatches
        print("comment:", cases[f["idx"]][2])


if __name__ == "__main__":
    main()
