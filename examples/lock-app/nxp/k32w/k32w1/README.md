# CHIP K32W1 Lock Example Application

The Project CHIP K32W1 Lock Example demonstrates how to remotely control a
light bulb. The light bulb is simulated using one of the LEDs from the expansion
board. It uses buttons to test turn on/turn off of the light bulb. You can use
this example as a reference for creating your own application.

The example is based on
[Project CHIP](https://github.com/project-chip/connectedhomeip) and the NXP K32W1
SDK, and supports remote access and control of a light bulb over a low-power,
802.15.4 Thread network.

The example behaves as a Project CHIP accessory, that is a device that can be
paired into an existing Project CHIP network and can be controlled by this
network.

<hr>

-   [CHIP K32W1 Lock Example Application](#chip-k32w-Lock-example-application) -
-   [Introduction](#introduction)
    -   [Bluetooth LE Advertising](#bluetooth-le-advertising)
    -   [Bluetooth LE Rendezvous](#bluetooth-le-rendezvous)
-   [Device UI](#device-ui)
-   [Building](#building)
-   [Manufacturing data](#manufacturing-data)
-   [Flashing and debugging](#flashdebug)
-   [Testing the example](#testing-the-example)

<hr>

<a name="intro"></a>

## Introduction

![K32W1 EVK](../../../../platform/nxp/k32w/k32w1/doc/images/k32w1-evk.jpg)

The K32W1 Lock example application provides a working demonstration of a
light bulb device, built using the Project CHIP codebase and the NXP K32W1
SDK. The example supports remote access (e.g.: using CHIP Tool from a mobile
phone) and control of a light bulb over a low-power, 802.15.4 Thread network. It
is capable of being paired into an existing Project CHIP network along with
other Project CHIP-enabled devices.

The CHIP device that runs the Lock application is controlled by the CHIP
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
be discoverable over Bluetooth LE. For security reasons, you must start
Bluetooth LE advertising manually after powering up the device by pressing
Button SW2.

### Bluetooth LE Rendezvous

In this example, the commissioning procedure (called rendezvous) is done over
Bluetooth LE between a CHIP device and the CHIP controller, where the controller
has the commissioner role.

To start the rendezvous, the controller must get the commissioning information
from the CHIP device. The data payload is encoded within a QR code, or printed to
the UART console.

### Thread Provisioning

Last part of the rendezvous procedure, the provisioning operation involves
sending the Thread network credentials from the CHIP controller to the CHIP
device. As a result, device is able to join the Thread network and communicate
with other Thread devices in the network.

## Device UI

The example application provides a simple UI that depicts the state of the
device and offers basic user control. This UI is implemented via the
general-purpose LEDs and buttons built in the K32W1 EVK board.

**LED 2** shows the overall state of the device and its connectivity. Four
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

**RGB LED** shows the state of the simulated light bulb. When the LED is lit the
light bulb is on; when not lit, the light bulb is off.

**Button SW2** can be used to start BLE adevertising. A SHORT press of the buttton 
will enable Bluetooth LE advertising for a predefined period of time.

**Button SW2 LP** can be used to reset the device to a default state. A LONG Press
Button SW2 initiates a factory reset. After an initial period of 3 seconds, LED 2
and RGB LED will flash in unison to signal the pending reset. After 6 seconds will
cause the device to reset its persistent configuration and initiate a reboot.
The reset action can be cancelled by press SW2 button at any point before the 6
second limit.

**Button SW3** can be used to change the state of the simulated light bulb. This
can be used to mimic a user manually operating a switch. The button behaves as a
toggle, swapping the state every time it is pressed.

**Button SW3 LP** can be used for joining a predefined Thread network advertised by
a Border Router. Default parameters for a Thread network are hard-coded and are
being used if this button is pressed. A LONG press of SW3 will trigger the action.

Directly on the development board, **Button USERINTERFACE** can be used for
enabling Bluetooth LE advertising for a predefined period of time. Also, pushing
this button starts the NFC emulation by writing the onboarding information in
the NTAG.

<a name="building"></a>

## Building

In order to build the Project CHIP example, we recommend using a Linux
distribution (the demo-application was compiled on Ubuntu 20.04).

-   Download [K32W1 SDK for Project CHIP](https://mcuxpresso.nxp.com/).
    Creating an nxp.com account is required before being able to download the
    SDK. Once the account is created, login and follow the steps for downloading
    K32W1 SDK. The SDK Builder UI selection should be similar with
    the one from the image below.
    ![MCUXpresso SDK Download](../../../../platform/nxp/k32w/k32w1/doc/images/mcux-sdk-download.JPG)

```
user@ubuntu:~/Desktop/git/connectedhomeip$ export NXP_K32W1_SDK_ROOT=/home/user/Desktop/SDK_K32W1/
user@ubuntu:~/Desktop/git/connectedhomeip$ source ./scripts/activate.sh
user@ubuntu:~/Desktop/git/connectedhomeip$ cd examples/Lock-app/nxp/k32w/k32w1
user@ubuntu:~/Desktop/git/connectedhomeip/examples/Lock-app/nxp/k32w/k32w1$ gn gen out/debug --args="chip_with_ot_cli=0 is_debug=false chip_crypto=\"mbedtls\""
user@ubuntu:~/Desktop/git/connectedhomeip/examples/Lock-app/nxp/k32w/k32w1$ ninja -C out/debug
```

In case that Openthread CLI is needed, chip_with_ot_cli build argument must be
set to 1.

The resulting output file can be found in out/debug/chip-k32w1-light-example.

Also chip-k32w1-light-example.srec cand be found in the directory. This can be used for flashing with jlink.exe

## Manufacturing data

Use `chip_with_factory_data=1` in the gn build command to enable factory data.

For a full guide on manufacturing flow, please see
[Guide for writing manufacturing data on NXP devices](../../../../../docs/guides/nxp_manufacturing_flow.md).

<a name="flashdebug"></a>

## Flashing and debugging

Program the CM33 image by using Jlink.exe using the following steps:

1. Copy chip-k32w1-light-example.srec to folder where jlink.exe is located.
2. Copy Matter_root/third_party/nxp/k32w1_sdk/Jlink_Script/K32W1.jlink folder where jlink.exe is located.
3. Open a command prompt and type: 

    **jlink.exe -device KW45B41Z83 -if SWD -speed 4000 -autoconnect 1 -CommanderScript K32W1.jlink**

Note: Steps 1 and 2 are not mandatory if the K32W1.jlink is modified to point to the corect location of the chip-k32w1-light-example.srec file on the machine. Furthermore when running the jlink.exe command the correct path to the script must be provided.

For NBU firmware programming steps see the K32W148-EVK NBU Programming User’s Guide (available from your NXP contact)

## Testing the example

The app can be deployed against any generic OpenThread Border Router. See the
guide
[Commissioning NXP K32W using Android CHIPTool](../../../docs/guides/nxp_k32w_android_commissioning.md)
for step-by-step instructions.
