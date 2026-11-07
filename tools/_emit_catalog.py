import json
d = json.load(open('docs/unicode18/inventory.json', encoding='utf-8'))
for r in d:
    if r.get('excluded'):
        continue
    cat = r['category']
    num = r['tr']
    title = r['title']
    url = r['url']
    st = r['status']
    print('    { UNICODOID_DOC_%s, %d, "%s", "%s", "%s" },' % (cat, num, title, url, st))
