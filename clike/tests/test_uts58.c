// Official Unicode 18.0.0 conformance tests for UTS #58 (Link Detection and
// Formatting) and constructed unit tests for the same algorithms.
//
// URL initiation is out of scope of the specification itself; the link
// detection harness pins the termination algorithm's behaviour given the
// position after the host, while the minimal escaping harness checks full
// conformance of URL Path/Query/Fragment formatting against the official
// test data (UTS58-C2).

#include "unicodoid/uts/uts58_link_detection.h"
#include "unicodoid/unicodoid_ucd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CP_MAX 512

static int g_checks = 0;
static int g_failures = 0;

#define CHECK(cond)                                                      \
    do {                                                                 \
        ++g_checks;                                                      \
        if (!(cond)) {                                                   \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
            ++g_failures;                                                \
        }                                                                \
    } while (0)

/* ------------------------------------------------------------------ */
/* UTF-8 helpers                                                       */
/* ------------------------------------------------------------------ */

static size_t utf8_encode(const uint32_t* cps, size_t count, uint8_t* out, size_t cap) {
    size_t off = 0;
    for (size_t i = 0; i < count; ++i) {
        uint32_t cp = cps[i];
        if (cp < 0x80) {
            if (off >= cap) return (size_t)-1;
            out[off++] = (uint8_t)cp;
        } else if (cp < 0x800) {
            if (off + 2 > cap) return (size_t)-1;
            out[off++] = (uint8_t)(0xC0 | (cp >> 6));
            out[off++] = (uint8_t)(0x80 | (cp & 0x3F));
        } else if (cp < 0x10000) {
            if (off + 3 > cap) return (size_t)-1;
            out[off++] = (uint8_t)(0xE0 | (cp >> 12));
            out[off++] = (uint8_t)(0x80 | ((cp >> 6) & 0x3F));
            out[off++] = (uint8_t)(0x80 | (cp & 0x3F));
        } else {
            if (off + 4 > cap) return (size_t)-1;
            out[off++] = (uint8_t)(0xF0 | (cp >> 18));
            out[off++] = (uint8_t)(0x80 | ((cp >> 12) & 0x3F));
            out[off++] = (uint8_t)(0x80 | ((cp >> 6) & 0x3F));
            out[off++] = (uint8_t)(0x80 | (cp & 0x3F));
        }
    }
    return off;
}

static size_t utf8_decode(const uint8_t* bytes, size_t length, uint32_t* cps, size_t cap) {
    size_t n = 0;
    size_t i = 0;
    while (i < length) {
        uint8_t b0 = bytes[i];
        uint32_t cp;
        size_t adv;
        if (b0 < 0x80) { cp = b0; adv = 1; }
        else if (b0 < 0xE0) { cp = (uint32_t)((b0 & 0x1F) << 6) | (uint32_t)(bytes[i+1] & 0x3F); adv = 2; }
        else if (b0 < 0xF0) { cp = (uint32_t)((b0 & 0x0F) << 12) | (uint32_t)((bytes[i+1] & 0x3F) << 6) | (uint32_t)(bytes[i+2] & 0x3F); adv = 3; }
        else { cp = (uint32_t)((b0 & 0x07) << 18) | (uint32_t)((bytes[i+1] & 0x3F) << 12) | (uint32_t)((bytes[i+2] & 0x3F) << 6) | (uint32_t)(bytes[i+3] & 0x3F); adv = 4; }
        if (n >= cap) return (size_t)-1;
        cps[n++] = cp;
        i += adv;
    }
    return n;
}

/* ------------------------------------------------------------------ */
/* Link detection harness (official LinkDetectionTest.txt)              */
/* ------------------------------------------------------------------ */

static const uint32_t MARK_OPEN  = 0x2E20; /* ⸠ */
static const uint32_t MARK_CLOSE = 0x2E21; /* ⸡ */

static size_t find_initiator(const uint32_t* cps, size_t n, size_t from, size_t to) {
    for (size_t i = from; i < to; ++i) {
        if (cps[i] == 0x2F || cps[i] == 0x3F || cps[i] == 0x23) return i;
        if (i + 2 < n && cps[i] == 0x003A && cps[i+1] == 0x007E && cps[i+2] == 0x003A) return i;
    }
    return (size_t)-1;
}

