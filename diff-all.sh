#!/bin/bash
WG="/home/zx2c4/Projects/WireGuard"
SELF="$(readlink -f "${BASH_SOURCE[0]}")"
IT="${SELF%/*}"

diff_it() {
	local base="$1"
	local foreign="$2"
	local file="$3"

	file="${file#"$base"}"
	[[ -f $base/$file && -f $foreign/$file ]] && git --no-pager diff --color=always --no-index "$base/$file" "$foreign/$file"
}

for i in "$WG/src"/*.c "$WG/src"/*.h; do
	diff_it "$WG/src" "$IT/drivers/net/wireguard" "$i"
done
for i in $(find "$WG/src/selftest" -type f); do
	diff_it "$WG/src/selftest" "$IT/drivers/net/wireguard/selftest" "$i"
done
for i in $(find "$WG/src/crypto/zinc" -type f); do
	diff_it "$WG/src/crypto/zinc" "$IT/lib/zinc" "$i"
done
for i in $(find "$WG/src/crypto/include/zinc" -type f); do
	diff_it "$WG/src/crypto/include/zinc" "$IT/include/zinc" "$i"
done
diff_it "$WG/src/tests" "$IT/tools/testing/selftests/wireguard" "netns.sh"
