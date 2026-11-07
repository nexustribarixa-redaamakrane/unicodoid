/* UTS #58 Link Detection and Formatting: URLs and Email Addresses.
 *
 * Unicode 18.0.0, Revision 3.  Implements the post-host part of the
 * specification: the link termination algorithm (§3.5.1), the minimal
 * escaping algorithm (§4.1) and the email local-part scan (§5.2).  URL
 * initiation (Scheme/Host/Port parsing and registrable-domain checks) is out
 * of scope, as it is in the specification itself.
 */

#include "unicodoid/uts/uts58_link_detection.h"
#include "unicodoid/unicodoid_ucd.h"

#include <stdlib.h>
#include <string.h>

/* Link_Term property values (generated tables). */
#define LT_INCLUDE 1u
#define LT_HARD    2u
#define LT_SOFT    3u
#define LT_CLOSE   4u
#define LT_OPEN    5u

/* Part numbering; -1 means "none". */
#define PART_NONE      (-1)
#define PART_PATH      0
#define PART_QUERY     1
#define PART_FRAGMENT  2
#define PART_DIRECTIVE 3

/* ------------------------------------------------------------------------- */
/* Initiator / terminator / clearStackOpen matchers                          */
/* ------------------------------------------------------------------------- */

static bool match_terminator(const uint32_t* cps, size_t n, size_t i,
                             int part, size_t* advance) {
    uint32_t c = cps[i];
    switch (part) {
        case PART_PATH:
            if (c == 0x003Fu || c == 0x0023u) { /* '?' or '#' */
                if (advance) *advance = 1;
                return true;
            }
            return false;
        case PART_QUERY:
            if (c == 0x0023u) {                 /* '#' */
                if (advance) *advance = 1;
                return true;
            }
            return false;
        case PART_FRAGMENT:
            if (i + 2 < n && cps[i] == 0x003Au && cps[i + 1] == 0x007Eu &&
                cps[i + 2] == 0x003Au) {         /* ":~:" */
                if (advance) *advance = 3;
                return true;
            }
            return false;
        default:
            return false;                        /* directive has none */
    }
}

static bool match_initiator(const uint32_t* cps, size_t n, size_t i,
                            int previous_part, size_t* advance,
                            int* out_part) {
    if (i < n && cps[i] == 0x002Fu) {            /* '/' -> path */
        *advance = 1;
        *out_part = PART_PATH;
        return true;
    }
    if (i < n && cps[i] == 0x003Fu) {            /* '?' -> query */
        *advance = 1;
        *out_part = PART_QUERY;
        return true;
    }
    if (i < n && cps[i] == 0x0023u) {            /* '#' -> fragment */
        *advance = 1;
        *out_part = PART_FRAGMENT;
        return true;
    }
    if (i + 2 < n && cps[i] == 0x003Au && cps[i + 1] == 0x007Eu &&
        cps[i + 2] == 0x003Au) {                 /* ":~:" -> directive */
        if (previous_part == PART_FRAGMENT ||
            previous_part == PART_DIRECTIVE) {
            *advance = 3;
            *out_part = PART_DIRECTIVE;
            return true;
        }
        return false;                            /* not allowed here */
    }
    return false;
}

static bool match_clear_stack_open(const uint32_t* cps, size_t n, size_t i,
                                   int part, size_t* advance) {
    uint32_t c = cps[i];
    switch (part) {
        case PART_PATH:
            if (c == 0x002Fu) {                  /* '/' */
                *advance = 1;
                return true;
            }
            return false;
        case PART_QUERY:
            if (c == 0x003Du || c == 0x0026u) {  /* '=' or '&' */
                *advance = 1;
                return true;
            }
            return false;
        case PART_DIRECTIVE:
            if (c == 0x0026u || c == 0x002Cu) {  /* '&' or ',' */
                *advance = 1;
                return true;
            }
            if (i + 2 < n && cps[i] == 0x003Au && cps[i + 1] == 0x007Eu &&
                cps[i + 2] == 0x003Au) {         /* ":~:" */
                *advance = 3;
                return true;
            }
            return false;
        default:
            return false;                        /* fragment: none */
    }
}

/* ------------------------------------------------------------------------- */
/* Termination algorithm (§3.5.1)                                            */
/* ------------------------------------------------------------------------- */