static void run_detection_file(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "cannot open %s\n", path);
        ++g_failures;
        return;
    }

    char line[4096];
    size_t cases = 0;
    size_t links = 0;
    size_t checked = 0;
    size_t skipped_bare = 0;
    size_t skipped_email = 0;
    size_t failures = 0;

    while (fgets(line, sizeof line, f)) {
        if (line[0] == '#') continue;
        size_t line_len = strlen(line);
        while (line_len > 0 && (line[line_len-1] == '\n' || line[line_len-1] == '\r')) {
            line[--line_len] = '\0';
        }
        if (line_len == 0) continue;
        ++cases;

        uint32_t cps[CP_MAX];
        size_t n = utf8_decode((const uint8_t*)line, line_len, cps, CP_MAX);
        if (n == (size_t)-1) { fprintf(stderr, "decode failed\n"); ++failures; continue; }

        size_t i = 0;
        while (i < n) {
            if (cps[i] != MARK_OPEN) { ++i; continue; }
            size_t open = i;
            size_t close = (size_t)-1;
            for (size_t j = open + 1; j < n; ++j) {
                if (cps[j] == MARK_CLOSE) { close = j; break; }
            }
            if (close == (size_t)-1) { fprintf(stderr, "unpaired mark\n"); ++failures; break; }
            ++links;

            /* Find the first URL part initiator inside the marked span. */
            size_t init = find_initiator(cps, n, open + 1, close);
            if (init == (size_t)-1) {
                /* Bare domain: initiation out of scope.  However, links with
                 * an '@' must have a valid local part per UTS58 §5.2. */
                size_t at = (size_t)-1;
                for (size_t j = open + 1; j < close; ++j) {
                    if (cps[j] == 0x0040) { at = j; break; }
                }
                if (at != (size_t)-1) {
                    size_t local = unicodoid_uts58_email_local_part_start(cps, at);
                    ++skipped_email;
                    if (local != (size_t)-1 && local != open + 1) {
                        ++failures;
                        if (failures <= 10) {
                            fprintf(stderr, "email local part mismatch: span=[%zu,%zu) local=%zu\n",
                                    open + 1, close, local);
                        }
                    }
                } else {
                    ++skipped_bare; /* bare domain: initiation out of scope */
                }
                i = close + 1;
                continue;
            }

            size_t end = unicodoid_uts58_link_detection_end(cps, n, init);
            ++checked;
            if (end != close) {
                ++failures;
                if (failures <= 10) {
                    fprintf(stderr, "detection mismatch: span=[%zu,%zu) init=%zu end=%zu\n",
                            open + 1, close, init, end);
                }
            }
            i = close + 1;
        }
    }
    fclose(f);

    fprintf(stderr, "LinkDetectionTest.txt: %zu cases, %zu marked links, %zu checked, %zu bare (skipped), %zu email, %zu failures\n",
            cases, links, checked, skipped_bare, skipped_email, failures);
    CHECK(failures == 0);
}

/* ------------------------------------------------------------------ */
/* Link formatting harness (official LinkFormattingTest.txt)             */
/* ------------------------------------------------------------------ */

#define SYM_S 0x1D47A /* 𝑺 */
#define SYM_H 0x1D46F /* 𝑯 */
#define SYM_P 0x1D477 /* 𝑷 */
#define SYM_Q 0x1D478 /* 𝑸 */
#define SYM_V 0x1D47D /* 𝑽 */
#define SYM_F 0x1D46D /* 𝑭 */
#define SYM_D 0x1D46B /* 𝑫 */

static bool is_hex_digit(uint8_t b) {
    return (b >= '0' && b <= '9') || (b >= 'a' && b <= 'f') || (b >= 'A' && b <= 'F');
}

static void append_str(uint8_t* out, size_t* off, const char* s) {
    size_t n = strlen(s);
    memcpy(out + *off, s, n);
    *off += n;
}

/* Escape a literal '%' when followed by two hex digits; returns the escaped
 * length (always 3). */
static bool pct_followed_by_hex(const uint8_t* p, size_t n, size_t i) {
    return i + 2 < n && is_hex_digit(p[i + 1]) && is_hex_digit(p[i + 2]);
}

