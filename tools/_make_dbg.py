path = r"C:\Users\KARIMABENDA\Documents\unicodoid\clike\src\uts\uts10_collation.c"
src = open(path, encoding="utf-8").read()

dbg = src.replace(
    '#include "unicodoid/unicodoid_ucd.h"',
    '#include "unicodoid/unicodoid_ucd.h"\n#include <stdio.h>\n',
    1,
)

dbg = dbg.replace(
    'static void ce_push(ce_array_t* a, uint16_t p, uint16_t s, uint16_t t) {',
    'static void ce_push(ce_array_t* a, uint16_t p, uint16_t s, uint16_t t) {\n'
    '    fprintf(stderr, "push ce[%zu] = %04X.%04X.%04X\\n", a->count, p, s, t);\n',
    1,
)

dbg = dbg.replace(
    '    memset(keeps, 1, nfd->length);',
    '    memset(keeps, 1, nfd->length);\n'
    '    fprintf(stderr, "NFD:");\n'
    '    for (size_t k = 0; k < nfd->length; ++k) fprintf(stderr, " %04X", nfd->cps[k]);\n'
    '    fprintf(stderr, "\\n");',
    1,
)

dbg = dbg.replace(
    'int unicodoid_uts10_compare(',
    'size_t uts10_sortkey_test(const uint32_t* cps, size_t length, unicodoid_uts10_unit_t* out, size_t cap);\n'
    'int unicodoid_uts10_compare(',
    1,
)

open(r"C:\Users\KARIMABENDA\Documents\unicodoid\tools\_col13.c", "w", encoding="utf-8").write(dbg)
print("ok", len(dbg))
