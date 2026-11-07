using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.CompilerServices;
using System.Text;
using Unicodoid;
using Unicodoid.Systems;

namespace Unicodoid.Tests;

internal static partial class Program
{
    private static int s_checks;
    private static int s_failures;

    private static void Check(
        bool condition,
        [CallerArgumentExpression("condition")] string expression = "",
        [CallerLineNumber] int line = 0)
    {
        ++s_checks;
        if (condition) return;
        Console.Error.WriteLine($"FAIL Program.cs:{line}: {expression}");
        ++s_failures;
    }

    private static void CheckStatus(
        UnicodoidStatus actual,
        UnicodoidStatus expected,
        [CallerArgumentExpression("actual")] string expression = "",
        [CallerLineNumber] int line = 0)
    {
        ++s_checks;
        if (actual == expected) return;
        Console.Error.WriteLine(
            $"FAIL Program.cs:{line}: {expression} returned {actual} ({actual.ToMessage()}), expected {expected}");
        ++s_failures;
    }

    private static void CheckBytes(
        ReadOnlySpan<byte> actual,
        ReadOnlySpan<byte> expected,
        [CallerArgumentExpression("actual")] string expression = "",
        [CallerLineNumber] int line = 0)
    {
        ++s_checks;
        if (actual.SequenceEqual(expected)) return;
        Console.Error.WriteLine(
            $"FAIL Program.cs:{line}: {expression} bytes [{Convert.ToHexString(actual)}], " +
            $"expected [{Convert.ToHexString(expected)}]");
        ++s_failures;
    }

    private static void TestStatusAndRegistry()
    {
        Check(UnicodoidStatus.Ok.ToMessage() == "ok");
        Check(UnicodoidStatus.State.ToMessage() == "invalid codec state");
        Check(UnicodoidStatus.Overflow.ToMessage() == "numeric overflow");

        EncodingSystemRegistry.Cleanup();
        CheckStatus(EncodingSystemRegistry.Init(), UnicodoidStatus.Ok, "Init");
        CheckStatus(EncodingSystemRegistry.Init(), UnicodoidStatus.Ok, "Init idempotent");
        Check(EncodingSystemRegistry.Count == 8, "registry count");

        IEncodingSystem? utf8 = EncodingSystemRegistry.FindById("org.unicode.utf8");
        Check(ReferenceEquals(utf8, Utf8System.Instance), "find utf8");
        Check(ReferenceEquals(EncodingSystemRegistry.FindByName("UTF-8"), utf8), "find by name");
        Check(EncodingSystemRegistry.FindById("org.example.missing") is null, "missing system");
        Check(EncodingSystemRegistry.GetAt(0) is not null, "get at zero");
        Check(EncodingSystemRegistry.GetAt(EncodingSystemRegistry.Count) is null, "get out of range");

        CheckStatus(EncodingSystemRegistry.Register(utf8!), UnicodoidStatus.AlreadyExists, "duplicate register");
        CheckStatus(EncodingSystemRegistry.Unregister("org.unicode.utf8"), UnicodoidStatus.Ok, "unregister");
        Check(EncodingSystemRegistry.FindById("org.unicode.utf8") is null, "unregistered");
        CheckStatus(EncodingSystemRegistry.Unregister("org.unicode.utf8"), UnicodoidStatus.NotFound, "unregister missing");
        EncodingSystemRegistry.Cleanup();
    }

    private static void TestUtf8()
    {
        IEncodingSystem sys = Utf8System.Instance;
        var codec = new Codec(sys);

        Span<byte> buffer = stackalloc byte[8];
        CheckStatus(codec.EncodeChar(0x00E9, buffer, out int written), UnicodoidStatus.Ok, "encode e-acute");
        Check(written == 2 && buffer[0] == 0xC3 && buffer[1] == 0xA9, "e-acute bytes");

        CheckStatus(codec.DecodeChar(buffer[..written], out CharacterValue ch, out int read), UnicodoidStatus.Ok, "decode e-acute");
        Check(ch.Value == 0x00E9 && read == 2, "e-acute decoded");

        CheckStatus(codec.EncodeChar(0x1F600, buffer, out written), UnicodoidStatus.Ok, "encode emoji");
        Check(written == 4 && buffer[0] == 0xF0 && buffer[1] == 0x9F && buffer[2] == 0x98 && buffer[3] == 0x80,
            "emoji bytes");
        CheckStatus(codec.DecodeChar(buffer[..written], out ch, out read), UnicodoidStatus.Ok, "decode emoji");
        Check(ch.Value == 0x1F600 && read == 4, "emoji decoded");

        CheckStatus(codec.DecodeChar(new byte[] { 0xC0, 0x80 }, out _, out _),
            UnicodoidStatus.InvalidCodeUnit, "overlong");
        CheckStatus(codec.DecodeChar(new byte[] { 0xE2, 0x82 }, out _, out _),
            UnicodoidStatus.IncompleteSequence, "truncated");
        CheckStatus(codec.DecodeChar(new byte[] { 0xED, 0xA0, 0x80 }, out _, out _),
            UnicodoidStatus.InvalidCodeUnit, "surrogate");

        CheckStatus(codec.Validate(new byte[] { 0x41, 0xFF, 0x42 }, out int invalidOffset),
            UnicodoidStatus.InvalidCodeUnit, "validate bad mix");
        Check(invalidOffset == 1, "invalid offset");

        CharacterValue[] chars = { 0x41, 0x00E9, 0x1F600 };
        CheckStatus(codec.EncodeAll(chars, out byte[] encoded), UnicodoidStatus.Ok, "encode all");
        Check(encoded.Length == 7, "encoded length");

        CheckStatus(codec.DecodeAll(encoded, out CharacterValue[] decoded), UnicodoidStatus.Ok, "decode all");
        Check(decoded.Length == 3 && decoded[0].Value == 0x41 && decoded[1].Value == 0x00E9 && decoded[2].Value == 0x1F600,
            "decoded values");
    }

    private static void TestAsciiAndConlang()
    {
        IEncodingSystem ascii = AsciiSystem.Instance;
        Check(ascii.HasUnicodeProfile, "ascii has profile");
        Check(!ascii.IsStateful, "ascii stateless");

        var codec = new Codec(ascii);
        Span<byte> buffer = stackalloc byte[8];
        CheckStatus(codec.EncodeChar('A', buffer, out int written), UnicodoidStatus.Ok, "ascii A");
        Check(written == 1 && buffer[0] == 'A', "ascii A bytes");
        CheckStatus(codec.EncodeChar(0x80, buffer, out _), UnicodoidStatus.UnmappableChar, "ascii high");
        CheckStatus(codec.DecodeChar(new byte[] { 0x80 }, out _, out _), UnicodoidStatus.InvalidCodeUnit, "ascii decode high");

        IEncodingSystem conlang = KupoConlangSystem.Instance;
        Check(!conlang.HasUnicodeProfile, "conlang no profile");
        Check(conlang.UnicodeProfile is null, "conlang profile null");

        var conlangCodec = new Codec(conlang);
        CheckStatus(conlangCodec.EncodeChar(0x3FFF, buffer, out written), UnicodoidStatus.Ok, "conlang 3FFF");
        Check(written == 2 && buffer[0] == 0xBF && buffer[1] == 0xFF, "conlang bytes");
        CheckStatus(conlangCodec.DecodeChar(buffer[..written], out CharacterValue ch, out int read), UnicodoidStatus.Ok, "conlang decode");
        Check(ch.Value == 0x3FFF && read == 2, "conlang value");
        CheckStatus(conlangCodec.EncodeChar(0x4000, buffer, out _), UnicodoidStatus.UnmappableChar, "conlang overflow");

        CheckStatus(UnicodeProperties.TryGetGeneralCategory(conlang, 'A', out _),
            UnicodoidStatus.Unsupported, "conlang category unsupported");
    }

    private static void TestStrings()
    {
        IEncodingSystem sys = Utf8System.Instance;
        TextString str = TextString.FromChars(sys, new CharacterValue[] { 'H', 'i' });
        Check(str.Length == 2 && str.Bytes[0] == 'H' && str.Bytes[1] == 'i', "string Hi");
        CheckStatus(str.TryGetCharLength(out int charLength), UnicodoidStatus.Ok, "char length");
        Check(charLength == 2, "char length value");
        CheckStatus(str.TryGetCharAt(1, out CharacterValue ch), UnicodoidStatus.Ok, "get char at 1");
        Check(ch.Value == 'i', "char at 1 value");
        CheckStatus(str.TryGetCharAt(2, out _), UnicodoidStatus.NotFound, "get char out of range");

        CheckStatus(str.TryAppendChar(0x1F600), UnicodoidStatus.Ok, "append emoji");
        CheckStatus(str.TryGetCharLength(out charLength), UnicodoidStatus.Ok, "char length after append");
        Check(charLength == 3, "char length after append value");

        TextString clone = str.Clone();
        Check(str.ContentEquals(clone), "string clone equal");
        Check(str.CompareTo(clone) == 0, "string compare equal");

        TextString withZero = TextString.FromChars(sys, new CharacterValue[] { 'A', 0x00, 'B' });
        Check(withZero.Length == 3, "embedded zero length");
        CheckStatus(withZero.TryGetCharLength(out charLength), UnicodoidStatus.Ok, "embedded zero char length");
        Check(charLength == 3, "embedded zero char count");
        CheckStatus(withZero.TryGetCharAt(1, out ch), UnicodoidStatus.Ok, "embedded zero get");
        Check(ch.Value == 0, "embedded zero value");

        CheckStatus(TextString.TryFromBytes(sys, new byte[] { 0x41, 0xC0, 0x80 }, out _),
            UnicodoidStatus.InvalidCodeUnit, "bad byte string");
    }

