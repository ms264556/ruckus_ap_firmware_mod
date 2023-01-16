#!/bin/bash

# This sample is inspired by the repeater/extender changes suggested by aiman_al_hadhr1 here:-
# https://community.ruckuswireless.com/t5/Access-Points-Indoor-and-Outdoor/Ruckus-7341-U-act-as-repeater/m-p/31821/highlight/true#M8936
# (I couldn't make the suggested changes work, so I instead borrowed the P300 bridging config).

function WlansBridge {
    pushd $1 >/dev/null
    sudo chmod o=rw *
    sudo chmod o=rwx .

    echo -n 0 >wlan-auth-type
    echo 2 >wlan-cipher-type
    echo 1 >wlan-created-defined
    echo 0 >wlan-encrypt-state
    echo -n 1 >wlan-encrypt-type
    echo -n 1 >wlan-if-flags
    echo -n 1 >wlan-if-parent
    echo -n 1 >wlan-init-noup
    echo allow >wlan-isallowed
    echo uplink_ssid >wlan-ssid
    echo -n up >wlan-state
    echo sta >wlan-type
    echo Wireless Bridge >wlan-userdef-text
    rm -f wlan-wpa-eap-enable
    echo -n uplink_pass >wlan-wpa-passphrase
    echo -n 2 >wlan-wpa-type
    
    sudo chmod o=rx .
    sudo chmod o=r *
    popd >/dev/null
}

function ModSoloImageAddUplinks {
    pushd packaging/rootfs/defaults
    
    while IFS= read -d '' wlandir; do
        WlansBridge "${wlandir}" </dev/null
    done < <(find . -type d -path "*/$1" -print0)
    
    popd
}

ModSoloImageAddUplinks "wlans/wlan15"
