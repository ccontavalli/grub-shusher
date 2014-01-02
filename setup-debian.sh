#!/bin/bash
#
# Configures grub-shusher on a debian system. Suitable to be
# invoked from apt.conf, as a post invoke script. See README.md
# for more details.

set -e

# If no disk was provided, exit with error.
test -n "$1" || {
  echo "Use: $0 DISK" 1>&2
  echo 1>&2
  echo "For example: $0 /dev/sda" 1>&2
  echo 1>&2
  echo "You need to specify the grub installation disk." 1>&2
  exit 1
}

# Enter the directory where grub-shusher was installed.
cd "$(dirname "$(readlink -f "$(which "$0")")")"

# Compile and run it.
make
./grub-kernel /boot/grub/kernel.img
./grub-kernel /usr/lib/grub/i386-pc/kernel.img
grub-install "$1"
./mbr "$1"
