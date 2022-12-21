#!/bin/bash

img=$(readlink -f "${1}")
bin_dir=$(dirname "$(readlink -f -- "$0")")/bin

sudo rm -rf packaging
mkdir -p packaging/image

pushd packaging/image >/dev/null
"$bin_dir/shred-package" "${img}" || { echo "Error: shred failed"; exit 1; }
sed -i '$ s/\xff*$//' ruckus_kernel.img
sudo "$bin_dir/unsquashfs" -dest ../rootfs ruckus_rootfs.img
popd >/dev/null