    private static UnicodoidStatus ConlangBridge(
        IEncodingSystem sourceSystem,
        CharacterValue sourceChar,
        IEncodingSystem destinationSystem,
        out CharacterValue destinationChar)
    {
        destinationChar = CharacterValue.Invalid;
        if (sourceChar.Value > 0x10FFFF) return UnicodoidStatus.UnmappableChar;
        destinationChar = sourceChar;
        return UnicodoidStatus.Ok;
    }

    private static void TestTranscoding()
    {
        IEncodingSystem utf8 = Utf8System.Instance;
        IEncodingSystem utf16 = Utf16LeSystem.Instance;
        IEncodingSystem ascii = AsciiSystem.Instance;
        IEncodingSystem conlang = KupoConlangSystem.Instance;

        byte[] text = { (byte)'h', 0xC3, 0xA9, (byte)'l', (byte)'l', (byte)'o' }; // héllo
        TextString src = TextString.FromBytes(utf8, text);

        var config = new TranscodeConfig();
        CheckStatus(Transcoding.TryString(src, utf16, out TextString? mid, config), UnicodoidStatus.Ok, "utf8 to utf16");
        CheckStatus(Transcoding.TryString(mid!, utf8, out TextString? roundtrip, config), UnicodoidStatus.Ok, "utf16 to utf8");
        CheckBytes(roundtrip!.Bytes, src.Bytes, "utf16 roundtrip");

        CheckStatus(Transcoding.TryString(src, utf8, out TextString? same, config), UnicodoidStatus.Ok, "same system");
        CheckBytes(same!.Bytes, src.Bytes, "same system bytes");

        TextString asciiStr = TextString.FromBytes(ascii, new byte[] { (byte)'H', (byte)'i' });
        CheckStatus(Transcoding.TryString(asciiStr, utf8, out TextString? wide, config), UnicodoidStatus.Ok, "ascii to utf8");
        CheckBytes(wide!.Bytes, new byte[] { (byte)'H', (byte)'i' }, "ascii to utf8 bytes");

        TextString conlangStr = TextString.FromChars(conlang, new CharacterValue[] { 'A' });
        CheckStatus(Transcoding.TryString(conlangStr, utf8, out _, config),
            UnicodoidStatus.UnmappableChar, "no implicit identity");

        config.Bridge = ConlangBridge;
        CheckStatus(Transcoding.TryString(conlangStr, utf8, out TextString? bridged, config), UnicodoidStatus.Ok, "bridged");
        Check(bridged!.Length == 1 && bridged.Bytes[0] == 'A', "bridged bytes");

        config = new TranscodeConfig { Policy = TranscodePolicy.Replace };
        TextString accented = TextString.FromBytes(utf8, new byte[] { 0xC3, 0xA9 });
        CheckStatus(Transcoding.TryString(accented, ascii, out TextString? replaced, config), UnicodoidStatus.Ok, "replace policy");
        Check(replaced!.Length == 1 && replaced.Bytes[0] == '?', "replacement byte");

        CheckStatus(Transcoding.Bytes(utf8, text, utf16, Span<byte>.Empty, out int required, null),
            UnicodoidStatus.BufferTooSmall, "required size");
        Check(required > 0, "required size positive");
    }

    private static void TestCatalog()
    {
        Check(UnicodeCatalog.Count > 10, "catalog count");
        Check(UnicodeCatalog.TryAt(0, out UnicodeSpecRef first), "catalog first");
        Check(!string.IsNullOrEmpty(first.Title) && !string.IsNullOrEmpty(first.Url), "catalog first fields");

        Check(Ucd.Version == "18.0.0", "UCD version is 18.0.0");
        Utf8System utf8ForVersion = Utf8System.Instance;
        var utf8UnicodeVersion = utf8ForVersion.UnicodeProfile.UnicodeVersion;
        Check(utf8UnicodeVersion.Major == 18 && utf8UnicodeVersion.Minor == 0 && utf8UnicodeVersion.Build == 0,
              "utf8 Unicode version is 18.0.0");

        Check(UnicodeCatalog.TryGet(UnicodeDocCategory.Uax, 15, out UnicodeSpecRef normalization), "uax 15");
        Check(normalization.Category == UnicodeDocCategory.Uax && normalization.Title.Contains("Normalization"), "uax 15 title");

        Check(UnicodeCatalog.TryGet(UnicodeDocCategory.Uax, 31, out UnicodeSpecRef identifiers), "uax 31");
        Check(identifiers.Title.Contains("Identifiers"), "uax 31 title");

        Check(UnicodeCatalog.TryGet(UnicodeDocCategory.Utn, 1, out UnicodeSpecRef indic), "utn 1");
        Check(indic.Title.Contains("Indic Language Collation"), "utn 1 title");

        Check(!UnicodeCatalog.TryGet(UnicodeDocCategory.Uax, 999, out _), "missing spec");
        Check(!UnicodeCatalog.TryAt(UnicodeCatalog.Count, out _), "spec out of range");
    }

    private static void TestScalarProperties()
    {
        Check(UnicodeProperties.GeneralCategory('A') == UnicodeCategory.UppercaseLetter, "A category");
        Check(UnicodeProperties.GeneralCategory('a') == UnicodeCategory.LowercaseLetter, "a category");
        Check(UnicodeProperties.GeneralCategory('1') == UnicodeCategory.DecimalNumber, "1 category");
        Check(UnicodeProperties.GeneralCategory(' ') == UnicodeCategory.SpaceSeparator, "space category");
        Check(UnicodeProperties.GeneralCategory(0x0301) == UnicodeCategory.Mark, "acute category");
        Check(UnicodeProperties.GeneralCategory(0x000A) == UnicodeCategory.Control, "LF category");

        Check(UnicodeProperties.IsAlphabetic('A'), "A alphabetic");
        Check(!UnicodeProperties.IsAlphabetic('1'), "1 not alphabetic");
        Check(UnicodeProperties.IsNumeric('9'), "9 numeric");
        Check(UnicodeProperties.IsWhitespace('\t'), "tab whitespace");
        Check(UnicodeProperties.IsWhitespace(' '), "space whitespace");
        Check(UnicodeProperties.IsControl('\n'), "LF control");
        Check(UnicodeProperties.IsPunctuation('!'), "! punctuation");
        Check(UnicodeProperties.IsMark(0x0301), "acute mark");

        Check(UnicodeProperties.CombiningClass(0x0301) == 230, "acute ccc");
        Check(UnicodeProperties.CombiningClass('a') == 0, "a ccc");

        Check(UnicodeProperties.ToUppercase('a') == 'A', "uppercase a");
        Check(UnicodeProperties.ToLowercase('A') == 'a', "lowercase A");
        Check(UnicodeProperties.GetEastAsianWidth('A') == EastAsianWidth.Narrow, "A width");
        Check(UnicodeProperties.GetEastAsianWidth(0xAC00) == EastAsianWidth.Wide, "Hangul width");

        IEncodingSystem utf8 = Utf8System.Instance;
        CheckStatus(UnicodeProperties.TryGetGeneralCategory(utf8, 'A', out UnicodeCategory category), UnicodoidStatus.Ok, "system category");
        Check(category == UnicodeCategory.UppercaseLetter, "system category value");
        CheckStatus(UnicodeProperties.TryToUppercase(utf8, 'a', out CharacterValue upper), UnicodoidStatus.Ok, "system uppercase");
        Check(upper.Value == 'A', "system uppercase value");
    }

