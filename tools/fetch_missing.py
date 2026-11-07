import re
import urllib.request
from pathlib import Path

BASE = "https://www.unicode.org/Public/18.0.0/"
DEST = Path("data/ucd/18.0.0")


def get(url: str) -> bytes:
    req = urllib.request.Request(url, headers={"User-Agent": "Unicodoid-data/1.0"})
    with urllib.request.urlopen(req, timeout=180) as resp:
        return resp.read()


# Retry emoji-data with a long timeout.
rel = "ucd/emoji/emoji-data.txt"
payload = get(BASE + rel)
(DEST / rel).parent.mkdir(parents=True, exist_ok=True)
(DEST / rel).write_bytes(payload)
print(rel, len(payload))

# Discover the IVD directory contents.
for url in (
    BASE + "ivd/",
    "https://www.unicode.org/ivd/",
    "https://www.unicode.org/ivd/data/",
):
    try:
        html = get(url).decode("utf-8", "replace")
        links = sorted(set(re.findall(r'href="([^"]+)"', html)))
        print("URL", url, "links", [x for x in links if "ivd" in x.lower() or x.lower().endswith(".txt")][:30])
    except Exception as exc:  # noqa: BLE001
        print("URL", url, "ERR", exc)