/* Interior escaping for path segments (UTS58 §4.1): literal '/', '?', '#'
 * are escaped, and a literal '%' followed by two hex digits as %25. */
static size_t interior_escape_path(const char* input, uint8_t* out) {
    size_t off = 0;
    const uint8_t* p = (const uint8_t*)input;
    size_t n = strlen(input);
    for (size_t i = 0; i < n; ++i) {
        uint8_t b = p[i];
        switch (b) {
            case '/': append_str(out, &off, "%2F"); break;
            case '?': append_str(out, &off, "%3F"); break;
            case '#': append_str(out, &off, "%23"); break;
            case '%':
                if (pct_followed_by_hex(p, n, i)) append_str(out, &off, "%25");
                else out[off++] = '%';
                break;
            default:
                out[off++] = b;
                break;
        }
    }
    return off;
}

/* Interior escaping for query keys and values (UTS58 §4.1): literal '+',
 * '#', '=', '&' are escaped, and a literal '%' followed by two hex digits. */
static size_t interior_escape_query(const char* input, uint8_t* out) {
    size_t off = 0;
    const uint8_t* p = (const uint8_t*)input;
    size_t n = strlen(input);
    for (size_t i = 0; i < n; ++i) {
        uint8_t b = p[i];
        switch (b) {
            case '+': append_str(out, &off, "%2B"); break;
            case '#': append_str(out, &off, "%23"); break;
            case '=': append_str(out, &off, "%3D"); break;
            case '&': append_str(out, &off, "%26"); break;
            case '%':
                if (pct_followed_by_hex(p, n, i)) append_str(out, &off, "%25");
                else out[off++] = '%';
                break;
            default:
                out[off++] = b;
                break;
        }
    }
    return off;
}

/* Interior escaping for the fragment: the first character of a literal
 * ":~:" is escaped, and a literal '%' followed by two hex digits. */
static size_t interior_escape_fragment(const char* input, uint8_t* out) {
    size_t off = 0;
    const uint8_t* p = (const uint8_t*)input;
    size_t n = strlen(input);
    for (size_t i = 0; i < n; ++i) {
        if (p[i] == ':' && i + 2 < n && p[i + 1] == '~' && p[i + 2] == ':') {
            append_str(out, &off, "%3A~:");
            i += 2;
            continue;
        }
        if (p[i] == '%' && pct_followed_by_hex(p, n, i)) {
            append_str(out, &off, "%25");
            continue;
        }
        out[off++] = p[i];
    }
    return off;
}

/* Interior escaping for fragment directives: literal '&', ',' and the
 * first character of a literal ":~:" are escaped; a literal '%' followed
 * by two hex digits as well. */
static size_t interior_escape_directive(const char* input, uint8_t* out) {
    size_t off = 0;
    const uint8_t* p = (const uint8_t*)input;
    size_t n = strlen(input);
    for (size_t i = 0; i < n; ++i) {
        if (p[i] == ':' && i + 2 < n && p[i + 1] == '~' && p[i + 2] == ':') {
            append_str(out, &off, "%3A~:");
            i += 2;
            continue;
        }
        switch (p[i]) {
            case '&': append_str(out, &off, "%26"); break;
            case ',': append_str(out, &off, "%2C"); break;
            case '%':
                if (pct_followed_by_hex(p, n, i)) append_str(out, &off, "%25");
                else out[off++] = '%';
                break;
            default:
                out[off++] = p[i];
                break;
        }
    }
    return off;
}

typedef struct fmt_token {
    uint32_t symbol;
    char value[512];
} fmt_token_t;

static bool is_structure_symbol(uint32_t cp) {
    return cp == SYM_S || cp == SYM_H || cp == SYM_P || cp == SYM_Q ||
           cp == SYM_V || cp == SYM_F || cp == SYM_D;
}

static size_t utf8_char_len(uint8_t b0) {
    if (b0 < 0x80) return 1;
    if (b0 < 0xE0) return 2;
    if (b0 < 0xF0) return 3;
    return 4;
}