    private static void TestNormalization()
    {
        IEncodingSystem utf8 = Utf8System.Instance;
        byte[] decomposed = { 0x65, 0xCC, 0x81 };
        byte[] composed = { 0xC3, 0xA9 };

        Check(!Normalization.IsNormalized(utf8, decomposed, NormalizationForm.Nfc), "decomposed not NFC");
        Check(Normalization.IsNormalized(utf8, composed, NormalizationForm.Nfc), "composed NFC");

        Span<byte> output = stackalloc byte[32];
        CheckStatus(Normalization.Normalize(utf8, decomposed, NormalizationForm.Nfc, output, out int written),
            UnicodoidStatus.Ok, "NFC normalize");
        CheckBytes(output[..written], composed, "NFC bytes");

        CheckStatus(Normalization.Normalize(utf8, composed, NormalizationForm.Nfd, output, out written),
            UnicodoidStatus.Ok, "NFD normalize");
        CheckBytes(output[..written], decomposed, "NFD bytes");

        byte[] ligature = { 0xEF, 0xAC, 0x81 }; // U+FB01
        byte[] fi = { 0x66, 0x69 };
        CheckStatus(Normalization.Normalize(utf8, ligature, NormalizationForm.Nfkd, output, out written),
            UnicodoidStatus.Ok, "NFKD ligature");
        CheckBytes(output[..written], fi, "NFKD ligature bytes");
        CheckStatus(Normalization.Normalize(utf8, ligature, NormalizationForm.Nfc, output, out written),
            UnicodoidStatus.Ok, "NFC ligature");
        CheckBytes(output[..written], ligature, "NFC ligature bytes");

        byte[] hangul = { 0xEA, 0xB0, 0x80 }; // U+AC00
        byte[] jamo = { 0xE1, 0x84, 0x80, 0xE1, 0x85, 0xA1 };
        CheckStatus(Normalization.Normalize(utf8, hangul, NormalizationForm.Nfd, output, out written),
            UnicodoidStatus.Ok, "Hangul NFD");
        CheckBytes(output[..written], jamo, "Hangul NFD bytes");
        CheckStatus(Normalization.Normalize(utf8, jamo, NormalizationForm.Nfc, output, out written),
            UnicodoidStatus.Ok, "Hangul NFC");
        CheckBytes(output[..written], hangul, "Hangul NFC bytes");

        byte[] unordered = { 0x61, 0xCC, 0x81, 0xCC, 0xA7 };
        byte[] ordered = { 0x61, 0xCC, 0xA7, 0xCC, 0x81 };
        CheckStatus(Normalization.Normalize(utf8, unordered, NormalizationForm.Nfd, output, out written),
            UnicodoidStatus.Ok, "canonical ordering");
        CheckBytes(output[..written], ordered, "canonical ordering bytes");

        CheckStatus(Normalization.Normalize(utf8, decomposed, NormalizationForm.Nfc, Span<byte>.Empty, out int required),
            UnicodoidStatus.BufferTooSmall, "normalization required");
        Check(required == composed.Length, "normalization required value");

        CheckStatus(Normalization.Normalize(KupoConlangSystem.Instance, new byte[] { (byte)'A' }, NormalizationForm.Nfc, output, out _),
            UnicodoidStatus.Unsupported, "normalization opt-in");
    }

    private static List<TextSegment> CollectSegments(
        IEncodingSystem system,
        ReadOnlySpan<byte> data,
        BoundaryType boundaryType)
    {
        var segments = new List<TextSegment>();
        UnicodoidStatus status = Segmenter.TryCreate(system, data, boundaryType, out Segmenter? segmenter);
        CheckStatus(status, UnicodoidStatus.Ok, "segmenter create");
        if (segmenter is not null)
        {
            segments.AddRange(segmenter.Segments());
        }
        return segments;
    }

    private static void TestSegmentation()
    {
        IEncodingSystem utf8 = Utf8System.Instance;

        List<TextSegment> graphemes = CollectSegments(utf8, new byte[] { 0x61, 0xCC, 0x81, 0x62 }, BoundaryType.Grapheme);
        Check(graphemes.Count == 2, "grapheme count");
        Check(graphemes[0].ByteOffset == 0 && graphemes[0].ByteLength == 3, "grapheme first");
        Check(graphemes[0].CharCount == 2, "grapheme char count");
        Check(graphemes[1].ByteOffset == 3 && graphemes[1].ByteLength == 1, "grapheme second");

        List<TextSegment> words = CollectSegments(
            utf8, new byte[] { (byte)'h', (byte)'e', (byte)'l', (byte)'l', (byte)'o', (byte)' ',
                               (byte)'w', (byte)'o', (byte)'r', (byte)'l', (byte)'d' }, BoundaryType.Word);
        Check(words.Count == 3, "word count");
        Check(words[0].ByteLength == 5 && words[1].ByteLength == 1 && words[2].ByteLength == 5, "word lengths");

        List<TextSegment> sentences = CollectSegments(
            utf8, new byte[] { (byte)'H', (byte)'i', (byte)'.', (byte)' ',
                               (byte)'B', (byte)'y', (byte)'e', (byte)'.' }, BoundaryType.Sentence);
        Check(sentences.Count == 2, "sentence count");
        Check(sentences[0].ByteLength == 4 && sentences[1].ByteLength == 4, "sentence lengths");

        List<TextSegment> lines = CollectSegments(
            utf8, new byte[] { (byte)'a', (byte)'b', (byte)'c', (byte)' ',
                               (byte)'d', (byte)'e', (byte)'f' }, BoundaryType.Line);
        // UAX #14 LB18 breaks after spaces, so the space stays with "abc".
        Check(lines.Count == 2, "line count");
        Check(lines[0].ByteLength == 4 && lines[1].ByteLength == 3, "line lengths");

        CheckStatus(Segmenter.TryCreate(KupoConlangSystem.Instance, new byte[] { (byte)'A' }, BoundaryType.Grapheme, out _),
            UnicodoidStatus.Unsupported, "segmentation opt-in");
    }

    private static TextString MakeString(IEncodingSystem system, params byte[] bytes) =>
        TextString.FromBytes(system, bytes);

    private static void TestCollation()
    {
        IEncodingSystem utf8 = Utf8System.Instance;

        TextString a = MakeString(utf8, (byte)'a');
        TextString b = MakeString(utf8, (byte)'b');
        TextString upperA = MakeString(utf8, (byte)'A');
        TextString e = MakeString(utf8, (byte)'e');
        TextString eAcute = MakeString(utf8, 0xC3, 0xA9);
        TextString file2 = MakeString(utf8, (byte)'f', (byte)'i', (byte)'l', (byte)'e', (byte)'2');
        TextString file10 = MakeString(utf8, (byte)'f', (byte)'i', (byte)'l', (byte)'e', (byte)'1', (byte)'0');

        CollationOptions options = CollationOptions.Default;
        Check(Collation.Compare(a, b, options) < 0, "a < b");
        Check(Collation.Compare(b, a, options) > 0, "b > a");
        Check(Collation.Compare(a, a, options) == 0, "a == a");

        options.Strength = CollationStrength.Primary;
        Check(Collation.Compare(a, upperA, options) == 0, "primary case-insensitive");
        Check(Collation.Compare(e, eAcute, options) == 0, "primary accent-insensitive");

        options = CollationOptions.Default;
        Check(Collation.Compare(a, upperA, options) < 0, "tertiary case");
        Check(Collation.Compare(e, eAcute, options) < 0, "secondary accent");

        options.NumericOrdering = true;
        Check(Collation.Compare(file2, file10, options) < 0, "numeric ordering");
        options.NumericOrdering = false;
        Check(Collation.Compare(file2, file10, options) > 0, "lexical ordering");

        options = CollationOptions.Default;
        CheckStatus(Collation.TryGetSortKey(a, options, Span<byte>.Empty, out int needed),
            UnicodoidStatus.BufferTooSmall, "sort key required");
        Check(needed > 0, "sort key required positive");

        Span<byte> keyA = stackalloc byte[64];
        Span<byte> keyB = stackalloc byte[64];
        CheckStatus(Collation.TryGetSortKey(a, options, keyA, out int writtenA), UnicodoidStatus.Ok, "sort key a");
        CheckStatus(Collation.TryGetSortKey(b, options, keyB, out int writtenB), UnicodoidStatus.Ok, "sort key b");
        Check(writtenA > 0 && writtenB > 0, "sort key lengths");
        int common = Math.Min(writtenA, writtenB);
        Check(keyA[..common].SequenceCompareTo(keyB[..common]) < 0, "sort key ordering");

        TextString conlangStr = MakeString(KupoConlangSystem.Instance, (byte)'A');
        CheckStatus(Collation.TryGetSortKey(conlangStr, options, keyA, out _),
            UnicodoidStatus.Unsupported, "collation opt-in");
    }

