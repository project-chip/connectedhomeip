# CHIP K32W061 Lighting Example Application

The Project CHIP K32W061 Lighting Example demonstrates how to remotely control a
light bulb. The light bulb is simulated using one of the LEDs from the expansion
board. It uses buttons to test turn on/turn off of the light bulb. You can use
this example as a reference for creating your own application.

The example is based on
[Project CHIP](https://github.com/project-chip/connectedhomeip) and the NXP K32W
SDK, and supports remote access and control of a light bulb over a low-power,
802.15.4 Thread network.

The example behaves as a Project CHIP accessory, that is a device that can be
paired into an existing Project CHIP network and can be controlled by this
network.

<hr>

-   [CHIP K32W Lighting Example Application](#chip-k32w-lighting-example-application) -
-   [Introduction](#introduction)
    -   [Bluetooth LE Advertising](#bluetooth-le-advertising)
    -   [Bluetooth LE Rendezvous](#bluetooth-le-rendezvous)
-   [Device UI](#device-ui)
-   [Building](#building)
-   [Flashing and debugging](#flashdebug)
-   [Testing the example](#testing-the-example)

<hr>

<a name="intro"></a>

## Introduction

![K32W061 DK6](../../platform/k32w/doc/images/k32w-dk6.jpg)

The K32W lighting example application provides a working demonstration of a
light bulb device, built using the Project CHIP codebase and the NXP K32W061
SDK. The example supports remote access (e.g.: using CHIP Tool from a mobile
phone) and control of a light bulb over a low-power, 802.15.4 Thread network. It
is capable of being paired into an existing Project CHIP network along with
other Project CHIP-enabled devices.

The example targets the
[NXP K32W061 DK6](https://www.nxp.com/products/wireless/thread/k32w061-41-high-performance-secure-and-ultra-low-power-mcu-for-zigbeethread-and-bluetooth-le-5-0-with-built-in-nfc-option:K32W061_41)
development kit, but is readily adaptable to other K32W-based hardware.

The CHIP device that runs the lighting application is controlled by the CHIP
controller device over the Thread protocol. By default, the CHIP device has
Thread disabled, and it should be paired over Bluetooth LE with the CHIP
controller and obtain configuration from it. The actions required before
establishing full communication are described below.

The example also comes with a test mode, which allows to start Thread with the
default settings by pressing a button. However, this mode does not guarantee
that the device will be able to communicate with the CHIP controller and other
devices.

### Bluetooth LE Advertising

In this example, to commission the device onto a Project CHIP network, it must
be discoverable over Bluetooth LE. Bluetooth LE advertising is started
automatically when the device is powered up.

### Bluetooth LE Rendezvous

In this example, the commissioning procedure (called rendezvous) is done over
Bluetooth LE between a CHIP device and the CHIP controller, where the controller
has the commissioner role.

To start the rendezvous, the controller must get the commissioning information
from the CHIP device. The data payload is encoded within a QR code, printed to
the UART console.

### Thread Provisioning

Last part of the rendezvous procedure, the provisioning operation involves
sending the Thread network credentials from the CHIP controller to the CHIP
device. As a result, device is able to join the Thread network and communicate
with other Thread devices in the network.

## Device UI

The example application provides a simple UI that depicts the state of the
device and offers basic user control. This UI is implemented via the
general-purpose LEDs and buttons built in to the OM15082 Expansion board
attached to the DK6 board.

**LED D2** shows the overall state of the device and its connectivity. Four
states are depicted:

-   _Short Flash On (50ms on/950ms off)_ &mdash; The device is in an
    unprovisioned (unpaired) state and is waiting for a commissioning
    application to connect.

*   _Rapid Even Flashing (100ms on/100ms off)_ &mdash; The device is in an
    unprovisioned state and a commissioning application is connected via BLE.

-   _Short Flash Off (950ms on/50ms off)_ &mdash; The device is full
    provisioned, but does not yet have full network (Thread) or service
    connectivity.

*   _Solid On_ &mdash; The device is fully provisioned and has full network and
    service connectivity.

**LED D3** shows the state of the simulated light bulb. When the LED is lit the
light bulb is on; when not lit, the light bulb is off.

**Button SW2** can be used to change the state of the simulated light bulb. This
can be used to mimic a user manually operating a switch. The button behaves as a
toggle, swapping the state every time it is pressed.

**Button SW3** can be used to reset the device to a default state. Pressing and
holding Button SW3 for 6 seconds initiates a factory reset. After an initial
period of 3 seconds, LED2 D2 and D3 will flash in unison to signal the pending
reset. Holding the button past 6 seconds will cause the device to reset its
persistent configuration and initiate a reboot. The reset action can be
cancelled by releasing the button at any point before the 6 second limit.

**Button SW4** can be used for joining a predefined Thread network advertised by
a Border Router. Default parameters for a Thread network are hard-coded and are
being used if this button is pressed.

The remaining two LEDs (D1/D2) and button (SW1) are unused.

<a name="building"></a>

## Building

In order to build the Project CHIP example, we recommend using a Linux
distribution (the demo-application was compiled on Ubuntu 20.04).

-   Download [K32W061 SDK 2.6.2 for Project CHIP](https://mcuxpresso.nxp.com/).
    Creating an nxp.com account is required before being able to download the
    SDK. Once the account is created, login and follow the steps for downloading
    SDK_2.6.2_K32W061DK6. The SDK Builder UI selection should be similar with
    the one from the image below.
    ![MCUXpresso SDK Download](../../platform/k32w/doc/images/mcux-sdk-download.JPG)

-   Start building the application

```
user@ubuntu:~/Desktop/git/connectedhomeip$ export K32W061_SDK_ROOT=/home/user/Desktop/SDK_2.6.2_K32W061DK6/
user@ubuntu:~/Desktop/git/connectedhomeip$ ./third_party/k32w_sdk/mr2_fixes/patch_k32w_mr2_sdk.sh
user@ubuntu:~/Desktop/git/connectedhomeip$ source ./scripts/activate.sh
user@ubuntu:~/Desktop/git/connectedhomeip/third_party/openthread/repo$ cd examples/lighting-app/k32w/
user@ubuntu:~/Desktop/git/connectedhomeip/examples/lighting-app/k32w$ gn gen out/debug --args="k32w_sdk_root=\"${K32W061_SDK_ROOT}\" is_debug=true"
user@ubuntu:~/Desktop/git/connectedhomeip/examples/lightin-app/k32w$ ninja -C out/debug
user@ubuntu:~/Desktop/git/connectedhomeip/examples/lighting-app/k32w$ $K32W061_SDK_ROOT/tools/imagetool/sign_images.sh out/debug/
```

Note that "patch_k32w_mr2_sdk.sh" script must be run for patching the K32W061
SDK 2.6.2.

In case signing errors are encountered when running the "sign_images.sh" script
install the recommanded packages (python version > 3, pip3, pycrypto,
pycryptodome):

```
user@ubuntu:~$ python3 --version
Python 3.8.2
user@ubuntu:~$ pip3 --version
pip 20.0.2 from /usr/lib/python3/dist-packages/pip (python 3.8)
user@ubuntu:~$ pip3 list | grep -i pycrypto
pycrypto               2.6.1
pycryptodome           3.9.8
```

The resulting output file can be found in out/debug/chip-k32w061-light-example.

<a name="flashdebug"></a>

## Flashing and debugging

Program the firmware using the official
[OpenThread Flash Instructions](https://github.com/openthread/openthread/blob/master/examples/platforms/k32w/k32w061/README.md#flash-binaries).

All you have to do is to replace the Openthread binaries from the above
documentation with _out/debug/chip-k32w061-light-example.bin_ if DK6Programmer
is used or with _out/debug/chip-k32w061-light-example_ if MCUXpresso is used.

## Testing the example

The app can be deployed against any generic OpenThread Border Router. See the
guide
[Commissioning NXP K32W using Android CHIPTool](../../../docs/guides/nxp_k32w_android_commissioning.md)
for step-by-step instructions.
