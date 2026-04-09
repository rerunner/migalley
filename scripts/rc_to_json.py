#!/usr/bin/env python3
import re
import json
import sys
from pathlib import Path

# ------------------------------------------------------------
# Utility: basic comment stripping and line normalization
# ------------------------------------------------------------

def strip_line_comments(line: str) -> str:
    # Remove // comments
    line = re.sub(r'//.*$', '', line)
    # Remove /* ... */ comments (single-line)
    line = re.sub(r'/\*.*?\*/', '', line)
    return line.rstrip('\n')

def safe_int(s, context=None):
    s = s.strip()
    if not s: return 0
    # RERUN: Strip common Win32 RC suffixes like 'L' (long) or 'U' (unsigned)
    s = re.sub(r'([0-9A-Fa-f])[LUlu]+$', r'\1', s)
    # Evaluate simple math (e.g. 15+10) or hex strings
    try: 
        ns = {"__builtins__": None}
        if context: ns.update(context)
        return int(eval(s, ns, {}))
    except: return 0

def normalize_whitespace(s: str) -> str:
    return re.sub(r'\s+', ' ', s).strip()

# ------------------------------------------------------------
# Parse RESOURCE.H: collect #define NAME NUMBER
# ------------------------------------------------------------

def parse_resource_header(path: Path):
    name_to_id = {}
    id_to_name = {}
    # RERUN: Match signed integers and hex values
    define_re = re.compile(r'#define\s+(\w+)\s+(-?0x[0-9A-Fa-f]+|-?\d+)')
    with path.open(encoding='latin-1', errors='ignore') as f:
        for raw in f:
            line = strip_line_comments(raw)
            m = define_re.match(line)
            if not m:
                continue
            name = m.group(1)
            try:
                num = int(m.group(2), 0) # Handles decimal and hex
            except ValueError: continue
            name_to_id[name] = num
            # Prefer first name for a given ID, but keep list if needed
            if num not in id_to_name:
                id_to_name[num] = name
            else:
                # RERUN FIX: If we have an ID collision, prefer the IDS_ or FIL_ version.
                # This ensures that we correctly identify and mask these IDs in DLGINIT blocks.
                cur_name = id_to_name[num]
                if not (cur_name.startswith("IDS_") or cur_name.startswith("FIL_")):
                    if name.startswith("IDS_") or name.startswith("FIL_"):
                        id_to_name[num] = name
    return name_to_id, id_to_name

# ------------------------------------------------------------
# Token helpers for RC parsing
# ------------------------------------------------------------

def parse_quoted_string(s: str):
    # Handles "..." with \" inside
    m = re.match(r'"((?:[^"\\]|\\.)*)"', s)
    if not m:
        return None, s
    text = m.group(1)
    rest = s[m.end():]
    # Unescape \" and \\ etc.
    text = bytes(text, 'utf-8').decode('unicode_escape')
    return text, rest.lstrip()

def parse_int(s: str):
    m = re.match(r'(-?\d+)', s)
    if not m:
        return None, s
    return int(m.group(1)), s[m.end():].lstrip()

def parse_identifier(s: str):
    m = re.match(r'\s*([A-Za-z_]\w*)', s)
    if not m:
        return None, s
    return m.group(1), s[m.end():].lstrip()

def split_commas(s: str):
    # Split by commas but keep quoted strings intact
    parts = []
    current = []
    in_quote = False
    escape = False
    for ch in s:
        if escape:
            current.append(ch)
            escape = False
        elif ch == '\\':
            current.append(ch)
            escape = True
        elif ch == '"':
            current.append(ch)
            in_quote = not in_quote
        elif ch == ',' and not in_quote:
            part = ''.join(current).strip()
            if part:
                parts.append(part)
            current = []
        else:
            current.append(ch)
    if current:
        part = ''.join(current).strip()
        if part:
            parts.append(part)
    return parts

# ------------------------------------------------------------
# STRINGTABLE parsing
# ------------------------------------------------------------

def parse_stringtable_block(lines, idx, name_to_id, id_to_name):
    """
    lines[idx] is the line with 'STRINGTABLE' or 'STRINGTABLE ...'
    Returns (block_dict, new_idx)
    """
    block = {
        "type": "STRINGTABLE",
        "attributes": [],
        "entries": []
    }

    # Collect attributes on same line (e.g. STRINGTABLE DISCARDABLE)
    header = normalize_whitespace(strip_line_comments(lines[idx]))
    header_rest = header[len("STRINGTABLE"):].strip()
    if header_rest:
        block["attributes"] = header_rest.split()

    idx += 1
    # Expect BEGIN
    while idx < len(lines):
        line = strip_line_comments(lines[idx]).strip()
        if not line:
            idx += 1
            continue
        if line.upper().startswith("BEGIN"):
            idx += 1
            break
        # Some RCs put BEGIN on same line; handle that too
        if "BEGIN" in line.upper():
            idx += 1
            break
        idx += 1

    # Now parse entries until END
    while idx < len(lines):
        raw = lines[idx]
        line = strip_line_comments(raw).strip()
        if not line:
            idx += 1
            continue
        if line.upper().startswith("END"):
            idx += 1
            break

        # Typical forms:
        #   IDS_SOMETHING "Text"
        #   123, "Text"
        #   IDS_SOMETHING, "Text"
        # Possibly multiple per line, but Mig Alley likely uses one per line.
        # We'll handle one per line robustly.
        parts = split_commas(line)
        if len(parts) >= 2:
            key = parts[0].strip()
            text_part = ','.join(parts[1:]).strip()
        else:
            # Maybe: IDS_SOMETHING "Text" (no comma)
            m = re.match(r'(\S+)\s+(.*)$', line)
            if not m:
                idx += 1
                continue
            key = m.group(1)
            text_part = m.group(2).strip()

        # Parse key: could be numeric or identifier
        try:
            key_id = int(key)
            key_name = id_to_name.get(key_id)
        except ValueError:
            key_name = key
            key_id = name_to_id.get(key_name)

        text, _ = parse_quoted_string(text_part)
        entry = {
            "id": key_id,
            "name": key_name,
            "raw_key": key,
            "text": text
        }
        block["entries"].append(entry)
        idx += 1

    return block, idx