    private static void TestStatefulCodec()
    {
        IEncodingSystem sys = ShiftModal16System.Instance;
        Check(sys.IsStateful, "shiftmodal stateful");
        Check(!sys.HasUnicodeProfile, "shiftmodal no profile");
        Check(sys.StateSize > 0, "shiftmodal state size");
        Check(sys.Model == CharModel.ModalStateful, "shiftmodal model");

        CharacterValue[] chars = { 0x41, 0x4000, 0x4001, 0x42, 0x0100 };
        byte[] expected = { 0x41, 0x0E, 0x00, 0x01, 0x0F, 0x42, 0x1B, 0x00 };

        CheckStatus(sys.GetByteCount(chars, out int needed), UnicodoidStatus.Ok, "shiftmodal byte count");
        Check(needed == expected.Length, "shiftmodal byte count value");

        var codec = new Codec(sys);
        CheckStatus(codec.EncodeAll(chars, out byte[] encoded), UnicodoidStatus.Ok, "shiftmodal encode");
        CheckBytes(encoded, expected, "shiftmodal bytes");

        CheckStatus(codec.DecodeAll(encoded, out CharacterValue[] decoded), UnicodoidStatus.Ok, "shiftmodal decode");
        Check(decoded.Length == 5, "shiftmodal decoded count");
        Check(decoded[0].Value == 0x41 && decoded[1].Value == 0x4000 && decoded[2].Value == 0x4001 &&
              decoded[3].Value == 0x42 && decoded[4].Value == 0x0100, "shiftmodal decoded values");

        CheckStatus(codec.CountChars(encoded, out int charCount), UnicodoidStatus.Ok, "shiftmodal count");
        Check(charCount == 5, "shiftmodal count value");
        CheckStatus(codec.Validate(encoded, out _), UnicodoidStatus.Ok, "shiftmodal validate");

        codec.Reset();
        Span<byte> buffer = stackalloc byte[16];
        CheckStatus(codec.EncodeChar(0x41, buffer, out int written), UnicodoidStatus.Ok, "shiftmodal reset encode");
        Check(written == 1 && buffer[0] == 0x41, "shiftmodal reset bytes");

        CheckStatus(codec.Validate(new byte[] { 0x41, 0x1B }, out int invalidOffset),
            UnicodoidStatus.IncompleteSequence, "shiftmodal trailing escape");
        Check(invalidOffset == 1, "shiftmodal trailing escape offset");

        CheckStatus(codec.Validate(new byte[] { 0x0E }, out _), UnicodoidStatus.Ok, "shiftmodal lone shift");
        codec.Reset();
        CheckStatus(codec.CountChars(new byte[] { 0x0E }, out int controlChars), UnicodoidStatus.Ok, "shiftmodal control count");
        Check(controlChars == 0, "shiftmodal control count value");

        CharacterValue[] reserved = { 0x000E, 0x000F, 0x001B, 0x400F, 0x401B };
        foreach (CharacterValue ch in reserved)
        {
            codec.Reset();
            CheckStatus(codec.EncodeChar(ch, buffer, out _), UnicodoidStatus.UnmappableChar, "shiftmodal reserved");
        }
    }

    private static void TestStatefulString()
    {
        IEncodingSystem sys = ShiftModal16System.Instance;
        CharacterValue[] chars = { 0x41, 0x4000, 0x4001, 0x42, 0x0100 };

        TextString str = TextString.FromChars(sys, chars);
        Check(str.Length == 8, "stateful string length");
        CheckStatus(str.TryGetCharLength(out int charLength), UnicodoidStatus.Ok, "stateful char length");
        Check(charLength == 5, "stateful char length value");

        CheckStatus(str.TryGetCharAt(1, out CharacterValue ch), UnicodoidStatus.Ok, "stateful char at 1");
        Check(ch.Value == 0x4000, "stateful char at 1 value");
        CheckStatus(str.TryGetCharAt(4, out ch), UnicodoidStatus.Ok, "stateful char at 4");
        Check(ch.Value == 0x0100, "stateful char at 4 value");

        TextString clone = str.Clone();
        Check(str.ContentEquals(clone), "stateful clone equal");

        CheckStatus(clone.TryAppendChar(0x4002), UnicodoidStatus.Ok, "stateful clone append");
        Check(clone.Length == 10 && clone.Bytes[8] == 0x0E && clone.Bytes[9] == 0x02, "stateful clone bytes");
        Check(str.Length == 8, "stateful original unchanged");

        str.ResetState();
        CheckStatus(str.TryAppendChar(0x41), UnicodoidStatus.Ok, "stateful reset append");
        Check(str.Length == 9 && str.Bytes[8] == 0x41, "stateful reset bytes");
    }

    private static void TestAppendBytesState()
    {
        IEncodingSystem sys = ShiftModal16System.Instance;
        var str = new TextString(sys, 8);

        CheckStatus(str.TryAppendChar(0x4000), UnicodoidStatus.Ok, "append shifted");
        Check(str.Length == 2 && str.Bytes[0] == 0x0E && str.Bytes[1] == 0x00, "append shifted bytes");

        CheckStatus(str.TryAppendBytes(new byte[] { 0x01 }), UnicodoidStatus.Ok, "append continuation");
        Check(str.Length == 3 && str.Bytes[2] == 0x01, "append continuation bytes");

        CheckStatus(str.TryAppendBytes(new byte[] { 0x0F }), UnicodoidStatus.Ok, "append shift in");
        CheckStatus(str.TryAppendChar(0x41), UnicodoidStatus.Ok, "append after shift in");
        Check(str.Length == 5 && str.Bytes[4] == 0x41, "append after shift in bytes");

        CheckStatus(str.TryGetCharLength(out int charLength), UnicodoidStatus.Ok, "append bytes char length");
        Check(charLength == 3, "append bytes char length value");
    }

    private static UnicodoidStatus ShiftModalBridge(
        IEncodingSystem sourceSystem,
        CharacterValue sourceChar,
        IEncodingSystem destinationSystem,
        out CharacterValue destinationChar)
    {
        destinationChar = CharacterValue.Invalid;
        if (sourceChar.Value >= 0x4000 && sourceChar.Value <= 0x40FF)
        {
            destinationChar = sourceChar.Value & 0xFF;
            return UnicodoidStatus.Ok;
        }
        if (sourceChar.Value <= 0x3FFF)
        {
            destinationChar = sourceChar;
            return UnicodoidStatus.Ok;
        }
        return UnicodoidStatus.UnmappableChar;
    }

    private static void TestStatefulTranscode()
    {
        IEncodingSystem shift = ShiftModal16System.Instance;
        IEncodingSystem conlang = KupoConlangSystem.Instance;

        TextString src = TextString.FromChars(shift, new CharacterValue[] { 0x4001, 0x41 });
        var config = new TranscodeConfig();

        CheckStatus(Transcoding.TryString(src, conlang, out _, config),
            UnicodoidStatus.UnmappableChar, "stateful no implicit bridge");

        config.Bridge = ShiftModalBridge;
        CheckStatus(Transcoding.TryString(src, conlang, out TextString? output, config),
            UnicodoidStatus.Ok, "stateful bridged");
        Check(output!.Length == 2 && output.Bytes[0] == 0x01 && output.Bytes[1] == 0x41, "stateful bridged bytes");

        config = new TranscodeConfig();
        CheckStatus(Transcoding.TryString(src, shift, out TextString? same, config), UnicodoidStatus.Ok, "stateful same");
        CheckBytes(same!.Bytes, src.Bytes, "stateful same bytes");
    }

    // ---- Official Unicode 18.0.0 segmentation conformance ----------------

    private static string FindAuxiliaryDirectory()
    {
        string? dir = AppContext.BaseDirectory;
        while (dir is not null)
        {
            string candidate = Path.Combine(
                dir, "data", "ucd", "18.0.0", "ucd", "auxiliary");
            if (Directory.Exists(candidate)) return candidate;
            dir = Directory.GetParent(dir)?.FullName;
        }
        throw new DirectoryNotFoundException(
            "data/ucd/18.0.0/ucd/auxiliary was not found above " + AppContext.BaseDirectory);
    }

    private static string FindUcdDirectory()
    {
        string? dir = AppContext.BaseDirectory;
        while (dir is not null)
        {
            string candidate = Path.Combine(
                dir, "data", "ucd", "18.0.0", "ucd");
            if (Directory.Exists(candidate)) return candidate;
            dir = Directory.GetParent(dir)?.FullName;
        }
        throw new DirectoryNotFoundException(
            "data/ucd/18.0.0/ucd was not found above " + AppContext.BaseDirectory);
    }

    private static uint[] ParseScalarSequence(string field)
    {
        if (string.IsNullOrWhiteSpace(field)) return Array.Empty<uint>();
        string[] tokens = field.Split(
            (char[]?)null, StringSplitOptions.RemoveEmptyEntries);
        var result = new uint[tokens.Length];
        for (int i = 0; i < tokens.Length; ++i)
        {
            result[i] = uint.Parse(
                tokens[i],
                System.Globalization.NumberStyles.HexNumber,
                System.Globalization.CultureInfo.InvariantCulture);
        }
        return result;
    }

    private static byte[] Utf8Encode(uint[] cps)
    {
        var bytes = new List<byte>(cps.Length * 4);
        foreach (uint cp in cps)
        {
            if (cp < 0x80) bytes.Add((byte)cp);
            else if (cp < 0x800)
            {
                bytes.Add((byte)(0xC0 | (cp >> 6)));
                bytes.Add((byte)(0x80 | (cp & 0x3F)));
            }
            else if (cp < 0x10000)
            {
                bytes.Add((byte)(0xE0 | (cp >> 12)));
                bytes.Add((byte)(0x80 | ((cp >> 6) & 0x3F)));
                bytes.Add((byte)(0x80 | (cp & 0x3F)));
            }
            else
            {
                bytes.Add((byte)(0xF0 | (cp >> 18)));
                bytes.Add((byte)(0x80 | ((cp >> 12) & 0x3F)));
                bytes.Add((byte)(0x80 | ((cp >> 6) & 0x3F)));
                bytes.Add((byte)(0x80 | (cp & 0x3F)));
            }
        }
        return bytes.ToArray();
    }

