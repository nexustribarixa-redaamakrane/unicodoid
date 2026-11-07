from pathlib import Path
import re

for n in (25, 54, 35, 59):
    p = Path(f"data/reports/tr{n}.html")
    if not p.exists():
        print("=" * 20, "tr", n, "MISSING")
        continue
    t = p.read_text(encoding="utf-8", errors="replace")
    title = re.search(r"<title>(.*?)</title>", t, re.S | re.I)
    print("=" * 20, "tr", n)
    print("TITLE:", re.sub(r"\s+", " ", title.group(1))[:180] if title else None)
    for label in ("Version", "Date", "Revision", "Status"):
        m = re.search(
            rf"<t[hd][^>]*>\s*{label}\s*</t[hd]>\s*<td[^>]*>(.*?)</td>",
            t,
            re.S | re.I,
        )
        if m:
            print(label + ":", re.sub(r"<[^>]+>", " ", m.group(1))[:300].replace("\n", " "))
    m = re.search(
        r"<h[1-6][^>]*>\s*Status\s*</h[1-6]>(.*?)(?:<h[1-6]|<hr)",
        t,
        re.S | re.I,
    )
    if m:
        block = re.sub(r"<[^>]+>", " ", m.group(1))[:400].replace("\n", " ")
        print("STATUSBLOCK:", block)
