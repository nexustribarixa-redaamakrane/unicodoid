path = r"C:\Users\KARIMABENDA\Documents\unicodoid\tools\gen_ucd.py"
src = open(path, encoding="utf-8").read()

# Rename ONLY the C-emitted data symbol (header + source), not the Rust/C# ones.
src = src.replace(
    'w("extern const unicodoid_ucd_pair_t unicodoid_ucd_link_bracket[];")',
    'w("extern const unicodoid_ucd_pair_t unicodoid_ucd_link_bracket_map[];")',
    1,
)
src = src.replace(
    'w("extern const size_t unicodoid_ucd_link_bracket_count;")',
    'w("extern const size_t unicodoid_ucd_link_bracket_map_count;")',
    1,
)
src = src.replace(
    's("const unicodoid_ucd_pair_t unicodoid_ucd_link_bracket[] = {")',
    's("const unicodoid_ucd_pair_t unicodoid_ucd_link_bracket_map[] = {")',
    1,
)
src = src.replace(
    's(f"const size_t unicodoid_ucd_link_bracket_count = {len(data[\'link_bracket\'])};")',
    's(f"const size_t unicodoid_ucd_link_bracket_map_count = {len(data[\'link_bracket\'])};")',
    1,
)

# Bump generator version since the emitted surface changed.
src = src.replace('GENERATOR_VERSION = "1.0.0"', 'GENERATOR_VERSION = "1.1.0"', 1)

open(path, "w", encoding="utf-8").write(src)
print("done" if "link_bracket_map_count" in src and "1.1.0" in src else "CHECK FAILED")
