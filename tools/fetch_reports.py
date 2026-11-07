#!/usr/bin/env python3
"""Fetch Unicode technical report headers (TR 1..60) and build an inventory.

The official report page is the authority for title, version, date and status.
Only current UAX/UTS/UTR documents are retained; stabilized, superseded,
withdrawn and suspended reports are recorded but marked as excluded.
"""

from __future__ import annotations

import concurrent.futures
import json
import re
import sys
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CACHE = ROOT / "data" / "reports"
OUT = ROOT / "docs" / "unicode18" / "inventory.json"
BASE = "https://www.unicode.org/reports/tr{n}/"

EXCLUDED_TITLE_WORDS = (
    "superseded",
    "stabilized",
    "withdrawn",
    "suspended",
    "obsolete",
)

# Reports served as PDFs cannot be parsed as HTML.  Their metadata is taken
# from the official revision page listed here (verified 2026-11-05).
OVERRIDES = {
    25: {
        "category": "UTR",
        "title": "Unicode Support for Mathematics",
        "version": "Revision 16 (Draft 5)",
        "date": "2026-10-10",
        "revision": "16 (Draft 5)",
        "status": "Proposed Update",
        "url": "https://www.unicode.org/reports/tr25/tr25-16.html",
        "latest_url": "https://www.unicode.org/reports/tr25/",
        "excluded": False,
        "exclusion_reason": "",
        "note": "Official document is distributed as PDF; metadata read from tr25-16.html.",
    },
    54: {
        "category": "UTR",
        "title": "Unicode Mongolian 12.1 Baseline",
        "version": "Revision 1",
        "date": "2019-06-06",
        "revision": "1",
        "status": "Current",
        "url": "https://www.unicode.org/reports/tr54/tr54-1.html",
        "latest_url": "https://www.unicode.org/reports/tr54/",
        "excluded": False,
        "exclusion_reason": "",
        "note": "Official document is distributed as PDF; metadata read from tr54-1.html and PDF metadata.",
    },
    35: {
        "category": "UTS",
        "title": "Unicode Common Locale Data & Rules (CLDR) Part 1: Core",
        "version": "CLDR 49 (living document)",
        "date": "2026-10-21",
        "status": "Current",
        "url": "https://www.unicode.org/reports/tr35/",
        "latest_url": "https://www.unicode.org/reports/tr35/",
        "excluded": False,
        "exclusion_reason": "",
        "note": "Living CLDR specification; not synchronized to a single Unicode version.",
    },
}


def fetch(n: int) -> tuple[int, str | None, str | None]:
    cache = CACHE / f"tr{n}.html"
    if cache.exists() and cache.stat().st_size > 1000:
        return n, cache.read_text(encoding="utf-8", errors="replace"), None
    url = BASE.format(n=n)
    try:
        req = urllib.request.Request(url, headers={"User-Agent": "Unicodoid-inventory/1.0"})
        with urllib.request.urlopen(req, timeout=45) as resp:
            text = resp.read().decode("utf-8", errors="replace")
        cache.parent.mkdir(parents=True, exist_ok=True)
        cache.write_text(text, encoding="utf-8")
        return n, text, None
    except Exception as exc:  # noqa: BLE001 - report per document
        return n, None, str(exc)


def clean(fragment: str) -> str:
    fragment = re.sub(r"<!--.*?-->", " ", fragment, flags=re.S)
    fragment = re.sub(r"<[^>]+>", " ", fragment)
    fragment = (
        fragment.replace("&nbsp;", " ")
        .replace("&amp;", "&")
        .replace("&lt;", "<")
        .replace("&gt;", ">")
        .replace("&quot;", '"')
    )
    return re.sub(r"\s+", " ", fragment).strip()


def field(html: str, label: str) -> str:
    m = re.search(
        rf"<t[hd][^>]*>\s*{label}\s*</t[hd]>\s*<td[^>]*>(.*?)</td>",
        html,
        re.S | re.I,
    )
    return clean(m.group(1)) if m else ""


def field_link(html: str, label: str) -> str:
    m = re.search(
        rf"<t[hd][^>]*>\s*{label}\s*</t[hd]>\s*<td[^>]*>(.*?)</td>",
        html,
        re.S | re.I,
    )
    if not m:
        return ""
    link = re.search(r'href="([^"]+)"', m.group(1), re.I)
    return link.group(1) if link else ""


