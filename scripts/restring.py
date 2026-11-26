#!/usr/bin/env python3
# restring.py -- rewrite constant strings in a relocatable ARM ELF object
#
# Usage: scripts/restring.py <list> <object>...
#
# <list> has one replacement per line, "<object> | <old> | <new>" (blank
# lines and lines starting with # are skipped); every line whose <object> is
# the basename of a given object (without .o_shipped / .o) is applied to that
# object, in place.
#
# A vendor object that only exists as a finished .ko can still name paths the
# firmware does not have (infinity2m's mi_vdec opens its decoder firmware at
# "/config/vdec_fw/..."). The string is overwritten where it lies, with a
# string no longer than the old one, NUL-padded to the old length: no section
# changes size, no symbol, relocation or instruction moves. Each old string
# must occur exactly once in the object's string sections (SHF_STRINGS), as a
# whole NUL-terminated string, and every relocation that points into it must
# point at its first byte (checked for the REL relocations the ARM objects
# carry: R_ARM_ABS32 against the section, the addend in place). A string
# that is already replaced is reported and left alone, so a second run
# changes nothing.
import os
import struct
import sys

SHT_REL = 9
SHF_STRINGS = 0x20
R_ARM_ABS32 = 2


def sections(elf):
    if elf[:4] != b'\x7fELF' or elf[4] != 1 or elf[5] != 1:
        sys.exit('not a 32-bit little-endian ELF file')
    shoff, = struct.unpack_from('<I', elf, 0x20)
    shentsize, shnum, shstrndx = struct.unpack_from('<HHH', elf, 0x2e)
    secs = []
    for i in range(shnum):
        name, typ, flags, _, off, size, link, info, _, entsize = \
            struct.unpack_from('<10I', elf, shoff + i * shentsize)
        secs.append(dict(name=name, type=typ, flags=flags, off=off,
                         size=size, link=link, info=info, entsize=entsize))
    strtab = secs[shstrndx]
    for s in secs:
        n = elf[strtab['off'] + s['name']:]
        s['name'] = n[:n.index(b'\0')].decode()
    return secs


def references(elf, secs, target):
    """Addends of R_ARM_ABS32 relocations against section <target>'s symbol;
    fails on any other relocation type against it."""
    out = []
    for rel in secs:
        if rel['type'] != SHT_REL:
            continue
        symtab = secs[rel['link']]
        patched = secs[rel['info']]
        for i in range(rel['size'] // 8):
            r_off, r_info = struct.unpack_from('<II', elf, rel['off'] + i * 8)
            st_name, st_value, _, st_info, _, st_shndx = struct.unpack_from(
                '<IIIBBH', elf, symtab['off'] + (r_info >> 8) * 16)
            if st_shndx != target:
                continue
            if r_info & 0xff != R_ARM_ABS32:
                sys.exit('relocation type %d against %s: not checked'
                         % (r_info & 0xff, secs[target]['name']))
            addend, = struct.unpack_from('<I', elf, patched['off'] + r_off)
            out.append(st_value + addend)
    return out


def find(elf, sec, s):
    data = elf[sec['off']:sec['off'] + sec['size']]
    hits, at = [], data.find(s + b'\0')
    while at >= 0:
        if at == 0 or data[at - 1] == 0:
            hits.append(at)
        at = data.find(s + b'\0', at + 1)
    return hits


def restring(path, pairs):
    elf = bytearray(open(path, 'rb').read())
    secs = sections(elf)
    strsecs = [(i, s) for i, s in enumerate(secs) if s['flags'] & SHF_STRINGS]
    for old, new in pairs:
        if len(new) > len(old):
            sys.exit('%s: "%s" is longer than "%s"' % (path, new, old))
        hits = [(i, s, at) for i, s in strsecs for at in find(elf, s, old)]
        if not hits:
            if any(find(elf, s, new) for _, s in strsecs):
                print('%s: "%s" already there' % (path, new.decode()))
                continue
            sys.exit('%s: "%s" not found' % (path, old.decode()))
        if len(hits) > 1:
            sys.exit('%s: "%s" found %d times' % (path, old.decode(), len(hits)))
        i, s, at = hits[0]
        refs = references(elf, secs, i)
        inside = [r for r in refs if at < r < at + len(old) + 1]
        if inside:
            sys.exit('%s: "%s" is referenced at +%d, not only at its start'
                     % (path, old.decode(), inside[0] - at))
        n = refs.count(at)
        o = s['off'] + at
        elf[o:o + len(old)] = new + b'\0' * (len(old) - len(new))
        print('%s: %s+0x%x "%s" -> "%s" (%d reference%s)'
              % (path, s['name'], at, old.decode(), new.decode(),
                 n, '' if n == 1 else 's'))
    open(path, 'wb').write(elf)


def main():
    if len(sys.argv) < 3:
        sys.exit('usage: restring.py <list> <object>...')
    todo = {}
    for line in open(sys.argv[1]):
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        obj, old, new = (f.strip() for f in line.split('|'))
        todo.setdefault(obj, []).append((old.encode(), new.encode()))
    for path in sys.argv[2:]:
        base = os.path.basename(path)
        for ext in ('.o_shipped', '.o'):
            if base.endswith(ext):
                base = base[:-len(ext)]
        if base not in todo:
            sys.exit('%s: no strings listed for "%s"' % (path, base))
        restring(path, todo[base])


main()
