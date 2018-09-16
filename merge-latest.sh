#!/bin/bash
WG="/home/zx2c4/Projects/WireGuard"
SELF="$(readlink -f "${BASH_SOURCE[0]}")"
IT="${SELF%/*}"
set -x

git -C "$WG" diff where-we-left-off..master "$WG/src"/*.c "$WG/src"/*.h "$WG/src/selftest/" | patch -d "$IT/drivers/net/wireguard" -p2
git -C "$WG" diff where-we-left-off..master "$WG/src/uapi" | patch -d "$IT/include/uapi/linux" -p3
git -C "$WG" diff where-we-left-off..master "$WG/src/crypto/zinc" | patch -d "$IT/lib/zinc" -p4
git -C "$WG" diff where-we-left-off..master "$WG/src/crypto/include/zinc" | patch -d "$IT/include/zinc" -p5
cp "$WG/src/tests/netns.sh" "$IT/tools/testing/selftests/wireguard/netns.sh"

git -C "$WG" tag -f where-we-left-off
