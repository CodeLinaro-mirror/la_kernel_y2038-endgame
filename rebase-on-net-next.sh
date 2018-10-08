#!/bin/bash
SELF="$(readlink -f "${BASH_SOURCE[0]}")"
cd "${SELF%/*}"
set -ex
git fetch net-next
git rebase FETCH_HEAD