# ------------------------------------------------------------
# DIALOG / DIALOGEX parsing
# ------------------------------------------------------------

CONTROL_KEYWORDS = {
    "PUSHBUTTON",
    "DEFPUSHBUTTON",
    "LTEXT",
    "RTEXT",
    "CTEXT",
    "GROUPBOX",
    "EDITTEXT",
    "COMBOBOX",
    "LISTBOX",
    "SCROLLBAR",
    "ICON",
    "CONTROL",
    "CHECKBOX",
    "AUTOCHECKBOX",
    "RADIOBUTTON",
    "AUTORADIOBUTTON",
    "STATE3",
    "AUTOCHECKBOX",
}

def parse_dialog_header(line: str, name_to_id=None, id_to_name=None):
    """
    Examples:
      IDD_ABOUTBOX DIALOG 0, 0, 186, 95
      IDD_ABOUTBOX DIALOGEX 0, 0, 186, 95
      100 DIALOGEX 0, 0, 186, 95
    """
    # Strip comments and normalize a bit, but don't rely on splitting by spaces for numbers
    line = normalize_whitespace(strip_line_comments(line))

    m = re.match(r'^\s*([A-Za-z0-9_]+),?\s+(DIALOGEX|DIALOG)\s*(.*)$', line, re.IGNORECASE)
    if not m:
        return None

    name = m.group(1) # [A-Za-z0-9_]+ ensures name is captured without the comma
    dlg_type = m.group(2).upper()
    rest = m.group(3).strip()

    # RERUN: Robust coordinate extraction. Dialog headers can have attributes 
    # (DISCARDABLE, PRELOAD) anywhere. We look for the 4 numeric tokens.
    raw_tokens = []
    for t in split_commas(rest):
        # Sub-split by space to separate attributes from numbers
        raw_tokens.extend(t.split())
    
    numeric_tokens = []
    for t in raw_tokens:
        t = t.strip()
        # Check if it looks like a number, hex, or simple expression
        if t and (t[0].isdigit() or t[0] in ('-', '(', '0')):
            numeric_tokens.append(t)

    parts = numeric_tokens

    if len(parts) >= 4:
        x = safe_int(parts[0], name_to_id)
        y = safe_int(parts[1], name_to_id)
        w = safe_int(parts[2], name_to_id)
        h = safe_int(parts[3], name_to_id)
    else:
        x = y = w = h = None

    return {
        "name": name,
        "dialog_type": dlg_type,
        "x": x,
        "y": y,
        "width": w,
        "height": h,
    }

