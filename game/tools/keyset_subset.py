"""Private minimal keyset for this port's generation-1 homebrew NCAs.

Do not pass unrelated firmware/device keys to legacy packer parsers.
Never print key values; callers put the destination in a private temp dir.
"""
import os
from pathlib import Path

def write_subset(source, destination):
    values = {}
    for line in Path(source).read_text(encoding='utf-8-sig').splitlines():
        if '=' in line:
            name, value = line.split('=', 1)
            values[name.strip().lower()] = value.strip()
    needed = {'header_key': 64, 'key_area_key_application_00': 32}
    for name, size in needed.items():
        value = values.get(name, '')
        if len(value) != size or any(c not in '0123456789abcdefABCDEF' for c in value):
            raise SystemExit('Required homebrew packaging key is missing or malformed: ' + name)
    fd = os.open(destination, os.O_CREAT | os.O_EXCL | os.O_WRONLY, 0o600)
    with os.fdopen(fd, 'w') as output:
        for name in needed:
            output.write(name + ' = ' + values[name] + '\n')
    return Path(destination)
