#!/bin/bash

img=$(readlink -f "${1}")
bin_dir=$(dirname "$(readlink -f -- "$0")")/bin

sudo rm -rf packaging
mkdir -p packaging/image

pushd packaging/image >/dev/null
"$bin_dir/shred-package" "${img}" || { echo "Error: shred failed"; exit 1; }

# extract rootfs magic
dd if=ruckus_rootfs.img of=ruckus_rootfs_magic.img bs=4 count=1 2>/dev/null
if [ "$(cat ruckus_rootfs_magic.img)" != "hsqs" ]; then
    printf 'hsqs' | dd of=ruckus_rootfs.img bs=4 count=1 conv=notrunc 2>/dev/null
fi
# strip kernel padding
sed -i '$ s/\xff*$//' ruckus_kernel.img

# extract rootfs
(( compression_byte=$(dd if=ruckus_rootfs.img bs=1 skip=20 count=1 2>/dev/null | od -An -t u1) ))
case "$compression_byte" in
  2)
    # use custom unsquashfs
    sudo "$bin_dir/unsquashfs" -dest ../rootfs ruckus_rootfs.img
    ;;
  4)
    # use system unsquashfs
    sudo unsquashfs -dest ../rootfs ruckus_rootfs.img
    ;;
  *)
    echo "Unsupported SquashFS compression method $compression_byte."
    exit 1
    ;;
esac

popd >/dev/null
