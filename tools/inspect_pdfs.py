from pathlib import Path
import re

for n in (25, 54):
    p = Path(f"data/reports/tr{n}.html")
    data = p.read_bytes()
    print("=" * 20, n, len(data))
    for key in (b"/Title", b"/Author", b"/CreationDate", b"/ModDate", b"/Subject"):
        for m in re.finditer(re.escape(key) + rb"\s*\(([^)]*)\)", data):
            raw = m.group(1)
            try:
                text = raw.decode("utf-16-be") if raw.startswith(b"\xfe\xff") else raw.decode("latin-1")
            except Exception:
                text = repr(raw)
            print(key.decode(), "=", text[:200])
            break
    # PDF may use hex strings
    for m in re.finditer(rb"/Title\s*<([0-9A-Fa-f]+)>", data):
        try:
            print("/Title hex =", bytes.fromhex(m.group(1).decode()).decode("utf-16-be", "replace")[:200])
        except Exception:
            pass
        break
