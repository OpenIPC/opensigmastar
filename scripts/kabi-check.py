#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""kabi-check.py -- does a kernel keep the ABI the relinked vendor modules expect?

The relinked modules (kernel/obj/<CHIPARCH>/*.o_shipped) are SigmaStar's
code compiled against the SDK kernel's headers: the layout of every kernel
structure they allocate, initialise or dereference, the inline header code
they carry (kmalloc, current, copy_from_user, ...), the kernel's exports
and their prototypes are baked into them. Relinking gives them this
kernel's vermagic and struct module, but not this kernel's structures.

  extract  (maintainers) read the DWARF of the SDK's original .ko files, and
           a probe built against the SDK kernel tree, and write
           kernel/kabi/<CHIPARCH>.json: the kernel types the vendor code can
           reach, how it uses them, the kernel exports it imports with their
           prototypes, the kmalloc size classes it indexes, the inline kernel
           functions compiled into it and the kernel constants behind them.
  check    build a probe object against a target kernel tree (built, or
           after "make modules_prepare"), with debug info whatever the
           kernel's config, and compare it with that file. Exit status 1
           when something the blobs use differs.
  dump     print what the probe (or any ELF with DWARF) says about one type.

Only binutils' readelf/objdump are needed (CROSS_COMPILE's, or the host's);
no Python packages beyond the standard library.
"""

import argparse
import json
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile
from collections import defaultdict

# --------------------------------------------------------------------------
# DWARF, from "readelf --debug-dump=info"

KEEP = {
    'DW_AT_name', 'DW_AT_type', 'DW_AT_byte_size', 'DW_AT_data_member_location',
    'DW_AT_bit_size', 'DW_AT_bit_offset', 'DW_AT_data_bit_offset',
    'DW_AT_decl_file', 'DW_AT_decl_line', 'DW_AT_upper_bound', 'DW_AT_count',
    'DW_AT_const_value', 'DW_AT_declaration', 'DW_AT_external', 'DW_AT_inline',
    'DW_AT_abstract_origin', 'DW_AT_specification', 'DW_AT_stmt_list',
    'DW_AT_comp_dir', 'DW_AT_low_pc', 'DW_AT_location', 'DW_AT_prototyped',
    'DW_AT_ranges',
}
REFS = {'DW_AT_type', 'DW_AT_abstract_origin', 'DW_AT_specification'}
STRS = {'DW_AT_name', 'DW_AT_comp_dir'}

DIE_RE = re.compile(r'^\s*<(\d+)><([0-9a-f]+)>: Abbrev Number: (\d+)(?: \((\w+)\))?')
AT_RE = re.compile(r'^\s*<[0-9a-f]+>\s+(DW_AT_\w+)\s*:\s?(.*)$')
CU_RE = re.compile(r'^\s*Compilation Unit @ offset (?:0x)?([0-9a-f]+):')


class Die:
    __slots__ = ('off', 'tag', 'at', 'kids', 'parent')

    def __init__(self, off, tag, parent):
        self.off, self.tag, self.at, self.kids, self.parent = off, tag, {}, [], parent

    def get(self, k, d=None):
        return self.at.get(k, d)


def _int(v):
    v = v.strip().split()[0] if v.strip() else '0'
    try:
        return int(v, 0)
    except ValueError:
        try:
            return int(v, 16)
        except ValueError:
            return None


def _attr(name, v):
    if name in REFS:
        m = re.search(r'<(0x[0-9a-f]+)>', v)
        return int(m.group(1), 16) if m else None
    if name in STRS:
        m = re.match(r'^\((?:indirect|indexed)[^)]*\):\s?(.*)$', v)
        return (m.group(1) if m else v).strip()
    if name == 'DW_AT_data_member_location':
        m = re.search(r'DW_OP_plus_uconst: (\d+)', v)
        return int(m.group(1)) if m else _int(v)
    if name in ('DW_AT_location', 'DW_AT_low_pc', 'DW_AT_ranges'):
        return True
    return _int(v)


def tool(cross, name):
    """CROSS_COMPILE's binutils tool, else the host's."""
    if cross and shutil.which(cross + name):
        return cross + name
    if shutil.which(name):
        return name
    sys.exit(f'kabi-check: no {cross or ""}{name} found')


def read_cus(path, readelf):
    """Yield (root Die, {offset: Die}) for each compilation unit."""
    p = subprocess.Popen([readelf, '--debug-dump=info', path], stdout=subprocess.PIPE,
                         stderr=subprocess.DEVNULL, text=True, errors='replace',
                         bufsize=1 << 20)
    dies, stack, cur, root = {}, [], None, None
    for line in p.stdout:
        m = DIE_RE.match(line)
        if m:
            depth, off, abbrev, tag = int(m.group(1)), int(m.group(2), 16), m.group(3), m.group(4)
            if abbrev == '0':
                if stack:
                    stack.pop()
                cur = None
                continue
            if depth == 0:
                if root is not None:
                    yield root, dies
                dies, stack = {}, []
            del stack[depth:]
            parent = stack[-1] if stack else None
            cur = Die(off, tag, parent)
            dies[off] = cur
            if parent is not None:
                parent.kids.append(cur)
            else:
                root = cur
            stack.append(cur)
            continue
        if cur is None:
            if CU_RE.match(line) and root is not None:
                yield root, dies
                root, dies, stack = None, {}, []
            continue
        m = AT_RE.match(line)
        if m and m.group(1) in KEEP:
            cur.at[m.group(1)] = _attr(m.group(1), m.group(2))
    if root is not None:
        yield root, dies
    p.wait()


def read_line_files(path, readelf):
    """{stmt_list offset: {file index: path}} from the DWARF 2-4 line tables."""
    p = subprocess.Popen([readelf, '--debug-dump=rawline', path], stdout=subprocess.PIPE,
                         stderr=subprocess.DEVNULL, text=True, errors='replace',
                         bufsize=1 << 20)
    tables, off, dirs, files, mode = {}, None, {}, None, None
    for line in p.stdout:
        if line.startswith('  [0x') or line.startswith('  Line Number'):
            mode = None
            continue
        s = line.strip()
        if s.startswith('Offset:'):
            off = int(s.split()[1], 16)
            dirs, files = {}, {}
            tables[off] = files
            mode = None
        elif s.startswith('The Directory Table'):
            mode = 'd'
        elif s.startswith('The File Name Table'):
            mode = 'f'
        elif mode == 'd' and s:
            parts = s.split(None, 1)
            if len(parts) == 2 and parts[0].isdigit():
                dirs[int(parts[0])] = parts[1]
        elif mode == 'f' and s and s[0].isdigit():
            parts = s.split(None, 4)
            if len(parts) == 5:
                d = dirs.get(int(parts[1]), '')
                files[int(parts[0])] = (d + '/' + parts[4]) if d else parts[4]
    p.wait()
    return tables


def file_class(path):
    """kernel (the kernel's own headers), sstar (the kernel's SigmaStar
    headers), compiler, or vendor (the SDK's sources and headers)."""
    if path is None:
        return 'vendor'
    p = re.sub(r'^.*/kbuild/[^/]+/', '', path)
    p = re.sub(r'^(\./)+', '', p)
    if p.startswith('include/uapi/mstar/'):
        return 'vendor'     # fbdev's ioctl structures (SDK copy of mstarFb.h)
    if p.startswith(('include/', 'arch/')):
        return 'kernel'
    if p.startswith('drivers/sstar/') or p.startswith('drivers/mstar/'):
        return 'sstar'
    if '/lib/gcc/' in p:
        return 'compiler'
    return 'vendor'


def norm_path(path):
    p = re.sub(r'^.*/kbuild/[^/]+/', '', path or '')
    return re.sub(r'^(\./)+', '', p)


# --------------------------------------------------------------------------
# Type descriptions

AGG = ('DW_TAG_structure_type', 'DW_TAG_union_type')
KW = {'DW_TAG_structure_type': 'struct', 'DW_TAG_union_type': 'union',
      'DW_TAG_enumeration_type': 'enum'}
QUAL = {'DW_TAG_const_type': 'const', 'DW_TAG_volatile_type': 'volatile',
        'DW_TAG_restrict_type': None, 'DW_TAG_atomic_type': '_Atomic'}


class CU:
    """Type naming and layouts within one compilation unit."""

    def __init__(self, root, dies, files=None):
        self.root, self.dies, self.files = root, dies, files or {}
        self._name = {}

    def t(self, d):
        r = d.get('DW_AT_type')
        return self.dies.get(r) if r is not None else None

    def origin(self, d):
        seen = 0
        while d is not None and seen < 4:
            r = d.get('DW_AT_abstract_origin') or d.get('DW_AT_specification')
            if r is None:
                return d
            d = self.dies.get(r)
            seen += 1
        return d

    def decl_file(self, d):
        d = self.origin(d)
        f = d.get('DW_AT_decl_file') if d is not None else None
        return self.files.get(f) if f is not None else None

    def strip(self, d):
        """Remove qualifiers (not typedefs)."""
        while d is not None and d.tag in QUAL:
            d = self.t(d)
        return d

    def strip_all(self, d):
        """Remove qualifiers and typedefs."""
        while d is not None and (d.tag in QUAL or d.tag == 'DW_TAG_typedef'):
            d = self.t(d)
        return d

    def sizeof(self, d):
        d = self.strip(d)
        if d is None:
            return None
        if 'DW_AT_byte_size' in d.at:
            return d.at['DW_AT_byte_size']
        if d.tag == 'DW_TAG_typedef':
            return self.sizeof(self.t(d))
        if d.tag == 'DW_TAG_array_type':
            n = self.sizeof(self.t(d))
            for dim in self.dims(d):
                if n is None or dim is None:
                    return 0 if dim is None else None
                n *= dim
            return n
        return None

    def dims(self, d):
        out = []
        for k in d.kids:
            if k.tag == 'DW_TAG_subrange_type':
                # a flexible array member is [] (GCC 8) or [0] (GCC 13)
                if k.at.get('DW_AT_count'):
                    out.append(k.at['DW_AT_count'])
                elif k.at.get('DW_AT_upper_bound') is not None and k.at['DW_AT_upper_bound'] >= 0:
                    out.append(k.at['DW_AT_upper_bound'] + 1)
                else:
                    out.append(None)
        return out

    def key(self, d):
        """The name a type is compared under, or None if anonymous."""
        d = self.strip(d)
        if d is None:
            return None
        if d.tag in KW and 'DW_AT_name' in d.at:
            return f"{KW[d.tag]} {d.at['DW_AT_name']}"
        if d.tag == 'DW_TAG_typedef':
            return f"typedef {d.at['DW_AT_name']}"
        return None

    def name(self, d):
        if d is None:
            return 'void'
        r = self._name.get(d.off)
        if r is None:
            self._name[d.off] = '...'      # recursion guard
            r = self._name[d.off] = self._mkname(d)
        return r

    def _mkname(self, d):
        tag = d.tag
        if tag == 'DW_TAG_base_type':
            return d.get('DW_AT_name', '?')
        if tag in KW:
            n = d.get('DW_AT_name')
            if n:
                return f'{KW[tag]} {n}'
            return f'{KW[tag]} {{anon {self.sizeof(d)}}}'
        if tag == 'DW_TAG_typedef':
            return d.get('DW_AT_name', '?')
        if tag in QUAL:
            t = self.t(d)
            q = QUAL[tag]
            if q is None:
                return self.name(t)
            st = self.strip(t)
            if st is not None and st.tag == 'DW_TAG_pointer_type':
                return f'{self.name(t)} {q}'
            return f'{q} {self.name(t)}'
        if tag in ('DW_TAG_pointer_type', 'DW_TAG_reference_type'):
            t = self.t(d)
            st = self.strip(t)
            if st is not None and st.tag == 'DW_TAG_subroutine_type':
                return self.sig(st, '(*)')
            return f'{self.name(t)} *'
        if tag == 'DW_TAG_array_type':
            dims = ''.join(f'[{x if x is not None else ""}]' for x in self.dims(d))
            return f'{self.name(self.t(d))}{dims}'
        if tag == 'DW_TAG_subroutine_type':
            return self.sig(d, '')
        if tag == 'DW_TAG_unspecified_type':
            return d.get('DW_AT_name', '?')
        return tag

    def sig(self, d, inner, fname=None):
        """Prototype of a subroutine type or subprogram."""
        params = []
        for k in d.kids:
            if k.tag == 'DW_TAG_formal_parameter':
                params.append(self.name(self.t(self.origin(k)) if 'DW_AT_type' not in k.at
                                        else self.t(k)))
            elif k.tag == 'DW_TAG_unspecified_parameters':
                params.append('...')
        if not params and d.get('DW_AT_prototyped'):
            params = ['void']
        ret = self.name(self.t(d))
        return f"{ret} {fname or inner}({', '.join(params)})"

    def layout(self, d):
        """Description of a named type: struct/union members (anonymous
        members flattened, named anonymous aggregates expanded as a.b),
        enumerators, or a typedef's target."""
        d0 = d
        if d.tag == 'DW_TAG_typedef':
            t = self.t(d)
            st = self.strip(t)
            rec = {'kind': 'typedef', 'type': self.name(t), 'size': self.sizeof(t)}
            if st is not None and st.tag in AGG + ('DW_TAG_enumeration_type',) \
                    and 'DW_AT_name' not in st.at:
                rec.update(self.layout(st))     # typedef struct { ... } x_t;
                rec['kind'] = 'typedef ' + KW[st.tag]
            return rec
        if d.tag == 'DW_TAG_enumeration_type':
            mask = (1 << (8 * (d.get('DW_AT_byte_size') or 4))) - 1
            vals = {k.at['DW_AT_name']: k.at.get('DW_AT_const_value', 0) & mask
                    for k in d.kids if k.tag == 'DW_TAG_enumerator'}
            return {'kind': 'enum', 'size': d.get('DW_AT_byte_size'), 'values': vals}
        members = []

        def walk(sd, base, prefix):
            for k in sd.kids:
                if k.tag != 'DW_TAG_member':
                    continue
                n = k.get('DW_AT_name')
                t = self.t(k)
                st = self.strip(t)
                off = k.get('DW_AT_data_member_location', 0)
                if 'DW_AT_bit_size' in k.at:
                    bs = k.at['DW_AT_bit_size']
                    if 'DW_AT_data_bit_offset' in k.at:
                        bo = k.at['DW_AT_data_bit_offset']
                    else:
                        size = k.get('DW_AT_byte_size') or self.sizeof(t) or 4
                        bo = off * 8 + size * 8 - k.at.get('DW_AT_bit_offset', 0) - bs
                    bo += base * 8
                    members.append([prefix + (n or ''), f'{bo // 8}.{bo % 8}', f'{bs}b',
                                    self.name(t)])
                    continue
                anon = st is not None and st.tag in AGG and 'DW_AT_name' not in st.at
                if n is None and anon:
                    walk(st, base + off, prefix)
                    continue
                members.append([prefix + (n or ''), base + off, self.sizeof(t), self.name(t)])
                if anon:
                    walk(st, base + off, prefix + n + '.')
        walk(d0, 0, '')
        return {'kind': KW[d0.tag], 'size': d0.get('DW_AT_byte_size'), 'members': members}

    def refs(self, d):
        """[(type key, 'value'|'pointer')] a type's members (or a typedef)
        refer to; anonymous aggregates are looked through."""
        out = []

        def ref(t, mode, depth=0):
            if t is None or depth > 8:
                return
            t = self.strip(t)
            if t is None:
                return
            k = self.key(t)
            if k:
                out.append((k, mode))
                return
            if t.tag == 'DW_TAG_array_type':
                ref(self.t(t), mode, depth + 1)
            elif t.tag in ('DW_TAG_pointer_type', 'DW_TAG_reference_type'):
                ref(self.t(t), 'pointer', depth + 1)
            elif t.tag == 'DW_TAG_subroutine_type':
                ref(self.t(t), 'pointer', depth + 1)
                for p in t.kids:
                    if p.tag == 'DW_TAG_formal_parameter':
                        ref(self.t(p), 'pointer', depth + 1)
            elif t.tag in AGG:
                for m in t.kids:
                    if m.tag == 'DW_TAG_member':
                        ref(self.t(m), mode, depth + 1)
        if d.tag == 'DW_TAG_typedef':
            ref(self.t(d), 'value')
        elif d.tag in AGG:
            for m in d.kids:
                if m.tag == 'DW_TAG_member':
                    ref(self.t(m), 'value')
        return out

    def use_of(self, t):
        """(type key, 'value'|'pointer') for a variable/parameter type."""
        mode = 'value'
        for _ in range(16):
            t = self.strip(t)
            if t is None:
                return None, None
            k = self.key(t)
            if k:
                return k, mode
            if t.tag == 'DW_TAG_array_type':
                t = self.t(t)
            elif t.tag in ('DW_TAG_pointer_type', 'DW_TAG_reference_type'):
                mode = 'pointer'
                t = self.t(t)
            else:
                return None, None
        return None, None


# --------------------------------------------------------------------------
# ELF symbols, and kmalloc size classes from the disassembly

def elf_symbols(path):
    """(defined, undefined) global symbol names of an ELF32 little-endian file."""
    with open(path, 'rb') as f:
        data = f.read()
    if data[:4] != b'\x7fELF' or data[4] != 1:
        sys.exit(f'kabi-check: {path}: not a 32-bit ELF file')
    shoff, = struct.unpack_from('<I', data, 0x20)
    shentsize, shnum, shstrndx = struct.unpack_from('<HHH', data, 0x2e)
    secs = [struct.unpack_from('<IIIIIIIIII', data, shoff + i * shentsize) for i in range(shnum)]
    defined, undefined = set(), set()
    for s in secs:
        if s[1] != 2:           # SHT_SYMTAB
            continue
        strtab = secs[s[6]]
        for i in range(s[5] // 16):
            name, value, size, info, other, shndx = struct.unpack_from('<IIIBBH', data, s[4] + i * 16)
            if info >> 4 not in (1, 2) or not name:     # GLOBAL, WEAK
                continue
            end = data.index(b'\0', strtab[4] + name)
            n = data[strtab[4] + name:end].decode()
            (undefined if shndx == 0 else defined).add(n)
    return defined, undefined


def elf_objects(path):
    """{symbol: (section name, bytes, set of relocated offsets)} of the
    data objects (local or global) in sections with contents."""
    with open(path, 'rb') as f:
        data = f.read()
    shoff, = struct.unpack_from('<I', data, 0x20)
    shentsize, shnum, shstrndx = struct.unpack_from('<HHH', data, 0x2e)
    secs = [struct.unpack_from('<IIIIIIIIII', data, shoff + i * shentsize) for i in range(shnum)]
    strs = secs[shstrndx][4]

    def cstr(off):
        return data[off:data.index(b'\0', off)].decode(errors='replace')
    names = [cstr(strs + sh[0]) for sh in secs]
    relocs = defaultdict(set)
    for sh in secs:
        if sh[1] == 9:          # SHT_REL
            for i in range(sh[5] // 8):
                off, = struct.unpack_from('<I', data, sh[4] + i * 8)
                relocs[sh[7]].add(off)
    out = {}
    for sh in secs:
        if sh[1] != 2:
            continue
        strtab = secs[sh[6]][4]
        for i in range(sh[5] // 16):
            name, value, size, info, other, shndx = struct.unpack_from('<IIIBBH', data, sh[4] + i * 16)
            if info & 0xf != 1 or not name or not size or shndx >= len(secs) or shndx == 0:
                continue        # STT_OBJECT with a size, in a real section
            tsec = secs[shndx]
            if tsec[1] == 8:    # SHT_NOBITS (.bss): zero, nothing initialised
                continue
            body = data[tsec[4] + value:tsec[4] + value + size]
            rel = {o - value for o in relocs.get(shndx, ()) if value <= o < value + size}
            out[cstr(strtab + name)] = (names[shndx], body, rel)
    return out


def elf_modinfo(path):
    """{key: value} of a .ko's .modinfo section."""
    with open(path, 'rb') as f:
        data = f.read()
    shoff, = struct.unpack_from('<I', data, 0x20)
    shentsize, shnum, shstrndx = struct.unpack_from('<HHH', data, 0x2e)
    secs = [struct.unpack_from('<IIIIIIIIII', data, shoff + i * shentsize) for i in range(shnum)]
    names = secs[shstrndx]
    out = {}
    for sh in secs:
        end = data.index(b'\0', names[4] + sh[0])
        if data[names[4] + sh[0]:end] == b'.modinfo':
            for kv in data[sh[4]:sh[4] + sh[5]].split(b'\0'):
                k, _, v = kv.decode(errors='replace').partition('=')
                if k:
                    out.setdefault(k, v)
    return out


GPL_LICENSES = {'GPL', 'GPL v2', 'GPL and additional rights', 'Dual BSD/GPL',
                'Dual MIT/GPL', 'Dual MPL/GPL'}


INSN_RE = re.compile(r'^\s*([0-9a-f]+):\s+([0-9a-f]{4}(?: [0-9a-f]{4})?|[0-9a-f]{8})\s+(\S+)\s*(.*)$')
RELOC_RE = re.compile(r'^\s*([0-9a-f]+): (R_ARM_\w+)\s+(\S+)')
FUNC_RE = re.compile(r'^[0-9a-f]+ <(.+)>:$')


def kmalloc_sites(path, objdump):
    """[(function, kmalloc_caches index or None)] for each place the code
    loads kmalloc_caches: the inline kmalloc() of a constant size, which
    indexes the array by the size class kmalloc_index() chose at compile
    time."""
    out = subprocess.run([objdump, '-dr', path], capture_output=True, text=True,
                         errors='replace').stdout.splitlines()
    lits = {}
    insns = []          # (address, mnemonic, operands, function, comment)
    func = sec = prev = None
    for line in out:
        if line.startswith('Disassembly of section '):
            sec = line.split()[-1].rstrip(':')
            continue
        m = FUNC_RE.match(line)
        if m:
            func = m.group(1)
            continue
        m = RELOC_RE.match(line)
        if m:
            if m.group(3) == 'kmalloc_caches':
                word = prev[2] if prev is not None and prev[1] == '.word' else '0'
                lits[(sec, int(m.group(1), 16))] = int(word, 16) if word.startswith('0x') else 0
            continue
        m = INSN_RE.match(line)
        if m:
            ops, _, comment = m.group(4).partition('\t')
            prev = ((sec, int(m.group(1), 16)), m.group(3), ops.strip(), func, comment)
            insns.append(prev)
    sites = []
    for i, (addr, mn, ops, fn, comment) in enumerate(insns):
        if not mn.startswith('ldr') or '[pc' not in ops:
            continue
        m = re.search(r'@ \(([0-9a-f]+)', comment)
        if not m:
            continue
        lit = (addr[0], int(m.group(1), 16))
        if lit not in lits:
            continue
        reg = ops.split(',')[0].strip()
        idx = None
        for a2, mn2, ops2, fn2, _ in insns[i + 1:i + 16]:
            if fn2 != fn:
                break
            m2 = re.match(r'^(\w+),\s*\[(\w+)(?:,\s*#(-?\d+))?\]$', ops2)
            if mn2.startswith('ldr') and m2 and m2.group(2) == reg:
                idx = (int(m2.group(3) or 0) + lits[lit]) // 4
                break
            if ops2.split(',')[0].strip() == reg:
                break       # register overwritten
        sites.append((fn, idx))
    return sites


# --------------------------------------------------------------------------
# What the vendor modules use

USES = ('storage', 'import', 'inline', 'deref')
# storage: the vendor code holds an object of the type in its own memory
#          (global, static, stack, or embedded by value in a vendor
#          structure), so the type's size and every member it or the kernel
#          touch must match;
# import:  type of a kernel variable the vendor code reads (init_task, ...);
# inline:  parameter/local type of a kernel header function inlined into, or
#          compiled out of line in, the vendor code;
# deref:   the vendor code holds a pointer to it (variable, parameter,
#          structure member) and may dereference it.


def pointee_key(cu, t, klass):
    """Key of the kernel struct/union (or typedef of one) a pointer type
    points to, else None."""
    t = cu.strip_all(t)
    if t is None or t.tag != 'DW_TAG_pointer_type':
        return None
    p = cu.strip(cu.t(t))
    if p is None:
        return None
    k = cu.key(p)
    if not k:
        return None
    if p.tag == 'DW_TAG_typedef':
        q = cu.strip_all(p)
        if q is None or q.tag not in AGG:
            return None
        if cu.key(q):
            k, p = cu.key(q), q     # typedef struct x x_t: compare struct x
    elif p.tag not in AGG:
        return None
    if klass(p) not in ('kernel', 'sstar'):
        return None
    return k


class Scan:
    def __init__(self):
        self.types = {}             # key -> layout record
        self.conflicts = defaultdict(set)
        self.refs = defaultdict(set)
        self.uses = defaultdict(lambda: defaultdict(set))   # key -> use -> {module}
        self.where = defaultdict(set)                        # key -> examples
        self.inlined = defaultdict(lambda: defaultdict(int))  # fn -> module -> count
        self.outofline = defaultdict(set)                    # fn -> {module}
        self.decls = {}             # imported name -> {file, line, kind, type}
        self.params = defaultdict(dict)     # module -> function -> [struct key|None, ...]
        self.statics = defaultdict(list)    # module -> [(variable, struct key)]
        self.vret = defaultdict(dict)       # module -> function -> struct key returned

    def use(self, key, how, mod, where):
        self.uses[key][how].add(mod)
        if len(self.where[key]) < 6:
            self.where[key].add(where)

    def cu(self, cu, mod, imports):
        kclass = {}

        def klass(d):
            if d.off not in kclass:
                kclass[d.off] = file_class(cu.decl_file(d))
            return kclass[d.off]

        def note(t, mod, where, inline=False):
            k, mode = cu.use_of(t)
            if not k:
                return
            d = cu.strip(t)
            # find the named type DIE to classify it
            while d is not None and not cu.key(d):
                d = cu.strip(cu.t(d))
            if d is None or klass(d) not in ('kernel', 'sstar'):
                return
            how = 'inline' if inline else ('storage' if mode == 'value' else 'deref')
            self.use(k, how, mod, where)

        def visit(d, ctx):
            tag = d.tag
            if tag in KW or tag == 'DW_TAG_typedef':
                k = cu.key(d)
                if k and not d.get('DW_AT_declaration'):
                    c = klass(d)
                    if c in ('kernel', 'sstar'):
                        rec = cu.layout(d)
                        rec['file'] = norm_path(cu.decl_file(d))
                        rec['line'] = d.get('DW_AT_decl_line')
                        rec['class'] = c
                        old = self.types.get(k)
                        if old is None:
                            self.types[k] = rec
                        elif {x: y for x, y in old.items() if x != 'line'} != \
                                {x: y for x, y in rec.items() if x != 'line'}:
                            self.conflicts[k].add(mod)
                        for r in cu.refs(d):
                            self.refs[k].add(r)
                    elif c == 'vendor' and tag in AGG:
                        for m in d.kids:
                            if m.tag == 'DW_TAG_member':
                                note(cu.t(m), mod, f'{mod}: {k}.{m.get("DW_AT_name", "")}')
            elif tag == 'DW_TAG_subprogram':
                o = cu.origin(d)
                name = o.get('DW_AT_name') if o is not None else None
                c = klass(d)
                if d.get('DW_AT_declaration'):
                    if name in imports and name not in self.decls:
                        self.decls[name] = {'kind': 'func', 'file': norm_path(cu.decl_file(d)),
                                            'line': d.get('DW_AT_decl_line')}
                    return
                if c in ('kernel', 'sstar'):
                    if d.get('DW_AT_low_pc') or d.get('DW_AT_ranges'):
                        if name:
                            self.outofline[name].add(mod)
                    ctx = ('inline', name)
                    for k in d.kids:
                        if k.tag == 'DW_TAG_formal_parameter' and 'DW_AT_type' in k.at:
                            note(cu.t(k), mod, f'{mod}: inline {name}()', inline=True)
                    if cu.t(d) is not None:
                        note(cu.t(d), mod, f'{mod}: inline {name}()', inline=True)
                else:
                    ctx = ('vendor', name)
                    if cu.t(d) is not None:
                        note(cu.t(d), mod, f'{mod}: {name}() return')
                    if name and (d.get('DW_AT_low_pc') or d.get('DW_AT_ranges')):
                        regs = []
                        for k in d.kids:
                            if k.tag != 'DW_TAG_formal_parameter':
                                continue
                            t = cu.t(k) if 'DW_AT_type' in k.at else cu.t(cu.origin(k))
                            if cu.sizeof(t) != 4 or len(regs) == 4:
                                break
                            regs.append(pointee_key(cu, t, klass))
                        if any(regs):
                            self.params[mod][name] = regs
                        rk = pointee_key(cu, cu.t(o), klass)
                        if rk:
                            self.vret[mod][name] = rk
            elif tag == 'DW_TAG_inlined_subroutine':
                o = cu.origin(d)
                if o is not None and klass(o) in ('kernel', 'sstar'):
                    self.inlined[o.get('DW_AT_name', '?')][mod] += 1
            elif tag in ('DW_TAG_variable', 'DW_TAG_formal_parameter') and 'DW_AT_type' in d.at:
                name = d.get('DW_AT_name', '')
                if ctx is None:
                    if d.get('DW_AT_declaration'):
                        if name in imports:
                            self.decls.setdefault(name, {
                                'kind': 'var', 'file': norm_path(cu.decl_file(d)),
                                'line': d.get('DW_AT_decl_line'), 'type': cu.name(cu.t(d))})
                            k, mode = cu.use_of(cu.t(d))
                            if k:
                                self.use(k, 'import', mod, f'{mod}: {name}')
                        return
                    if klass(d) == 'vendor':
                        note(cu.t(d), mod, f'{mod}: variable {name}')
                        k, mode = cu.use_of(cu.t(d))
                        if k and mode == 'value' and name:
                            self.statics[mod].append((name, k))
                elif ctx[0] == 'inline':
                    note(cu.t(d), mod, f'{mod}: inline {ctx[1]}()', inline=True)
                else:
                    where = 'parameter' if tag == 'DW_TAG_formal_parameter' else 'local'
                    note(cu.t(d), mod, f'{mod}: {ctx[1]}() {where} {name}')
            if tag in ('DW_TAG_subprogram', 'DW_TAG_lexical_block',
                       'DW_TAG_inlined_subroutine'):
                for k in d.kids:
                    visit(k, ctx)

        if (cu.root.get('DW_AT_name') or '').endswith('.mod.c'):
            return      # modpost's struct module: not part of the relinked object
        for k in cu.root.kids:
            visit(k, None)

    def ranked(self):
        """key -> (uses, depth): depth 0 for a type the vendor code uses
        directly, n for one reached through n pointers from those; by-value
        members of storage types are storage too."""
        uses = {k: set(v) for k, v in self.uses.items() if k in self.types}
        work = [k for k, u in uses.items() if 'storage' in u]
        while work:
            k = work.pop()
            for r, mode in self.refs.get(k, ()):
                if mode == 'value' and r in self.types and 'storage' not in uses.setdefault(r, set()):
                    uses[r].add('storage')
                    work.append(r)
        depth = {k: 0 for k in uses}
        frontier = list(uses)
        while frontier:
            nxt = []
            for k in frontier:
                for r, mode in self.refs.get(k, ()):
                    if r not in self.types:
                        continue
                    d = depth[k] + (1 if mode == 'pointer' else 0)
                    if d < depth.get(r, 99):
                        depth[r] = d
                        nxt.append(r)
            frontier = nxt
        mods = {k: set().union(*v.values()) for k, v in self.uses.items() if k in self.types}
        for k in sorted(depth, key=depth.get):
            for r, mode in self.refs.get(k, ()):
                if r in depth and depth[r] == depth[k] + (1 if mode == 'pointer' else 0):
                    mods.setdefault(r, set()).update(mods.get(k, ()))
        return uses, depth, mods



# --------------------------------------------------------------------------
# Which members the vendor code reads and writes
#
# A small dataflow pass over the Thumb-2/ARM disassembly of each vendor
# function. Registers are known to hold a pointer to a kernel structure when
# they get it from a parameter (r0-r3 at entry, typed by the DWARF), from a
# kernel function returning one (its prototype), from a kernel variable
# (init_task, registered_fb[], mem_map, ...), from "current" (sp masked to
# the thread_info), or by loading a pointer member of a known structure.
# Loads and stores through such a register are member accesses. The state
# is kept per basic block and merged by intersection, so a value is only
# used where every path agrees. It finds less than the code does (pointers
# spilled to the stack, register-indexed accesses, code reached through jump
# tables), never a member the code does not touch.

def ptr_key(tstr, types):
    """Type key a 'struct x *' / 'x_t *' type string points to."""
    m = re.match(r'^((?:struct|union) \w+|\w+) \*$', tstr or '')
    if not m:
        return None
    k = m.group(1) if ' ' in m.group(1) else 'typedef ' + m.group(1)
    return k if k in types and types[k].get('members') is not None else None


COND = ('eq', 'ne', 'cs', 'cc', 'hs', 'lo', 'mi', 'pl', 'vs', 'vc', 'hi', 'ls',
        'ge', 'lt', 'gt', 'le', 'al')
LOADS = {'ldr': 4, 'ldrb': 1, 'ldrh': 2, 'ldrsb': 1, 'ldrsh': 2, 'ldrd': 8, 'ldrex': 4}
STORES = {'str': 4, 'strb': 1, 'strh': 2, 'strd': 8}
NODEST = {'cmp', 'cmn', 'tst', 'teq', 'nop', 'push', 'dmb', 'dsb', 'isb', 'pld', 'pli',
          'cpsid', 'cpsie', 'svc', 'bkpt', 'udf', 'mcr', 'mcrr', 'sev', 'wfi', 'wfe',
          'yield', 'clrex', 'vpush', 'vpop', 'vstr', 'vstmia', 'vstmdb', 'msr'}
REG_RE = re.compile(r'\b(r\d+|ip|lr|sl|fp|sb)\b')
ALIAS = {'ip': 'r12', 'sl': 'r10', 'fp': 'r11', 'sb': 'r9'}


def _reg(x):
    x = x.strip()
    return ALIAS.get(x, x)


def split_mnemonic(mn):
    """(base mnemonic, conditional?) of e.g. 'ldrne.w' -> ('ldr', True)."""
    m = mn.split('.')[0]
    for base in sorted(set(LOADS) | set(STORES) | {'mov', 'movs', 'add', 'adds', 'addw', 'sub',
                                                   'subs', 'subw', 'bic', 'bics', 'ldmia',
                                                   'ldm', 'stmia', 'stm', 'pop', 'mvn',
                                                   'mvns'}, key=len, reverse=True):
        if m == base:
            return base, False
        if m.startswith(base) and m[len(base):] in COND:
            return base, True
    return m, False


class Accesses:
    def __init__(self, types, exports):
        self.types = types              # key -> layout (with members)
        self.ret = {}                   # function -> struct key it returns
        self.var = {}                   # kernel variable -> (kind, key)
        for s, e in exports.items():
            if e.get('kind') == 'func' and e.get('proto'):
                m = re.match(r'^((?:struct|union) \w+) \*\s', e['proto'])
                if m and m.group(1) in types:
                    self.ret[s] = m.group(1)
            elif e.get('kind') == 'var' and e.get('type'):
                t = e['type']
                m = re.match(r'^((?:struct|union) \w+)$', t)
                if m and m.group(1) in types:
                    self.var[s] = ('obj', m.group(1))
                m = re.match(r'^((?:struct|union) \w+) \*(?:\[\d*\])?$', t)
                if m and m.group(1) in types:
                    self.var[s] = ('ptrcell', m.group(1))
        self.seen = defaultdict(lambda: defaultdict(set))   # key -> (off, path, rw) -> mods
        self.sites = defaultdict(set)                       # (key, off) -> {(mod, sec, addr)}
        self.masks = defaultdict(set)                       # sp masks -> mods
        self.cache = {}
        self.where = None

    def member(self, key, off, depth=0):
        """(path, type string) of the member of key covering byte off."""
        ck = (key, off)
        if ck in self.cache:
            return self.cache[ck]
        r = None
        t = self.types.get(key)
        best = None
        for m in (t or {}).get('members', ()):
            mo, ms = m[1], m[2]
            if isinstance(mo, str):     # bitfield "byte.bit", size "Nb"
                b, bit = (int(x) for x in mo.split('.'))
                mo, ms = b, (bit + int(ms[:-1]) + 7) // 8
            if ms is None:
                continue
            if mo <= off < mo + max(ms, 1) or (ms == 0 and mo == off):
                if best is None or mo >= best[1]:
                    best = (m[0], mo, ms, m[3])
        if best:
            name, mo, ms, tstr = best
            sub = re.match(r'^((?:struct|union) \w+)$', tstr)
            if sub and sub.group(1) in self.types and depth < 6:
                p2, t2 = self.member(sub.group(1), off - mo, depth + 1)
                r = (f'{name}.{p2}' if p2 else name, t2)
            else:
                r = (name, tstr)
        self.cache[ck] = r or (None, None)
        return self.cache[ck]

    def module(self, path, objdump, params, mod, vret=None):
        """Scan one module. params: function -> [struct key or None for r0..]."""
        out = subprocess.run([objdump, '-dr', path], capture_output=True, text=True,
                             errors='replace').stdout.splitlines()
        funcs, lits = [], {}
        cur = sec = None
        last = None
        for line in out:
            if line.startswith('Disassembly of section '):
                sec = line.split()[-1].rstrip(':')
                continue
            m = FUNC_RE.match(line)
            if m:
                cur = [m.group(1), sec, []]
                funcs.append(cur)
                continue
            m = RELOC_RE.match(line)
            if m:
                a = (sec, int(m.group(1), 16))
                word = last[2] if last is not None and last[1] == '.word' else '0'
                add = int(word, 16) if word.startswith('0x') else 0
                lits[a] = (m.group(3), add)
                if cur and cur[2] and cur[2][-1][0] == a[1]:
                    cur[2][-1][4] = m.group(3)      # call target
                continue
            m = INSN_RE.match(line)
            if m and cur is not None:
                ops, _, comment = m.group(4).partition('\t')
                last = [int(m.group(1), 16), m.group(3), ops.strip(), comment, None]
                cur[2].append(last)
        for name, sec, insns in funcs:
            self.function(name, sec, insns, lits, params.get(name), mod, vret or {})

    def function(self, name, sec, insns, lits, params, mod, vret):
        insns = [i for i in insns if not i[1].startswith('.')]
        if not insns:
            return
        addrs = [i[0] for i in insns]
        index = {a: n for n, a in enumerate(addrs)}
        succ = defaultdict(list)
        leaders = {0}
        ends = set()
        for n, (a, mn, ops, cm, tgt) in enumerate(insns):
            base = mn.split('.')[0]
            kind = None
            if base in ('b', 'bal'):
                kind = 'jump'
            elif base.startswith('b') and base[1:] in COND:
                kind = 'cond'
            elif base in ('cbz', 'cbnz'):
                kind = 'cond'
            elif base in ('bx',) or (base in ('pop', 'ldmia', 'ldm') and 'pc' in ops) or \
                    (base.startswith('ldr') and ops.startswith('pc')):
                kind = 'ret'
            elif base in ('tbb', 'tbh'):
                kind = 'ret'        # successors unknown: their blocks start empty
            if kind is None:
                continue
            ends.add(n)
            if n + 1 < len(insns):
                leaders.add(n + 1)
            if kind in ('jump', 'cond'):
                m = re.search(r'\b([0-9a-f]+) <', ops)
                if m and int(m.group(1), 16) in index:
                    t = index[int(m.group(1), 16)]
                    leaders.add(t)
                    succ[n].append(t)
            if kind == 'cond' and n + 1 < len(insns):
                succ[n].append(n + 1)
        starts = sorted(leaders)
        blocks = {}
        for k, st in enumerate(starts):
            en = (starts[k + 1] if k + 1 < len(starts) else len(insns)) - 1
            blocks[st] = en
        bsucc = {}
        for st, en in blocks.items():
            if en in ends:
                bsucc[st] = [t for t in succ.get(en, [])]
            else:
                bsucc[st] = [en + 1] if en + 1 < len(insns) else []
        preds = defaultdict(list)
        for st, ss in bsucc.items():
            for t in ss:
                preds[t].append(st)
        entry = {}
        for i, k in enumerate((params or [])[:4]):
            if k:
                entry[f'r{i}'] = ('ptr', k, 0)
        out = {}
        work = list(starts)
        IN = {}
        rounds = 0
        while work and rounds < 20000:
            rounds += 1
            st = work.pop(0)
            if st == 0:
                state = dict(entry)
            else:
                ps = [out[p] for p in preds[st] if p in out]
                if not ps:
                    state = {}
                else:
                    state = dict(ps[0])
                    for o in ps[1:]:
                        state = {r: v for r, v in state.items() if o.get(r) == v}
            if IN.get(st) == state and st in out:
                continue
            IN[st] = state
            s2 = dict(state)
            for n in range(st, blocks[st] + 1):
                self.step(insns[n], s2, sec, lits, mod, vret, record=False)
            if out.get(st) != s2 or st not in out:
                out[st] = s2
                for t in bsucc[st]:
                    if t not in work:
                        work.append(t)
        for st in starts:
            s2 = dict(IN.get(st, {}))
            for n in range(st, blocks[st] + 1):
                self.step(insns[n], s2, sec, lits, mod, vret, record=True)

    def access(self, val, off, size, rw, mod, record, where=None):
        if not record or val is None or val[0] != 'ptr':
            return
        key, base = val[1], val[2]
        t = self.types.get(key, {})
        if t.get('kind') == 'typedef' and t.get('type') in self.types:
            key = t['type']         # typedef struct x x_t: the struct's members
        size = self.types.get(key, {}).get('size') or 0
        if not 0 <= base + off < size:
            return      # container_of() into the vendor's own structure
        path, _ = self.member(key, base + off)
        self.seen[key][(base + off, path or '?', rw)].add(mod)
        self.sites[(key, path or '?')].add((mod, self.where))

    def load_result(self, val, off):
        if val is None:
            return None
        if val[0] == 'ptrcell':
            return ('ptr', val[1], 0)
        if val[0] != 'ptr':
            return None
        path, tstr = self.member(val[1], val[2] + off)
        k = ptr_key(tstr, self.types)
        return ('ptr', k, 0) if k else None

    def step(self, insn, S, sec, lits, mod, vret, record):
        a, mn, ops, cm, tgt = insn
        self.where = (sec, a)
        base, cond = split_mnemonic(mn)
        o = [x.strip() for x in re.split(r',(?![^\[{]*[\]}])', ops)] if ops else []
        if base in ('bl', 'blx'):
            for r in ('r0', 'r1', 'r2', 'r3', 'r12', 'lr'):
                S.pop(r, None)
            callee = tgt
            if not callee:
                m = re.search(r'<([^+>]+)>', ops)
                callee = m.group(1) if m else None
            k = self.ret.get(callee) or vret.get(callee)
            if k:
                S['r0'] = ('ptr', k, 0)
            return
        if base in LOADS or base in STORES:
            size = LOADS.get(base) or STORES.get(base)
            regs = [_reg(x) for x in o if not x.startswith('[') and not x.startswith('#')]
            mem = next((x for x in o if x.startswith('[')), None)
            if mem is None:
                for r in regs:
                    S.pop(r, None)
                return
            mm = re.match(r'^\[(\w+)(?:,\s*#(-?\d+))?\](!?)$', mem)
            post = re.match(r'^#(-?\d+)$', o[-1]) if o and o[-1] != mem else None
            if base in LOADS and mm and mm.group(1) == 'pc':
                m = re.search(r'@ \(([0-9a-f]+)', cm)
                d = regs[0] if regs else None
                if d:
                    S.pop(d, None)
                if m and d and not cond:
                    lit = lits.get((sec, int(m.group(1), 16)))
                    if lit and lit[0] in self.var:
                        kind, key = self.var[lit[0]]
                        S[d] = ('ptr', key, lit[1]) if kind == 'obj' else ('ptrcell', key)
                return
            if not mm:          # register offset
                if base in LOADS:
                    for r in regs:
                        S.pop(r, None)
                return
            rb = _reg(mm.group(1))
            off = int(mm.group(2) or 0)
            bval = S.get(rb)
            rw = 'r' if base in LOADS else 'w'
            if size == 8:
                self.access(bval, off, 4, rw, mod, record)
                self.access(bval, off + 4, 4, rw, mod, record)
            else:
                self.access(bval, off, size, rw, mod, record)
            if base in LOADS:
                res = [self.load_result(bval, off)] if size == 4 else [None, None]
                for i, r in enumerate(regs[:2 if size == 8 else 1]):
                    v = res[i] if i < len(res) else None
                    if v and not cond:
                        S[r] = v
                    else:
                        S.pop(r, None)
            if mm.group(3) or post:     # writeback
                inc = off if mm.group(3) else int(post.group(1))
                if bval and bval[0] == 'ptr' and not cond:
                    S[rb] = ('ptr', bval[1], bval[2] + inc)
                else:
                    S.pop(rb, None)
            return
        if base in ('ldmia', 'ldm', 'pop', 'stmia', 'stm'):
            lst = re.search(r'\{([^}]*)\}', ops)
            regs = [_reg(x) for x in lst.group(1).split(',')] if lst else []
            if base in ('ldmia', 'ldm', 'stmia', 'stm') and o:
                rb = _reg(o[0].rstrip('!'))
                bval = S.get(rb)
                for i, _ in enumerate(regs):
                    self.access(bval, 4 * i, 4, 'r' if base.startswith('ld') else 'w', mod, record)
                if o[0].endswith('!'):
                    if bval and bval[0] == 'ptr' and not cond:
                        S[rb] = ('ptr', bval[1], bval[2] + 4 * len(regs))
                    else:
                        S.pop(rb, None)
            if base in ('ldmia', 'ldm', 'pop'):
                for r in regs:
                    S.pop(r, None)
            return
        if not o:
            return
        d = _reg(o[0])
        if base in NODEST or base.startswith('it') or base.startswith('b') and base != 'bic' \
                and base != 'bics' and base != 'bfi' and base != 'bfc' or base in ('cbz', 'cbnz'):
            return
        new = None
        if base in ('mov', 'movs') and len(o) == 2:
            if o[1] == 'sp':
                new = ('sp', 0)
            elif REG_RE.fullmatch(o[1]) or o[1] in ALIAS:
                new = S.get(_reg(o[1]))
        elif base in ('add', 'adds', 'addw', 'sub', 'subs', 'subw'):
            src, imm = (o[1], o[2]) if len(o) == 3 else (o[0], o[1] if len(o) > 1 else '')
            m = re.match(r'^#(-?\d+)$', imm)
            v = S.get(_reg(src))
            if m and v and v[0] == 'ptr':
                n = int(m.group(1)) * (1 if base.startswith('add') else -1)
                if v[2] + n >= 0:
                    new = ('ptr', v[1], v[2] + n)
        elif base in ('bic', 'bics'):
            src, imm = (o[1], o[2]) if len(o) == 3 else (o[0], o[1] if len(o) > 1 else '')
            m = re.match(r'^#(\d+)$', imm)
            v = S.get(_reg(src))
            if m and v and v[0] == 'sp':
                mask = v[1] | int(m.group(1))
                new = ('sp', mask)
                if mask >= 4095 and (mask + 1) & mask == 0:
                    if record:
                        self.masks[mask + 1].add(mod)
                    new = ('ptr', 'struct thread_info', 0) if 'struct thread_info' in self.types \
                        else None
        if base in ('umull', 'smull', 'umlal', 'smlal', 'mrc', 'mrrc'):
            for x in o:
                if REG_RE.fullmatch(x) or x in ALIAS:
                    S.pop(_reg(x), None)
            return
        if new is not None and not cond:
            S[d] = new
        else:
            S.pop(d, None)

    def statics(self, path, objs, mod):
        """Members a vendor object of a kernel type is initialised with at
        build time (DEFINE_MUTEX, a file_operations table, ...): non-zero
        or relocated words of its initial value. They count as writes: the
        kernel reads them with its own layout."""
        syms = elf_objects(path)
        for name, key in objs:
            if key not in self.types:
                continue
            hit = syms.get(name) or next((v for n, v in syms.items()
                                          if n.startswith(name + '.')), None)
            if hit is None:
                continue
            sec, body, rel = hit
            esize = self.types[key].get('size') or 0
            if not esize:
                continue
            self.where = ('static', name)
            for off in range(0, len(body) - 3, 4):
                if off in rel or body[off:off + 4] != b'\0\0\0\0':
                    self.access(('ptr', key, 0), off % esize, 4, 'w', mod, True)

    def result(self):
        """key -> sorted [[offset, member, 'r'|'w'|'rw', [modules]]]"""
        out = {}
        for key, accs in self.seen.items():
            merged = {}
            for (off, path, rw), mods in accs.items():
                e = merged.setdefault((off, path), [set(), set()])
                e[0].add(rw)
                e[1] |= mods
            # [offset, member, r/w, modules, instructions, static initialisers]
            out[key] = [[off, path, ''.join(sorted(rws, reverse=True)), sorted(mods),
                         sum(1 for m, w in self.sites[(key, path)] if w[0] != 'static'),
                         sum(1 for m, w in self.sites[(key, path)] if w[0] == 'static')]
                        for (off, path), (rws, mods) in sorted(merged.items())]
        return out

# --------------------------------------------------------------------------
# The probe: an object built against the target kernel, with debug info,
# that includes the headers the vendor code was built with and takes the
# type of every kernel symbol it imports.

# Kernel constants the vendor code may have compiled in through inline
# header code or macros, and the options that decide them.
CONSTS = [
    'THREAD_SIZE', 'THREAD_START_SP', 'PAGE_SHIFT', 'PAGE_OFFSET', 'TASK_SIZE',
    'PHYS_OFFSET', 'HZ', 'L1_CACHE_BYTES', 'ARCH_DMA_MINALIGN',
    'KMALLOC_SHIFT_LOW', 'KMALLOC_SHIFT_HIGH', 'KMALLOC_SHIFT_MAX', 'KMALLOC_MIN_SIZE',
    'NR_KMALLOC_TYPES', 'MAX_ORDER', 'NR_CPUS', 'MAX_NUMNODES', 'TASK_COMM_LEN',
    'FB_MAX', 'PREEMPT_BITS', 'BITS_PER_LONG', 'PAGE_SIZE', 'IDR_BITS',
    'TIMER_IRQSAFE', 'TIMER_DEFERRABLE', 'WORK_STRUCT_FLAG_BITS', 'MAX_SCHEDULE_TIMEOUT',
    'VM_IO', 'VM_PFNMAP', 'VM_DONTEXPAND', 'VM_DONTDUMP', 'VM_SHARED', 'VM_LOCKED',
    'MAP_SHARED', 'MAP_POPULATE', 'PROT_READ', 'PROT_WRITE',
    'O_RDONLY', 'O_WRONLY', 'O_RDWR', 'O_CREAT', 'O_TRUNC', 'O_APPEND', 'O_LARGEFILE',
    'GFP_KERNEL', 'GFP_ATOMIC', 'GFP_DMA', '__GFP_ZERO', '__GFP_NOWARN', '__GFP_HIGHMEM',
    'UMH_WAIT_PROC', 'UMH_WAIT_EXEC', 'UMH_NO_WAIT',
    'L_PTE_PRESENT', 'L_PTE_YOUNG', 'L_PTE_DIRTY', 'L_PTE_RDONLY', 'L_PTE_XN', 'L_PTE_SHARED',
    'L_PTE_MT_UNCACHED', 'L_PTE_MT_BUFFERABLE', 'L_PTE_MT_WRITETHROUGH',
    'L_PTE_MT_WRITEBACK', 'L_PTE_MT_DEV_SHARED', 'L_PTE_MT_MASK',
]
CONFIGS = [
    'SMP', 'PREEMPT', 'PREEMPT_COUNT', 'DEBUG_PREEMPT', 'DEBUG_SPINLOCK',
    'DEBUG_LOCK_ALLOC', 'LOCKDEP', 'PROVE_LOCKING', 'LOCK_STAT', 'DEBUG_MUTEXES',
    'DEBUG_ATOMIC_SLEEP', 'DEBUG_OBJECTS', 'SLUB', 'SLAB', 'SLOB', 'SLUB_DEBUG', 'KASAN',
    'FLATMEM', 'SPARSEMEM', 'HIGHMEM', 'ZONE_DMA', 'ARM_LPAE', 'CPU_SW_DOMAIN_PAN',
    'CPU_USE_DOMAINS', 'ARM_PATCH_PHYS_VIRT', 'THUMB2_KERNEL', 'MMU', 'CGROUPS', 'MEMCG',
    'NUMA', 'TRACING', 'TIMER_STATS', 'VMAP_STACK', 'THREAD_INFO_IN_TASK',
    'STACKPROTECTOR', 'CC_STACKPROTECTOR', 'STACKPROTECTOR_PER_TASK', 'GCC_PLUGINS',
    'MODULE_UNLOAD', 'MODVERSIONS', 'FB', 'FB_DEFERRED_IO', 'FB_TILEBLITTING', 'FB_BACKLIGHT',
    'ARM_MODULE_PLTS', 'UACCESS_WITH_MEMCPY', 'CPU_SPECTRE', 'HARDEN_BRANCH_PREDICTOR',
    'ARM_UNWIND', 'FRAME_POINTER', 'SECCOMP', 'AUDITSYSCALL', 'SCHED_INFO', 'SCHEDSTATS',
    'TASK_DELAY_ACCT', 'FAIR_GROUP_SCHED', 'RT_GROUP_SCHED', 'CPUSETS', 'KEYS', 'SECURITY',
    'FSNOTIFY', 'EPOLL', 'FILE_LOCKING', 'QUOTA', 'BLOCK', 'SYSFS', 'PROC_FS', 'DEBUG_FS',
    'PM', 'PM_SLEEP', 'OF', 'COMMON_CLK', 'GENERIC_IRQ_PROBE',
]


def probe_source(spec, drop, late=()):
    """C source of the probe. spec: headers, exports; drop: line keys to
    leave out (those that do not compile against the target); late: headers
    to include after the others (they need something those declare)."""
    lines = ['/* generated by scripts/kabi-check.py */']
    hs = [h for h in spec['headers'] if 'h:' + h not in late]
    hs += [h for h in spec['headers'] if 'h:' + h in late]
    for h in hs:
        k = 'h:' + h
        if k in drop:
            continue
        alts = include_alternatives(h)
        cond = ' || '.join(f'__has_include({a})' for a in alts)
        lines.append(f'#if {cond} /* {k} */')
        for i, a in enumerate(alts):
            lines.append(('#if ' if i == 0 else '#elif ') + f'__has_include({a})')
            lines.append(f'#include {a}')
        lines.append('#endif')
        lines.append('#endif')
    lines.append('enum kabi_const {')
    for c in CONSTS:
        k = 'c:' + c
        if k in drop:
            continue
        lines.append(f'#ifdef {c}')
        lines.append(f'\tKABI_C_{c} = ({c}), /* {k} */')
        lines.append('#endif')
    for c in CONFIGS:
        lines.append(f'\tKABI_CONFIG_{c} = IS_ENABLED(CONFIG_{c}),')
    lines.append('\tKABI_C_END')
    lines.append('};')
    lines.append('enum kabi_const kabi_const_ref;')
    for s in spec['exports']:
        k = 'x:' + s
        if k in drop:
            continue
        lines.append(f'__typeof__(&{s}) kabi_x_{s}; /* {k} */')
    for t in spec.get('types', ()):
        kind, _, name = t.partition(' ')
        if kind in ('struct', 'union'):
            k = 't:' + t
            if k not in drop:
                lines.append(f'{t} *kabi_t_{kind}_{name}; /* {k} */')
    return '\n'.join(lines) + '\n'


def include_alternatives(h):
    """How a header recorded in the vendor DWARF is included, most specific
    first."""
    p = norm_path(h)
    m = re.match(r'^include/uapi/(.*)$', p)
    if m:
        return [f'<{m.group(1)}>', f'<uapi/{m.group(1)}>']
    m = re.match(r'^include/(.*)$', p)
    if m:
        return [f'<{m.group(1)}>']
    m = re.match(r'^arch/[^/]+/include/(?:uapi/|generated/(?:uapi/)?)?(asm/.*)$', p)
    if m:
        return [f'<{m.group(1)}>']
    m = re.match(r'^drivers/[sm]star/include/(?:[^/]+/)?([^/]+)$', p)
    if m:
        return [f'"{m.group(1)}"']
    return [f'"{os.path.basename(p)}"']


FIRST = ['include/linux/kernel.h', 'include/linux/module.h', 'include/linux/types.h',
         'include/linux/sched.h', 'include/linux/fs.h', 'include/linux/mm.h',
         'include/linux/slab.h', 'include/linux/device.h']


def header_order(headers):
    def rank(h):
        if h in FIRST:
            return (0, FIRST.index(h))
        c = file_class(h)
        if h.startswith('include/linux/'):
            return (1, h)
        if h.startswith('include/'):
            return (2, h)
        if c == 'kernel':
            return (3, h)
        return (4, h)
    return sorted(set(headers) | set(FIRST), key=rank)


def kernel_srctree(kdir):
    """The source tree behind a kernel build directory."""
    src = os.path.join(kdir, 'source')
    if os.path.isdir(src):
        return os.path.realpath(src)
    return os.path.realpath(kdir)


def sstar_include_dirs(srctree, spec, chiparch):
    """-I for the SigmaStar headers the vendor code included, wherever this
    kernel keeps them."""
    want = {os.path.basename(h) for h in spec['headers'] if file_class(h) == 'sstar'}
    base = os.path.join(srctree, 'drivers', 'sstar')
    if not os.path.isdir(base):
        return []
    found = {}
    for root, dirs, files in os.walk(base):
        dirs.sort()
        for f in files:
            if f in want:
                found.setdefault(f, []).append(root)
    dirs = []
    for d in [os.path.join(base, 'include'), os.path.join(base, 'include', chiparch or '')]:
        if os.path.isdir(d):
            dirs.append(d)
    for f in sorted(found):
        if any(os.path.isfile(os.path.join(d, f)) for d in dirs):
            continue
        cands = sorted(found[f], key=lambda r: (chiparch not in r, 'include' not in r, len(r)))
        dirs.append(cands[0])
    return dirs


def build_probe(kdir, spec, cross, arch, chiparch, workdir, verbose=False):
    """Build the probe against kdir; returns (object path, dropped keys)."""
    srctree = kernel_srctree(kdir)
    incs = sstar_include_dirs(srctree, spec, chiparch)
    os.makedirs(os.path.join(workdir, '.tmp_versions'), exist_ok=True)     # 4.x Kbuild
    with open(os.path.join(workdir, 'Kbuild'), 'w') as f:
        f.write('obj-m := kabi_probe.o\n')
        # full struct debug info whatever the kernel's DEBUG_INFO_REDUCED says
        # (-femit-struct-debug-baseonly would leave the headers' structs out)
        f.write('ccflags-y := -g -gdwarf-4 -fno-eliminate-unused-debug-types '
                '-femit-struct-debug-detailed=any -fno-var-tracking -w ' +
                ' '.join('-I' + d for d in incs) + '\n')
    drop, late = set(), set()
    for attempt in range(40):
        src = probe_source(spec, drop, late)
        with open(os.path.join(workdir, 'kabi_probe.c'), 'w') as f:
            f.write(src)
        obj = os.path.join(workdir, 'kabi_probe.o')
        if os.path.exists(obj):
            os.unlink(obj)
        # -f Makefile: past an SDK "makefile" wrapper, which loses single targets
        cmd = ['make', '-C', kdir, '-f', 'Makefile', f'M={workdir}', f'ARCH={arch}',
               'kabi_probe.o']
        if cross:
            cmd.append(f'CROSS_COMPILE={cross}')
        r = subprocess.run(cmd, capture_output=True, text=True, errors='replace')
        if r.returncode == 0 and os.path.exists(obj):
            return obj, drop
        bad = probe_errors(r.stdout + r.stderr, src)
        if verbose:
            print(f'probe attempt {attempt}: dropping {sorted(bad)}', file=sys.stderr)
        if not bad or bad <= drop:
            sys.stderr.write(r.stdout[-4000:] + r.stderr[-4000:])
            sys.exit('kabi-check: the probe does not build against ' + kdir)
        # a header that fails is first moved after the others, then dropped
        for k in bad:
            if k.startswith('h:') and k not in late:
                late.add(k)
            else:
                drop.add(k)
    sys.exit('kabi-check: the probe does not build against ' + kdir)


def probe_errors(out, src):
    """Keys of the probe lines behind the compiler's errors."""
    lines = src.split('\n')
    bad = set()
    chain = []
    for l in out.split('\n'):
        m = re.search(r'kabi_probe\.c:(\d+)', l)
        if l.startswith('In file included from') or l.lstrip().startswith('from '):
            if m:
                chain.append(int(m.group(1)))
            continue
        if ': error:' in l or ': fatal error:' in l:
            n = chain[0] if chain else (int(m.group(1)) if m else None)
            if n is not None:
                key = line_key(lines, n)
                if key:
                    bad.add(key)
        chain = []
    return bad


def line_key(lines, n):
    """The key of probe line n (1-based): its own /* key */, or that of the
    #if block around an #include."""
    for i in range(n - 1, max(-1, n - 8), -1):
        m = re.search(r'/\* ([hcxt]:[^*]+) \*/', lines[i])
        if m:
            return m.group(1).strip()
    return None


def read_probe(obj, readelf, want=()):
    """Types, export prototypes and constants from a probe object (or any
    ELF with DWARF, a vmlinux: every named type, first definition wins, and
    the prototypes of the functions and variables in want)."""
    types, exports, consts = {}, {}, {}
    want = set(want)
    for root, dies in read_cus(obj, readelf):
        cu = CU(root, dies)
        for d in root.kids:
            tag = d.tag
            name = d.get('DW_AT_name')
            if name in want and name not in exports and d.get('DW_AT_external') \
                    and not d.get('DW_AT_declaration'):
                if tag == 'DW_TAG_subprogram':
                    exports[name] = {'kind': 'func', 'proto': cu.sig(d, '', name)}
                elif tag == 'DW_TAG_variable' and 'DW_AT_type' in d.at:
                    exports[name] = {'kind': 'var', 'type': cu.name(cu.t(d))}
            if (tag in KW or tag == 'DW_TAG_typedef') and not d.get('DW_AT_declaration'):
                k = cu.key(d)
                if k and k not in types:
                    types[k] = cu.layout(d)
                if tag == 'DW_TAG_enumeration_type' and d.get('DW_AT_name') == 'kabi_const':
                    for e in d.kids:
                        n = e.get('DW_AT_name', '')
                        v = e.get('DW_AT_const_value')
                        if n.startswith('KABI_C_') and n != 'KABI_C_END':
                            consts[n[7:]] = v
                        elif n.startswith('KABI_CONFIG_'):
                            consts['CONFIG_' + n[12:]] = v
            elif tag == 'DW_TAG_variable' and (d.get('DW_AT_name') or '').startswith('kabi_x_'):
                sym = d.at['DW_AT_name'][7:]
                t = cu.strip(cu.t(d))           # pointer to the symbol's type
                st = cu.strip(cu.t(t)) if t is not None else None
                if st is not None and st.tag == 'DW_TAG_subroutine_type':
                    exports[sym] = {'kind': 'func', 'proto': cu.sig(st, '', sym)}
                else:
                    exports[sym] = {'kind': 'var', 'type': cu.name(cu.t(t)) if t is not None else '?'}
    return types, exports, consts


def config_deps(srctree, path, line):
    """CONFIG_ options a type's definition depends on in the source: those
    tested inside its body, and those of the #if blocks around it."""
    f = os.path.join(srctree, path)
    if not line or not os.path.isfile(f):
        return []
    with open(f, errors='replace') as fh:
        text = fh.read().split('\n')
    out = set()
    stack = []
    for l in text[:line - 1]:
        s = l.strip()
        m = re.match(r'#\s*(if|ifdef|ifndef|elif|else|endif)\b(.*)', s)
        if not m:
            continue
        if m.group(1) in ('if', 'ifdef', 'ifndef'):
            stack.append(set(re.findall(r'CONFIG_\w+', m.group(2))))
        elif m.group(1) == 'elif' and stack:
            stack[-1] |= set(re.findall(r'CONFIG_\w+', m.group(2)))
        elif m.group(1) == 'endif' and stack:
            stack.pop()
    for st in stack:
        out |= st
    depth, seen = 0, False
    for l in text[line - 1:line + 2000]:
        s = l.strip()
        if s.startswith('#') and re.match(r'#\s*(if|ifdef|ifndef|elif)\b', s):
            out.update(re.findall(r'CONFIG_\w+', s))
        code = re.sub(r'//.*|/\*.*?\*/', '', l)
        depth += code.count('{') - code.count('}')
        if code.count('{'):
            seen = True
        if seen and depth <= 0 or not seen and ';' in code:
            break       # end of the body, or a typedef/declaration of one line
    return sorted({c.replace('_MODULE', '') for c in out})


# --------------------------------------------------------------------------
# extract

def cmd_extract(a):
    readelf = tool(a.cross, 'readelf')
    objdump = tool(a.cross, 'objdump')
    mods = sorted(a.ko)
    syms = {os.path.basename(m)[:-3]: elf_symbols(m) for m in mods}
    vendor_defined = set().union(*(d for d, u in syms.values()))
    sc = Scan()
    imports = {}
    for m in mods:
        mod = os.path.basename(m)[:-3]
        imp = syms[mod][1] - vendor_defined
        imports[mod] = sorted(imp)
        files = read_line_files(m, readelf)
        for root, dies in read_cus(m, readelf):
            sc.cu(CU(root, dies, files.get(root.get('DW_AT_stmt_list'))), mod, imp)
        print(f'{mod}: {len(imp)} kernel imports', file=sys.stderr)
    uses, depth, tmods = sc.ranked()
    keep = sorted(k for k in sc.types if k in depth)
    srctree = kernel_srctree(a.sdk_kdir)
    exports = defaultdict(lambda: {'modules': []})
    for mod, imp in imports.items():
        for s in imp:
            exports[s]['modules'].append(mod)
    headers = {sc.types[k]['file'] for k in keep}
    headers |= {d['file'] for s, d in sc.decls.items() if s in exports and d.get('file')
                and file_class(d['file']) in ('kernel', 'sstar')}
    headers = [h for h in header_order(headers) if file_class(h) in ('kernel', 'sstar')]
    spec = {'headers': headers, 'exports': sorted(exports), 'types': keep}
    work = a.workdir or tempfile.mkdtemp(prefix='kabi-')
    try:
        obj, dropped = build_probe(a.sdk_kdir, spec, a.cross, a.arch, a.chiparch, work, a.verbose)
        ptypes, pexports, consts = read_probe(obj, readelf)
    finally:
        if not a.workdir and not a.keep:
            shutil.rmtree(work, ignore_errors=True)
    out_types = {}
    for k in keep:
        r = dict(sc.types[k])
        r['use'] = sorted(uses.get(k, ()))
        r['depth'] = depth[k]
        r["modules"] = sorted(tmods.get(k, ()))
        r['where'] = sorted(sc.where.get(k, ()))[:3]
        cfg = config_deps(srctree, r['file'], r.get('line'))
        if cfg:
            r['configs'] = cfg
        if k in ptypes and not same_layout(r, ptypes[k]):
            print(f'warning: {k}: the SDK kernel probe differs from the vendor DWARF',
                  file=sys.stderr)
        out_types[k] = r
    for s, e in exports.items():
        d = sc.decls.get(s)
        if d and d.get('file'):
            f = d['file']
            if file_class(f) == 'vendor':
                f = 'vendor/' + os.path.basename(f)    # the SDK's own source
            e['decl'] = f"{f}:{d.get('line')}"
        if s in pexports:
            e.update(pexports[s])
            e['hdr'] = True         # declared in the SDK kernel's headers
        elif d and d.get('kind') == 'var':
            e.update({'kind': 'var', 'type': d.get('type')})
    kmalloc = []
    acc = Accesses(out_types, exports)
    for m in mods:
        mod = os.path.basename(m)[:-3]
        for fn, idx in kmalloc_sites(m, objdump):
            kmalloc.append({'module': mod, 'function': fn, 'index': idx})
        acc.module(m, objdump, sc.params[mod], mod, sc.vret[mod])
        acc.statics(m, sc.statics[mod], mod)
    for k, lst in acc.result().items():
        if k in out_types:
            out_types[k]['access'] = lst
    print(f'member accesses found in {len(acc.seen)} types', file=sys.stderr)
    data = {
        'chiparch': a.chiparch,
        'about': a.about,
        'modules': {m: len(v) for m, v in imports.items()},
        'licenses': {os.path.basename(m)[:-3]: elf_modinfo(m).get('license', '') for m in mods},
        'consts': consts,
        'kmalloc': kmalloc,
        'thread_size': sorted(acc.masks),
        'exports': {s: dict(sorted(e.items())) for s, e in sorted(exports.items())},
        'inlined': {f: dict(sorted(v.items())) for f, v in sorted(sc.inlined.items())},
        'outofline': {f: sorted(v) for f, v in sorted(sc.outofline.items())},
        'headers': headers,
        'types': out_types,
    }
    write_json(a.out, data)
    print(f'{a.out}: {len(out_types)} types, {len(exports)} exports, '
          f'{len(kmalloc)} kmalloc sites, {len(sc.inlined)} inlined kernel functions',
          file=sys.stderr)


def write_json(path, data):
    """One line per type/export: compact, and diffs stay readable."""
    with open(path, 'w') as f:
        f.write('{\n')
        items = list(data.items())
        for i, (k, v) in enumerate(items):
            f.write(f'{json.dumps(k)}: ')
            if isinstance(v, dict) and k in ('types', 'exports', 'inlined', 'outofline', 'consts'):
                f.write('{\n')
                sub = list(v.items())
                for j, (k2, v2) in enumerate(sub):
                    f.write(f'  {json.dumps(k2)}: {json.dumps(v2, separators=(",", ":"))}')
                    f.write(',\n' if j < len(sub) - 1 else '\n')
                f.write('}')
            else:
                f.write(json.dumps(v, separators=(',', ':')))
            f.write(',\n' if i < len(items) - 1 else '\n')
        f.write('}\n')


LAYOUT_KEYS = ('kind', 'size', 'members', 'values', 'type')


def norm_members(ms):
    return [[m[0], m[1], m[2], m[3].replace('_Bool', 'bool')] for m in ms]


def same_layout(a, b):
    for k in LAYOUT_KEYS:
        x, y = a.get(k), b.get(k)
        if k == 'members' and x is not None and y is not None:
            x, y = norm_members(x), norm_members(y)
        if x != y:
            return False
    return True


# --------------------------------------------------------------------------
# check

class Canon:
    """Type strings with scalar typedefs spelled out (u32, __u32 and
    unsigned int are the same type), and struct wait_queue_head /
    struct __wait_queue_head the same when the typedef says so."""

    def __init__(self, types, other=None):
        self.types, self.memo = types, {}
        self.other = other if other is not None else types

    def __call__(self, t):
        if t is None:
            return None
        r = self.memo.get(t)
        if r is None:
            self.memo[t] = t
            r = self.memo[t] = re.sub(r'\b[A-Za-z_]\w*\b', self.word, t).replace('_Bool', 'bool')
        return r

    def word(self, m):
        w = m.group(0)
        # this side's typedef, else the other side's (va_list and others the
        # vendor file does not record)
        rec = self.types.get('typedef ' + w) or self.other.get('typedef ' + w)
        if rec and rec.get('kind') == 'typedef' and rec.get('type') and \
                not re.match(r'^(struct|union|enum) ', rec['type']):
            return self(rec['type'])
        return w


def is_fnptr(t):
    return t is not None and '(*)' in t


def fn_kind(a, b, ca, cb):
    """'fntype' when two function pointer types pass their arguments
    differently, else 'type' (other pointee types only)."""
    def classes(t, canon):
        m = re.match(r'^(.*?)\s*\(\*\)\((.*)\)$', t or '')
        if not m:
            return None
        ret, params = split_proto(f'{m.group(1)} f({m.group(2)})')
        return [abi_class(ret, canon)] + [abi_class(x, canon) for x in params]
    x, y = classes(a, ca), classes(b, cb)
    return 'type' if x is not None and x == y else 'fntype'


def diff_type(a, b, ca=None, cb=None):
    """[(kind, text)] differences of type b (target) from a (vendor);
    kind is 'size', 'offset', 'type', 'fntype' (a function pointer's
    prototype), 'removed', 'added', 'value'. ca/cb canonicalise type
    strings on each side."""
    ca = ca or (lambda x: x)
    cb = cb or (lambda x: x)
    if b is None:
        return [('removed', 'not in the target kernel')]
    out = []
    if a.get('kind') != b.get('kind'):
        out.append(('type', f"{a.get('kind')} -> {b.get('kind')}"))
    if a.get('size') != b.get('size'):
        out.append(('size', f"size {a.get('size')} -> {b.get('size')}"))
    if 'type' in a and ca(a.get('type')) != cb(b.get('type')) and not a.get('members') \
            and not a.get('values'):
        out.append((fn_kind(a.get('type'), b.get('type'), ca, cb) if is_fnptr(a.get('type'))
                    else 'type', f"{a.get('type')} -> {b.get('type')}"))
    if 'members' in a:
        bm = {m[0]: m for m in norm_members(b.get('members') or [])}
        am = norm_members(a['members'])
        names = {m[0] for m in am}
        for n, off, size, t in am:
            m = bm.get(n)
            if m is None:
                out.append(('removed', f'{n or "<unnamed>"} (at {off}) removed'))
                continue
            if m[1] != off:
                out.append(('offset', f'{n} offset {off} -> {m[1]}'))
            if m[2] != size:
                out.append(('size', f'{n} size {size} -> {m[2]}'))
            if ca(t) != cb(m[3]):
                out.append((fn_kind(t, m[3], ca, cb) if is_fnptr(t) else 'type',
                            f'{n} type {t} -> {m[3]}'))
        for n, m in bm.items():
            if n not in names:
                out.append(('added', f'{n} added at {m[1]}'))
    if 'values' in a:
        bv = b.get('values') or {}
        for n, v in a['values'].items():
            if n not in bv:
                out.append(('removed', f'{n} removed'))
            elif bv[n] != v:
                out.append(('value', f'{n} {v} -> {bv[n]}'))
    return out


def resolve_path(types, key, path):
    """(offset, size, type) of member path (a.b.c) in type key, or None."""
    off = 0
    t = types.get(key)
    parts = path.split('.')
    i = 0
    while t is not None and i < len(parts):
        ms = t.get('members') or []
        # anonymous aggregates are flattened as "a.b" too: try the longest name
        for j in range(len(parts), i, -1):
            name = '.'.join(parts[i:j])
            m = next((m for m in ms if m[0] == name), None)
            if m:
                break
        if m is None:
            return None
        o = m[1] if not isinstance(m[1], str) else m[1]
        if isinstance(o, str):
            return (o, m[2], m[3]) if j == len(parts) else None
        off += o
        i = j
        if i == len(parts):
            return (off, m[2], m[3].replace('_Bool', 'bool'))
        sub = re.match(r'^((?:struct|union) \w+)$', m[3])
        t = types.get(sub.group(1)) if sub else None
    return None


def member_at(types, key, off, depth=0):
    """(offset, size, type) of the innermost member of key at byte off."""
    t = types.get(key) or {}
    best = None
    for m in t.get('members') or ():
        if isinstance(m[1], str) or m[2] is None:
            continue
        if m[1] <= off < m[1] + max(m[2], 1) and (best is None or m[1] >= best[1]):
            best = m
    if best is None:
        return None
    sub = re.match(r'^((?:struct|union) \w+)$', best[3])
    if sub and sub.group(1) in types and depth < 6:
        inner = member_at(types, sub.group(1), off - best[1], depth + 1)
        if inner:
            return (best[1] + inner[0], inner[1], inner[2])
    return (best[1], best[2], best[3])


def impacted(key, r, base_types, ttypes, ca, cb):
    """Accessed members of key whose offset, size or type differ."""
    out = []
    for acc in r.get('access', ()):
        off, path, rw, mods = acc[:4]
        if path == '?':
            continue
        a = resolve_path(base_types, key, path)
        b = resolve_path(ttypes, key, path)
        if b is None and a is not None and not isinstance(a[0], str):
            # renamed in place (wait_queue_head.task_list -> head)?
            b2 = member_at(ttypes, key, a[0])
            if b2 and b2[:2] == a[:2] and ca(a[2]) == cb(b2[2]):
                b = a
        # same place and size with another type (unsigned long -> __poll_t,
        # atomic_t -> refcount_t) reads the same; a function pointer does not
        if a is None or b is None or a[:2] != b[:2] or \
                (ca(a[2]) != cb(b[2]) and (is_fnptr(a[2]) or is_fnptr(b[2]))):
            out.append((path, rw, a, b, mods, acc[4] if len(acc) > 4 else None,
                        acc[5] if len(acc) > 5 else 0))
    return out


def importance(r):
    """Sort key and class of a vendor-used type: A storage (the vendor code
    owns such objects), B used directly (imported variable, inline kernel
    code, pointer the vendor code holds), C/D reached through 1/2+ pointers."""
    use = set(r.get('use', ()))
    if r['depth'] == 0 and 'storage' in use:
        return 'A'
    if r['depth'] == 0 and use:
        return 'B'
    return 'C' if r['depth'] <= 1 else 'D'


def level(cls, diffs, imp, base_size, target_size):
    """'!' breaks the vendor code, '?' changed where it may (no access to
    the changed members was found), ' ' harmless for how it is used."""
    kinds = {k for k, _ in diffs} - {'renamed'}
    if not kinds:
        return ' '
    if imp or ('fntype' in kinds and cls == 'A'):
        return '!'      # an accessed member moved, or the kernel calls the
                        # vendor's function with another prototype
    if cls == 'A':
        if kinds <= {'type', 'value'}:
            return '?'      # same layout, other types (atomic_t -> refcount_t ...)
        if 'removed' in kinds and any(t == 'not in the target kernel' for _, t in diffs):
            return '!'
        if target_size is not None and base_size is not None and target_size > base_size:
            return '!'      # the kernel writes past the vendor's object
        return '?'          # fits, and no member the vendor code touches moved
    return '?' if matters(cls, diffs) else ' '


def matters(cls, diffs):
    """Whether a difference can break the vendor code, by how it uses the
    type. Members appended to a type the vendor code only reaches by
    pointer are harmless; for objects in vendor memory every size change
    is (the kernel writes the larger object)."""
    kinds = {k for k, _ in diffs}
    if cls == 'A':
        return bool(kinds)
    real = kinds - {'added'}
    if real == {'size'} and all(t.startswith('size ') for k, t in diffs if k == 'size'):
        return False        # grew or shrank at the end only
    return bool(real)


def split_proto(p):
    """('ret', [params]) of 'ret name(a, b)'."""
    m = re.match(r'^(.*?)\s*\b\w+\((.*)\)$', p or '')
    if not m:
        return None, []
    params, depth, cur = [], 0, ''
    for ch in m.group(2):
        if ch == ',' and depth == 0:
            params.append(cur.strip())
            cur = ''
            continue
        depth += (ch == '(') - (ch == ')')
        cur += ch
    if cur.strip():
        params.append(cur.strip())
    return m.group(1).strip(), [x for x in params if x != 'void']


def abi_class(t, canon):
    """How an argument travels: pointers are all alike at the call."""
    if '*' in t or '[' in t:
        return 'ptr'
    return canon(t)


def proto_change(old, new, ca, cb):
    """None (same), 'ret' (only a return value the vendor code never had),
    'types' (same registers, other pointee types), or 'args'."""
    r1, p1 = split_proto(old)
    r2, p2 = split_proto(new)
    if [ca(x) for x in p1] == [cb(x) for x in p2] and ca(r1) == cb(r2):
        return None
    if [abi_class(x, ca) for x in p1] != [abi_class(x, cb) for x in p2]:
        return 'args'
    if r1 != 'void' and abi_class(r1, ca) != abi_class(r2, cb):
        return 'args'
    if [ca(x) for x in p1] == [cb(x) for x in p2] and r1 == 'void':
        return 'ret'
    return 'types'


def export_class(e):
    """kernel, or sstar: a SigmaStar platform export (CamOs, MIU, MMU,
    msys, padmux...), which depends on the SigmaStar port of the kernel
    rather than its version."""
    d = e.get('decl')
    if d:
        return 'kernel' if file_class(d.split(':')[0]) == 'kernel' else 'sstar'
    return 'kernel'


def read_symvers(kdir):
    p = os.path.join(kdir, 'Module.symvers')
    if not os.path.isfile(p):
        return None
    out = {}
    with open(p) as f:
        for l in f:
            parts = l.rstrip('\n').split('\t')
            if len(parts) >= 4:
                out[parts[1]] = parts[3]        # EXPORT_SYMBOL or EXPORT_SYMBOL_GPL
    return out


def kmalloc_ok(idx, c):
    """Whether kmalloc_caches[idx] exists in a kernel with constants c and
    holds objects of the size the SDK kernel's kmalloc_index() meant."""
    lo, hi, mn = c.get('KMALLOC_SHIFT_LOW'), c.get('KMALLOC_SHIFT_HIGH'), c.get('KMALLOC_MIN_SIZE')
    if None in (lo, hi, mn) or idx is None:
        return None
    if idx > hi:
        return False
    if idx >= lo:
        return True
    return (idx == 1 and mn <= 32) or (idx == 2 and mn <= 64)


def cmd_check(a):
    readelf = tool(a.cross, 'readelf')
    with open(a.json) as f:
        base = json.load(f)
    spec = {'headers': base['headers'], 'exports': sorted(base['exports']),
            'types': sorted(base['types'])}
    work = a.workdir or tempfile.mkdtemp(prefix='kabi-')
    try:
        if a.dwarf:
            obj, dropped = a.dwarf, set()
        else:
            obj, dropped = build_probe(a.kdir, spec, a.cross, a.arch, base['chiparch'],
                                       work, a.verbose)
        ttypes, texports, tconsts = read_probe(obj, readelf, base['exports'] if a.dwarf else ())
    finally:
        if not a.workdir and not a.keep:
            shutil.rmtree(work, ignore_errors=True)
    symvers = read_symvers(a.kdir) if a.kdir else None
    if a.dwarf:
        # a vmlinux describes definitions, often with other typedef spellings
        # than the headers' declarations: existence only (Module.symvers)
        texports = {}
        dropped = {'x:' + s for s in base['exports']}
    res = compare(base, ttypes, texports, tconsts, symvers, dropped)
    res['target'] = a.name or a.kdir or a.dwarf
    report(res, base, a)
    if a.save:
        with open(a.save, 'w') as f:
            json.dump(res, f, indent=1, sort_keys=True)
    return 1 if res['breaking'] else 0


# Types the kernel renamed; compared under the new name when the old one is
# gone (the report says so).
RENAMED = {
    'struct __wait_queue_head': 'struct wait_queue_head',
    'struct __wait_queue': 'struct wait_queue_entry',
    'typedef wait_queue_t': 'struct wait_queue_entry',
    'struct timespec': 'struct __kernel_old_timespec',
    'struct timeval': 'struct __kernel_old_timeval',
    'typedef __kernel_time_t': 'typedef __kernel_old_time_t',
}


def compare(base, ttypes, texports, tconsts, symvers, dropped):
    for old, new in RENAMED.items():
        if old not in ttypes and new in ttypes:
            r = dict(ttypes[new])
            r['renamed'] = new
            if r.get('kind') == 'struct' and old.startswith('typedef '):
                r['kind'] = 'typedef struct'
            ttypes[old] = r
    types = []
    ca, cb = Canon(base['types'], ttypes), Canon(ttypes, base['types'])
    for k, r in base['types'].items():
        d = diff_type(r, ttypes.get(k), ca, cb)
        if not d:
            continue
        cls = importance(r)
        imp = impacted(k, r, base['types'], ttypes, ca, cb)
        lv = level(cls, d, imp, r.get('size'), (ttypes.get(k) or {}).get('size'))
        if (ttypes.get(k) or {}).get('renamed'):
            d.insert(0, ('renamed', f"now {ttypes[k]['renamed']}"))
        types.append({'type': k, 'class': cls, 'depth': r['depth'], 'use': r.get('use', []),
                      'modules': r.get('modules', []), 'diffs': d, 'level': lv,
                      'matters': lv == '!', 'impacted': imp,
                      'accessed': len(r.get('access', ())),
                      'configs': r.get('configs', [])})
    types.sort(key=lambda t: ('!? '.index(t['level']), t['class'], -len(t['modules']), t['type']))
    exports = []
    for s, e in base['exports'].items():
        t = texports.get(s)
        exported = None if symvers is None else s in symvers
        prob, lv = [], ' '
        if exported is False:
            prob.append('not exported')
            lv = '!'
        elif exported and symvers[s].endswith('_GPL'):
            lic = base.get('licenses', {})
            bad = [m for m in e['modules'] if lic.get(m) not in GPL_LICENSES]
            if bad:
                prob.append(f"GPL-only in the target, imported by non-GPL {', '.join(bad)}")
                lv = '!'
        if t is None:
            if exported is None and 'x:' + s in dropped and e.get('hdr'):
                prob.append('not declared in the headers')
                lv = '!'
        elif e.get('kind') == 'func' and t.get('kind') == 'func':
            if e.get('proto') and t.get('proto'):
                ch = proto_change(e['proto'], t['proto'], ca, cb)
                if ch:
                    prob.append(f"prototype {e['proto']} -> {t['proto']}")
                    lv = min(lv, {'args': '!', 'types': '?', 'ret': ' '}[ch], key='!? '.index)
                    if ch == 'ret':
                        prob[-1] += ' (only a return value added)'
        elif e.get('kind') and e.get('kind') != t.get('kind'):
            prob.append(f"{e.get('kind')} -> {t.get('kind')}")
            lv = '!'
        elif s == 'kmalloc_caches':
            pass        # checked below, by the indexes the inline kmalloc() uses
        elif e.get('type') and ca(e['type']) != cb(t.get('type')):
            prob.append(f"type {e['type']} -> {t['type']}")
            lv = '!'
        if prob:
            exports.append({'symbol': s, 'modules': e['modules'], 'problems': prob,
                            'level': lv, 'class': export_class(e),
                            'missing': exported is False})
    exports.sort(key=lambda x: (x['class'] != 'kernel', '!? '.index(x['level']), x['symbol']))
    consts = []
    for c, v in sorted(base['consts'].items() if tconsts else ()):
        tv = tconsts.get(c)
        if tv != v:
            # None: not a macro the probe can see there (TASK_COMM_LEN is an
            # enum since 5.17); not counted, but listed
            consts.append({'name': c, 'sdk': v, 'target': tv,
                           'baked': c in BAKED and tv is not None and
                           not (c in HARMLESS_OFF and not tv)})
    kmalloc = []
    for s in base['kmalloc']:
        ok = kmalloc_ok(s['index'], tconsts)
        kmalloc.append(dict(s, ok=ok))
    kc = texports.get('kmalloc_caches', {}).get('type')
    m = re.match(r'^struct kmem_cache \*(?:\[\d+\])*\[(\d+)\]$', kc or '')
    for k in kmalloc:
        if k['ok'] is not False and m and k['index'] is not None and k['index'] >= int(m.group(1)):
            k['ok'] = False     # the last dimension is the size classes
    review = any(t['level'] == '?' for t in types)
    breaking = (any(t['matters'] for t in types) or any(x['level'] == '!' for x in exports)
                or any(c['baked'] for c in consts) or any(k['ok'] is False for k in kmalloc))
    return {'types': types, 'exports': exports, 'consts': consts, 'kmalloc': kmalloc,
            'kmalloc_caches': kc, 'symvers': symvers is not None, 'breaking': breaking,
            'review': review,
            'ntypes': len(base['types']), 'nexports': len(base['exports']),
            'dropped': sorted(dropped)}


# Options the vendor code was built with that a target may turn off: the
# vendor's inline code then does work the kernel does not need (with PAN
# off the user domain is already open).
HARMLESS_OFF = {'CONFIG_CPU_SW_DOMAIN_PAN'}

# Constants the vendor code has compiled in (through the inline functions or
# macros named), so a different value in the target breaks it.
BAKED = {
    'THREAD_SIZE': 'current_thread_info()/get_current(): current, set_fs(), preempt_count',
    'THREAD_START_SP': 'task_pt_regs()',
    'PAGE_SHIFT': 'page arithmetic everywhere',
    'PAGE_SIZE': 'page arithmetic everywhere',
    'PAGE_OFFSET': 'mi_sys tests kernel addresses against it',
    'HZ': 'msecs_to_jiffies() of constants, HZ in timeouts',
    # (KMALLOC_*: the kmalloc_caches[] indexes are checked one by one)
    'TASK_COMM_LEN': 'task_struct.comm copies',
    'FB_MAX': 'registered_fb[] scans',
    'GFP_KERNEL': 'allocation flags', 'GFP_ATOMIC': 'allocation flags',
    'GFP_DMA': 'allocation flags', '__GFP_ZERO': 'allocation flags',
    '__GFP_NOWARN': 'allocation flags', '__GFP_HIGHMEM': 'allocation flags',
    'VM_IO': 'vma->vm_flags in mmap', 'VM_PFNMAP': 'vma->vm_flags in mmap',
    'VM_DONTEXPAND': 'vma->vm_flags in mmap', 'VM_DONTDUMP': 'vma->vm_flags in mmap',
    'VM_SHARED': 'vma->vm_flags', 'VM_LOCKED': 'vma->vm_flags / do_mmap()',
    'MAP_SHARED': 'do_mmap()', 'MAP_POPULATE': 'do_mmap()',
    'PROT_READ': 'do_mmap()', 'PROT_WRITE': 'do_mmap()',
    'O_RDONLY': 'filp_open()', 'O_WRONLY': 'filp_open()', 'O_RDWR': 'filp_open()',
    'O_CREAT': 'filp_open()', 'O_TRUNC': 'filp_open()', 'O_APPEND': 'filp_open()',
    'O_LARGEFILE': 'filp_open()',
    'UMH_WAIT_PROC': 'call_usermodehelper()', 'UMH_WAIT_EXEC': 'call_usermodehelper()',
    'UMH_NO_WAIT': 'call_usermodehelper()',
    'TIMER_IRQSAFE': 'INIT_DELAYED_WORK()', 'WORK_STRUCT_FLAG_BITS': 'work_pending()',
    'L_PTE_PRESENT': 'pgprot_*() of vm_page_prot', 'L_PTE_YOUNG': 'pgprot_*()',
    'L_PTE_DIRTY': 'pgprot_*()', 'L_PTE_RDONLY': 'pgprot_*()', 'L_PTE_XN': 'pgprot_*()',
    'L_PTE_SHARED': 'pgprot_*()', 'L_PTE_MT_UNCACHED': 'pgprot_noncached()',
    'L_PTE_MT_BUFFERABLE': 'pgprot_writecombine()', 'L_PTE_MT_WRITETHROUGH': 'pgprot_*()',
    'L_PTE_MT_WRITEBACK': 'pgprot_*()', 'L_PTE_MT_DEV_SHARED': 'pgprot_*()',
    'L_PTE_MT_MASK': 'pgprot_noncached()/pgprot_writecombine()',
    'CONFIG_ARM_PATCH_PHYS_VIRT': '__pv_stub fixups (.pv_table) in the vendor code',
    'CONFIG_CPU_SW_DOMAIN_PAN': 'uaccess_save_and_enable() in the inline user copies',
    'CONFIG_CPU_USE_DOMAINS': 'set_fs()/modify_domain() inline',
    'CONFIG_THREAD_INFO_IN_TASK': 'current_thread_info()->task',
    'CONFIG_ARM_LPAE': 'pgprot_t / pte layout, phys_addr_t',
    'CONFIG_HIGHMEM': 'page_address()',
    'CONFIG_STACKPROTECTOR_PER_TASK': '__stack_chk_guard (the vendor code reads the global canary)',
    'CONFIG_GCC_PLUGINS': '__stack_chk_guard (the vendor code reads the global canary)',
}


def report(res, base, a):
    w = sys.stdout.write
    w(f"kernel ABI check: {res['target']}\n")
    w(f"  vendor modules: {', '.join(sorted(base['modules']))} ({base.get('about', '')})\n")
    bad = [t for t in res['types'] if t['level'] == '!']
    unsure = [t for t in res['types'] if t['level'] == '?']
    w(f"\ntypes: {res['ntypes']} kernel types the vendor code reaches; "
      f"{res['ntypes'] - len(res['types'])} identical, {len(res['types'])} differ: "
      f"{len(bad)} break it (!), {len(unsure)} changed where no access was found (?)\n")
    for t in res['types']:
        if t['level'] == ' ' and not a.all:
            continue
        w(f"{t['level']} [{t['class']}] {t['type']}  ({', '.join(t['use']) or 'reached'}; "
          f"{', '.join(t['modules']) or '-'})\n")
        for path, rw, x, y, mods, n, st in t['impacted']:
            sites = (f'{n} instruction{"s" if n != 1 else ""} ' if n else '') + \
                    (f'{st} static object{"s" if st != 1 else ""} ' if st else '')
            w(f"      accessed ({rw}, {sites}{', '.join(mods)}): {path} "
              f"{fmt_member(x)} -> {fmt_member(y)}\n")
        lim = None if a.all else (4 if t['impacted'] else 8)
        for kind, text in t['diffs'][:lim]:
            w(f'      {text}\n')
        if lim and len(t['diffs']) > lim:
            w(f"      ... {len(t['diffs']) - lim} more\n")
    if not a.all:
        quiet = len(res['types']) - len(bad) - len(unsure)
        if quiet:
            w(f"  ({quiet} more differ harmlessly for how they are used: --all lists them)\n")
    w("  [A] objects in vendor memory, [B] used directly, [C] one pointer away, "
      "[D] further\n")
    w(f"\nexports: {res['nexports']} kernel symbols imported"
      f"{'' if res['symvers'] else ' (no Module.symvers: existence not checked)'}; "
      f"{len(res['exports'])} with problems\n")
    cls = None
    for x in res['exports']:
        if x['class'] != cls:
            cls = x['class']
            w('  kernel:\n' if cls == 'kernel' else
              '  SigmaStar platform (drivers/sstar of the target, not the kernel version):\n')
        w(f"{x['level']} {x['symbol']}: {'; '.join(x['problems'])}  ({', '.join(x['modules'])})\n")
    w(f"\nconstants and options: {len(res['consts'])} differ\n")
    for c in res['consts']:
        why = BAKED.get(c['name'])
        mark = '!' if c.get('baked') else ' '
        w(f"{mark} {c['name']}: {fmt(c['sdk'])} -> {fmt(c['target'])}"
          f"{'  (' + why + ')' if why else ''}\n")
    w(f"\nkmalloc_caches: {res['kmalloc_caches']}\n")
    for k in res['kmalloc']:
        if k['ok'] is not True:
            w(f"! {k['module']}: {k['function']}() uses kmalloc_caches[{k['index']}]: "
              f"{'missing in the target' if k['ok'] is False else 'cannot tell'}\n")
    idx = sorted({k['index'] for k in res['kmalloc']}, key=lambda x: (x is None, x))
    w(f"  {len(res['kmalloc'])} inline kmalloc() sites, indexes {idx}\n")
    # exports the SDK's headers did not declare either (compiler helpers,
    # SigmaStar variables) are not worth a line
    dropped = [d for d in res['dropped'] if not d.startswith('x:') or
               base['exports'].get(d[2:], {}).get('hdr')]
    if dropped:
        w(f"\nprobe: left out (does not compile against the target): "
          f"{', '.join(dropped)}\n")
    if res['breaking']:
        v = 'DIFFERENT where the vendor code depends on it'
    elif res['review']:
        v = 'compatible as far as the accesses found go; review the (?) types'
    else:
        v = 'compatible'
    w(f"\nverdict: {v}\n")


def fmt_member(m):
    if m is None:
        return 'gone'
    return f'@{m[0]} size {m[1]} {m[2]}'


def fmt(v):
    return hex(v) if isinstance(v, int) and v > 4096 else str(v)


def cmd_dump(a):
    readelf = tool(a.cross, 'readelf')
    types, exports, consts = read_probe(a.elf, readelf)
    for n in a.names:
        if n in types:
            print(json.dumps({n: types[n]}, indent=1))
        elif n in exports:
            print(json.dumps({n: exports[n]}, indent=1))
        elif n in consts:
            print(n, consts[n])
        else:
            print(f'{n}: not found')


def main():
    p = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    p.add_argument('--cross', default=os.environ.get('CROSS_COMPILE', ''),
                   help='binutils/compiler prefix (default: $CROSS_COMPILE)')
    p.add_argument('--arch', default=os.environ.get('ARCH', 'arm'))
    p.add_argument('-v', '--verbose', action='store_true')
    sub = p.add_subparsers(dest='cmd', required=True)
    e = sub.add_parser('extract', help='write the vendor ABI file from the SDK .ko files')
    e.add_argument('--chiparch', required=True)
    e.add_argument('--sdk-kdir', required=True,
                   help='the SDK kernel tree, configured as the SDK built it (modules_prepare)')
    e.add_argument('--about', default='', help='where the .ko files come from')
    e.add_argument('--out', required=True)
    e.add_argument('--workdir', help='build the probe here and keep it')
    e.add_argument('--keep', action='store_true', help='keep the probe build directory')
    e.add_argument('ko', nargs='+')
    c = sub.add_parser('check', help='compare a kernel tree with the vendor ABI file')
    c.add_argument('--json', required=True, help='kernel/kabi/<CHIPARCH>.json')
    c.add_argument('--kdir', help='target kernel build tree (built or modules_prepare)')
    c.add_argument('--dwarf', help='read the target types from this ELF (a probe object, '
                   'or a vmlinux with debug info) instead of building the probe')
    c.add_argument('--name', help='name the target in the report')
    c.add_argument('--workdir', help='build the probe here and keep it')
    c.add_argument('--keep', action='store_true')
    c.add_argument('--all', action='store_true', help='also list harmless differences')
    c.add_argument('--save', help='write the result as JSON')
    d = sub.add_parser('dump', help='print types/exports/constants from a probe object')
    d.add_argument('elf')
    d.add_argument('names', nargs='+')
    a = p.parse_args()
    if a.cmd == 'check' and not (a.kdir or a.dwarf):
        p.error('check needs --kdir or --dwarf')
    sys.exit({'extract': cmd_extract, 'check': cmd_check, 'dump': cmd_dump}[a.cmd](a) or 0)


if __name__ == '__main__':
    main()