    private static uint[] Utf8Decode(byte[] bytes)
    {
        var result = new List<uint>(bytes.Length);
        int i = 0;
        while (i < bytes.Length)
        {
            byte b0 = bytes[i];
            uint cp; int advance;
            if (b0 < 0x80) { cp = b0; advance = 1; }
            else if (b0 < 0xE0) { cp = (uint)((b0 & 0x1F) << 6) | (uint)(bytes[i + 1] & 0x3F); advance = 2; }
            else if (b0 < 0xF0) { cp = (uint)((b0 & 0x0F) << 12) | (uint)((bytes[i + 1] & 0x3F) << 6) | (uint)(bytes[i + 2] & 0x3F); advance = 3; }
            else { cp = (uint)((b0 & 0x07) << 18) | (uint)((bytes[i + 1] & 0x3F) << 12) | (uint)((bytes[i + 2] & 0x3F) << 6) | (uint)(bytes[i + 3] & 0x3F); advance = 4; }
            result.Add(cp);
            i += advance;
        }
        return result.ToArray();
    }

    private static uint[] NormalizeCps(uint[] cps, NormalizationForm form)
    {
        byte[] input = Utf8Encode(cps);
        byte[] output = Normalization.Normalize(Utf8System.Instance, input, form);
        return Utf8Decode(output);
    }

    private static void TestNormalizationConformance()
    {
        string path = Path.Combine(FindUcdDirectory(), "NormalizationTest.txt");
        int cases = 0;
        int checks = 0;
        int failures = 0;

        foreach (string raw in File.ReadLines(path))
        {
            string line = raw.Split('#')[0].Trim();
            if (line.Length == 0 || line.StartsWith('@')) continue;

            string[] fields = line.Split(';');
            if (fields.Length < 5) continue;

            uint[] source = ParseScalarSequence(fields[0]);
            uint[] nfc = ParseScalarSequence(fields[1]);
            uint[] nfd = ParseScalarSequence(fields[2]);
            uint[] nfkc = ParseScalarSequence(fields[3]);
            uint[] nfkd = ParseScalarSequence(fields[4]);
            if (source.Length == 0) continue;
            ++cases;

            (NormalizationForm Form, uint[] Input, uint[] Expected)[] invariants =
            {
                (NormalizationForm.Nfc, source, nfc),
                (NormalizationForm.Nfc, nfc, nfc),
                (NormalizationForm.Nfc, nfd, nfc),
                (NormalizationForm.Nfc, nfkc, nfkc),
                (NormalizationForm.Nfc, nfkd, nfkc),
                (NormalizationForm.Nfd, source, nfd),
                (NormalizationForm.Nfd, nfc, nfd),
                (NormalizationForm.Nfd, nfd, nfd),
                (NormalizationForm.Nfd, nfkc, nfkd),
                (NormalizationForm.Nfd, nfkd, nfkd),
                (NormalizationForm.Nfkc, source, nfkc),
                (NormalizationForm.Nfkc, nfc, nfkc),
                (NormalizationForm.Nfkc, nfd, nfkc),
                (NormalizationForm.Nfkc, nfkc, nfkc),
                (NormalizationForm.Nfkc, nfkd, nfkc),
                (NormalizationForm.Nfkd, source, nfkd),
                (NormalizationForm.Nfkd, nfc, nfkd),
                (NormalizationForm.Nfkd, nfd, nfkd),
                (NormalizationForm.Nfkd, nfkc, nfkd),
                (NormalizationForm.Nfkd, nfkd, nfkd),
            };

            foreach ((NormalizationForm form, uint[] input, uint[] expected) in invariants)
            {
                ++checks;
                uint[] actual = NormalizeCps(input, form);
                if (actual.AsSpan().SequenceEqual(expected)) continue;

                ++failures;
                if (failures <= 10)
                {
                    Console.Error.WriteLine(
                        $"NormalizationTest.txt case {cases}: {form} input=[{string.Join(", ", Array.ConvertAll(input, c => $"U+{c:X4}"))}]\n" +
                        $"  expected=[{string.Join(", ", Array.ConvertAll(expected, c => $"U+{c:X4}"))}]\n" +
                        $"  actual  =[{string.Join(", ", Array.ConvertAll(actual, c => $"U+{c:X4}"))}]");
                }
            }
        }

        Check(failures == 0,
            $"NormalizationTest.txt: {failures} of {checks} checks failed across {cases} cases");
        Console.WriteLine($"NormalizationTest.txt: {cases} cases, {checks} checks passed");
    }

    /// <summary>
    /// Official Unicode 18.0.0 conformance test for UTS #10.  The vectors are
    /// read directly from the UCA distribution in
    /// <c>data/ucd/18.0.0/uca/test/CollationTest/</c>.  Each line of
    /// <c>CollationTest_NON_IGNORABLE_SHORT.txt</c> must sort after (or equal
    /// to) the previous line; the official file is non-decreasing.
    /// </summary>
    private static void TestCollationConformance()
    {
        string ucd = FindUcdDirectory();
        string path = Path.Combine(
            Path.GetDirectoryName(ucd)!, "uca", "test", "CollationTest",
            "CollationTest_NON_IGNORABLE_SHORT.txt");

        CollationOptions options = CollationOptions.Default;
        uint[]? previous = null;
        int cases = 0;
        int skipped = 0;
        int failures = 0;

        foreach (string raw in File.ReadLines(path))
        {
            string line = raw.Trim();
            if (line.Length == 0 || line.StartsWith('#') || line.StartsWith('@')) continue;

            uint[] cps = ParseScalarSequence(line.Split(';')[0]);
            if (cps.Length == 0) continue;
            if (Array.Exists(cps, cp => cp >= 0xD800 && cp <= 0xDFFF))
            {
                ++skipped;
                continue;
            }

            if (previous is not null &&
                Collation.CompareScalars(previous, cps, options) > 0)
            {
                ++failures;
                if (failures <= 10)
                {
                    Console.Error.WriteLine(
                        $"CollationTest_NON_IGNORABLE_SHORT.txt: decreasing order at case {cases + 1}");
                }
            }

            previous = cps;
            ++cases;
        }

        Check(failures == 0 && cases > 200_000,
            $"CollationTest_NON_IGNORABLE_SHORT.txt: {failures} failures across {cases} cases ({skipped} skipped)");
        Console.WriteLine(
            $"CollationTest_NON_IGNORABLE_SHORT.txt: {cases} cases, {skipped} skipped, {failures} failures");
    }

    private static string DataRoot() => Path.GetDirectoryName(FindUcdDirectory())!;

    private static uint[] DecodeRunes(string text)
    {
        var result = new List<uint>(text.Length);
        foreach (Rune rune in text.EnumerateRunes()) result.Add((uint)rune.Value);
        return result.ToArray();
    }

    /// <summary>Official Unicode 18.0.0 conformance test for UAX #34.</summary>
    private static void TestUax34Conformance()
    {
        string path = Path.Combine(FindUcdDirectory(), "NamedSequences.txt");
        int lines = 0;
        int failures = 0;

        foreach (string raw in File.ReadLines(path))
        {
            string line = raw.Trim();
            if (line.Length == 0 || line.StartsWith('#') || line.StartsWith('@')) continue;
            int semi = line.IndexOf(';');
            if (semi < 0) continue;

            string name = line[..semi].Trim();
            uint[] expected = ParseScalarSequence(line[(semi + 1)..]);
            uint[]? actual = Uax34.Find(name);
            ++lines;

            if (actual is null || !actual.AsSpan().SequenceEqual(expected))
            {
                ++failures;
                if (failures <= 10)
                {
                    Console.Error.WriteLine($"UAX34: lookup mismatch for {name}");
                }
            }
            if (!Uax34.IsRegistered(expected))
            {
                ++failures;
                if (failures <= 10)
                {
                    Console.Error.WriteLine($"UAX34: sequence not registered for {name}");
                }
            }
        }

        Check(lines > 400, "NamedSequences.txt is populated");
        Check(Uax34.Count == lines, "UAX34 count matches data file");
        Check(!Uax34.IsRegistered(new uint[] { 0xD800, 0x0020 }), "UAX34 surrogate rejected");
        Check(!Uax34.IsRegistered(Array.Empty<uint>()), "UAX34 empty rejected");
        Check(Uax34.Find("NOT A REAL SEQUENCE NAME") is null, "UAX34 unknown name rejected");
        Check(failures == 0, $"UAX34 conformance failures: {failures}");
        Console.WriteLine($"NamedSequences.txt: {lines} sequences validated");
    }

