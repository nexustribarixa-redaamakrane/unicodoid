using System;
using System.Collections.Generic;

namespace Unicodoid;

/// <summary>URL parts for minimal escaping (UTS58 §4.1).</summary>
public enum UrlPart
{
    Path,
    Query,
    Fragment,
    FragmentDirective
}

/// <summary>
/// UTS #58 Link Detection and Formatting: URLs and Email Addresses
/// (Unicode 18.0.0, Revision 3).
///
/// Implements the code point machinery that operates after the host has been
/// parsed:
/// <list type="bullet">
/// <item>Link_Term / Link_Bracket / Link_Email property lookups,</item>
/// <item>the link termination algorithm (UTS58 §3.5.1),</item>
/// <item>the URL minimal escaping algorithm (UTS58 §4.1),</item>
/// <item>the email local-part backward scan (UTS58 §5.2).</item>
/// </list>
///
/// URL <em>initiation</em> (finding the Scheme, Host and optional Port,
/// including registrable-domain and TLD checks) is explicitly out of scope, as
/// it is in the specification itself (UTS58 §1.4 and §3.2).
/// </summary>
public static class Uts58
{
    private const int LtInclude = 1;
    private const int LtHard = 2;
    private const int LtSoft = 3;
    private const int LtClose = 4;
    private const int LtOpen = 5;

    private const int PartNone = -1;
    private const int PartPath = 0;
    private const int PartQuery = 1;
    private const int PartFragment = 2;
    private const int PartDirective = 3;

    private static bool IsColonTildeColon(ReadOnlySpan<uint> cps, int i) =>
        i + 2 < cps.Length && cps[i] == 0x003A && cps[i + 1] == 0x007E && cps[i + 2] == 0x003A;

    private static bool MatchTerminator(ReadOnlySpan<uint> cps, int i, int part, out int advance)
    {
        advance = 0;
        uint c = cps[i];
        switch (part)
        {
            case PartPath:
                if (c is 0x003F or 0x0023)
                {
                    advance = 1;
                    return true;
                }
                return false;
            case PartQuery:
                if (c == 0x0023)
                {
                    advance = 1;
                    return true;
                }
                return false;
            case PartFragment:
                if (IsColonTildeColon(cps, i))
                {
                    advance = 3;
                    return true;
                }
                return false;
            default:
                return false;
        }
    }

    private static bool MatchInitiator(
        ReadOnlySpan<uint> cps, int i, int previousPart, out int advance, out int newPart)
    {
        advance = 0;
        newPart = PartNone;
        if (i < cps.Length && cps[i] == 0x002F)
        {
            advance = 1;
            newPart = PartPath;
            return true;
        }
        if (i < cps.Length && cps[i] == 0x003F)
        {
            advance = 1;
            newPart = PartQuery;
            return true;
        }
        if (i < cps.Length && cps[i] == 0x0023)
        {
            advance = 1;
            newPart = PartFragment;
            return true;
        }
        if (IsColonTildeColon(cps, i))
        {
            if (previousPart is PartFragment or PartDirective)
            {
                advance = 3;
                newPart = PartDirective;
                return true;
            }
        }
        return false;
    }

    private static bool MatchClearStackOpen(ReadOnlySpan<uint> cps, int i, int part, out int advance)
    {
        advance = 0;
        uint c = cps[i];
        switch (part)
        {
            case PartPath:
                if (c == 0x002F)
                {
                    advance = 1;
                    return true;
                }
                return false;
            case PartQuery:
                if (c is 0x003D or 0x0026)
                {
                    advance = 1;
                    return true;
                }
                return false;
            case PartDirective:
                if (c is 0x0026 or 0x002C)
                {
                    advance = 1;
                    return true;
                }
                if (IsColonTildeColon(cps, i))
                {
                    advance = 3;
                    return true;
                }
                return false;
            default:
                return false;
        }
    }

