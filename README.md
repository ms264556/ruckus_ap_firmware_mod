# Ruckus AP Firmware Modding Tools
Scripts &amp; tools to modify Ruckus AP software images

## Ruckus AP Firmware Notes

Ruckus AP firmwares are delivered as .bl7 files, which are flashed directly onto the target MTD.
The .bl7 contains a header, a kernel image, a rootfs image & an optional trailer.

The AP's kernel splits the active MTD into two - a first MTD containing the header + kernel & a second MTD containing the rootfs + trailer.  
Because of this, the .bl7 contains padding after the kernel section, such that the rootfs begins on a new MTD page.

The header is 160 bytes.
You can use the `dump-header` tool to display a .bl7 file's header.

|Byte Offset|Field|Type|Description|
|----|----|-----|----|
|0x00|magic|uint8_t[4]|Header magic: "`RCKS`"|
|0x04|next_image|uint32_t|Offset to next image (i.e. the rootfs)|
|0x08|invalid|uint8_t|Only autoboot from this image if invalid is `0`|
|0x09|hdr_len|uint8_t|Length of this header|
|0x0A|compression|uint8_t[2]|Compression scheme, default l7, [`l7`\|`zl`\|`xx`]|
|0x0C|entry_point|uint32_t|Execution entry point|
|0x10|binl7_len|uint32_t|Length of (compressed) binary image|
|0x14|timestamp|uint32_t|Timestamp|
|0x18|signature|uint8_t[16]|MD5 checksum|
|0x28|hdr_version|uint16_t|Header version|
|0x2A|hdr_cksum|uint16_t|Header checksum|
|0x2C|version|uint8_t[16]|FW version string|
|0x3C|product|uint8_t|Product class, e.g. AP vs Adapter|
|0x3D|architecture|uint8_t|Architecture, e.g. mips, le/be, etc.|
|0x3E|chipset|uint8_t|Chipset, e.g. AR531X|
|0x3F|board_type|uint8_t|V54 board type|
|0x40|customer|uint8_t[32]|Customer ID, NULL ==> any customer|
|0x60|product_v2|uint8_t[12]|FWv2 product string|
|0x6C|version_v2|uint8_t[19]|FWv2 version string|
|0x7F|board_class|uint8_t|FWv2 board class|
|0x80|load_address|uint32_t|Load address|
|0x84|image_type|uint32_t|0=Unsigned Image (UI) / 1=Intermediate Signed Image (ISI) / 2=Fully Signed Image (FSI)|
|0x88|tail_offset|uint32_t|Offset to the signatures within the binl7. Only ISI stores signatures within the binl7, so this is 0 for UI & FSI images.|
|0x8C|_pad_end|uint8_t[20]|Pad out to 160 bytes|

The simplest firmwares are unsigned (called UI or USI).  
* The UI header contains an MD5 hash for the kernel + rootfs, and a checksum for the header itself.  

Signed firmwares have an appended trailer containing a digital signature.  
> There are two types of signed firmwares: ISI (intermediate Signed) and FSI (Fully Signed).  
* ISI firmwares sign the kernel & rootfs, with the header MD5 covering the kernel + rootfs + trailer.
* FSI firmwares sign the header + kernel + rootfs, with the header MD5 covering only the kernel + rootfs.

An AP running UI firmware can only install UI and ISI firmwares.  
An AP running FSI firmware will only install ISI and FSI firmwares.  
An AP running ISI firmware will allow any type of firmware to be installed.

The `shred-package` tool will split any type of firmware, but the `build-package` tool can only create UI images because we don't know the Ruckus signing key.  
So if your AP is running FSI firmware (e.g. SmartZone 3.6+, ZoneDirector 10.1+, Unleashed 200.7+ or Solo 110+) then you must first install an ISI image (e.g. Solo 104 or 106) before you can install a firmware modded with these tools.
> Alternatively, the existence of a file `/tmp/ignore_sign` on the AP will bypass the firmware signing checks. 

The included `mksquashfs` and `unsquashfs` tools are the specific version Ruckus uses to create their lzma compressed rootfs images.  
> If your AP image is very old (e.g. Solo pre-9.6) then it needs an older squashfs release. I didn't bother getting this to build, sorry, but the source is [here](https://github.com/ms264556/Xclaim_Task/blob/33093a71ca7a536ed7132d5f1be80d9d18d01398/buildroot/dl/squashfs2.1-r2.tar.gz).

> If your AP is a newer model (e.g. Rx50) then it needs a newer squashfs release. The `extract.sh` script will use your distro's standard packaged `unsquashfs` if necessary but I haven't updated `rebuild.sh` to check for and and use your distro's standard packaged `mksquashfs`.  

## Building

> This "works on my PC" (Ubuntu on WSL2)  

```bash
# Prerequisites
sudo apt install build-essential libz-dev

# Build C stuff
cd src
make install
cd ..

```

## Sample - modify Solo AP Firmware to enable Wireless Uplink

The provided mod_example.sh enables wireless bridge/repeater/extender functionality.

If you are here because you specifically require this functionality then [there's an easier way](https://ms264556.net/pages/StandaloneWirelessBridgeRepeater).

> The sample script will only work on Solo/Standalone 9.6 - 9.8 firmware images.  

```bash
# Extract firmware from a .bl7 Solo Image
# (download the appropriate Solo image for your AP model
# from https://support.ruckuswireless.com/software)
./extract.sh ~/zf7962_9.8.3.0.14.bl7
# Modify Solo/Standalone firmware to enable wireless uplink
./mod_example.sh
# Build modified Image
./rebuild.sh
```

Now you can:-
* Install the modified firmware (`packaging/ruckus_package.bl7`)
* Do a factory reset
* Go to `Configuration` > `Radio 5G` > `Wireless Bridge`
  * Enter your `SSID`
  * Choose `Encryption Method` = `WPA`
  * Enter your `Passphrase`
* Press `Update Settings` & Say `OK` to the VLAN error popup.
* Press `Update Settings` again. 

Your AP will now associate instead with the chosen SSID.
