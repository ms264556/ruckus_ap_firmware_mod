#!/bin/bash

# This sample implements the repeater/extender changes suggested by aiman_al_hadhr1 here:-
# https://community.ruckuswireless.com/t5/Access-Points-Indoor-and-Outdoor/Ruckus-7341-U-act-as-repeater/m-p/31821/highlight/true#M8936

function WlansWlan30 {
    pushd $1
    sudo chmod o=rw *
    echo -n up >wlan-state
    echo -n ap >wlan-type
    echo -n wlan-trunk-ap >wlan-userdef-text
    echo -n 0 >wlan-ssid-hide
    echo -n wlan-trunk-ap >wlan-ssid
    echo -n 30 >wlan-mac-alloc-unit
    echo -n 0 >wlan-if-flags
    sudo chmod o=r *
    popd
}

function WlansWlan31 {
    pushd $1
    sudo chmod o=rw *
    echo -n up >wlan-state
    echo -n sta >wlan-type
    echo -n wlan-trunk-station >wlan-userdef-text
    echo -n wlan-trunk-station >wlan-ssid
    echo -n 31 >wlan-mac-alloc-unit
    echo -n 1 >wlan-if-flags
    sudo chmod o=r *
    popd
}

function IfsWlan30 {
    pushd $1
    sudo chmod o=rw *
    sudo chmod o=rwx .
    echo -n 0 >br
    echo -n 1 >port-type
    echo -n 1 >untag-vid
    echo -n 1 >vlans
    rm -f port-isolation-mode
    sudo chmod o=rx .
    sudo chmod o=r *
    popd
}

function IfsWlan31 {
    pushd $1
    sudo chmod o=rw *
    sudo chmod o=rwx .
    echo -n 1 >port-type
    echo -n 1 >untag-vid
    echo -n 1-4094 >vlans
    rm -f port-isolation-mode
    sudo chmod o=rx .
    sudo chmod o=r *
    popd
}

function IncreaseMaxWlans {
    sudo find . -name "wlan-maxwlans2" -exec chmod o=rw {} \; -exec sh -c 'echo -n 20 > {}' \;  -exec chmod o=rw {} \;
}

function ModSoloImageAddUplinks {
    pushd packaging/rootfs
    WlansWlan30 "defaults/common/4bss/wlans/wlan30"
    WlansWlan30 "defaults/common/hz-4bss/wlans/wlan30"
    WlansWlan31 "defaults/common/4bss/wlans/wlan31"
    WlansWlan31 "defaults/common/hz-4bss/wlans/wlan31"
    IfsWlan30 "defaults/common/4bss/ifs/wlan30"
    IfsWlan30 "defaults/common/hz-4bss/ifs/wlan30"
    IfsWlan31 "defaults/common/4bss/ifs/wlan31"
    IfsWlan31 "defaults/common/hz-4bss/ifs/wlan31"
    IncreaseMaxWlans
    popd
}

ModSoloImageAddUplinks >/dev/null