    /// <summary>
    /// Link detection termination algorithm (UTS58 §3.5.1).  <paramref name="start"/>
    /// is the index immediately after the host, at the beginning of the Path,
    /// Query or Fragment, if any.  Returns the offset one past the end of the
    /// final URL part, or <paramref name="start"/> when no part is found.
    /// </summary>
    public static int LinkDetectionEnd(ReadOnlySpan<uint> cps, int start)
    {
        int lastSafe = start;
        int part = PartNone;
        int previousPart = PartNone;
        int i = start;
        var stack = new List<uint>(125);

        while (i < cps.Length)
        {
            // Step (a): a terminator ends the current part.
            if (part != PartNone && MatchTerminator(cps, i, part, out _))
            {
                previousPart = part;
                part = PartNone;
                // Do not advance i; the code point is reconsidered below.
            }

            // Step (b): with no part active, try to start one.
            if (part == PartNone)
            {
                if (MatchInitiator(cps, i, previousPart, out int advance, out int newPart))
                {
                    part = newPart;
                    i += advance;
                    lastSafe = i;
                    stack.Clear();
                    continue;
                }
                return lastSafe;
            }

            // Step (c): a clearStackOpen element clears the bracket stack.
            if (MatchClearStackOpen(cps, i, part, out int clearAdvance))
            {
                i += clearAdvance;
                lastSafe = i;
                stack.Clear();
                continue;
            }

            // Steps (d)-(i): classify the code point with Link_Term.
            switch (Ucd.LinkTerm(cps[i]))
            {
                case LtInclude:
                    lastSafe = i + 1;
                    ++i;
                    break;
                case LtSoft:
                    ++i;
                    break;
                case LtHard:
                    return lastSafe;
                case LtOpen:
                    if (stack.Count == 125) return lastSafe;
                    stack.Add(cps[i]);
                    lastSafe = i + 1;
                    ++i;
                    break;
                case LtClose:
                {
                    if (stack.Count == 0) return lastSafe;
                    uint lastOpen = stack[^1];
                    stack.RemoveAt(stack.Count - 1);
                    if (Ucd.TryLinkBracket(cps[i], out uint expected) && expected == lastOpen)
                    {
                        lastSafe = i + 1;
                        ++i;
                    }
                    else
                    {
                        return lastSafe;
                    }
                    break;
                }
                default:
                    return lastSafe;
            }
        }

        return lastSafe;
    }

    /// <summary>
    /// Email local-part detection (UTS58 §5.2).  Scans backwards from the
    /// <c>@</c> at <paramref name="atIndex"/> and returns the offset of the
    /// beginning of the local part, optionally including a <c>mailto:</c>
    /// prefix.  Returns -1 when there is no valid local part.
    /// </summary>
    public static int EmailLocalPartStart(ReadOnlySpan<uint> cps, int atIndex)
    {
        if (atIndex <= 0 || atIndex > cps.Length) return -1;

        // A '.' immediately before '@' fails.
        if (cps[atIndex - 1] == 0x002E) return -1;

        int start = atIndex;
        for (int i = atIndex - 1; i >= 0; --i)
        {
            uint cp = cps[i];
            if (cp == 0x002E)
            {
                if (i + 1 < atIndex && cps[i + 1] == 0x002E) return -1;
                continue; // medial dot: keep scanning
            }
            if (!Ucd.LinkEmail(cp))
            {
                start = i + 1;
                break;
            }
            start = i;
        }

        // The local part may not begin with '.' or be empty.
        if (start < atIndex && cps[start] == 0x002E) return -1;
        if (start == atIndex) return -1;

        // Include a leading "mailto:" if present.
        ReadOnlySpan<uint> mailto = ['m', 'a', 'i', 'l', 't', 'o', ':'];
        if (start >= 7 && cps.Slice(start - 7, 7).SequenceEqual(mailto)) start -= 7;

        return start;
    }

    private static int Utf8Encode(uint cp, Span<byte> output)
    {
        if (cp < 0x80)
        {
            output[0] = (byte)cp;
            return 1;
        }
        if (cp < 0x800)
        {
            output[0] = (byte)(0xC0 | (cp >> 6));
            output[1] = (byte)(0x80 | (cp & 0x3F));
            return 2;
        }
        if (cp < 0x10000)
        {
            output[0] = (byte)(0xE0 | (cp >> 12));
            output[1] = (byte)(0x80 | ((cp >> 6) & 0x3F));
            output[2] = (byte)(0x80 | (cp & 0x3F));
            return 3;
        }
        output[0] = (byte)(0xF0 | (cp >> 18));
        output[1] = (byte)(0x80 | ((cp >> 12) & 0x3F));
        output[2] = (byte)(0x80 | ((cp >> 6) & 0x3F));
        output[3] = (byte)(0x80 | (cp & 0x3F));
        return 4;
    }

