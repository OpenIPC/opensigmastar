#!/bin/sh
# weaken.sh -- let source objects replace parts of a relinked vendor object
#
# Usage: scripts/weaken.sh <nm> <objcopy> <vendor.o> <out.o> <source.o>...
#
# Every global symbol that both <vendor.o> and one of the <source.o> files
# define becomes weak in <out.o>. Linked together, the source definition
# wins and every reference resolves to it: the vendor's own calls between
# its parts, its __ksymtab entries (EXPORT_SYMBOL), its initcalls. The
# vendor's copy of the code stays in the module, unreferenced.
#
# This works because the vendor objects reach their global functions and
# data through relocations against the symbol, not against .text plus an
# offset; it does not hold for calls a compiler resolved itself (static or
# inlined functions), which is why whole parts (every global of disp, panel,
# rgn) are replaced, never single functions out of one.
set -e

[ $# -ge 5 ] || { sed -n '4p' "$0" >&2; exit 1; }
NM=$1 OBJCOPY=$2 IN=$3 OUT=$4
shift 4

defined() {
	"$NM" --defined-only "$@" | awk 'NF == 3 && $2 ~ /^[A-Z]$/ { print $3 }' |
		sort -u
}

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
defined "$IN" > "$tmp/vendor"
defined "$@" > "$tmp/source"
comm -12 "$tmp/vendor" "$tmp/source" > "$tmp/both"

for s in init_module cleanup_module __this_module; do
	if grep -qx "$s" "$tmp/both"; then
		echo "weaken.sh: a source object defines $s" >&2
		exit 1
	fi
done

sed 's/^/--weaken-symbol=/' "$tmp/both" > "$tmp/args"
# shellcheck disable=SC2046 # one option per line, no spaces in symbol names
"$OBJCOPY" $(cat "$tmp/args") "$IN" "$OUT"
