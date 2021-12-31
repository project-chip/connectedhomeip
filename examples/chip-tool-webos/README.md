# Guide for building Matter example for webOS OSE

## Purpose

This doc provides the build and test guide of Matter client example for webOS OSE.

## Tested on:
- [webOS OSE 2.0](https://www.webosose.org/)

## Prerequisite

Before budiling and testing Matter example for webOS, you need following items.

: Ubuntu 18.04 machine (used for building webOS OSE)
: Network router
: 2 raspberrypi4 H/W (First rpi4 used for controller, and second used for controlee)
: Flash webOS OSE image on the first raspberrypi4 referring to Step 1 of "Build Instructions" below.
: Flash Ubuntu 20.04 or higher on the second raspberrypi4 by referring https://ubuntu.com/download/raspberry-pi
: Then, checkout the project-chip/connectedhomeip and build all-clusters-app on examples/all-clusters-app.


## Definition
WEBOS_BUILD_ROOT:
- Build root path of your webOS OSE build set up on your build machine. ex) /home/cabin15/build-webos

CHIP_ROOT:
- Root path of the connectedhomeip repository. ex) /home/cabin15/connectedhomeip

IP_ADDRESS_OF_YOUR_RASPBERRYPI4:
- IP address of other raspberrypi device connected on your router. ex) 192.168.0.120


## Build Instructions
Step 1: Build and Flash the webOS OSE

Set up a webOS OSE build as explained at:

webOS OSE build instruction: http://webosose.org/discover/setting/building-webos-ose
Next, flash the build image to your Raspberry Pi 4 by the webOS flash instruction:
http://webosose.org/discover/setting/flashing-webos-OSE

After going through webOS OSE build and flash instruction, you are ready to start building Matter client example for webOS OSE.

Here is a brief commands used for webOS OSE build.

$ git clone https://github.com/webosose/build-webos.git
$ sudo scripts/prerequisites.sh
$ ./mcf -p 16 -b 16 raspberrypi4
$ make webos-image


Step 2: Setting recipe of Matter client example for webOS OSE
Checkout cabin15/connectedhomeip on github and move the chip recipe files located under ${CHIP_ROOT}/examples/chip-tool-webos/recipe
to your webOS OSE build tree by using below command.

$ cp -r ${CHIP_ROOT}/examples/chip-tool-webos/recipe/chip ${WEBOS_BUILD_ROOT}/meta-webosose/meta-webos/recipes-connectivity

ex) cp -r /home/worker/connectedhomeip/examples/chip-tool-webos/recipe/chip /home/worker/ose/build-webos/meta-webosose/meta-webos/recipes-connectivity


Step 3: Build Matter client example for webOS OSE
Go to your webOS OSE build root and build the Matter client example for webOS OSE.

$ cd ${WEBOS_BUILD_ROOT}
$ source oe-init-build-env
$ bitbake chip



Step 4: Get installation package
Go to the location where installation package file is generated after Step 3, and copy it to your local.

$ cd ${WEBOS_BUILD_ROOT}/BUILD/work/raspberrypi4-webos-linux-gnueabi/chip/1.0-r0/deploy-ipks/raspberrypi4
$ scp chip_1.0-r0_raspberrypi4.ipk root@${IP_ADDRESS_OF_YOUR_RASPBERRYPI4}:/home/root



ex) cd ~/ose/build-webos/BUILD/work/raspberrypi4-webos-linux-gnueabi/chip/1.0-r0/deploy-ipks/raspberrypi4

      scp chip_1.0-r0_raspberrypi4.ipk root@192.168.0.120


Step 5: Install the package on your raspberrypi4
Go into the raspberrypi4 shell using ssh and install the package.

$ ssh root@${IP_ADDRESS_OF_YOUR_RASPBERRYPI4}
# cd /home/root
# opkg install chip_1.0-r0_raspberrypi4.ipk

Step 6: Reboot your raspberrypi4

Step 7: Launch all-clusters-app on other machine on the same subnet.
On the second raspberrypi4 where all-clusters-app is built, launch all-clusters-app so that controlee can discover ghe neighborhood.

Step 8: Test if discover works using luna-send command
Go into the raspberrypi4 shell using ssh. Launch below command on the raspberrypi4 shell.
Discover API works successfully if you see below json return.

$ ssh root@${IP_ADDRESS_OF_YOUR_RASPBERRYPI4}
root@raspberrypi4:/var/rootdirs/home/root#
root@raspberrypi4:/var/rootdirs/home/root# luna-send -f -n 1 luna://com.webos.service.matter/sendCommand '{"clusterName":"discover", "commandName":"commissionables"}'
{
    "returnValue": true,
    "nodes": [
        {
            "hostName": "EEAABADABAD0DDCA",
    	    "rotatingId": "",
	    "deviceName": "",
	    "productId": "65279",
	    "longDiscriminator": "3840",
	    "deviceType": "0",
  	    "vendorId": "9050",
	    "pairingHint": "33",
	    "pairingInstruction": "",
	    "port": "5540",
	    "commissioningMode": "1",
	    "address": [
	        "fe80::870:daaf:910d:24ba",
	        "192.168.0.112"
    	    ]
        }
    ]
}
root@raspberrypi4:/var/rootdirs/home/root#