def parse_dialog_block(lines, idx, name_to_id, id_to_name):
    """
    lines[idx] is the line with '... DIALOG...' or '... DIALOGEX...'
    Returns (dialog_dict, new_idx)
    """
    header_line = strip_line_comments(lines[idx])
    dlg_info = parse_dialog_header(header_line, name_to_id, id_to_name)
    if not dlg_info:
        # Fallback: store raw
        dlg = {
            "type": "DIALOG",
            "raw_header": header_line,
            "properties": {},
            "controls": []
        }
    else:
        dlg = {
            "type": "DIALOG",
            "name": dlg_info["name"],
            "id": None,
            "numeric_id": None,
            "dialog_type": dlg_info["dialog_type"],
            "rect": [
                dlg_info["x"] if dlg_info["x"] is not None else 0,
                dlg_info["y"] if dlg_info["y"] is not None else 0,
                dlg_info["width"] if dlg_info["width"] is not None else 0,
                dlg_info["height"] if dlg_info["height"] is not None else 0
            ],
            "properties": {},
            "controls": []
        }
        # Map name to numeric ID if possible
        try:
            numeric = int(dlg_info["name"])
            dlg["numeric_id"] = numeric
            dlg["id"] = id_to_name.get(numeric)
        except ValueError:
            dlg["id"] = dlg_info["name"]
            dlg["numeric_id"] = name_to_id.get(dlg_info["name"])

    idx += 1
    # Expect BEGIN
    while idx < len(lines):
        line = strip_line_comments(lines[idx]).strip()
        if not line:
            idx += 1
            continue
        if line.upper().startswith("BEGIN"):
            idx += 1
            break
        if "BEGIN" in line.upper():
            idx += 1
            break
        idx += 1

    # Now parse dialog contents until END
        # Now parse dialog contents until END
    while idx < len(lines):
        raw = lines[idx]
        line = strip_line_comments(raw).strip()
        if not line:
            idx += 1
            continue
        if line.upper().startswith("END"):
            idx += 1
            break

        upper = line.upper().strip()

        # Properties: STYLE, EXSTYLE, CAPTION, FONT, CLASS, MENU, LANGUAGE, etc.
        if upper.startswith("STYLE"):
            dlg["properties"]["STYLE"] = line.strip()[len("STYLE"):].strip()
            idx += 1
            continue
        elif upper.startswith("EXSTYLE"):
            dlg["properties"]["EXSTYLE"] = line.strip()[len("EXSTYLE"):].strip()
            idx += 1
            continue
        elif upper.startswith("CAPTION"):
            text, _ = parse_quoted_string(line.strip()[len("CAPTION"):].strip())
            dlg["properties"]["CAPTION"] = text
            idx += 1
            continue
        elif upper.startswith("FONT"):
            rest = line[len("FONT"):].strip()
            parts = split_commas(rest)
            font = {}
            if parts:
                try:
                    font["size"] = int(parts[0])
                except ValueError:
                    font["size"] = parts[0]
            if len(parts) >= 2:
                face, _ = parse_quoted_string(parts[1].strip())
                font["face"] = face
            if len(parts) >= 3:
                font["weight"] = parts[2].strip()
            if len(parts) >= 4:
                font["italic"] = parts[3].strip()
            if len(parts) >= 5:
                font["charset"] = parts[4].strip()
            dlg["properties"]["FONT"] = font
            idx += 1
            continue
        elif upper.startswith("CLASS"):
            rest = line[len("CLASS"):].strip()
            if rest.startswith('"'):
                cls, _ = parse_quoted_string(rest)
            else:
                cls = rest
            dlg["properties"]["CLASS"] = cls
            idx += 1
            continue
        elif upper.startswith("MENU"):
            dlg["properties"]["MENU"] = line[len("MENU"):].strip()
            idx += 1
            continue
        elif upper.startswith("LANGUAGE"):
            dlg["properties"]["LANGUAGE"] = line[len("LANGUAGE"):].strip()
            idx += 1
            continue
        elif upper.startswith("LEFTMARGIN"):
            rest = line[len("LEFTMARGIN"):].strip().lstrip(',').strip()
            if "rect" not in dlg or not dlg["rect"]:
                dlg["rect"] = [None, None, None, None]
            dlg["rect"][0] = safe_int(rest, name_to_id)
            idx += 1
            continue
        elif upper.startswith("TOPMARGIN"):
            rest = line[len("TOPMARGIN"):].strip().lstrip(',').strip()
            if "rect" not in dlg or not dlg["rect"]:
                dlg["rect"] = [None, None, None, None]
            dlg["rect"][1] = safe_int(rest, name_to_id)
            idx += 1
            continue
        elif upper.startswith("RIGHTMARGIN"):
            rest = line[len("RIGHTMARGIN"):].strip().lstrip(',').strip()
            if "rect" not in dlg or not dlg["rect"]:
                dlg["rect"] = [None, None, None, None]
            dlg["rect"][2] = safe_int(rest, name_to_id)
            idx += 1
            continue
        elif upper.startswith("BOTTOMMARGIN"):
            rest = line[len("BOTTOMMARGIN"):].strip().lstrip(',').strip()
            if "rect" not in dlg or not dlg["rect"]:
                dlg["rect"] = [None, None, None, None]
            dlg["rect"][3] = safe_int(rest, name_to_id)
            idx += 1
            continue

        # Controls: handle multi-line CONTROL / PUSHBUTTON / etc.
        keyword = line.split()[0]
        if keyword.upper() in CONTROL_KEYWORDS:
            # Build a logical line by joining continuation lines
            logical = line
            j = idx + 1
            while j < len(lines):
                next_raw = lines[j]
                next_line = strip_line_comments(next_raw).strip()
                if not next_line:
                    j += 1
                    continue
                # RERUN: Join lines if the current ends with a separator OR if the next starts with a pipe
                if logical.rstrip().endswith(',') or logical.rstrip().endswith('|') or next_line.startswith('|'):
                    logical += " " + next_line
                    j += 1
                    continue
                break

            ctrl = parse_control_line(logical, name_to_id, id_to_name)
            dlg["controls"].append(ctrl)
            idx = j
            continue

        # Unknown line inside dialog; keep raw
        dlg.setdefault("other_lines", []).append(line)
        idx += 1

    return dlg, idx

