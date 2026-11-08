#!/usr/bin/env python3
"""Download the official Unicode data used by Unicodoid.

All Public files are fetched from https://www.unicode.org/Public/<version>/
and stored under data/ucd/<version> with a SHA-256 manifest.  The generator
consumes only these files; no hand-curated property table is treated as
authoritative.  UTR #59 data is served from the reports tree instead and is
fetched from https://www.unicode.org/reports/tr59/east-asian-spacing.txt.

Usage:
    python tools/fetch_ucd.py [--unicode-version 18.0.0]
"""

from __future__ import annotations

import argparse
import concurrent.futures
import hashlib
import json
import re
import sys
import urllib.request
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DEFAULT_UNICODE_VERSION = "18.0.0"
DEST = ROOT / "data" / "ucd" / DEFAULT_UNICODE_VERSION
BASE = f"https://www.unicode.org/Public/{DEFAULT_UNICODE_VERSION}/"
TR59_URL = "https://www.unicode.org/reports/tr59/east-asian-spacing.txt"
TR59_REL = "tr59/east-asian-spacing.txt"

FILES = [
    # Core UCD
    "ucd/ReadMe.txt",
    "ucd/UnicodeData.txt",
    "ucd/DerivedCoreProperties.txt",
    "ucd/DerivedNormalizationProps.txt",
    "ucd/CompositionExclusions.txt",
    "ucd/NormalizationCorrections.txt",
    "ucd/Scripts.txt",
    "ucd/ScriptExtensions.txt",
    "ucd/Blocks.txt",
    "ucd/EastAsianWidth.txt",
    "ucd/LineBreak.txt",
    "ucd/extracted/DerivedBidiClass.txt",
    "ucd/extracted/DerivedCombiningClass.txt",
    "ucd/extracted/DerivedGeneralCategory.txt",
    "ucd/extracted/DerivedDecompositionType.txt",
    "ucd/extracted/DerivedEastAsianWidth.txt",
    "ucd/extracted/DerivedNumericType.txt",
    "ucd/extracted/DerivedNumericValues.txt",
    "ucd/extracted/DerivedBinaryProperties.txt",
    "ucd/extracted/DerivedJoiningType.txt",
    "ucd/extracted/DerivedJoiningGroup.txt",
    "ucd/BidiBrackets.txt",
    "ucd/BidiMirroring.txt",
    "ucd/CaseFolding.txt",
    "ucd/SpecialCasing.txt",
    "ucd/PropList.txt",
    "ucd/PropertyAliases.txt",
    "ucd/PropertyValueAliases.txt",
    "ucd/DerivedAge.txt",
    "ucd/HangulSyllableType.txt",
    "ucd/NamedSequences.txt",
    "ucd/NamedSequencesProv.txt",
    "ucd/StandardizedVariants.txt",
    "ucd/VerticalOrientation.txt",
    "ucd/IndicSyllabicCategory.txt",
    "ucd/IndicPositionalCategory.txt",
    "ucd/ArabicShaping.txt",
    "ucd/CJKRadicals.txt",
    "ucd/EquivalentUnifiedIdeograph.txt",
    "ucd/DoNotEmit.txt",
    "ucd/Jamo.txt",
    "ucd/NameAliases.txt",
    "ucd/USourceData.txt",
    "ucd/Unikemet.txt",
    "ucd/JurchenSources.txt",
    "ucd/NushuSources.txt",
    "ucd/SealSources.txt",
    "ucd/TangutSources.txt",
    # Conformance tests
    "ucd/NormalizationTest.txt",
    "ucd/BidiTest.txt",
    "ucd/BidiCharacterTest.txt",
    "ucd/auxiliary/GraphemeBreakProperty.txt",
    "ucd/auxiliary/WordBreakProperty.txt",
    "ucd/auxiliary/SentenceBreakProperty.txt",
    "ucd/auxiliary/GraphemeBreakTest.txt",
    "ucd/auxiliary/WordBreakTest.txt",
    "ucd/auxiliary/SentenceBreakTest.txt",
    "ucd/auxiliary/LineBreakTest.txt",
    # Emoji
    "ucd/emoji/emoji-data.txt",
    "ucd/emoji/emoji-variation-sequences.txt",
    "emoji/emoji-sequences.txt",
    "emoji/emoji-zwj-sequences.txt",
    "emoji/emoji-test.txt",
    # UCA / UTS #10
    "uca/allkeys.txt",
    "uca/CollationTest.zip",
    # UTS #39
    "security/confusables.txt",
    "security/IdentifierStatus.txt",
    "security/IdentifierType.txt",
    "security/intentional.txt",
    # UTS #46
    "idna/IdnaMappingTable.txt",
    # UTS #37
    "ivd/ivd.txt",
    # UTS #58 linkification
    "linkification/LinkBracket.txt",
    "linkification/LinkDetectionTest.txt",
    "linkification/LinkEmail.txt",
    "linkification/LinkFormattingTest.txt",
    "linkification/LinkTerm.txt",
]