size_t unicodoid_uts58_link_detection_end(
    const uint32_t* cps, size_t length, size_t start) {
    size_t last_safe = start;
    int part = PART_NONE;
    int previous_part = PART_NONE;
    size_t i = start;
    uint32_t stack[125];
    size_t stack_len = 0;

    while (i < length) {
        size_t advance = 0;

        /* Step (a): a terminator ends the current part. */
        if (part != PART_NONE &&
            match_terminator(cps, length, i, part, &advance)) {
            previous_part = part;
            part = PART_NONE;
            /* Do not advance i; the code point is reconsidered below. */
        }

        /* Step (b): with no part active, try to start one. */
        if (part == PART_NONE) {
            int new_part = PART_NONE;
            if (match_initiator(cps, length, i, previous_part, &advance,
                                &new_part)) {
                part = new_part;
                i += advance;
                last_safe = i;
                stack_len = 0;
                continue;
            }
            return last_safe;
        }

        /* Step (c): a clearStackOpen element clears the bracket stack. */
        if (match_clear_stack_open(cps, length, i, part, &advance)) {
            i += advance;
            last_safe = i;
            stack_len = 0;
            continue;
        }

        /* Steps (d)-(i): classify cp[i] with Link_Term. */
        switch (unicodoid_ucd_link_term(cps[i])) {
            case LT_INCLUDE:
                last_safe = i + 1;
                ++i;
                break;
            case LT_SOFT:
                ++i;
                break;
            case LT_HARD:
                return last_safe;
            case LT_OPEN:
                if (stack_len == 125) return last_safe;
                stack[stack_len++] = cps[i];
                last_safe = i + 1;
                ++i;
                break;
            case LT_CLOSE: {
                uint32_t last_open = 0;
                uint32_t expected = 0;
                if (stack_len == 0) return last_safe;
                last_open = stack[--stack_len];
                if (unicodoid_ucd_link_bracket(cps[i], &expected) &&
                    expected == last_open) {
                    last_safe = i + 1;
                    ++i;
                } else {
                    return last_safe;
                }
                break;
            }
            default:
                return last_safe;
        }
    }

    return last_safe;
}

/* ------------------------------------------------------------------------- */
/* Email local-part scan (§5.2)                                              */
/* ------------------------------------------------------------------------- */

size_t unicodoid_uts58_email_local_part_start(
    const uint32_t* cps, size_t at_index) {
    if (!cps || at_index == 0) return (size_t)-1;

    /* cp[i] == '.' immediately before '@': fail. */
    if (cps[at_index - 1] == 0x002Eu) return (size_t)-1;

    size_t start = at_index;
    for (size_t i = at_index; i-- > 0;) {
        uint32_t cp = cps[i];
        if (cp == 0x002Eu) {                     /* '.' */
            if (i + 1 < at_index && cps[i + 1] == 0x002Eu) return (size_t)-1;
            continue;                            /* medial dot: keep scanning */
        }
        if (!unicodoid_ucd_link_email(cp)) {
            start = i + 1;
            break;
        }
        start = i;
    }

    /* The local part may not begin with '.'. */
    if (start < at_index && cps[start] == 0x002Eu) return (size_t)-1;
    /* The local part may not be empty. */
    if (start == at_index) return (size_t)-1;



    /* Include a leading "mailto:" if present. */
    if (start >= 7) {
        static const uint32_t MAILTO[7] = { 'm', 'a', 'i', 'l', 't', 'o', ':' };
        bool match = true;
        for (size_t k = 0; k < 7; ++k) {
            if (cps[start - 7 + k] != MAILTO[k]) { match = false; break; }
        }
        if (match) start -= 7;
    }

    return start;
}

/* ------------------------------------------------------------------------- */
/* Percent escaping helpers                                                  */
/* ------------------------------------------------------------------------- */

static size_t utf8_encode_cp(uint32_t cp, uint8_t out[4]) {
    if (cp < 0x80) { out[0] = (uint8_t)cp; return 1; }
    if (cp < 0x800) {
        out[0] = (uint8_t)(0xC0 | (cp >> 6));
        out[1] = (uint8_t)(0x80 | (cp & 0x3F));
        return 2;
    }
    if (cp < 0x10000) {
        out[0] = (uint8_t)(0xE0 | (cp >> 12));
        out[1] = (uint8_t)(0x80 | ((cp >> 6) & 0x3F));
        out[2] = (uint8_t)(0x80 | (cp & 0x3F));
        return 3;
    }
    out[0] = (uint8_t)(0xF0 | (cp >> 18));
    out[1] = (uint8_t)(0x80 | ((cp >> 12) & 0x3F));
    out[2] = (uint8_t)(0x80 | ((cp >> 6) & 0x3F));
    out[3] = (uint8_t)(0x80 | (cp & 0x3F));
    return 4;
}

static char hex_digit(uint8_t v) {
    return (char)(v < 10 ? ('0' + v) : ('A' + (v - 10)));
}

static size_t percent_escape_cp(uint32_t cp, uint8_t out[12]) {
    uint8_t utf8[4];
    size_t n = utf8_encode_cp(cp, utf8);
    size_t off = 0;
    for (size_t i = 0; i < n; ++i) {
        out[off++] = (uint8_t)'%';
        out[off++] = (uint8_t)hex_digit((uint8_t)(utf8[i] >> 4));
        out[off++] = (uint8_t)hex_digit((uint8_t)(utf8[i] & 0x0F));
    }
    return off;
}

typedef struct byte_buf {
    uint8_t* data;
    size_t len;
    size_t cap;
    bool oom;
} byte_buf_t;

