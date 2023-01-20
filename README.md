# Ruckus AP Firmware Modding Tools
Scripts &amp; tools to modify Ruckus AP software images

## Sample - modify Solo AP Firmware to enable Wireless Uplink

The provided mod_example.sh enables wireless bridge/repeater/extender functionality.

If you are here because you specifically require this functionality then [there's an easier way to enable it here](https://ms264556.github.io/Hackery/pages/StandaloneWirelessBridgeRepeater.html).

> "Works on my PC" (Ubuntu 22.04 WSL2)  

> The sample script will only work on old (9.6 - 104.0) images.  

```bash
# Prerequisites
sudo apt install build-essentials
sudo apt install libz-dev

# Build C stuff
cd src
make install
cd ..

# Extract firmware from a .bl7 Solo Image (download the appropriate Solo image for your AP model from https://support.ruckuswireless.com/software)
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

Your AP will now fall off the wired network, and associate intsead with the chosen SSID.