def fetch(rel: str) -> tuple[str, str | None]:
    target = DEST / rel
    if target.exists() and target.stat().st_size > 0:
        return rel, None
    url = BASE + rel
    try:
        req = urllib.request.Request(url, headers={"User-Agent": "Unicodoid-data/1.0"})
        with urllib.request.urlopen(req, timeout=120) as resp:
            payload = resp.read()
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(payload)
        return rel, None
    except Exception as exc:  # noqa: BLE001
        return rel, str(exc)


def fetch_tr59() -> tuple[str, str | None]:
    target = DEST / TR59_REL
    if target.exists() and target.stat().st_size > 0:
        return TR59_REL, None
    try:
        req = urllib.request.Request(TR59_URL, headers={"User-Agent": "Unicodoid-data/1.0"})
        with urllib.request.urlopen(req, timeout=120) as resp:
            payload = resp.read()
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(payload)
        return TR59_REL, None
    except Exception as exc:  # noqa: BLE001
        return TR59_REL, str(exc)


def unzip_collation_test() -> str | None:
    """Extract CollationTest.zip to uca/test/; return an error string or None."""
    archive = DEST / "uca" / "CollationTest.zip"
    if not archive.exists():
        return None
    target_dir = DEST / "uca" / "test"
    expected = target_dir / "CollationTest" / "CollationTest_NON_IGNORABLE_SHORT.txt"
    if expected.exists() and expected.stat().st_size > 0:
        return None
    try:
        target_dir.mkdir(parents=True, exist_ok=True)
        with zipfile.ZipFile(archive) as zf:
            zf.extractall(target_dir)
        return None
    except Exception as exc:  # noqa: BLE001
        return str(exc)


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def _parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Download official Unicode data for Unicodoid.",
    )
    parser.add_argument(
        "--unicode-version",
        default=DEFAULT_UNICODE_VERSION,
        help="Unicode version to fetch (default: %(default)s)",
    )
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    global DEST, BASE
    args = _parse_args(argv)
    version = args.unicode_version.strip()
    if not re.fullmatch(r"\d+\.\d+\.\d+", version):
        print(
            f"invalid --unicode-version {version!r}: expected X.Y.Z (e.g. 18.0.0)",
            file=sys.stderr,
        )
        return 2
    DEST = ROOT / "data" / "ucd" / version
    BASE = f"https://www.unicode.org/Public/{version}/"

    DEST.mkdir(parents=True, exist_ok=True)
    failures: list[tuple[str, str]] = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=6) as pool:
        futures = {pool.submit(fetch, rel): rel for rel in FILES}
        futures[pool.submit(fetch_tr59)] = TR59_REL
        for fut in concurrent.futures.as_completed(futures):
            rel, err = fut.result()
            if err:
                failures.append((rel, err))
                print(f"FAIL {rel}: {err}", file=sys.stderr)
            else:
                print(f"ok   {rel}")

    err = unzip_collation_test()
    if err:
        failures.append(("uca/CollationTest.zip (extract)", err))
        print(f"FAIL uca/CollationTest.zip extract: {err}", file=sys.stderr)
    else:
        print("ok   uca/CollationTest.zip extracted")

    all_files = FILES + [TR59_REL]
    manifest = {
        "unicode_version": version,
        "base_url": BASE,
        "retrieved": "2026-11-05",
        "files": {},
    }
    for rel in all_files:
        path = DEST / rel
        if path.exists():
            manifest["files"][rel] = {
                "bytes": path.stat().st_size,
                "sha256": sha256(path),
            }
    (DEST / "MANIFEST.json").write_text(
        json.dumps(manifest, indent=2, ensure_ascii=False), encoding="utf-8"
    )
    print(f"wrote {DEST / 'MANIFEST.json'}")
    if failures:
        print(f"{len(failures)} file(s) failed", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
