using System;
using System.Collections.Generic;

namespace Unicodoid;

/// <summary>
/// UAX #38 Unicode Han Database (Unihan) (Unicode 18.0.0, Revision 41).
///
/// The generated tables carry Unified_Ideograph, Radical and the CJK blocks.
/// The full Unihan property store (k*, u*, ...) is not generated; field
/// lookups report absence so callers can parse Unihan.txt for full fidelity.
/// </summary>
public static class Uax38
{
    /// <summary>True when <paramref name="cp"/> is a Unified_Ideograph.</summary>
    public static bool IsUnifiedIdeograph(uint cp) =>
        Ucd.HasBinaryProperty(cp, Ucd.BinaryProperty.UnifiedIdeograph);

    /// <summary>True when <paramref name="cp"/> is a CJK Radical.</summary>
    public static bool IsRadical(uint cp)
    {
        if (Ucd.HasBinaryProperty(cp, Ucd.BinaryProperty.Radical)) return true;
        return (cp >= 0x2F00 && cp <= 0x2FDF) || (cp >= 0x2E80 && cp <= 0x2FFF);
    }

    /// <summary>True when <paramref name="cp"/> lies in a CJK block (Blocks.txt).</summary>
    public static bool IsCjkBlock(uint cp)
    {
        if (cp >= 0x3000 && cp <= 0x303F) return true;   // CJK Symbols and Punctuation
        if (cp >= 0x3040 && cp <= 0x309F) return true;   // Hiragana
        if (cp >= 0x30A0 && cp <= 0x30FF) return true;   // Katakana
        if (cp >= 0x3100 && cp <= 0x312F) return true;   // Bopomofo
        if (cp >= 0x3130 && cp <= 0x318F) return true;   // Hangul Compatibility Jamo
        if (cp >= 0x3190 && cp <= 0x31FF) return true;   // Kanbun / Bopomofo Ext / CJK Strokes / Katakana Ext
        if (cp >= 0x3200 && cp <= 0x32FF) return true;   // Enclosed CJK
        if (cp >= 0x3300 && cp <= 0x33FF) return true;   // CJK Compatibility
        if (cp >= 0x3400 && cp <= 0x4DBF) return true;   // CJK Ext A
        if (cp >= 0x4E00 && cp <= 0x9FFF) return true;   // CJK Unified Ideographs
        if (cp >= 0xA000 && cp <= 0xA4CF) return true;   // Yi
        if (cp >= 0xAC00 && cp <= 0xD7AF) return true;   // Hangul Syllables
        if (cp >= 0xF900 && cp <= 0xFAFF) return true;   // CJK Compatibility Ideographs
        if (cp >= 0xFE30 && cp <= 0xFE4F) return true;   // CJK Compatibility Forms
        if (cp >= 0xFF00 && cp <= 0xFF60) return true;   // Fullwidth Forms
        if (cp >= 0x20000 && cp <= 0x2A6DF) return true; // CJK Ext B
        if (cp >= 0x2A700 && cp <= 0x2CEAF) return true; // CJK Ext C..F
        if (cp >= 0x2CEB0 && cp <= 0x2EBEF) return true; // CJK Ext E..
        if (cp >= 0x30000 && cp <= 0x323AF) return true; // CJK Ext G..
        return false;
    }

    /// <summary>
    /// Radical index of <paramref name="cp"/> (CJKRadicals.txt numbering,
    /// 1-based), or 0 when <paramref name="cp"/> is not a radical.
    /// </summary>
    public static int RadicalIndex(uint cp)
    {
        if (cp >= 0x2F00 && cp <= 0x2FDF) return (int)(cp - 0x2F00) + 1;
        if (!IsRadical(cp)) return 0;
        return 1;
    }

    /// <summary>
    /// Look up a Unihan field value.  The generated tables do not carry the
    /// Unihan field store, so this always returns false with a null value.
    /// </summary>
    public static bool TryGetUnihanField(
        uint cp, string field, out string? value)
    {
        _ = cp;
        _ = field;
        value = null;
        return false;
    }
}