def parse_control_line(line: str, name_to_id, id_to_name):
    """
    Parse a single control line inside a dialog.
    Handles:
      PUSHBUTTON "Text", id, x, y, w, h, style, exstyle
      LTEXT "Text", id, x, y, w, h, style
      CONTROL "Text", id, "Class", style, x, y, w, h, exstyle
      ICON id, x, y, w, h, style
    We keep any extra trailing tokens as raw.
    """
    original = line

    keyword, rest = parse_identifier(line)
    ctrl = {
        "raw": original,
        "keyword": keyword,
        "text": None,
        "id": None,
        "name": None,
        "class": None,
        "rect": None,
        "style": None,
        "exstyle": None,
        "extra": None,
    }

    parts = split_commas(rest)

    # CONTROL is special: CONTROL "Text", id, "Class", style, x, y, w, h, exstyle
    if keyword.upper() == "CONTROL":
        if len(parts) < 5:
            ctrl["extra"] = parts
            return ctrl
        # text
        text, _ = parse_quoted_string(parts[0].strip()) if parts[0].strip().startswith('"') else (None, "")
        ctrl["text"] = text
        # id
        id_token = parts[1].strip()
        ctrl["name"] = id_token
        # Handle symbolic arithmetic (e.g. IDC_STATIC+1) which int() fails on
        if '+' in id_token:
             id_token = id_token.split('+')[0].strip()
        try:
            num = int(id_token)
            ctrl["id"] = num
            ctrl["name"] = id_to_name.get(num, id_token)
        except ValueError:
            ctrl["id"] = name_to_id.get(id_token)
        # class
        cls_token = parts[2].strip()
        if cls_token.startswith('"'):
            cls, _ = parse_quoted_string(cls_token)
        else:
            cls = cls_token
        ctrl["class"] = cls
        # style
        ctrl["style"] = parts[3].strip()
        
        # RERUN: Robust rect detection for CONTROL. Coordinates are 4 integers.
        # They might be merged into the style part or separated by commas.
        all_sub_parts = []
        for p in parts[3:]:
            sub = re.split(r'[\s\|]+', p)
            all_sub_parts.extend([s.strip() for s in sub if s.strip()])
        
        numeric_indices = []
        for i, p in enumerate(all_sub_parts):
            if p and (p[0].isdigit() or p[0] in ('-', '(')):
                numeric_indices.append(i)

        if len(numeric_indices) >= 4:
            idx = numeric_indices[0] # Use the first sequence of numbers found
            x = safe_int(all_sub_parts[idx], name_to_id)
            y = safe_int(all_sub_parts[idx+1], name_to_id)
            w = safe_int(all_sub_parts[idx+2], name_to_id)
            h = safe_int(all_sub_parts[idx+3], name_to_id)
            ctrl["rect"] = [x, y, w, h]
        # exstyle
        if len(parts) >= 9:
            ctrl["exstyle"] = parts[8].strip()
        if len(parts) > 9:
            ctrl["extra"] = parts[9:]
        return ctrl

    # ICON can be: ICON id, x, y, w, h, style
    if keyword.upper() == "ICON":
        if len(parts) >= 1:
            id_token = parts[0].strip()
            ctrl["name"] = id_token
            try:
                num = int(id_token)
                ctrl["id"] = num
                ctrl["name"] = id_to_name.get(num, id_token)
            except ValueError:
                ctrl["id"] = name_to_id.get(id_token)
        if len(parts) >= 5:
            try:
                x = int(parts[1]); y = int(parts[2])
                w = int(parts[3]); h = int(parts[4])
                ctrl["rect"] = [x, y, w, h]
            except ValueError:
                ctrl["rect"] = [parts[1], parts[2], parts[3], parts[4]]
        if len(parts) >= 6:
            ctrl["style"] = parts[5].strip()
        if len(parts) > 6:
            ctrl["extra"] = parts[6:]
        return ctrl

    # Generic: KEYWORD "Text", id, x, y, w, h, [style, exstyle...]
    if parts:
        # text
        first = parts[0].strip()
        if first.startswith('"'):
            text, _ = parse_quoted_string(first)
            ctrl["text"] = text
            idx = 1
        else:
            text = None
            idx = 0

        # id
        if idx < len(parts):
            id_token = parts[idx].strip()
            ctrl["name"] = id_token
            try:
                num = int(id_token)
                ctrl["id"] = num
                ctrl["name"] = id_to_name.get(num, id_token)
            except ValueError:
                ctrl["id"] = name_to_id.get(id_token)
            idx += 1

        # rect
        if idx + 3 < len(parts):
            # RERUN: Use safe_int to evaluate expressions/constants in coordinates
            x = safe_int(parts[idx], name_to_id)
            y = safe_int(parts[idx+1], name_to_id)
            w = safe_int(parts[idx+2], name_to_id)
            h = safe_int(parts[idx+3], name_to_id)
            ctrl["rect"] = [x, y, w, h]
            idx += 4

        # style
        if idx < len(parts):
            ctrl["style"] = parts[idx].strip()
            idx += 1

        # exstyle
        if idx < len(parts):
            ctrl["exstyle"] = parts[idx].strip()
            idx += 1

        if idx < len(parts):
            ctrl["extra"] = parts[idx:]

    return ctrl

def _try_parse_button_metadata(words, name_to_id, id_to_name):
    """
    Attempts to parse the specific binary structure for Rowan's custom buttons.
    Returns a metadata dictionary or None.
    """
    try:
        # Signature check for this specific control type
        if not (len(words) > 20 and words[0] in (0x0021, 0x0020) and words[1] == 0x0000):
            return None

        metadata = {}

        # --- Find the 0xFFFFFFFF sentinel, which is a very strong anchor ---
        sentinel_pos = -1
        for i in range(len(words) - 1):
            if words[i] == 0xffff and words[i+1] == 0xffff:
                sentinel_pos = i
                break

        if sentinel_pos != -1 and sentinel_pos + 6 <= len(words):
            # --- Parse data from the sentinel ---
            metadata['sentinel'] = sentinel_pos
            icon_enum = words[sentinel_pos + 2] | (words[sentinel_pos + 3] << 16)
            metadata['icon_enum'] = icon_enum

            # Attempt to find the icon name from the enum value using the header defines
            icon_name_from_enum = id_to_name.get(icon_enum)
            if icon_name_from_enum:
                metadata['icon_name'] = icon_name_from_enum

            metadata['data_version'] = words[sentinel_pos + 4] | (words[sentinel_pos + 5] << 16)

            # --- Find the start of the fixed-field data after the copyright string ---
            # The sequence 0x0001, 0x0001 seems to be a reliable marker.
            fields_start_pos = -1
            # The copyright string is typically 34 chars, starting at index 2.
            # So the fields should start around index 36.
            for i in range(30, len(words) - 8): # Search after typical copyright area
                if words[i] == 0x0001 and words[i+1] == 0x0001:
                    # Check if it's followed by a plausible structure to be more certain
                    fields_start_pos = i
                    break

            if fields_start_pos != -1:
                pos = fields_start_pos
                metadata['flags1'] = words[pos]
                metadata['flags2'] = words[pos+1]
                pos += 2
                metadata['value1'] = words[pos] | (words[pos+1] << 16)
                pos += 2
                metadata['value2'] = words[pos] | (words[pos+1] << 16)
                pos += 2
                metadata['frame_count'] = words[pos] | (words[pos+1] << 16)

        # --- Construct byte stream to search for FIL_ICON string ---
        # This is useful if sentinel is missing OR to correct/verify the name
        le_bytes = bytearray()
        for w in words:
            le_bytes.extend([w & 0xFF, (w >> 8) & 0xFF])

        # Search for a pattern like `b'FIL_ICON_...'`
        found_match = None
        # RERUN: Collect all matches
        matches = list(re.finditer(b'FIL_[A-Z0-9_]+', le_bytes))
        
        # Prefer matches containing "ICON"
        for m in matches:
            found_str = m.group(0).decode('ascii')
            if 'ICON' in found_str:
                found_match = m
                metadata['icon_name'] = found_str
                break
        
        # Fallback to first FIL_ match if no ICON string found
        if not found_match and matches:
            found_match = matches[0]
            metadata['icon_name'] = found_match.group(0).decode('ascii')
        
        # If sentinel was missing but we found the string, try to recover the enum
        if 'icon_enum' not in metadata and found_match:
            # First, check if name matches a known ID
            if metadata['icon_name'] in name_to_id:
                metadata['icon_enum'] = name_to_id[metadata['icon_name']]
            else:
                # Fallback: Extract from byte stream relative to string start
                # Pattern observed: [enum_lo] [enum_hi] [00] [03] [00] [00] [00] [len] "FIL_..."
                # Bytes at match.start() - 9 and - 8 correspond to the enum.
                start_idx = found_match.start()
                if start_idx >= 9:
                    # Heuristic check: does it look like the pattern? (Type byte at -5)
                    # The type byte is usually 0x03, but can be 0x01, 0x02 etc.
                    if 0 < le_bytes[start_idx-5] < 0x10:
                         enum_val = le_bytes[start_idx-9] | (le_bytes[start_idx-8] << 8)
                         if enum_val > 0:
                             metadata['icon_enum'] = enum_val

        # If we have at least an icon_enum or an icon_name, consider it a valid metadata block
        if metadata.get('icon_enum') or metadata.get('icon_name'):
            return metadata

        return None

    except (IndexError, TypeError, ValueError):
        return None

