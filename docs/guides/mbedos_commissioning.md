![ARM Mbed-OS logo](https://raw.githubusercontent.com/ARMmbed/mbed-os/master/logo.png)

# Matter Arm Mbed OS provisioning guide

-   [Overview](#overview)
-   [Prerequisites](#prerequisites)
-   [CHIPTool for Android](#chiptool-for-android)
    -   [Building and installing](#building-and-installing)
    -   [Accessory Matter device setup](#accessory-matter-device-setup)
    -   [Device commissioning for Android](#device-commissioning-for-android)
    -   [Sending ZCL commands](#sending-zcl-commands-android)
-   [POSIX CLI CHIPTool](#posix-cli-chiptool)
    -   [Building](#building)
    -   [Device commissioning for CLI](#device-commissioning-for-cli)
    -   [Sending ZCL commands](#sending-zcl-commands-posix)
-   [Python Device Controller](#python-device-controller)
    -   [Building and installing](#building-and-installing-1)
    -   [Device commissioning for Python Device Controller](#device-commissioning-for-python-device-controller)
    -   [Sending ZCL commands](#sending-zcl-commands-python)
        -   [ZCL commands details](#zcl-commands-details)

<hr>

## Overview

This document provides a step-by-step guide how to commission any Matter
application. For demonstration purposes the Lighting app is used.

The provisioning process is composed of the following stages:

-   CHIPTool discovers a Matter accessory device over Bluetooth Low Energy
    (BLE).

-   CHIPTool establishes a secure channel to the device over BLE, and sends
    network credentials data.

BLE is only used during first phase. Afterwards, only the IP connectivity
between the smartphone and the accessory device is needed to send messages.

## Prerequisites

To complete all the steps in the tutorial, you need:

-   A smartphone with Android 8+ or PC with Ubuntu 20.04 and Bluetooth
    connectivity

-   A WiFi Access Point (smartphone router, standalone AP, wireless router or
    PC)

-   Any currently supported target device (for example, a Cypress PSoC6
    CY8CPROTO-062-4343W board)

## CHIPTool for Android

### Building and installing

To make provisioning possible and to control the Matter device from your Android
based smartphone, you must first build and install the CHIPTool application.

To build the CHIPTool application for your smartphone, read
[Android building guide](android_building.md).

After building, install the application by completing the following steps:

1.  Install the Android Debug Bridge (adb) package by running the following
    command:

        ```
        $ sudo apt install android-tools-adb
        ```

2.  Enable **USB debugging** on your smartphone. See the
    [Configure on-device developer options](https://developer.android.com/studio/debug/dev-options)
    guide on the Android Studio hub for detailed information.
3.  If the **Install via USB** option is supported for your Android version,
    turn it on.
4.  Plug your smartphone into a USB port on your PC.
5.  Run the following command to install the application, with _matter-dir_
    replaced with the path to the Matter source directory:

        $ adb install out/android-$TARGET_CPU-chip-tool/outputs/apk/debug/app-debug.apk

6.  Navigate to settings on your smartphone and grant **Camera** and
    **Location** permissions to CHIPTool.

Android CHIPTool is now ready to be used for commissioning.

### Accessory Matter device setup

To prepare the accessory Matter device for commissioning (called rendezvous),
complete the following steps:

-   Open a serial terminal session to connect to the UART console of the
    accessory device. You can use **mbed-tools** for this purpose
    ([mbed-tools](https://github.com/ARMmbed/mbed-tools)):

        ```
        mbed-tools sterm -p /dev/ttyACM0 -b 115200 -e off
        ```

To start the rendezvous, CHIPTool must get the commissioning information from
the Matter device. The data payload is encoded within a QR code and is printed
to the UART console.

-   Reset the device.

-   Find a message similar to the following one in the application logs:

        ```
        [INFO][CHIP]: [SVR]Copy/paste the below URL in a browser to see the QR Code:
        [INFO][CHIP]: [SVR]https://project-chip.github.io/connectedhomeip/qrcode.html?data=MT%3AYNJV7VSC00CMVH7SR00
        ```

-   Open URL from the console to display the QR in a web browser.

### Device commissioning for Android

To commission Matter device onto the network created complete the following
steps:

-   Enable Bluetooth and Location services on your smartphone.

-   Connect the smartphone to the WiFi Network

-   Open the CHIPTool application on your smartphone.

-   Tap the 'PROVISION CHIP DEVICE WITH WI-FI' button and scan the commissioning
    QR code.

-   Go through the the paring and connecting Bluetooth on your smartphone (you
    will see a few pop-up messages appear as the commissioning progresses.
    Finally, the network settings screen appears.

-   In the network settings screen enter the Wi-Fi credentials and tap the Save
    Network button to send a WiFi provisioning message to the accessory device.

-   After successful completion of the process, the application returns to the
    main screen.

### Sending ZCL commands Android

After the accessory device has been successfully commissioned to the network, it
is possible to communicate with it using IP. Matter uses Zigbee Cluster Library
(ZCL) protocol which defines common means for applications to communicate.

Communication with the device via ZCL commands is possible by using buttons of
the main screen.

For example, selecting the 'LIGHT ON/OFF & LEVEL CLUSTER' button opens the
screen which allows controlling the light dimming. Tap either the ON or the OFF
button to toggle between min and max brightness. Use the slider to modify the
brightness between 0-255.

If **Lighting LED** is available then brightness change can be observed.

> For more details about Android CHIPTool please visit
> [CHIPTool](../../examples/android/CHIPTool/README.md)

## POSIX CLI CHIPTool

### Building

To make provisioning possible and to control the Matter device from Linux-based
device, you can build and run the Matter Client example application on it.

To build the POSIX CLI CHIPTool application check the guide
[POSIX CLI guide](../../examples/chip-tool/README.md).

### Device commissioning for CLI

In order to send commands to a device, it must be paired with the client and
connected to the network.

To run the commissioning process via BLE, run the built executable and pass it
the node id to assign to the newly-commissioned node, network ssid and password,
discriminator and pairing code of the remote device.

Example:

    $ chip-tool pairing ble-wifi node_id_to_assign network_ssid network_password 20202021 3840

### Sending ZCL commands POSIX

If the commissioning process was successful, it is possible to send a ZCL
command to the device which initiate a certain action.

To send a ZCL commands, run the executable and pass it the target cluster name,
the target command name as well as an endpoint id.

The endpoint id must be between 1 and 240.

For example:

    $ chip-tool onoff on 1

The client will send a single command packet and then exit.

> For more details about POSIX CLI CHIPTool please visit
> [POSIX CLI CHIPTool](../../examples/chip-tool/README.md)

## Python Device Controller

### Building and installing

To make provisioning possible and to control the Matter device with Python
application, you can build and run the Python CHIP controller.

To build and install the Python Device Controller application check the guide
[Python Device Controller guide](python_chip_controller_building.md).

### Device commissioning for Python Device Controller

In order to send commands to a device, it must be paired with the client and
connected to the network.

To run the auto commissioning process via BLE:

-   Run Device Controller:

        chip-device-ctrl

-   Scan BLE devices:

        chip-device-ctrl > ble-scan

-   Pass the Wi-Fi credentials to the device:

        chip-device-ctrl > set-pairing-wifi-credential ssid credentials

-   Connect the device via BLE (provide the accessory device discriminator,
    setup pin code and node ID):

        chip-device-ctrl > connect -ble 3840 20202021 1234

### Sending ZCL commands Python

If the commissioning process was successful, it is possible to send a ZCL
command to the device which initiates a certain action.

`zcl <Cluster> <Command> <NodeId> <EndpointId> <GroupId> [arguments]`

Example:

    chip-device-ctrl > zcl LevelControl MoveWithOnOff 12344321 1 0 moveMode=1 rate=2

#### ZCL commands details

To get the list of supported clusters run:

    chip-device-ctrl > zcl ?

To get the list of available commands in cluster run:

    chip-device-ctrl > zcl ? <Cluster>

**Format of arguments**

For any integer and char string (null terminated) types, just use `key=value`,
for example: `rate=2`, `string=123`, `string_2="123 456"`

For byte string type, use `key=encoding:value`, currently, we support `str` and
`hex` encoding, the `str` encoding will encode a NULL terminated string. For
example, `networkId=hex:0123456789abcdef` (for
`[0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef]`), `ssid=str:Test` (for
`['T', 'e', 's', 't', 0x00]`).

For boolean type, use `key=True` or `key=False`