    private static void ValidateEmojiSequence(
        uint[] cps, string expectedType, ref int failures, ref int checks)
    {
        string? actualType = Uts51.IsRgi(cps);
        ++checks;
        if (actualType != expectedType)
        {
            ++failures;
            if (failures <= 10)
            {
                Console.Error.WriteLine(
                    $"UTS51: expected {expectedType}, got {actualType ?? "<none>"} for " +
                    string.Join(" ", Array.ConvertAll(cps, cp => cp.ToString("X4"))));
            }
        }

        ++checks;
        if (!Uts51.IsWellFormed(cps))
        {
            ++failures;
            if (failures <= 10)
            {
                Console.Error.WriteLine(
                    $"UTS51: RGI sequence not well formed ({expectedType})");
            }
        }
    }

    private static int ValidateEmojiFile(
        string path, ref int failures, ref int checks)
    {
        int sequences = 0;
        foreach (string raw in File.ReadLines(path))
        {
            string line = raw.Trim();
            if (line.Length == 0 || line.StartsWith('#') || line.StartsWith('@')) continue;

            string[] fields = line.Split(';');
            if (fields.Length < 2) continue;
            string typeField = fields[1].Trim().Split(' ', '\t')[0];
            if (typeField.Length == 0) continue;

            string field = fields[0].Trim();
            int range = field.IndexOf("..", StringComparison.Ordinal);
            if (range >= 0)
            {
                uint first = uint.Parse(
                    field[..range].Trim(),
                    System.Globalization.NumberStyles.HexNumber,
                    System.Globalization.CultureInfo.InvariantCulture);
                uint last = uint.Parse(
                    field[(range + 2)..].Trim(),
                    System.Globalization.NumberStyles.HexNumber,
                    System.Globalization.CultureInfo.InvariantCulture);
                for (uint cp = first; cp <= last; ++cp)
                {
                    ValidateEmojiSequence(new[] { cp }, typeField, ref failures, ref checks);
                    ++sequences;
                }
                continue;
            }

            uint[] cps = ParseScalarSequence(field);
            if (cps.Length == 0) continue;
            ValidateEmojiSequence(cps, typeField, ref failures, ref checks);
            ++sequences;
        }
        return sequences;
    }

    /// <summary>Official Unicode 18.0.0 conformance test for UTS #51.</summary>
    private static void TestUts51Conformance()
    {
        string dir = Path.Combine(DataRoot(), "emoji");
        int failures = 0;
        int checks = 0;
        int a = ValidateEmojiFile(
            Path.Combine(dir, "emoji-sequences.txt"), ref failures, ref checks);
        int b = ValidateEmojiFile(
            Path.Combine(dir, "emoji-zwj-sequences.txt"), ref failures, ref checks);

        Check(Uts51.Count == a + b, "UTS51 count matches data files");

        uint[] keycap = { 0x31, 0xFE0F, 0x20E3 };
        Check(Uts51.IsRgi(keycap) == "Emoji_Keycap_Sequence", "UTS51 keycap type");
        Check(Uts51.IsWellFormed(keycap), "UTS51 keycap well formed");

        uint[] flag = { 0x1F1EB, 0x1F1F7 };
        Check(Uts51.IsRgi(flag) == "RGI_Emoji_Flag_Sequence", "UTS51 flag type");

        uint[] family = { 0x1F468, 0x200D, 0x1F469, 0x200D, 0x1F467 };
        Check(Uts51.IsRgi(family) == "RGI_Emoji_ZWJ_Sequence", "UTS51 ZWJ type");

        uint[] bogus = { 0x41, 0x200D, 0x42 };
        Check(Uts51.IsRgi(bogus) is null, "UTS51 non-emoji ZWJ rejected");
        Check(!Uts51.IsWellFormed(bogus), "UTS51 non-emoji ZWJ not well formed");
        Check(!Uts51.IsWellFormed(new uint[] { 0xD83D, 0xDE00 }), "UTS51 surrogate rejected");
        Check(!Uts51.IsWellFormed(Array.Empty<uint>()), "UTS51 empty rejected");

        Check(failures == 0, $"UTS51 conformance failures: {failures}");
        Console.WriteLine($"emoji-sequences: {a + b} RGI sequences validated");
    }

    private static int FindInitiator(uint[] cps, int from, int to)
    {
        for (int i = from; i < to; ++i)
        {
            if (cps[i] is 0x2F or 0x3F or 0x23) return i;
            if (i + 2 < cps.Length && cps[i] == 0x3A && cps[i + 1] == 0x7E && cps[i + 2] == 0x3A)
            {
                return i;
            }
        }
        return -1;
    }

    /// <summary>Official Unicode 18.0.0 conformance test for UTS #58 link detection.</summary>
    private static void TestUts58DetectionConformance()
    {
        const uint markOpen = 0x2E20;
        const uint markClose = 0x2E21;
        string path = Path.Combine(DataRoot(), "linkification", "LinkDetectionTest.txt");

        int cases = 0;
        int links = 0;
        int checkedLinks = 0;
        int skippedBare = 0;
        int skippedEmail = 0;
        int failures = 0;

        foreach (string raw in File.ReadLines(path))
        {
            string line = raw.TrimEnd('\r', '\n');
            if (line.Length == 0 || line.StartsWith('#')) continue;
            ++cases;
            uint[] cps = DecodeRunes(line);
            int n = cps.Length;

            int i = 0;
            while (i < n)
            {
                if (cps[i] != markOpen)
                {
                    ++i;
                    continue;
                }
                int open = i;
                int close = -1;
                for (int j = open + 1; j < n; ++j)
                {
                    if (cps[j] == markClose)
                    {
                        close = j;
                        break;
                    }
                }
                if (close < 0)
                {
                    ++failures;
                    Console.Error.WriteLine($"UTS58: unpaired mark in {line}");
                    break;
                }
                ++links;

                int init = FindInitiator(cps, open + 1, close);
                if (init < 0)
                {
                    int at = -1;
                    for (int j = open + 1; j < close; ++j)
                    {
                        if (cps[j] == 0x40)
                        {
                            at = j;
                            break;
                        }
                    }
                    if (at >= 0)
                    {
                        int local = Uts58.EmailLocalPartStart(cps, at);
                        ++skippedEmail;
                        if (local >= 0 && local != open + 1)
                        {
                            ++failures;
                            if (failures <= 10)
                            {
                                Console.Error.WriteLine(
                                    $"UTS58: email local part mismatch span=[{open + 1},{close}) local={local}");
                            }
                        }
                    }
                    else
                    {
                        ++skippedBare;
                    }
                }
                else
                {
                    int end = Uts58.LinkDetectionEnd(cps, init);
                    ++checkedLinks;
                    if (end != close)
                    {
                        ++failures;
                        if (failures <= 10)
                        {
                            Console.Error.WriteLine(
                                $"UTS58: detection mismatch span=[{open + 1},{close}) init={init} end={end}");
                        }
                    }
                }

                i = close + 1;
            }
        }

        Check(failures == 0, $"UTS58 detection failures: {failures}");
        Check(checkedLinks > 100, "LinkDetectionTest.txt is populated");
        Console.WriteLine(
            $"LinkDetectionTest.txt: {cases} cases, {links} links, {checkedLinks} checked, " +
            $"{skippedBare} bare, {skippedEmail} email, {failures} failures");
    }

    private const uint SymS = 0x1D47A;
    private const uint SymH = 0x1D46F;
    private const uint SymP = 0x1D477;
    private const uint SymQ = 0x1D478;
    private const uint SymV = 0x1D47D;
    private const uint SymF = 0x1D46D;
    private const uint SymD = 0x1D46B;

    private static bool IsStructureSymbol(uint cp) =>
        cp is SymS or SymH or SymP or SymQ or SymV or SymF or SymD;

    private static List<(uint Symbol, string Value)>? ParseStructureLine(string line)
    {
        int brace = line.IndexOf('{');
        if (brace < 0) return null;
        string rest = line[(brace + 1)..];

        var runes = new List<(int Index, Rune Rune)>();
        for (int index = 0; index < rest.Length;)
        {
            System.Buffers.OperationStatus status = Rune.DecodeFromUtf16(
                rest.AsSpan(index), out Rune rune, out int consumed);
            if (status != System.Buffers.OperationStatus.Done) break;
            runes.Add((index, rune));
            index += consumed;
        }

        var tokens = new List<(uint Symbol, string Value)>();
        int idx = 0;
        while (idx < runes.Count)
        {
            (int bytePos, Rune rune) = runes[idx];
            uint sym = (uint)rune.Value;
            if (!IsStructureSymbol(sym) || idx + 1 >= runes.Count || runes[idx + 1].Rune.Value != '=')
            {
                ++idx;
                continue;
            }

            int valueStart = runes[idx + 1].Index + 1;
            int vend = rest.Length;
            int j = idx + 2;
            while (j < runes.Count)
            {
                (int b, Rune c) = runes[j];
                if (c.Value == '}')
                {
                    vend = b;
                    break;
                }
                if (c.Value == ' ' &&
                    j + 1 < runes.Count &&
                    IsStructureSymbol((uint)runes[j + 1].Rune.Value) &&
                    j + 2 < runes.Count &&
                    runes[j + 2].Rune.Value == '=')
                {
                    vend = b;
                    break;
                }
                ++j;
            }

            tokens.Add((sym, rest[valueStart..vend].TrimEnd()));
            idx = j;
        }

        return tokens.Count == 0 ? null : tokens;
    }