def join_logical_lines(lines):
    """
    Join RC continuation lines into single logical lines.
    Rules:
      - If a line ends with ',' → continuation
      - If next line starts with '"', '{', digit, or identifier → continuation
    """
    out = []
    buf = ""

    def starts_continuation(s):
        s = s.lstrip()
        return (
            s.startswith('"') or
            s.startswith('{') or
            re.match(r'^[A-Za-z_]\w*', s) or
            re.match(r'^\d', s)
        )

    for raw in lines:
        line = strip_line_comments(raw).rstrip()

        if not buf:
            buf = line
        else:
            # If previous line ends with comma → continuation
            if buf.rstrip().endswith(','):
                buf += " " + line
                continue
            # If this line looks like a continuation → join
            if starts_continuation(line):
                buf += " " + line
                continue
            # Otherwise flush
            out.append(buf)
            buf = line

    if buf:
        out.append(buf)

    return out

def decode_dlginitt_dispatch(ctrl_name, msg, data_len, flags, words, name_to_id, id_to_name):
    # RERUN: Rowan controls (0x376) usually have packed metadata even without 0x21/20 signature
    # Check for signature OR check if message is standard Rowan init (0x376)
    if msg == "0x376" or (len(words) >= 2 and words[0] in (0x0021, 0x0020) and words[1] == 0x0000):
        return decode_full_dlginitt_entry(ctrl_name, msg, data_len, flags, words, name_to_id, id_to_name)
    else:
        return decode_simple_dlginitt_entry(ctrl_name, msg, data_len, flags, words, name_to_id, id_to_name)

def parse_dlginitt_block(lines, idx, name_to_id, id_to_name):
    """
    Parses a DLGINIT block.
    A DLGINIT block contains initialization data for controls in a dialog.
    It can have multiple entries, each for a different control.

    Example:
    IDD_MYDIALOG DLGINIT
    BEGIN
        IDC_MYCONTROL, 0x376, 120, 0
        0x0021, 0x0000, ... // data words
    END
    """
    header = strip_line_comments(lines[idx]).strip()
    tokens = header.split()
    name = tokens[0]  # e.g. IDD_MAPFILTERS

    dlginit_block = {
        "type": "DLGINIT",
        "name": name,
        "numeric_id": name_to_id.get(name),
        "entries": []
    }

    idx += 1
    # Find BEGIN
    while idx < len(lines):
        line = strip_line_comments(lines[idx]).strip()
        if not line:
            idx += 1
            continue
        if line.upper().startswith("BEGIN"):
            idx += 1
            break
        idx += 1

    # Parse entries until END
    while idx < len(lines):
        entry_line = strip_line_comments(lines[idx]).strip()
        if not entry_line:
            idx += 1
            continue
        if entry_line.upper().startswith("END"):
            idx += 1
            break

        # First line of entry: CONTROL_ID, msg, len, flags
        parts = split_commas(entry_line)
        if len(parts) < 4:
            idx += 1
            continue

        ctrl_name = parts[0].strip()
        msg = parts[1].strip()
        try:
            data_len = int(parts[2], 0)
            flags = int(parts[3], 0)
        except ValueError:
            # Handle cases where these might not be numbers
            idx += 1
            continue

        # Collect data words
        data_words = []
        idx += 1
        while idx < len(lines):
            l2 = strip_line_comments(lines[idx]).strip()
            if not l2 or l2.upper().startswith("//"):
                idx += 1
                continue
            # Stop if next entry or END
            # A new entry starts with an identifier, a comma, and then "0x"
            if re.match(r'^[A-Za-z_]\w*,\s*0x', l2) or l2.upper().startswith("END"):
                break

            # Parse hex words
            for w in l2.split(','):
                w = w.strip()
                if w:
                    try:
                        data_words.append(int(w, 16))
                    except ValueError:
                        pass # Ignore non-hex values
            idx += 1

        entry = decode_dlginitt_dispatch(ctrl_name, msg, data_len, flags, data_words, name_to_id, id_to_name)
        dlginit_block["entries"].append(entry)

    return dlginit_block, idx