def extract(n: int, html: str) -> dict:
    html = re.sub(r"<!--.*?-->", " ", html, flags=re.S)
    title_m = re.search(r"<title>(.*?)</title>", html, re.S | re.I)
    raw_title = clean(title_m.group(1)) if title_m else ""

    category = ""
    m = re.match(r"\s*(UAX|UTS|UTR)\s*#?\s*\d+", raw_title, re.I)
    if m:
        category = m.group(1).upper()
    elif re.search(r"Unicode Standard Annex", raw_title, re.I):
        category = "UAX"
    elif re.search(r"Unicode Technical Standard", raw_title, re.I):
        category = "UTS"
    elif re.search(r"Unicode Technical Report", raw_title, re.I):
        category = "UTR"

    title = re.sub(
        r"^\s*(?:UAX|UTS|UTR)\s*#?\s*\d+\s*[:\-–—]?\s*",
        "",
        raw_title,
        flags=re.I,
    ).strip()
    title = re.sub(
        r"^\s*Unicode\s+(?:Standard Annex|Technical Standard|Technical Report)\s*#?\s*\d+\s*[:\-–—]?\s*",
        "",
        title,
        flags=re.I,
    ).strip()

    version = field(html, "Version")
    date = field(html, "Date")
    revision = field(html, "Revision")
    this_version = field_link(html, "This Version")
    previous_version = field_link(html, "Previous Version")

    status_block = ""
    m = re.search(
        r"<h[1-6][^>]*>\s*Status\s*</h[1-6]>(.*?)(?:<h[1-6]|<hr)",
        html,
        re.S | re.I,
    )
    if m:
        status_block = clean(m.group(1))[:800]

    lower_title = raw_title.lower()
    non_report = (
        not raw_title
        or lower_title.startswith("index of")
        or re.match(r"^unicode\s+\d+\.\d+(\.\d+)?$", lower_title) is not None
    )
    excluded = non_report or any(word in lower_title for word in EXCLUDED_TITLE_WORDS)
    if not excluded and status_block:
        # Only an explicit first-sentence status counts, not boilerplate.
        first = status_block[:220].lower()
        excluded = any(
            phrase in first
            for phrase in (
                "this document is superseded",
                "this document has been superseded",
                "this document is stabilized",
                "this document has been stabilized",
                "this document is withdrawn",
                "this document has been withdrawn",
                "this document is suspended",
                "this document has been suspended",
            )
        )

    if "proposed update" in status_block.lower() and "approved for publication" not in status_block.lower():
        status_kind = "Proposed Update"
    elif "approved for publication" in status_block.lower():
        status_kind = "Current"
    elif "stabilized" in status_block.lower():
        status_kind = "Stabilized"
    else:
        status_kind = "Current" if not excluded else "Excluded"

    return {
        "tr": n,
        "category": category,
        "title": title,
        "version": version,
        "date": date,
        "revision": revision,
        "status": status_kind,
        "status_text": status_block,
        "url": this_version or BASE.format(n=n),
        "latest_url": BASE.format(n=n),
        "previous_url": previous_version,
        "excluded": excluded,
        "exclusion_reason": (
            "not a current technical report"
            if non_report
            else ("status: " + ", ".join(w for w in EXCLUDED_TITLE_WORDS if w in lower_title))
            if excluded
            else ""
        ),
    }


def main() -> int:
    CACHE.mkdir(parents=True, exist_ok=True)
    records: list[dict] = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=8) as pool:
        futures = {pool.submit(fetch, n): n for n in range(1, 61)}
        for fut in concurrent.futures.as_completed(futures):
            n, html, err = fut.result()
            if err:
                records.append(
                    {
                        "tr": n,
                        "excluded": True,
                        "exclusion_reason": f"fetch failed: {err}",
                    }
                )
                print(f"tr{n}: ERROR {err}", file=sys.stderr)
                continue
            rec = extract(n, html)
            if n in OVERRIDES:
                rec.update(OVERRIDES[n])
            elif not rec.get("version") and rec.get("revision"):
                rec["version"] = f"Revision {rec['revision']}"
            records.append(rec)
            print(
                f"tr{n}: {rec['category']:3} | {rec['version']:14} | {rec['date']:10} | "
                f"{rec['status']:14} | excluded={rec['excluded']} | {rec['title'][:64]}"
            )

    records.sort(key=lambda r: r["tr"])
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(records, indent=2, ensure_ascii=False), encoding="utf-8")
    print(f"wrote {OUT} ({len(records)} records)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
