#!/bin/bash
SELF="$(readlink -f "${BASH_SOURCE[0]}")"
cd "${SELF%/*}"
set -ex
git tag -f wireguard $(git log --grep="net: WireGuard" --max-count=1 --pretty=format:%H)
git tag -f zinc $(git log --grep="zinc: introduce" --max-count=1 --pretty=format:%H)
git tag -f simd $(git log --grep="asm: simd" --max-count=1 --pretty=format:%H)
git tag -f big_key_rewrite $(git log --grep="security/keys: rewrite" --max-count=1 --pretty=format:%H)
git push -f origin master:jd/wireguard wireguard zinc simd big_key_rewrite