static uint32_t utf8_read(const char* p, size_t* len) {
    const uint8_t* q = (const uint8_t*)p;
    if (q[0] < 0x80) { *len = 1; return q[0]; }
    if (q[0] < 0xE0) { *len = 2; return (uint32_t)((q[0] & 0x1F) << 6) | (uint32_t)(q[1] & 0x3F); }
    if (q[0] < 0xF0) { *len = 3; return (uint32_t)((q[0] & 0x0F) << 12) | (uint32_t)((q[1] & 0x3F) << 6) | (uint32_t)(q[2] & 0x3F); }
    *len = 4;
    return (uint32_t)((q[0] & 0x07) << 18) | (uint32_t)((q[1] & 0x3F) << 12) |
           (uint32_t)((q[2] & 0x3F) << 6) | (uint32_t)(q[3] & 0x3F);
}

/* Find the start of the next " SYMBOL=" token or the '}' terminator. */
static const char* find_next_token(const char* p) {
    while (*p) {
        if (*p == '}') return p;
        if (*p == ' ') {
            size_t len = 0;
            uint32_t sym = utf8_read(p + 1, &len);
            if (is_structure_symbol(sym) && p[1 + len] == '=') {
                return p;
            }
        }
        ++p;
    }
    return p;
}

static bool parse_structure_line(const char* line, fmt_token_t* out, size_t* out_count) {
    const char* p = strchr(line, '{');
    if (!p) return false;
    p += 1;

    size_t count = 0;
    while (*p) {
        size_t sym_len = 0;
        uint32_t sym = utf8_read(p, &sym_len);
        if (!is_structure_symbol(sym) || p[sym_len] != '=') {
            ++p;
            continue;
        }
        const char* vstart = p + sym_len + 1;
        const char* vend = find_next_token(vstart);
        size_t vlen = (size_t)(vend - vstart);
        while (vlen > 0 && vstart[vlen - 1] == ' ') --vlen;

        fmt_token_t* tok = &out[count++];
        tok->symbol = sym;
        if (vlen >= sizeof tok->value) vlen = sizeof tok->value - 1;
        memcpy(tok->value, vstart, vlen);
        tok->value[vlen] = '\0';
        p = vend;
        while (*p == ' ') ++p;
    }
    *out_count = count;
    return count > 0;
}

typedef struct fmt_parts {
    char scheme[32];
    char host[256];
    char path_segments[16][256];
    size_t path_segment_count;
    char query_keys[16][256];
    char query_values[16][256]; /* empty string when no 𝑽 follows the 𝑸 */
    size_t query_count;
    char fragment[512];
    size_t fragment_present;
    char directives[8][256];
    size_t directive_count;
} fmt_parts_t;

static size_t escape_part_cps(unicodoid_url_part_t part, const char* input,
                             bool is_last, uint8_t* out, size_t cap) {
    uint32_t cps[CP_MAX];
    size_t n = utf8_decode((const uint8_t*)input, strlen(input), cps, CP_MAX);
    if (n == (size_t)-1) return 0;
    return unicodoid_uts58_minimal_escape_part(part, cps, n, is_last, out, cap);
}

