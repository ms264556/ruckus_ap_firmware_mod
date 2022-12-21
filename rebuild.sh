#!/bin/bash

bin_dir=$(dirname "$(readlink -f -- "$0")")/bin

pushd packaging/image >/dev/null
sudo "$bin_dir/mksquashfs" ../rootfs ruckus_rootfs.img -all-root -noappend
"$bin_dir/build-package" ../ruckus_package.bl7 || { echo "Error: rebuild failed"; exit 1; }
popd >/dev/null
