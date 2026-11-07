path = r"C:\Users\KARIMABENDA\Documents\unicodoid\tools\gen_ucd.py"
src = open(path, encoding="utf-8").read()

# 1. New parse functions: insert before "def parse_collation():"
parse_block = '''
LINK_TERM_VALUES = {"Include": 1, "Hard": 2, "Soft": 3, "Close": 4, "Open": 5}
EAST_ASIAN_SPACING_VALUES = {"W": 1, "N": 2, "O": 3, "C": 4}


def parse_link_term():
    arr = bytearray(2 for _ in range(MAX_CP))
    for line in data_lines(DATA / "linkification" / "LinkTerm.txt"):
        fields = [f.strip() for f in line.split(";")]
        if len(fields) < 2:
            continue
        a, b = parse_range(fields[0])
        v = LINK_TERM_VALUES.get(fields[1])
        if v is None:
            continue
        for cp in range(a, b + 1):
            arr[cp] = v
    return arr


def parse_link_bracket():
    entries = []
    for line in data_lines(DATA / "linkification" / "LinkBracket.txt"):
        fields = [f.strip() for f in line.split(";")]
        if len(fields) < 2:
            continue
        a = int(fields[0], 16)
        b = int(fields[1], 16)
        entries.append((a, b))
    entries.sort(key=lambda e: e[0])
    return entries


def parse_link_email():
    arr = bytearray(MAX_CP)
    for line in data_lines(DATA / "linkification" / "LinkEmail.txt"):
        fields = line.split(";")
        a, b = parse_range(fields[0])
        for cp in range(a, b + 1):
            arr[cp] = 1
    return arr


def parse_east_asian_spacing():
    arr = bytearray(3 for _ in range(MAX_CP))  # missing values default to O (3)
    for line in data_lines(DATA / "tr59" / "east-asian-spacing.txt"):
        fields = [f.strip() for f in line.split(";")]
        if len(fields) < 2:
            continue
        a, b = parse_range(fields[0])
        v = EAST_ASIAN_SPACING_VALUES.get(fields[1])
        if v is None:
            continue
        for cp in range(a, b + 1):
            arr[cp] = v
    return arr


def parse_collation():'''
src = src.replace("def parse_collation():", parse_block, 1)

# 2. main(): add parsing calls after emoji_sequences = parse_emoji_sequences()
src = src.replace(
    """    named_sequences = parse_named_sequences()
    emoji_sequences = parse_emoji_sequences()
""",
    """    named_sequences = parse_named_sequences()
    emoji_sequences = parse_emoji_sequences()
    link_term = parse_link_term()
    link_email = parse_link_email()
    east_asian_spacing = parse_east_asian_spacing()
    link_bracket = parse_link_bracket()
""",
)

# 3. ranges for new tables
src = src.replace(
    """    for name, arr in binary.items():
        ranges["prop_" + name.lower()] = arr
""",
    """    ranges["link_term_range"] = link_term
    ranges["link_email_range"] = link_email
    ranges["east_asian_spacing_range"] = east_asian_spacing
    for name, arr in binary.items():
        ranges["prop_" + name.lower()] = arr
""",
)

# 4. pool additions for named/emoji sequences: after the collation multi block
src = src.replace(
    """        off_seq = pool_add(pool, list(seq))
        collation_multi.append(((len(seq) << 24) | off_seq, off, len(weights)))
""",
    """        off_seq = pool_add(pool, list(seq))
        collation_multi.append(((len(seq) << 24) | off_seq, off, len(weights)))

    named_seq_entries = []
    for name, seq in named_sequences:
        off = pool_add(pool, list(seq))
        named_seq_entries.append((name, off, len(seq)))

    emoji_seq_entries = []
    for seq, typ in emoji_sequences:
        off = pool_add(pool, list(seq))
        emoji_seq_entries.append((typ, off, len(seq)))
""",
)

# 5. data dict: add entries
src = src.replace(
    """        "named_sequences": [
            {"name": name, "sequence": seq} for name, seq in named_sequences
        ],
        "emoji_sequences": [
            {"sequence": seq, "type": typ} for seq, typ in emoji_sequences
        ],
    }""",
    """        "named_sequences": [
            {"name": name, "sequence": seq} for name, seq in named_sequences
        ],
        "emoji_sequences": [
            {"sequence": seq, "type": typ} for seq, typ in emoji_sequences
        ],
        "link_bracket": link_bracket,
        "named_seq_entries": named_seq_entries,
        "emoji_seq_entries": emoji_seq_entries,
    }""",
)

open(path, "w", encoding="utf-8").write(src)
print("stage1 ok")