def decode_simple_dlginitt_entry(ctrl_name, msg, data_len, flags, words, name_to_id, id_to_name):
    # Simple DLGINIT blocks usually contain UTF‑16 strings or raw bytes.
    # We decode UTF‑16 if possible; otherwise return hex words.

    # Try UTF‑16 decoding
    chars = []
    for w in words:
        if 0 < w < 0xD800:  # basic multilingual plane, avoid surrogates
            chars.append(chr(w))
        else:
            chars = None
            break

    if chars is not None:
        text = ''.join(chars).rstrip('\x00')
        return {
            "type": "simple",
            "control": ctrl_name,
            "message": msg,
            "length": data_len,
            "extra": flags,
            "strings": [text] if text else [],
            "identifiers": [],
            "numeric_ids": []
        }
    elif len(words) == 1:
        return {
            "type": "simple",
            "control": ctrl_name,
            "message": msg,
            "length": data_len,
            "extra": flags,
            "strings": [text] if text else [],
            "identifiers": [],
            "numeric_ids": []
        }

    # Fallback: raw hex words
    return {
        "type": "simple_raw",
        "control": ctrl_name,
        "message": msg,
        "length": data_len,
        "extra": flags,
        "raw_words": [f"0x{w:04x}" for w in words]
    }

def decode_full_dlginitt_entry(ctrl_name, msg, data_len, flags, words, name_to_id, id_to_name):
    pos = 2  # skip header

    copyright_chars = []
    while pos < len(words) and words[pos] not in (0x0000, 0x0001):
        if words[pos] < 256:
            copyright_chars.append(chr(words[pos]))
        pos += 1
    copyright_str = ''.join(copyright_chars)

    # After the loop, pos points to the terminator (0x0000 or 0x0001)
    if pos < len(words) and (words[pos] == 0x0000 or words[pos] == 0x0001):
        pos += 1 # Skip terminator
    
    # After the terminator, there is often a separator (seems to be always 0x0001)
    if pos < len(words) and words[pos] == 0x0001:
        pos += 1

    tail = words[pos:]

    # 1. Create byte stream (Little Endian) to find string literals
    le_bytes = bytearray()
    for w in tail:
        le_bytes.extend([w & 0xFF, (w >> 8) & 0xFF])
    
    le_blob = le_bytes.decode("latin-1", errors="ignore")

    # 2. Find identifier strings and their ranges in the blob
    #    We need raw strings for masking/replacing, and corrected strings for output.
    conf_identifiers = set()
    identifier_ranges = []

    # Helper to determine if a byte is a likely text character (printable ASCII, tab, newline)
    # We use this to avoid masking words that just happen to look like IDs but are actually text (e.g. " W").
    def is_text_char(b):
        return (32 <= b <= 126) or b in (9, 10, 13)

    # Regex for identifiers. FIL_... are often icons. IDS_... are strings.
    for m in re.finditer(r'(FIL_|IDS_)[A-Z0-9_]+', le_blob):
        conf_identifiers.add(m.group(0))
        identifier_ranges.append((m.start(), m.end()))

    # 3. Identify numeric IDs to mask, BUT skip if they fall inside a string literal
    # This fixes the regression where identifier strings (e.g. "IDS_BRIDGE") were being corrupted
    # because they accidentally contained bytes matching a numeric ID.
    # RERUN FIX: Scan byte-by-byte to handle misaligned IDs (packed after odd-length strings)
    mask_indices = set()
    for i in range(len(le_bytes) - 1):
        # Construct 16-bit value from bytes i, i+1
        w = le_bytes[i] | (le_bytes[i+1] << 8)

        # Check if this word corresponds to a known ID
        name = id_to_name.get(w)
        if name and (name.startswith("IDS_") or name.startswith("FIL_")):
            if is_text_char(le_bytes[i+1]):
                continue         
            # Check for overlap with string literals
            byte_start = i
            byte_end = i + 2
            overlap = False
            for r_start, r_end in identifier_ranges:
                # Overlap if not (byte_end <= r_start or byte_start >= r_end)
                if not (byte_end <= r_start or byte_start >= r_end):
                    overlap = True
                    break
            if not overlap:
                mask_indices.add(i)
                mask_indices.add(i+1)

    # 4. Create cleaned byte stream for scavenging
    le_bytes_clean = bytearray()
    for i, b in enumerate(le_bytes):
        le_bytes_clean.append(0 if i in mask_indices else b)

    search_blob = le_bytes_clean.decode("latin-1", errors="ignore")

    # 5. Remove raw identifiers from search blob to clean it up for text scavenging
    for ident in conf_identifiers:
        search_blob = search_blob.replace(ident, ' ') # Replace with space to avoid merging words

    # 6. Scavenge other strings
    other_strings = set()
    # Find sequences of printable characters that look like words or phrases.
    # This is a bit heuristic.
    potential_strings = re.findall(r'[a-zA-Z0-9\s\(\)\-.,:/\'!?]{3,}', search_blob)
    for s in potential_strings:
        s = s.strip()
        if s and len(s) > 2 and re.search(r'[a-zA-Z]', s):
            other_strings.add(s)

    # 7. Build High-Confidence Output Identifiers (found as actual strings in the binary block)
    inferred_identifiers = set()
    for w in tail:
        if w == 0: continue
        for val in (w, w >> 1):
            if val < 32: continue 
            name = id_to_name.get(val)
            if name and (name.startswith("IDS_") or name.startswith("FIL_")):
                if "TITLE" in name.upper() and name not in conf_identifiers:
                    continue
                if val > 100 or name in conf_identifiers:
                    inferred_identifiers.add(name)

    confident_output = set()
    for ident in conf_identifiers:
        if ident.startswith("FIL_"): confident_output.add("IF_L" + ident[4:])
        else: confident_output.add(ident)

    # 8. Smart Label Detection (Heuristic prioritization)
    label_id = None
    ctrl_base = ctrl_name.replace("IDC_", "").replace("CBO_", "").replace("BUT_", "")
    for ident in sorted(list(conf_identifiers)):
        if ident.startswith("IDS_") and ctrl_base in ident:
            label_id = ident
            break
    
    if not label_id:
        for ident in sorted(list(conf_identifiers)):
            if ident.startswith("IDS_") and "TITLE" not in ident.upper():
                label_id = ident
                break

    if not label_id:
        for ident in sorted(list(inferred_identifiers)):
            if ctrl_base in ident:
                label_id = ident
                break

    numeric_ids = []
    for w in tail:
        if w != 0:
            name = id_to_name.get(w)
            if not name: name = id_to_name.get(w >> 1)
            if name and name in (conf_identifiers | inferred_identifiers):
                numeric_ids.append(w)
                break # Only record first significant ID

    strings = []
    if copyright_str:
        strings.append(copyright_str)
    if other_strings:
        strings.extend(sorted(list(other_strings)))

    result = {
        "type": "full",
        "control": ctrl_name,
        "message": msg,
        "length": data_len,
        "extra": flags,
        "strings": strings,
        "identifiers": sorted(list(confident_output)),
        "numeric_ids": numeric_ids,
        "label_id": label_id,
        "inferred_ids": sorted(list(inferred_identifiers)) # RERUN: Keep inferred IDs separate to avoid noise
    }

    # --- NEW: Try to parse as a Rowan custom button ---
    button_metadata = _try_parse_button_metadata(words, name_to_id, id_to_name)
    if button_metadata:
        result["button_metadata"] = button_metadata
        # If we successfully parsed metadata, let's refine the icon/label from it
        if button_metadata.get("icon_name"):
            icon_name = button_metadata["icon_name"]
            if icon_name.startswith("FIL_ICON_"):
                result["icon"] = icon_name.replace("FIL_ICON_", "")
        if button_metadata.get("icon_enum"):
            label_id_from_enum = id_to_name.get(button_metadata["icon_enum"])
            if label_id_from_enum and label_id_from_enum.startswith("IDS_"):
                 result["label_id"] = label_id_from_enum

    # Automatically detect icon and label from identifiers
    icon_name = None
    icon_prefix = "IF_LICON_"

    for ident in result["identifiers"]:
        if ident.startswith(icon_prefix):
            # Extract the part between "IF_LICON_" and "_ON" or "_OFF"
            base_name = ident[len(icon_prefix):]
            if base_name.endswith("_ON"): icon_name = base_name[:-3]
            elif base_name.endswith("_OFF"): icon_name = base_name[:-4]
            else: icon_name = base_name 

    result["icon"] = icon_name

    # If no identifiers or other strings were found, it's likely a config block.
    # Add the raw words from the tail for inspection.
    if not result["identifiers"] and not other_strings:
        result["raw_words"] = [f"0x{w:04x}" for w in tail]

    return result


