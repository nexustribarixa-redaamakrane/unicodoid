import json

path = 'docs/unicode18/inventory.json'
d = json.load(open(path, encoding='utf-8'))

assert max(r['tr'] for r in d) == 60, 'expected max tr 60'
assert not any(r['tr'] == 61 for r in d)

entry = {
    "tr": 61,
    "category": "UTS",
    "title": "Unicode Set Notation",
    "version": "Version 1 (draft 4)",
    "date": "2026-10-01",
    "revision": "2",
    "status": "Draft",
    "status_text": "This is a draft document which may be updated, replaced, or superseded by other documents at any time. Publication does not imply endorsement by the Unicode Consortium. This is not a stable document; it is inappropriate to cite this document as other than a work in progress. A Unicode Technical Standard (UTS) is an independent specification. Conformance to the Unicode Standard does not imply conformance to any UTS.",
    "url": "https://www.unicode.org/reports/tr61/tr61-2.html",
    "latest_url": "https://www.unicode.org/reports/tr61/",
    "previous_url": "https://www.unicode.org/reports/tr61/tr61-1.html",
    "excluded": False,
    "exclusion_reason": "",
    "note": "Draft Unicode Set Notation; listed under Technical Standards on the official reports index. Added at user request after the original 1..60 scope."
}

d.append(entry)
d.sort(key=lambda r: r['tr'])

json.dump(d, open(path, 'w', encoding='utf-8'), indent=2, ensure_ascii=False)
print('records:', len(d))
for r in d:
    if not r.get('excluded'):
        print(r['category'], r['tr'], '|', r['status'], '|', r['title'])
