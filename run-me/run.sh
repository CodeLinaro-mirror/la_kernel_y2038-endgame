#!/bin/sh
SELF="$(readlink -f "${BASH_SOURCE[0]}")"
cd "${SELF%/*}"
set -ex
make -C .. O=$(pwd) -j9 "$@"
exec qemu-system-x86_64 -nodefaults -nographic -smp 4 -m 128M -serial stdio -no-reboot -monitor none -cpu host -machine q35,accel=kvm -kernel arch/x86/boot/bzImage
