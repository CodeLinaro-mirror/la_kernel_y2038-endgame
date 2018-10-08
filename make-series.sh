#!/bin/bash
SELF="$(readlink -f "${BASH_SOURCE[0]}")"
cd "${SELF%/*}"
set -ex

[[ $1 =~ ^v[0-9]+$ ]] || { echo "ERROR: pass the version directory as an argument" >&2; exit 1; }
mkdir -p "$1"
rm -fv "$1"/*.patch
git format-patch -o "$1" --notes --cover-letter --subject-prefix="PATCH net-next $1" net-next/master..master~
sed 's/\*\*\* SUBJECT HERE \*\*\*/WireGuard: Secure Network Tunnel/' "$1/0000-cover-letter.patch" | head -n 8 > "$1/0000-cover-letter.patch.tmp"
cat "$1/0000-cover-letter.patch.tmp" "changelog.txt" "zero-zero-text.txt" > "$1/0000-cover-letter.patch"
rm -f "$1/0000-cover-letter.patch.tmp"
