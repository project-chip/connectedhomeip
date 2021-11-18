# CHIP Ameba All Clusters Example

A prototype application that demonstrates device commissioning and cluster
control.

---

-   [CHIP Ameba All Clusters Example](#chip-ameba-all-clusters-example)
    -   [Supported Device](#supported-device)
    -   [Building the Example Application](#building-the-example-application)
    -   [Commissioning and cluster control](#commissioning-and-cluster-control)
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

          $ docker pull pankore/chip-build-ameba:latest

-   Run docker container:

          $ docker run -it -v ${CHIP_DIR}:/root/chip pankore/chip-build-ameba:latest

-   Setup build environment:

          $ source ./scripts/bootstrap.sh

-   To build the demo application:

          $ ./scripts/build/build_examples.py --target ameba-amebad-all-clusters build

    The output image files are stored in
    `out/ameba-amebad-all-clusters/asdk/image` folder.

    The bootloader image files are stored in
    `out/ameba-amebad-all-clusters/asdk/bootloader` folder.

-   After building the application, **Ameba Image Tool** is used to flash it to
    Ameba board.

1. Connect your device via USB and open Ameba Image Tool.
2. Select correct serial port and set baudrate as **115200**.
3. Browse and add the corresponding image files in the Flash Download list to
   the correct locations
4. Click **Download** button.

## Commissioning and Cluster Control

## Commissioning

There are two commissioning modes supported by Ameba platform:

### BLE mode

1. In "connectedhomeip/config/ameba/args.gni"

    - set `chip_bypass_rendezvous = false`
    - Set `chip_config_network_layer_ble = true`

2. In "connectedhomeip/src/platform/Ameba/CHIPDevicePlatformConfig.h"

    - Set `#define CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE 1`

3. Build and Flash
4. Use ATS\$ command to run all-cluster example.
5. Test with
   [Chip-Tool](https://github.com/project-chip/connectedhomeip/tree/master/examples/chip-tool)
   or
   [Python Controller](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/python_chip_controller_building.md).

### IP mode

1. In "connectedhomeip/config/ameba/args.gni"

    - set `chip_bypass_rendezvous = false`
    - Set `chip_config_network_layer_ble = false`

2. In "connectedhomeip/src/platform/Ameba/CHIPDevicePlatformConfig.h"

    - Set `#define CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE 0`

3. Build and Flash
4. Use ATS\$ command to run all-cluster example.
5. Connect to AP using `ATW0, ATW1, ATWC` commands
6. Test with
   [Chip-Tool](https://github.com/project-chip/connectedhomeip/tree/master/examples/chip-tool)
   or
   [Python Controller](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/python_chip_controller_building.md).

## Cluster Control

After successful commissioning, use the OnOff cluster command to control the
OnOff attribute. This allows you to toggle a parameter implemented by the device
to be On or Off.

-   Via
    [Chip-Tool](https://github.com/project-chip/connectedhomeip/tree/master/examples/chip-tool#using-the-client-to-send-matter-commands)

          $ ./chip-tool onoff on 1
          $ ./chip-tool onoff off 1

-   Via
    [Python Controller](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/python_chip_controller_building.md#step-8-control-application-zcl-clusters)

          $ chip-device-ctrl > zcl OnOff Toggle 1234 1 0