static bool reconstruct_expected(const fmt_parts_t* parts, uint8_t* out, size_t out_cap) {
    uint8_t tmp[2048];
    size_t off = 0;

    #define PUT(bytes, len)                                        \
        do {                                                        \
            if (off + (len) > out_cap) return false;                \
            memcpy(out + off, (bytes), (len));                      \
            off += (len);                                           \
        } while (0)

    PUT(parts->scheme, strlen(parts->scheme));
    PUT(parts->host, strlen(parts->host));

    /* Path: segments are escaped individually; separators stay raw. */
    bool has_path = parts->path_segment_count > 0;
    bool has_query = parts->query_count > 0;
    bool has_fragment = parts->fragment_present > 0;
    size_t directive_count = parts->directive_count;

    if (has_path) {
        uint8_t joined[CP_MAX * 4] = { 0 };
        size_t j = 0;
        for (size_t s = 0; s < parts->path_segment_count; ++s) {
            if (s) joined[j++] = '/';
            uint8_t esc[CP_MAX * 4] = { 0 };
            size_t elen = interior_escape_path(parts->path_segments[s], esc);
            memcpy(joined + j, esc, elen);
            j += elen;
        }
        joined[j] = '\0';
        bool last = !has_query && !has_fragment && directive_count == 0;
        size_t n = escape_part_cps(UNICODOID_URL_PART_PATH, (const char*)joined, last, tmp, sizeof tmp);
        PUT(tmp, n);
    }

    if (has_query) {
        uint8_t joined[CP_MAX * 4] = { 0 };
        size_t j = 0;
        for (size_t s = 0; s < parts->query_count; ++s) {
            if (s) joined[j++] = '&';
            uint8_t key[CP_MAX * 4] = { 0 };
            size_t klen = interior_escape_query(parts->query_keys[s], key);
            memcpy(joined + j, key, klen);
            j += klen;
            if (parts->query_values[s][0] != '\0') {
                joined[j++] = '=';
                uint8_t value[CP_MAX * 4] = { 0 };
                size_t vlen = interior_escape_query(parts->query_values[s], value);
                memcpy(joined + j, value, vlen);
                j += vlen;
            }
        }
        joined[j] = '\0';
        bool last = !has_fragment && directive_count == 0;
        size_t n = escape_part_cps(UNICODOID_URL_PART_QUERY, (const char*)joined, last, tmp, sizeof tmp);
        PUT(tmp, n);
    }

    if (has_fragment) {
        uint8_t esc[CP_MAX * 4] = { 0 };
        size_t elen = interior_escape_fragment(parts->fragment, esc);
        esc[elen] = '\0';
        bool last = directive_count == 0;
        size_t n = escape_part_cps(UNICODOID_URL_PART_FRAGMENT, (const char*)esc, last, tmp, sizeof tmp);
        PUT(tmp, n);
    }

    for (size_t s = 0; s < directive_count; ++s) {
        uint8_t esc[CP_MAX * 4] = { 0 };
        size_t elen = interior_escape_directive(parts->directives[s], esc);
        esc[elen] = '\0';
        bool last = (s + 1 == directive_count);
        size_t n = escape_part_cps(UNICODOID_URL_PART_FRAGMENT_DIRECTIVE, (const char*)esc, last, tmp, sizeof tmp);
        PUT(tmp, n);
    }

    out[off] = '\0';
    return true;
    #undef PUT
}

static void run_formatting_file(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "cannot open %s\n", path);
        ++g_failures;
        return;
    }

    char line[8192];
    char source_line[8192] = "";
    char expected_line[8192] = "";
    fmt_parts_t pending;
    bool have_structure = false;
    size_t cases = 0;
    size_t checked = 0;
    size_t failures = 0;

    memset(&pending, 0, sizeof pending);

    while (fgets(line, sizeof line, f)) {
        size_t line_len = strlen(line);
        while (line_len > 0 && (line[line_len-1] == '\n' || line[line_len-1] == '\r')) {
            line[--line_len] = '\0';
        }
        if (line_len == 0) continue;

        if (line[0] == '#') {
            if (line[2] == '{' || line[1] == '{') {
                have_structure = false;
                fmt_token_t tokens[64];
                size_t token_count = 0;
                if (parse_structure_line(line, tokens, &token_count)) {
                    memset(&pending, 0, sizeof pending);
                    size_t pending_q = (size_t)-1;
                    for (size_t t = 0; t < token_count; ++t) {
                        fmt_token_t* tok = &tokens[t];
                        switch (tok->symbol) {
                            case SYM_S: snprintf(pending.scheme, sizeof pending.scheme, "%s", tok->value); break;
                            case SYM_H: snprintf(pending.host, sizeof pending.host, "%s", tok->value); break;
                            case SYM_P:
                                if (pending.path_segment_count < 16) {
                                    snprintf(pending.path_segments[pending.path_segment_count],
                                             sizeof pending.path_segments[0], "%s", tok->value);
                                    ++pending.path_segment_count;
                                }
                                break;
                            case SYM_Q:
                                if (pending.query_count < 16) {
                                    snprintf(pending.query_keys[pending.query_count],
                                             sizeof pending.query_keys[0], "%s", tok->value);
                                    pending.query_values[pending.query_count][0] = '\0';
                                    pending_q = pending.query_count;
                                    ++pending.query_count;
                                }
                                break;
                            case SYM_V:
                                if (pending_q != (size_t)-1) {
                                    snprintf(pending.query_values[pending_q],
                                             sizeof pending.query_values[0], "%s", tok->value);
                                }
                                break;
                            case SYM_F:
                                snprintf(pending.fragment, sizeof pending.fragment, "%s", tok->value);
                                pending.fragment_present = 1;
                                break;
                            case SYM_D:
                                if (pending.directive_count < 8) {
                                    snprintf(pending.directives[pending.directive_count],
                                             sizeof pending.directives[0], "%s", tok->value);
                                    ++pending.directive_count;
                                }
                                break;
                            default:
                                break;
                        }
                    }
                    have_structure = true;
                }
            }
            continue;
        }

        /* Non-comment line: source then expected. */
        if (source_line[0] == '\0') {
            snprintf(source_line, sizeof source_line, "%s", line);
            continue;
        }
        snprintf(expected_line, sizeof expected_line, "%s", line);

        if (have_structure) {
            ++cases;
            uint8_t reconstructed[4096];
            if (!reconstruct_expected(&pending, reconstructed, sizeof reconstructed)) {
                fprintf(stderr, "reconstruct failed\n");
                ++failures;
            } else {
                ++checked;
                if (strcmp((const char*)reconstructed, expected_line) != 0) {
                    ++failures;
                    if (failures <= 10) {
                        fprintf(stderr, "formatting mismatch:\n  expected=%s\n  actual  =%s\n",
                                expected_line, (const char*)reconstructed);
                    }
                }
            }
        }

        source_line[0] = '\0';
        expected_line[0] = '\0';
        have_structure = false;
    }
    fclose(f);

    fprintf(stderr, "LinkFormattingTest.txt: %zu cases, %zu checked, %zu failures\n",
            cases, checked, failures);
    CHECK(failures == 0);
}

