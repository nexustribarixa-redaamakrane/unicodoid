#ifndef ConvertUTF_H
#define ConvertUTF_H

/* ---------------------------------------------------------------------
 * ConvertUTF.h -- Conversions between UTF-32, UTF-16, and UTF-8.
 *
 * Unicodoid port of the discontinued W3C/Unicode ConvertUTF library,
 * updated to the Unicode 18.0.0 data set.  The public API is unchanged
 * from the original (ConvertUTF8toUTF16, ConvertUTF16toUTF8, ...), so
 * existing callers continue to compile.  The well-formedness rules now
 * enforce the Unicode 18.0.0 maximum code point U+10FFFF.
 *
 * Copyright note (original library):
 *   Copyright 2001-2004 Unicode, Inc.  This source code is provided as
 *   is by Unicode, Inc.  No claims are made as to fitness for any
 *   particular purpose.  The license permits free use, copying, and
 *   redistribution provided this notice remains attached.
 * --------------------------------------------------------------------- */

#include <stddef.h>

/* ---------------------------------------------------------------------
    The following 4 definitions are compiler-specific.
    The C standard does not guarantee that wchar_t has at least
    16 bits, so wchar_t is no less portable than unsigned short!
    All should be unsigned values to avoid sign extension during
    bit mask & shift operations.
------------------------------------------------------------------------ */

typedef unsigned long  UTF32;   /* at least 32 bits */
typedef unsigned short UTF16;   /* at least 16 bits */
typedef unsigned char  UTF8;    /* typically 8 bits */
typedef unsigned char  Boolean; /* 0 or 1 */

/* Some fundamental constants */
#define UNI_REPLACEMENT_CHAR (UTF32)0x0000FFFD
#define UNI_MAX_BMP (UTF32)0x0000FFFF
#define UNI_MAX_UTF16 (UTF32)0x0010FFFF
#define UNI_MAX_UTF32 (UTF32)0x7FFFFFFF
#define UNI_MAX_LEGAL_UTF32 (UTF32)0x0010FFFF

typedef enum {
    conversionOK,   /* conversion successful */
    sourceExhausted, /* partial character in source, but hit end */
    targetExhausted, /* insuff. room in target for conversion */
    sourceIllegal  /* source sequence is illegal/malformed */
} ConversionResult;

typedef enum {
    strictConversion = 0,
    lenientConversion
} ConversionFlags;

/* Unicode version targeted by this port.  The conversion code itself is
 * version-independent; the maximum legal code point U+10FFFF has been
 * stable since Unicode 3.1, but the port is validated against the
 * Unicode 18.0.0 test data. */
#define UNICODOID_CONVERTUTF_UNICODE_VERSION_MAJOR 18
#define UNICODOID_CONVERTUTF_UNICODE_VERSION_MINOR 0
#define UNICODOID_CONVERTUTF_UNICODE_VERSION_PATCH 0
#define UNICODOID_CONVERTUTF_UNICODE_VERSION_STRING "18.0.0"

/* This is for C++ and does no harm in C */
#ifdef __cplusplus
extern "C" {
#endif

ConversionResult ConvertUTF8toUTF16 (
        const UTF8** sourceStart, const UTF8* sourceEnd,
        UTF16** targetStart, UTF16* targetEnd, ConversionFlags flags);

ConversionResult ConvertUTF16toUTF8 (
        const UTF16** sourceStart, const UTF16* sourceEnd,
        UTF8** targetStart, UTF8* targetEnd, ConversionFlags flags);

ConversionResult ConvertUTF8toUTF32 (
        const UTF8** sourceStart, const UTF8* sourceEnd,
        UTF32** targetStart, UTF32* targetEnd, ConversionFlags flags);

ConversionResult ConvertUTF32toUTF8 (
        const UTF32** sourceStart, const UTF32* sourceEnd,
        UTF8** targetStart, UTF8* targetEnd, ConversionFlags flags);

ConversionResult ConvertUTF16toUTF32 (
        const UTF16** sourceStart, const UTF16* sourceEnd,
        UTF32** targetStart, UTF32* targetEnd, ConversionFlags flags);

ConversionResult ConvertUTF32toUTF16 (
        const UTF32** sourceStart, const UTF32* sourceEnd,
        UTF16** targetStart, UTF16* targetEnd, ConversionFlags flags);

Boolean isLegalUTF8Sequence(const UTF8 *source, const UTF8 *sourceEnd);

#ifdef __cplusplus
}
#endif

#endif /* ConvertUTF_H */