    private static void PushCodePoint(List<byte> output, uint cp)
    {
        Span<byte> buffer = stackalloc byte[4];
        int n = Utf8Encode(cp, buffer);
        for (int i = 0; i < n; ++i) output.Add(buffer[i]);
    }

    private static void PushPercent(List<byte> output, uint cp)
    {
        Span<byte> buffer = stackalloc byte[4];
        int n = Utf8Encode(cp, buffer);
        const string hex = "0123456789ABCDEF";
        for (int i = 0; i < n; ++i)
        {
            output.Add((byte)'%');
            output.Add((byte)hex[buffer[i] >> 4]);
            output.Add((byte)hex[buffer[i] & 0x0F]);
        }
    }

    private static int PartNumber(UrlPart part) => part switch
    {
        UrlPart.Path => PartPath,
        UrlPart.Query => PartQuery,
        UrlPart.Fragment => PartFragment,
        UrlPart.FragmentDirective => PartDirective,
        _ => PartPath
    };

    private static ReadOnlySpan<byte> PartInitiator(UrlPart part) => part switch
    {
        UrlPart.Path => "/"u8,
        UrlPart.Query => "?"u8,
        UrlPart.Fragment => "#"u8,
        UrlPart.FragmentDirective => ":~:"u8,
        _ => ReadOnlySpan<byte>.Empty
    };

    /// <summary>
    /// URL Minimal Escaping Algorithm (UTS58 §4.1).  <paramref name="cps"/> are
    /// the decoded code points of the part <em>without</em> the initiator (which
    /// the algorithm adds).  When <paramref name="isLast"/> is true a final Soft
    /// code point of the whole URL is percent-escaped.
    /// </summary>
    public static byte[] MinimalEscapePart(UrlPart part, ReadOnlySpan<uint> cps, bool isLast)
    {
        var output = new List<byte>();
        output.AddRange(PartInitiator(part).ToArray());

        int copiedAlready = 0;
        int p = PartNumber(part);
        var stack = new List<uint>(125);

        for (int i = 0; i < cps.Length; ++i)
        {
            bool isTerminator = MatchTerminator(cps, i, p, out _);
            int term = isTerminator ? LtHard : Ucd.LinkTerm(cps[i]);

            if (MatchClearStackOpen(cps, i, p, out _)) stack.Clear();

            switch (term)
            {
                case LtInclude:
                    for (int k = copiedAlready; k < i; ++k) PushCodePoint(output, cps[k]);
                    PushCodePoint(output, cps[i]);
                    copiedAlready = i + 1;
                    break;
                case LtHard:
                    for (int k = copiedAlready; k < i; ++k) PushCodePoint(output, cps[k]);
                    PushPercent(output, cps[i]);
                    copiedAlready = i + 1;
                    break;
                case LtSoft:
                    break;
                case LtOpen:
                    if (stack.Count == 125)
                    {
                        for (int k = copiedAlready; k < i; ++k) PushCodePoint(output, cps[k]);
                        PushPercent(output, cps[i]);
                    }
                    else
                    {
                        stack.Add(cps[i]);
                        for (int k = copiedAlready; k < i; ++k) PushCodePoint(output, cps[k]);
                        PushCodePoint(output, cps[i]);
                    }
                    copiedAlready = i + 1;
                    break;
                case LtClose:
                {
                    bool matched = false;
                    if (stack.Count > 0)
                    {
                        uint lastOpen = stack[^1];
                        matched = Ucd.TryLinkBracket(cps[i], out uint expected) && expected == lastOpen;
                    }
                    if (matched)
                    {
                        stack.RemoveAt(stack.Count - 1);
                        for (int k = copiedAlready; k < i; ++k) PushCodePoint(output, cps[k]);
                        PushCodePoint(output, cps[i]);
                    }
                    else
                    {
                        for (int k = copiedAlready; k < i; ++k) PushCodePoint(output, cps[k]);
                        PushPercent(output, cps[i]);
                    }
                    copiedAlready = i + 1;
                    break;
                }
                default:
                    break;
            }
        }

        if (!isLast)
        {
            for (int k = copiedAlready; k < cps.Length; ++k) PushCodePoint(output, cps[k]);
        }
        else if (copiedAlready < cps.Length)
        {
            for (int k = copiedAlready; k < cps.Length - 1; ++k) PushCodePoint(output, cps[k]);
            PushPercent(output, cps[^1]);
        }

        return output.ToArray();
    }
}