    private sealed class FormatParts
    {
        public string Scheme = string.Empty;
        public string Host = string.Empty;
        public List<string> Path = new();
        public List<(string Key, string Value)> Query = new();
        public string? Fragment;
        public List<string> Directives = new();
    }

    private static FormatParts ApplyStructure(List<(uint Symbol, string Value)> tokens)
    {
        var parts = new FormatParts();
        int pendingQuery = -1;
        foreach ((uint symbol, string value) in tokens)
        {
            switch (symbol)
            {
                case SymS: parts.Scheme = value; break;
                case SymH: parts.Host = value; break;
                case SymP: parts.Path.Add(value); break;
                case SymQ:
                    parts.Query.Add((value, string.Empty));
                    pendingQuery = parts.Query.Count - 1;
                    break;
                case SymV:
                    if (pendingQuery >= 0)
                    {
                        parts.Query[pendingQuery] = (parts.Query[pendingQuery].Key, value);
                    }
                    break;
                case SymF: parts.Fragment = value; break;
                case SymD: parts.Directives.Add(value); break;
            }
        }
        return parts;
    }

    private static bool IsHexDigit(byte b) =>
        b is >= (byte)'0' and <= (byte)'9' or >= (byte)'a' and <= (byte)'f' or >= (byte)'A' and <= (byte)'F';

    private static bool PercentFollowedByHex(byte[] p, int i) =>
        i + 2 < p.Length && IsHexDigit(p[i + 1]) && IsHexDigit(p[i + 2]);

    private static void AppendAscii(List<byte> output, string text)
    {
        foreach (char c in text) output.Add((byte)c);
    }

    private static void InteriorEscapePath(byte[] input, List<byte> output)
    {
        for (int i = 0; i < input.Length; ++i)
        {
            switch (input[i])
            {
                case (byte)'/': AppendAscii(output, "%2F"); break;
                case (byte)'?': AppendAscii(output, "%3F"); break;
                case (byte)'#': AppendAscii(output, "%23"); break;
                case (byte)'%':
                    if (PercentFollowedByHex(input, i)) AppendAscii(output, "%25");
                    else output.Add((byte)'%');
                    break;
                default: output.Add(input[i]); break;
            }
        }
    }

    private static void InteriorEscapeQuery(byte[] input, List<byte> output)
    {
        for (int i = 0; i < input.Length; ++i)
        {
            switch (input[i])
            {
                case (byte)'+': AppendAscii(output, "%2B"); break;
                case (byte)'#': AppendAscii(output, "%23"); break;
                case (byte)'=': AppendAscii(output, "%3D"); break;
                case (byte)'&': AppendAscii(output, "%26"); break;
                case (byte)'%':
                    if (PercentFollowedByHex(input, i)) AppendAscii(output, "%25");
                    else output.Add((byte)'%');
                    break;
                default: output.Add(input[i]); break;
            }
        }
    }

    private static void InteriorEscapeFragment(byte[] input, List<byte> output)
    {
        for (int i = 0; i < input.Length;)
        {
            if (input[i] == (byte)':' && i + 2 < input.Length &&
                input[i + 1] == (byte)'~' && input[i + 2] == (byte)':')
            {
                AppendAscii(output, "%3A~:");
                i += 3;
                continue;
            }
            if (input[i] == (byte)'%' && PercentFollowedByHex(input, i))
            {
                AppendAscii(output, "%25");
                ++i;
                continue;
            }
            output.Add(input[i]);
            ++i;
        }
    }

    private static void InteriorEscapeDirective(byte[] input, List<byte> output)
    {
        for (int i = 0; i < input.Length;)
        {
            if (input[i] == (byte)':' && i + 2 < input.Length &&
                input[i + 1] == (byte)'~' && input[i + 2] == (byte)':')
            {
                AppendAscii(output, "%3A~:");
                i += 3;
                continue;
            }
            switch (input[i])
            {
                case (byte)'&': AppendAscii(output, "%26"); break;
                case (byte)',': AppendAscii(output, "%2C"); break;
                case (byte)'%':
                    if (PercentFollowedByHex(input, i)) AppendAscii(output, "%25");
                    else output.Add((byte)'%');
                    break;
                default: output.Add(input[i]); break;
            }
            ++i;
        }
    }

    private static void EscapePart(UrlPart part, List<byte> input, bool isLast, List<byte> output)
    {
        uint[] cps = Utf8Decode(input.ToArray());
        output.AddRange(Uts58.MinimalEscapePart(part, cps, isLast));
    }

    private static byte[] ReconstructExpected(FormatParts parts)
    {
        var output = new List<byte>();
        AppendAscii(output, parts.Scheme);
        AppendAscii(output, parts.Host);

        bool hasPath = parts.Path.Count > 0;
        bool hasQuery = parts.Query.Count > 0;
        bool hasFragment = parts.Fragment is not null;
        int directiveCount = parts.Directives.Count;

        if (hasPath)
        {
            var joined = new List<byte>();
            for (int s = 0; s < parts.Path.Count; ++s)
            {
                if (s > 0) joined.Add((byte)'/');
                InteriorEscapePath(Encoding.UTF8.GetBytes(parts.Path[s]), joined);
            }
            bool last = !hasQuery && !hasFragment && directiveCount == 0;
            EscapePart(UrlPart.Path, joined, last, output);
        }

        if (hasQuery)
        {
            var joined = new List<byte>();
            for (int s = 0; s < parts.Query.Count; ++s)
            {
                if (s > 0) joined.Add((byte)'&');
                InteriorEscapeQuery(Encoding.UTF8.GetBytes(parts.Query[s].Key), joined);
                if (parts.Query[s].Value.Length > 0)
                {
                    joined.Add((byte)'=');
                    InteriorEscapeQuery(Encoding.UTF8.GetBytes(parts.Query[s].Value), joined);
                }
            }
            bool last = !hasFragment && directiveCount == 0;
            EscapePart(UrlPart.Query, joined, last, output);
        }

        if (parts.Fragment is not null)
        {
            var escaped = new List<byte>();
            InteriorEscapeFragment(Encoding.UTF8.GetBytes(parts.Fragment), escaped);
            bool last = directiveCount == 0;
            EscapePart(UrlPart.Fragment, escaped, last, output);
        }

        for (int s = 0; s < parts.Directives.Count; ++s)
        {
            var escaped = new List<byte>();
            InteriorEscapeDirective(Encoding.UTF8.GetBytes(parts.Directives[s]), escaped);
            bool last = s + 1 == directiveCount;
            EscapePart(UrlPart.FragmentDirective, escaped, last, output);
        }

        return output.ToArray();
    }

    /// <summary>Official Unicode 18.0.0 conformance test for UTS #58 formatting.</summary>
    private static void TestUts58FormattingConformance()
    {
        string path = Path.Combine(DataRoot(), "linkification", "LinkFormattingTest.txt");
        string? source = null;
        FormatParts? pending = null;
        int cases = 0;
        int checkedCases = 0;
        int failures = 0;

        foreach (string raw in File.ReadLines(path))
        {
            string line = raw.TrimEnd('\r', '\n');
            if (line.Length == 0) continue;

            if (line.StartsWith('#'))
            {
                List<(uint Symbol, string Value)>? tokens = ParseStructureLine(line);
                if (tokens is not null) pending = ApplyStructure(tokens);
                continue;
            }

            if (source is null)
            {
                source = line;
                continue;
            }

            source = null;
            if (pending is null) continue;
            ++cases;
            byte[] actual = ReconstructExpected(pending);
            pending = null;
            ++checkedCases;
            if (!actual.AsSpan().SequenceEqual(Encoding.UTF8.GetBytes(line)))
            {
                ++failures;
                if (failures <= 10)
                {
                    Console.Error.WriteLine(
                        $"UTS58 formatting mismatch:\n  expected={line}\n  actual  ={Encoding.UTF8.GetString(actual)}");
                }
            }
        }

        Check(failures == 0, $"UTS58 formatting failures: {failures}");
        Check(checkedCases > 40, "LinkFormattingTest.txt is populated");
        Console.WriteLine(
            $"LinkFormattingTest.txt: {cases} cases, {checkedCases} checked, {failures} failures");
    }