/* ------------------------------------------------------------------ */
/* Unit tests                                                          */
/* ------------------------------------------------------------------ */

static void unit_email(void) {
    const char* cases[][2] = {
        { "contact abcd@example.com", "abcd" },
        { "contact x.abcd@example.com", "x.abcd" },
        { "contact アルベルト.アルベルト@example.com", "アルベルト.アルベルト" },
        { "contact a@b.c", "a" },
        { NULL, NULL },
    };
    for (size_t i = 0; cases[i][0] != NULL; ++i) {
        uint32_t cps[CP_MAX];
        size_t n = utf8_decode((const uint8_t*)cases[i][0], strlen(cases[i][0]), cps, CP_MAX);
        size_t at = (size_t)-1;
        for (size_t j = 0; j < n; ++j) { if (cps[j] == '@') { at = j; break; } }
        CHECK(at != (size_t)-1);
        if (at == (size_t)-1) continue;

        size_t start = unicodoid_uts58_email_local_part_start(cps, at);
        const char* expected = cases[i][1];
        /* The expected local part is a suffix of the line ending before '@'. */
        size_t expected_len = strlen(expected);
        size_t prefix_len = 0;
        /* find the byte offset where the expected local part begins */
        const char* line = cases[i][0];
        size_t line_len = strlen(line);
        /* locate at_index in bytes */
        size_t at_byte = 0;
        {
            size_t cp_i = 0, byte_i = 0;
            while (byte_i < line_len) {
                if (cp_i == at) { at_byte = byte_i; break; }
                uint8_t b0 = (uint8_t)line[byte_i];
                size_t adv = b0 < 0x80 ? 1 : b0 < 0xE0 ? 2 : b0 < 0xF0 ? 3 : 4;
                byte_i += adv;
                ++cp_i;
            }
        }
        if (start == (size_t)-1) {
            CHECK(0);
            continue;
        }
        /* convert 'start' (code point offset) back to byte offset */
        size_t byte_start = 0, cp_i = 0;
        while (cp_i < start) {
            uint8_t b0 = (uint8_t)line[byte_start];
            byte_start += b0 < 0x80 ? 1 : b0 < 0xE0 ? 2 : b0 < 0xF0 ? 3 : 4;
            ++cp_i;
        }
        size_t len = at_byte - byte_start;
        CHECK(len == expected_len && memcmp(line + byte_start, expected, expected_len) == 0);
        (void)line_len;
    }

    /* Failure cases: double dots, leading dot, trailing dot, empty local. */
    const char* bad[] = {
        "contact @example.com",
        "contact john..doe@example.com",
        "contact .john.doe@example.com",
        "contact john.@example.com",
    };
    for (size_t i = 0; i < sizeof bad / sizeof bad[0]; ++i) {
        uint32_t cps[CP_MAX];
        size_t n = utf8_decode((const uint8_t*)bad[i], strlen(bad[i]), cps, CP_MAX);
        size_t at = (size_t)-1;
        for (size_t j = 0; j < n; ++j) { if (cps[j] == '@') { at = j; break; } }
        CHECK(at != (size_t)-1);
        CHECK(unicodoid_uts58_email_local_part_start(cps, at) == (size_t)-1);
    }
}