static void buf_put(byte_buf_t* b, const uint8_t* bytes, size_t n) {
    if (b->oom) return;
    if (b->len + n > b->cap) {
        size_t cap = b->cap ? b->cap : 64;
        while (cap < b->len + n) cap *= 2;
        uint8_t* p = (uint8_t*)realloc(b->data, cap);
        if (!p) { b->oom = true; return; }
        b->data = p;
        b->cap = cap;
    }
    memcpy(b->data + b->len, bytes, n);
    b->len += n;
}

static void buf_cp(byte_buf_t* b, uint32_t cp) {
    uint8_t utf8[4];
    size_t n = utf8_encode_cp(cp, utf8);
    buf_put(b, utf8, n);
}

static void buf_percent(byte_buf_t* b, uint32_t cp) {
    uint8_t enc[12];
    size_t n = percent_escape_cp(cp, enc);
    buf_put(b, enc, n);
}

static const char* part_initiator(unicodoid_url_part_t part, size_t* len) {
    switch (part) {
        case UNICODOID_URL_PART_PATH:             *len = 1; return "/";
        case UNICODOID_URL_PART_QUERY:            *len = 1; return "?";
        case UNICODOID_URL_PART_FRAGMENT:         *len = 1; return "#";
        case UNICODOID_URL_PART_FRAGMENT_DIRECTIVE: *len = 3; return ":~:";
        default:                                  *len = 0; return "";
    }
}

static int part_number(unicodoid_url_part_t part) {
    switch (part) {
        case UNICODOID_URL_PART_PATH:              return PART_PATH;
        case UNICODOID_URL_PART_QUERY:             return PART_QUERY;
        case UNICODOID_URL_PART_FRAGMENT:          return PART_FRAGMENT;
        case UNICODOID_URL_PART_FRAGMENT_DIRECTIVE: return PART_DIRECTIVE;
        default:                                   return PART_PATH;
    }
}

/* ------------------------------------------------------------------------- */
/* Minimal Escaping Algorithm (§4.1)                                         */
/* ------------------------------------------------------------------------- */

size_t unicodoid_uts58_minimal_escape_part(
    unicodoid_url_part_t part,
    const uint32_t* cps, size_t length,
    bool is_last,
    uint8_t* out, size_t cap) {
    byte_buf_t buf = { 0 };
    size_t init_len = 0;
    const char* init = part_initiator(part, &init_len);
    buf_put(&buf, (const uint8_t*)init, init_len);

    size_t copied_already = 0;
    int p = part_number(part);
    uint32_t stack[125];
    size_t stack_len = 0;

    for (size_t i = 0; i < length; ++i) {
        size_t advance = 0;
        bool is_terminator = match_terminator(cps, length, i, p, &advance);
        uint8_t lt = is_terminator ? LT_HARD : unicodoid_ucd_link_term(cps[i]);

        if (match_clear_stack_open(cps, length, i, p, &advance)) {
            stack_len = 0;
        }

        switch (lt) {
            case LT_INCLUDE:
                for (size_t k = copied_already; k < i; ++k) buf_cp(&buf, cps[k]);
                buf_cp(&buf, cps[i]);
                copied_already = i + 1;
                break;
            case LT_HARD:
                for (size_t k = copied_already; k < i; ++k) buf_cp(&buf, cps[k]);
                buf_percent(&buf, cps[i]);
                copied_already = i + 1;
                break;
            case LT_SOFT:
                break;
            case LT_OPEN:
                if (stack_len == 125) {
                    for (size_t k = copied_already; k < i; ++k) buf_cp(&buf, cps[k]);
                    buf_percent(&buf, cps[i]);
                    copied_already = i + 1;
                } else {
                    stack[stack_len++] = cps[i];
                    for (size_t k = copied_already; k < i; ++k) buf_cp(&buf, cps[k]);
                    buf_cp(&buf, cps[i]);
                    copied_already = i + 1;
                }
                break;
            case LT_CLOSE: {
                uint32_t last_open = 0;
                uint32_t expected = 0;
                bool matched = false;
                if (stack_len > 0) {
                    last_open = stack[stack_len - 1];
                    matched = unicodoid_ucd_link_bracket(cps[i], &expected) &&
                              expected == last_open;
                }
                if (matched) {
                    --stack_len;
                    for (size_t k = copied_already; k < i; ++k) buf_cp(&buf, cps[k]);
                    buf_cp(&buf, cps[i]);
                    copied_already = i + 1;
                } else {
                    for (size_t k = copied_already; k < i; ++k) buf_cp(&buf, cps[k]);
                    buf_percent(&buf, cps[i]);
                    copied_already = i + 1;
                }
                break;
            }
            default:
                break;
        }
    }

    if (!is_last) {
        for (size_t k = copied_already; k < length; ++k) buf_cp(&buf, cps[k]);
    } else if (copied_already < length) {
        for (size_t k = copied_already; k < length - 1; ++k) buf_cp(&buf, cps[k]);
        buf_percent(&buf, cps[length - 1]);
    }

    size_t written = buf.len;
    if (buf.oom) {
        free(buf.data);
        return 0;
    }
    if (out && cap >= written) {
        memcpy(out, buf.data, written);
    }
    free(buf.data);
    return written;
}