# ------------------------------------------------------------
# MENU parsing
# ------------------------------------------------------------

def parse_menu_items(lines, idx, name_to_id, id_to_name):
    items = []
    while idx < len(lines):
        line = strip_line_comments(lines[idx]).strip()
        if not line:
            idx += 1
            continue
        if line.upper().startswith("END"):
            idx += 1
            break

        upper = line.upper()
        if upper.startswith("MENUITEM"):
            item_type = "MENUITEM"
            rest = line[len(item_type):].strip()
            
            item = {"type": item_type}
            
            if rest.upper() == "SEPARATOR":
                item["separator"] = True
            else:
                parts = split_commas(rest)
                text, _ = parse_quoted_string(parts[0])
                item["text"] = text
                if len(parts) > 1:
                    item_id_str = parts[1].strip()
                    item["id_name"] = item_id_str
                    item["id_numeric"] = name_to_id.get(item_id_str)
                if len(parts) > 2:
                    item["options"] = parts[2:]

            items.append(item)
            idx += 1

        elif upper.startswith("POPUP"):
            item_type = "POPUP"
            rest = line[len(item_type):].strip()
            
            parts = split_commas(rest)
            text, _ = parse_quoted_string(parts[0])
            
            popup = {
                "type": item_type,
                "text": text,
                "items": []
            }
            if len(parts) > 1:
                popup["options"] = parts[1:]

            # find BEGIN for the popup
            idx += 1
            while idx < len(lines):
                line_inner = strip_line_comments(lines[idx]).strip()
                if not line_inner:
                    idx += 1
                    continue
                if line_inner.upper().startswith("BEGIN"):
                    idx += 1
                    break
                idx += 1
            
            popup["items"], idx = parse_menu_items(lines, idx, name_to_id, id_to_name)
            items.append(popup)
        
        else:
            # Unknown item, e.g. could be a property inside a MENUEX
            items.append({"raw": line})
            idx += 1
    
    return items, idx

def parse_menu_block(lines, idx, name_to_id, id_to_name):
    """
    lines[idx] is the line with '... MENU ...'
    Returns (menu_dict, new_idx)
    """
    header = strip_line_comments(lines[idx])
    tokens = header.split()
    name = tokens[0]
    attributes = tokens[2:]

    menu = {
        "type": "MENU",
        "name": name,
        "numeric_id": name_to_id.get(name),
        "attributes": attributes,
        "items": []
    }

    # Find BEGIN
    idx += 1
    while idx < len(lines):
        line = strip_line_comments(lines[idx]).strip()
        if not line:
            idx += 1
            continue
        if line.upper().startswith("BEGIN"):
            idx += 1
            break
        idx += 1
    
    menu["items"], idx = parse_menu_items(lines, idx, name_to_id, id_to_name)
    
    return menu, idx