    /// <summary>Official Unicode 18.0.0 conformance test for UTR #59.</summary>
    private static void TestUtr59Conformance()
    {
        string path = Path.Combine(DataRoot(), "tr59", "east-asian-spacing.txt");
        int entries = 0;
        int failures = 0;

        foreach (string raw in File.ReadLines(path))
        {
            string line = raw.Trim();
            if (line.Length == 0 || line.StartsWith('#') || line.StartsWith('@')) continue;
            int semi = line.IndexOf(';');
            if (semi < 0) continue;

            string valueField = line[(semi + 1)..].TrimStart();
            int expected = valueField.Length == 0 ? -1 : valueField[0] switch
            {
                'W' => Eas.W,
                'N' => Eas.N,
                'O' => Eas.O,
                'C' => Eas.C,
                _ => -1
            };
            if (expected < 0) continue;

            string range = line[..semi].Trim();
            int dots = range.IndexOf("..", StringComparison.Ordinal);
            uint first;
            uint last;
            if (dots >= 0)
            {
                first = uint.Parse(range[..dots].Trim(),
                    System.Globalization.NumberStyles.HexNumber,
                    System.Globalization.CultureInfo.InvariantCulture);
                last = uint.Parse(range[(dots + 2)..].Trim(),
                    System.Globalization.NumberStyles.HexNumber,
                    System.Globalization.CultureInfo.InvariantCulture);
            }
            else
            {
                first = uint.Parse(range,
                    System.Globalization.NumberStyles.HexNumber,
                    System.Globalization.CultureInfo.InvariantCulture);
                last = first;
            }

            for (uint cp = first; cp <= last; ++cp)
            {
                if (Utr59.EastAsianSpacing(cp) != expected)
                {
                    ++failures;
                    if (failures <= 10)
                    {
                        Console.Error.WriteLine(
                            $"UTR59: U+{cp:X4} expected {expected} got {Utr59.EastAsianSpacing(cp)}");
                    }
                }
            }
            ++entries;
        }

        Check(entries > 1000, "east-asian-spacing.txt is populated");
        Check(failures == 0, $"UTR59 conformance failures: {failures}");

        Check(Utr59.EastAsianSpacing(0x0020) == Eas.O, "UTR59 SPACE is Other");
        Check(Utr59.EastAsianSpacing(0x4E00) == Eas.W, "UTR59 HAN is Wide");
        Check(Utr59.EastAsianSpacing(0x0041) == Eas.N, "UTR59 LATIN A is Narrow");
        Check(Utr59.EastAsianSpacing(0x0025) == Eas.C, "UTR59 PERCENT is Conditional");

        Check(Utr59.SpacingOffsets(new uint[] { 0x41, 0x6F22 }, false, false)
            .AsSpan().SequenceEqual(new[] { 1 }), "UTR59 A+Han inserts");
        Check(Utr59.SpacingOffsets(new uint[] { 0x6F22, 0x41 }, false, false)
            .AsSpan().SequenceEqual(new[] { 1 }), "UTR59 Han+A inserts");
        Check(Utr59.SpacingOffsets(new uint[] { 0x6F22, 0x3002 }, false, false).Length == 0,
            "UTR59 Han+O does not insert");
        Check(Utr59.SpacingOffsets(new uint[] { 0x6F22, 0x25 }, false, false).Length == 0,
            "UTR59 Han+percent non-Chinese does not insert");
        Check(Utr59.SpacingOffsets(new uint[] { 0x6F22, 0x25 }, true, false)
            .AsSpan().SequenceEqual(new[] { 1 }), "UTR59 Han+percent Chinese inserts");
        Check(Utr59.SpacingOffsets(new uint[] { 0x41, 0x0301, 0x6F22 }, false, false)
            .AsSpan().SequenceEqual(new[] { 2 }), "UTR59 combining mark stays in cluster");
        Check(Utr59.SpacingOffsets(new uint[] { 0x6F22, 0x20DD, 0x41 }, false, false).Length == 0,
            "UTR59 Me cluster is Other");
        Check(Utr59.SpacingOffsets(new uint[] { 0x6F22, 0x41, 0x6F22 }, false, false)
            .AsSpan().SequenceEqual(new[] { 1, 2 }), "UTR59 alternating insertions");

        Console.WriteLine($"east-asian-spacing.txt: {entries} ranges validated");
    }

    /// <summary>
    /// Parse a *BreakTest.txt file into (code points, expected boundary indices).
    /// The format interleaves a boundary marker with each code point; a ÷ at
    /// position k means a boundary before scalar k.  The line break file marks
    /// the implicit start-of-text boundary with ×.
    /// </summary>
    private static List<(uint[] Cps, int[] Boundaries)> ParseBreakTest(string path)
    {
        var cases = new List<(uint[], int[])>();
        int lineNumber = 0;

        foreach (string raw in File.ReadLines(path))
        {
            ++lineNumber;
            string data = raw.Split('#')[0].Trim();
            if (data.Length == 0) continue;

            string[] tokens = data.Split(
                (char[]?)null, StringSplitOptions.RemoveEmptyEntries);

            var cps = new List<uint>();
            var boundaries = new List<int>();
            int pos = 0;
            int idx = 0;

            if (tokens.Length > 0 && (tokens[0] == "÷" || tokens[0] == "×"))
            {
                if (tokens[0] == "÷") boundaries.Add(0);
                idx = 1;
            }

            while (idx < tokens.Length)
            {
                if (!uint.TryParse(
                        tokens[idx],
                        System.Globalization.NumberStyles.HexNumber,
                        System.Globalization.CultureInfo.InvariantCulture,
                        out uint cp))
                {
                    throw new FormatException(
                        $"{path}:{lineNumber}: invalid code point '{tokens[idx]}'");
                }
                cps.Add(cp);
                ++pos;
                ++idx;

                if (idx < tokens.Length)
                {
                    if (tokens[idx] == "÷") boundaries.Add(pos);
                    ++idx;
                }
            }

            // Every boundary type has a start-of-text boundary; the line break
            // file records it with × instead of ÷.
            if (boundaries.Count == 0 || boundaries[0] != 0) boundaries.Insert(0, 0);

            cases.Add((cps.ToArray(), boundaries.ToArray()));
        }

        return cases;
    }

    private static int[] ActualBoundaries(uint[] cps, BoundaryType boundary)
    {
        var builder = new StringBuilder(cps.Length);
        foreach (uint cp in cps)
        {
            builder.Append(new Rune(cp).ToString());
        }

        byte[] bytes = Encoding.UTF8.GetBytes(builder.ToString());
        TextSegment[] segments = Segmenter.Segment(Utf8System.Instance, bytes, boundary);

        var boundaries = new List<int> { 0 };
        int position = 0;
        foreach (TextSegment segment in segments)
        {
            position += segment.CharCount;
            boundaries.Add(position);
        }
        return boundaries.ToArray();
    }

    private static void RunConformanceSuite(string file, BoundaryType boundary)
    {
        string path = Path.Combine(FindAuxiliaryDirectory(), file);
        List<(uint[] Cps, int[] Boundaries)> cases = ParseBreakTest(path);
        Check(cases.Count > 0, $"{file} contains test cases");

        int failures = 0;
        for (int index = 0; index < cases.Count; ++index)
        {
            (uint[] cps, int[] expected) = cases[index];
            int[] actual = ActualBoundaries(cps, boundary);
            if (expected.AsSpan().SequenceEqual(actual)) continue;

            ++failures;
            if (failures <= 10)
            {
                Console.Error.WriteLine(
                    $"{file} case {index}: cps=[{string.Join(", ", Array.ConvertAll(cps, c => $"U+{c:X4}"))}]\n" +
                    $"  expected=[{string.Join(", ", expected)}]\n" +
                    $"  actual  =[{string.Join(", ", actual)}]");
            }
        }

        Check(failures == 0, $"{file}: {failures} of {cases.Count} cases failed");
    }

    private static void TestSegmentationConformance()
    {
        RunConformanceSuite("GraphemeBreakTest.txt", BoundaryType.Grapheme);
        RunConformanceSuite("WordBreakTest.txt", BoundaryType.Word);
        RunConformanceSuite("LineBreakTest.txt", BoundaryType.Line);
        RunConformanceSuite("SentenceBreakTest.txt", BoundaryType.Sentence);
    }

    private static int Main()
    {
        TestStatusAndRegistry();
        TestUtf8();
        TestAsciiAndConlang();
        TestStrings();
        TestTranscoding();
        TestCatalog();
        TestScalarProperties();
        TestNormalization();
        TestSegmentation();
        TestSegmentationConformance();
        TestNormalizationConformance();
        TestCollation();
        TestCollationConformance();
        TestUax34Conformance();
        TestUts51Conformance();
        TestUts58DetectionConformance();
        TestUts58FormattingConformance();
        TestUtr59Conformance();
        TestStatefulCodec();
        TestStatefulString();
        TestAppendBytesState();
        TestStatefulTranscode();
        TestConvertUtf();

        if (s_failures == 0)
        {
            Console.WriteLine($"Unicodoid.Tests: {s_checks} checks passed");
            return 0;
        }

        Console.Error.WriteLine($"Unicodoid.Tests: {s_failures} of {s_checks} checks failed");
        return 1;
    }
}
