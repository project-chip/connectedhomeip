# Matter `Genio` Light Switch Example

An example showing the use of Matter on the MediaTek `Genio` MT793X.

<hr>

-   [Matter Genio Light Switch Example](#matter-genio-light-switch-example)
    -   [Introduction](#introduction)
    -   [Building](#building)
    -   [Flashing the Application](#flashing-the-application)
    -   [Running the Complete Example](#running-the-complete-example)
        -   [Notes](#notes)

<hr>

## Introduction

The `Genio` (MT793X) light switch example provides a baseline demonstration of a
on-off light switch device, built using Matter and the MediaTek `Genio` SDK. It
can be controlled by a Chip controller over Wi-Fi network..

The `Genio` device can be commissioned over Bluetooth Low Energy where the
device and the Chip controller will exchange security information with the
Rendez-vous procedure. Network credentials are then provided to the `Genio`
device which will then join the network.

The light switch example is intended to serve both as a means to explore the
workings of Matter as well as a template for creating real products based on the
MediaTek platform.

## Building

-   Following the Linux related descriptions in
    [Build Matter](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/BUILDING.md)
    to prepare the build environment.

-   Supported hardware:

    `Genio` 130A (MT7931) board:

    -   `EK-AI7931LD KIT`

*   Build the example application:

          `cd ~/connectedhomeip`
          `./scripts/examples/gn_genio_example.sh ./examples/light-switch-app/genio` `./out/light-switch-app`

-   To delete generated executable, libraries and object files use:

          `$ cd ~/connectedhomeip`
          `$ rm -rf ./out/`

    OR use GN/Ninja directly

          `$ cd ~/connectedhomeip/examples/light-switch-app/genio`
          `$ git submodule update --init`
          `$ source third_party/connectedhomeip/scripts/activate.sh`
          `$ gn gen out/debug`
          `$ ninja -C out/debug`

-   To delete generated executable, libraries and object files use:

          `$ cd ~/connectedhomeip/examples/light-switch-app/genio`
          `$ rm -rf out/`

## Flashing the Application

-   Copy the GUI based
    [Flash Tool](https://github.com/MediaTek-Labs/genio-matter-bsp/tree/main/flash_tool/FlashBurningTool_V2.83).
    from the Linux Host that the example was build to a Windows PC.

    Flash Tool can be found in this source tree under this directory

    `third_party/mt793x_sdk/filogic/flash_tool`

-   On the Windows PC, run the Flash Tool

    1. Select the scatter.ini file in the `./out/light-switch-app` directory.
    2. Follow the instruction that comes with `EK-AI7931LD KIT` to switch the
       kit to download mode.
    3. Click `Download` on FLASH TOOL.

## Running the Complete Example

-   This example needs one lighting-app device and one light-switch-app device.

-   You can provision and control the Chip device using the python controller,
    Chip tool standalone, Android or iOS app

    [CHIP
    Tool]](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/chip_tool_guide.md)

-   First of all, you have to commission with the light-switch-app (nodeID 1)
    and the lighting-app (nodeID 2) with the CHIP tool by following commands.

    ```
      chip-tool pairing ble-wifi 1 my-ap myappassword 20202021 3840

      chip-tool pairing ble-wifi 2 my-ap myappassword 20202021 3840
    ```

-   Next, you have to configure the ACL in the lighting-app device to allow
    access from switch device and chip-tool, and binding the lighting-app device
    to the light-switch-app device by following commands.

    ```
      chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null },{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [1], "targets": null }]' 2 0

      chip-tool binding write binding '[{"fabricIndex": 1, "node":2, "endpoint":1, "cluster":6}]' 1 1

    ```

-   Once all setup successfully, you can control the lighting-app device through
    the Matter Shell on the light-switch-app device.

    ```
       > switch onoff on
       > switch onoff off
    ```

### Notes

-   Depending on your network settings your router might not provide native ipv6
    addresses to your devices (Border router / PC). If this is the case, you
    need to add a static ipv6 addresses on both device and then an ipv6 route to
    the border router on your PC
