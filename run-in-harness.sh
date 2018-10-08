#!/bin/bash
WG="/home/zx2c4/Projects/WireGuard"
SELF="$(readlink -f "${BASH_SOURCE[0]}")"
IT="${SELF%/*}"
set -ex

export GIT_URI_integration="$IT"
export KERNEL_VERSION=integration-git-debug
make -C "$WG/src/tests/qemu" -j$(nproc)