static void unit_detection(void) {
    /* Specification examples: soft vs hard termination.  start = index of the
     * first URL part initiator, i.e. the position after the host. */
    struct { const char* text; size_t start; size_t expected_end; } cases[] = {
        { "go to example.com/αβγ now", 17, 21 },
        { "go to example.com/αβγ.now", 17, 25 },
        { "go to example.com?q=a b", 17, 21 },
        { "example.com#frag ment", 11, 16 },
        { "example.com/a(b)c) x", 11, 17 },
        { "example.com/a(b)c) y", 11, 17 },
    };
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; ++i) {
        uint32_t cps[CP_MAX];
        size_t n = utf8_decode((const uint8_t*)cases[i].text, strlen(cases[i].text), cps, CP_MAX);
        size_t end = unicodoid_uts58_link_detection_end(cps, n, cases[i].start);
        size_t expected = cases[i].expected_end;
        if (end != expected) {
            fprintf(stderr, "detection case %zu: start=%zu end=%zu expected=%zu\n",
                    i, cases[i].start, end, expected);
            ++g_failures;
        }
        ++g_checks;
    }

    /* Soft run followed by end-of-string terminates the link at the last
     * non-soft code point. */
    {
        const char* text = "example.com/αβγ.";
        uint32_t cps[CP_MAX];
        size_t n = utf8_decode((const uint8_t*)text, strlen(text), cps, CP_MAX);
        size_t end = unicodoid_uts58_link_detection_end(cps, n, 11);
        /* trailing '.' is Soft; lastSafe stays after γ (offset 15) */
        CHECK(end == 15);
    }

    /* Close without a matching Open terminates the link. */
    {
        const char* text = "example.com/a)b";
        uint32_t cps[CP_MAX];
        size_t n = utf8_decode((const uint8_t*)text, strlen(text), cps, CP_MAX);
        size_t end = unicodoid_uts58_link_detection_end(cps, n, 11);
        CHECK(end == 13); /* 'a' included; ')' closes empty stack -> stop */
    }
}

static void unit_minimal_escape(void) {
    /* Path ending in a soft character must escape it. */
    const char* input = "δ.εζ.";
    uint8_t out[64];
    uint32_t cps[CP_MAX];
    size_t n = utf8_decode((const uint8_t*)input, strlen(input), cps, CP_MAX);
    size_t len = unicodoid_uts58_minimal_escape_part(
        UNICODOID_URL_PART_PATH, cps, n, true, out, sizeof out);
    CHECK(len == strlen("/δ.εζ%2E") && memcmp(out, "/δ.εζ%2E", len) == 0);

    /* Query with reserved characters escaped even when unmatched. */
    input = "q=a)b";
    n = utf8_decode((const uint8_t*)input, strlen(input), cps, CP_MAX);
    len = unicodoid_uts58_minimal_escape_part(
        UNICODOID_URL_PART_QUERY, cps, n, true, out, sizeof out);
    fprintf(stderr, "minimal escape of \"%s\" -> %.*s\n", input, (int)len, out);
}

int main(void) {
    setvbuf(stderr, NULL, _IONBF, 0);

    const char* data_dir = UNICODOID_DATA_DIR;
    char path[1024];

    unit_email();
    unit_detection();
    unit_minimal_escape();

    snprintf(path, sizeof path, "%s/linkification/LinkDetectionTest.txt", data_dir);
    run_detection_file(path);

    snprintf(path, sizeof path, "%s/linkification/LinkFormattingTest.txt", data_dir);
    run_formatting_file(path);

    fprintf(stderr, "uts58: %d checks, %d failures\n", g_checks, g_failures);
    if (g_failures == 0) {
        printf("test_uts58: PASS\n");
        return 0;
    }
    fprintf(stderr, "test_uts58: FAIL (%d failures)\n", g_failures);
    return 1;
}
