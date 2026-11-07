path = r"C:\Users\KARIMABENDA\Documents\unicodoid\tools\gen_ucd.py"
src = open(path, encoding="utf-8").read()

# ---------- emit_c: header ----------
src = src.replace(
    '    w("typedef struct { uint32_t cp; int64_t numerator; int64_t denominator; int64_t digit; } unicodoid_ucd_numeric_t;")',
    '''    w("typedef struct { uint32_t cp; int64_t numerator; int64_t denominator; int64_t digit; } unicodoid_ucd_numeric_t;")
    w("typedef struct { uint32_t offset; uint16_t length; } unicodoid_ucd_named_seq_t;")''',
    1,
)
src = src.replace(
    '''    for name in ("decomp", "upper_full", "lower_full", "title_full", "fold_full", "confusables", "idna_mappings"):
        w(f"extern const unicodoid_ucd_map_t unicodoid_ucd_{name}[];")
        w(f"extern const size_t unicodoid_ucd_{name}_count;")''',
    '''    w("extern const unicodoid_ucd_pair_t unicodoid_ucd_link_bracket[];")
    w("extern const size_t unicodoid_ucd_link_bracket_count;")
    for name in ("decomp", "upper_full", "lower_full", "title_full", "fold_full", "confusables", "idna_mappings"):
        w(f"extern const unicodoid_ucd_map_t unicodoid_ucd_{name}[];")
        w(f"extern const size_t unicodoid_ucd_{name}_count;")
    w("extern const char* const unicodoid_ucd_named_seq_names[];")
    w("extern const size_t unicodoid_ucd_named_seq_count;")
    w("extern const unicodoid_ucd_named_seq_t unicodoid_ucd_named_seq_index[];")
    w("extern const char* const unicodoid_ucd_emoji_seq_types[];")
    w("extern const size_t unicodoid_ucd_emoji_seq_count;")
    w("extern const unicodoid_ucd_named_seq_t unicodoid_ucd_emoji_seq_index[];")''',
    1,
)

# ---------- emit_c: source ----------
old_anchor = '''    s("const unicodoid_ucd_seq_t unicodoid_ucd_collation_single[] = {")'''
new_block = '''    s("const unicodoid_ucd_pair_t unicodoid_ucd_link_bracket[] = {")
    for a, b in data["link_bracket"]:
        s(f"{{0x{a:X},0x{b:X}}},")
    s("};")
    s(f"const size_t unicodoid_ucd_link_bracket_count = {len(data['link_bracket'])};")
    s("")

    def c_str(text):
        assert chr(92) not in text, text
        assert '"' not in text, text
        return text

    s("const char* const unicodoid_ucd_named_seq_names[] = {")
    for name, _off, _len in data["named_seq_entries"]:
        s(f'"{c_str(name)}",')
    s("};")
    s(f"const size_t unicodoid_ucd_named_seq_count = {len(data['named_seq_entries'])};")
    s("")
    s("const unicodoid_ucd_named_seq_t unicodoid_ucd_named_seq_index[] = {")
    for _name, off, length in data["named_seq_entries"]:
        s(f"{{0x{off:X},{length}}},")
    s("};")
    s("")
    s("const char* const unicodoid_ucd_emoji_seq_types[] = {")
    for typ, _off, _len in data["emoji_seq_entries"]:
        s(f'"{c_str(typ)}",')
    s("};")
    s(f"const size_t unicodoid_ucd_emoji_seq_count = {len(data['emoji_seq_entries'])};")
    s("")
    s("const unicodoid_ucd_named_seq_t unicodoid_ucd_emoji_seq_index[] = {")
    for _typ, off, length in data["emoji_seq_entries"]:
        s(f"{{0x{off:X},{length}}},")
    s("};")
    s("")

    s("const unicodoid_ucd_seq_t unicodoid_ucd_collation_single[] = {")'''
assert old_anchor in src
src = src.replace(old_anchor, new_block, 1)

# ---------- emit_rust ----------
old_r = """    w("pub static COMPOSITION: &[Comp] = &[")"""
new_r = """    w('pub static LINK_BRACKET: &[Pair] = &[')
    for a, b in data["link_bracket"]:
        w(f"({a}, {b}),")
    w("];")
    w("")
    w('pub static NAMED_SEQ_NAMES: &[&str] = &[')
    for name, _off, _len in data["named_seq_entries"]:
        w(f'"{name}",')
    w("];")
    w('pub static NAMED_SEQ_INDEX: &[(u32, u16)] = &[')
    for _name, off, length in data["named_seq_entries"]:
        w(f"({off}, {length}),")
    w("];")
    w("")
    w('pub static EMOJI_SEQ_TYPES: &[&str] = &[')
    for typ, _off, _len in data["emoji_seq_entries"]:
        w(f'"{typ}",')
    w("];")
    w('pub static EMOJI_SEQ_INDEX: &[(u32, u16)] = &[')
    for _typ, off, length in data["emoji_seq_entries"]:
        w(f"({off}, {length}),")
    w("];")
    w("")
    w("pub static COMPOSITION: &[Comp] = &[")"""
assert old_r in src
src = src.replace(old_r, new_r, 1)

# ---------- emit_csharp ----------
old_cs = '''    emit_table(
        "Comp",
        "Composition",
        data["composition"],
        lambda e: f"new({e[0]}u, {e[1]}u, {e[2]}u)",
    )'''
new_cs = '''    emit_table(
        "Comp",
        "Composition",
        data["composition"],
        lambda e: f"new({e[0]}u, {e[1]}u, {e[2]}u)",
    )

    emit_table(
        "Pair",
        "LinkBracket",
        data["link_bracket"],
        lambda e: f"new({e[0]}u, {e[1]}u)",
    )

    def csharp_str(text):
        assert chr(92) not in text, text
        assert '"' not in text, text
        return text

    w("    public static readonly string[] NamedSeqNames = new string[]")
    w("    {")
    for name, _off, _len in data["named_seq_entries"]:
        w(f'        "{csharp_str(name)}",')
    w("    };")
    w("")
    w("    public static readonly (uint, ushort)[] NamedSeqIndex = new (uint, ushort)[]")
    w("    {")
    for _name, off, length in data["named_seq_entries"]:
        w(f"        ({off}u, {length}),")
    w("    };")
    w("")
    w("    public static readonly string[] EmojiSeqTypes = new string[]")
    w("    {")
    for typ, _off, _len in data["emoji_seq_entries"]:
        w(f'        "{csharp_str(typ)}",')
    w("    };")
    w("")
    w("    public static readonly (uint, ushort)[] EmojiSeqIndex = new (uint, ushort)[]")
    w("    {")
    for _typ, off, length in data["emoji_seq_entries"]:
        w(f"        ({off}u, {length}),")
    w("    };")
    w("")'''
assert old_cs in src
src = src.replace(old_cs, new_cs, 1)

open(path, "w", encoding="utf-8").write(src)
print("stage2 ok")
