# CHIP Ameba Lighting Example

This example demonstrates the Matter Lighting application on Ameba platform.

---

-   [CHIP Ameba Lighting Example](#chip-ameba-lighting-example)
    -   [Supported Device](#supported-device)
    -   [Building the Example Application](#building-the-example-application)
    -   [Commissioning](#commissioning)
        -   [BLE mode](#ble-mode)
        -   [IP mode](#ip-mode)
    -   [Cluster control](#cluster-control)

---

## Supported Device

The CHIP demo application is supported on
[Ameba RTL8722DM Board](https://www.amebaiot.com/en/amebad).

## Building the Example Application

-   Pull docker image:

          $ docker pull ghcr.io/project-chip/chip-build-ameba:112

-   Run docker container:

          $ docker run -it -v ${CHIP_DIR}:/root/chip ghcr.io/project-chip/chip-build-ameba:112

-   Setup build environment:

          $ source ./scripts/bootstrap.sh

-   To build the demo application:

          $ ./scripts/build/build_examples.py --target ameba-amebad-lighting build

    The output image files are stored in `out/ameba-amebad-lighting/asdk/image`
    folder.

    The bootloader image files are stored in
    `out/ameba-amebad-lighting/asdk/bootloader` folder.

-   After building the application, **Ameba Image Tool** is used to flash it to
    Ameba board.

1. Connect your device via USB and open Ameba Image Tool.
2. Select correct serial port and set baudrate as **115200**.
3. Browse and add the corresponding image files in the Flash Download list to
   the correct locations
4. Click **Download** button.

## Commissioning

There are two commissioning modes supported by Ameba platform:

### BLE mode

1. Build and Flash
2. The example will run automatically after booting the Ameba board.
3. Test with
   [Chip-Tool](https://github.com/project-chip/connectedhomeip/tree/master/examples/chip-tool)

### IP mode

1. Build and Flash
2. The example will run automatically after booting the Ameba board.
3. Connect to AP using `ATW0, ATW1, ATWC` commands
4. Test with
   [Chip-Tool](https://github.com/project-chip/connectedhomeip/tree/master/examples/chip-tool)

## Cluster Control

After successful commissioning, use the OnOff cluster command to control the
OnOff attribute. This allows you to toggle a parameter implemented by the device
to be On or Off.

-   Via
    [Chip-Tool](https://github.com/project-chip/connectedhomeip/tree/master/examples/chip-tool#using-the-client-to-send-matter-commands)

          $ ./chip-tool onoff on ${NODE_ID_TO_ASSIGN} 1
          $ ./chip-tool onoff off ${NODE_ID_TO_ASSIGN} 1