# ------------------------------------------------------------
# Generic RC parsing: walk file, dispatch to handlers
# ------------------------------------------------------------

def parse_rc(path, name_to_id, id_to_name):
    with path.open(encoding='latin-1', errors='ignore') as f:
        raw_lines = f.readlines()

    result = {
        "file": str(path),
        "includes": [],
        "defines": [],  # we keep them from header separately, but also record any in RC
        "stringtables": [],
        "dialogs": [],
        "menus": [],
        "other_resources": []
    }

    include_re = re.compile(r'#include\s+(.*)')
    define_re = re.compile(r'#define\s+(\w+)\s+(.+)')

    lines = raw_lines   # <‑‑ ADD THIS LINE
    in_design_info = False

    idx = 0
    while idx < len(lines):
        raw = lines[idx]
        line = strip_line_comments(raw).strip()
        if not line:
            idx += 1
            continue

        # Preprocessor
        if line.startswith('#'):
            m_inc = include_re.match(line)
            if m_inc:
                result["includes"].append(m_inc.group(1).strip())
                idx += 1
                continue
            m_def = define_re.match(line)
            if m_def:
                result["defines"].append({
                    "name": m_def.group(1),
                    "value": m_def.group(2).strip()
                })
                idx += 1
                continue

        upper = line.upper()

        # RERUN: Robustly skip GUIDELINES/DESIGNINFO blocks to avoid duplicate dummy dialog entries.
        # These blocks often start with 'dialogID DESIGNINFO' or 'GUIDELINES DESIGNINFO'.
        if "DESIGNINFO" in upper:
            idx += 1
            depth = 0
            while idx < len(lines):
                l = strip_line_comments(lines[idx]).strip().upper()
                if l.startswith("BEGIN"):
                    depth += 1
                elif l.startswith("END"):
                    depth -= 1
                    if depth <= 0:
                        idx += 1
                        break
                idx += 1
            continue

        # STRINGTABLE
        if upper.startswith("STRINGTABLE"):
            block, idx = parse_stringtable_block(lines, idx, name_to_id, id_to_name)
            result["stringtables"].append(block)
            continue

        # DIALOG / DIALOGEX
        if not in_design_info and (" DIALOG " in upper or " DIALOGEX " in upper or upper.endswith(" DIALOG") or upper.endswith(" DIALOGEX")):
            dlg, idx = parse_dialog_block(lines, idx, name_to_id, id_to_name)
            result["dialogs"].append(dlg)
            continue

        # DLGINIT
        if upper.endswith(" DLGINIT") or " DLGINIT" in upper:
            dlginit, idx = parse_dlginitt_block(lines, idx, name_to_id, id_to_name)
            result.setdefault("dlginits", []).append(dlginit)
            continue

        tokens = line.split()
        if len(tokens) >= 2 and tokens[1].upper() == "MENU":
            menu, idx = parse_menu_block(lines, idx, name_to_id, id_to_name)
            result["menus"].append(menu)
            continue

        # Other resource types (BITMAP, ICON, CURSOR, ACCELERATORS, etc.)
        # For now, we just store them as raw blocks grouped by BEGIN/END if present.
        # You can extend this later for more structure.
        tokens = line.split()
        if len(tokens) >= 2 and tokens[1].upper() in {
            "BITMAP", "ICON", "CURSOR", "ACCELERATORS", "RCDATA", "WAVE", "AVI"
        }:
            block_type = tokens[1].upper()
            name = tokens[0]
            block = {
                "type": block_type,
                "name": name,
                "numeric_id": None,
                "id": None,
                "header": line,
                "body": []
            }
            # Map name to numeric ID if possible
            try:
                num = int(name)
                block["numeric_id"] = num
                block["id"] = id_to_name.get(num)
            except ValueError:
                block["id"] = name
                block["numeric_id"] = name_to_id.get(name)

            idx += 1
            # If followed by BEGIN/END, capture body
            if idx < len(lines) and "BEGIN" in strip_line_comments(lines[idx]).upper():
                idx += 1
                while idx < len(lines):
                    l2 = strip_line_comments(lines[idx])
                    if l2.strip().upper().startswith("END"):
                        idx += 1
                        break
                    block["body"].append(l2.rstrip('\n'))
                    idx += 1
            result["other_resources"].append(block)
            continue

        # Fallback: store as raw line if you want
        result.setdefault("raw_lines", []).append(line)
        idx += 1

    return result

# ------------------------------------------------------------
# Main entry: RC + H -> JSON
# ------------------------------------------------------------

def main():
    if len(sys.argv) != 4:
        print("Usage: rc_to_json.py MIG.RC RESOURCE.H output.json")
        sys.exit(1)

    rc_path = Path(sys.argv[1])
    h_path = Path(sys.argv[2])
    out_path = Path(sys.argv[3])

    name_to_id, id_to_name = parse_resource_header(h_path)
    rc_data = parse_rc(rc_path, name_to_id, id_to_name)

    # Also embed header defines in JSON for completeness
    rc_data["header_defines"] = [
        {"name": name, "id": num}
        for name, num in sorted(name_to_id.items(), key=lambda kv: kv[1])
    ]

    with out_path.open("w", encoding="utf-8") as f:
        json.dump(rc_data, f, indent=2, ensure_ascii=False)

    print(f"Wrote {out_path}")

if __name__ == "__main__":
    main()
