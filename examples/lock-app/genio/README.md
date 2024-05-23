# Matter `Genio` Lock Example

An example showing the use of Matter on the MediaTek `Genio` MT793X.

<hr>

-   [Matter Genio Lock Example](#matter-genio-lock-example)
    -   [Introduction](#introduction)
    -   [Building](#building)
    -   [Flashing the Application](#flashing-the-application)
    -   [Running the Complete Example](#running-the-complete-example)
        -   [Notes](#notes)

<hr>

## Introduction

The `Genio` (MT793X) lock example provides a baseline demonstration of a door
lock control device, built using Matter and the MediaTek `Genio` SDK. It can be
controlled by a Chip controller over Wi-Fi network.

The `Genio` device can be commissioned over Bluetooth Low Energy where the
device and the Chip controller will exchange security information with the
Rendez-vous procedure. Network credentials are then provided to the `Genio`
device which will then join the network.

The lighting example is intended to serve both as a means to explore the
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
          `./scripts/examples/gn_genio_example.sh ./examples/lighting-app/genio` `./out/lighting-app`

-   To delete generated executable, libraries and object files use:

          `$ cd ~/connectedhomeip`
          `$ rm -rf ./out/`

    OR use GN/Ninja directly

          `$ cd ~/connectedhomeip/examples/lighting-app/genio`
          `$ git submodule update --init`
          `$ source third_party/connectedhomeip/scripts/activate.sh`
          `$ gn gen out/debug`
          `$ ninja -C out/debug`

-   To delete generated executable, libraries and object files use:

          `$ cd ~/connectedhomeip/examples/lighting-app/genio`
          `$ rm -rf out/`

## Flashing the Application

-   Copy the GUI based
    [Flash Tool](https://github.com/MediaTek-Labs/genio-matter-bsp/tree/main/flash_tool/FlashBurningTool_V2.83).
    from the Linux Host that the example was build to a Windows PC.

    Flash Tool can be found in this source tree under this directory

    `third_party/mt793x_sdk/filogic/flash_tool`

-   On the Windows PC, run the Flash Tool

    1. Select the scatter.ini file in the `./out/lighting-app` directory.
    2. Follow the instruction that comes with `EK-AI7931LD KIT` to switch the
       kit to download mode.
    3. Click `Download` on FLASH TOOL.

## Running the Complete Example

-   You can provision and control the Chip device using the python controller,
    Chip tool standalone, Android or iOS app

    [CHIP
    Tool]](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/chip_tool_guide.md)

    Here is an example with the CHIP Tool controller:

    ```
      chiptool- pairing ble-wifi 1234 my-ap myappassword 20202021 3840

      chiptool onoff on 1 1

      chiptool onoff off 1 1
    ```

-   You can test the LED by light command as shown below

    ```
     > light
       help            Usage: light <subcommand>
       onoff           Usage: light onoff <subcommand>
       color           Usage: light color <r:0~255> <g:0~255> <b:0~255>
       level           Usage: light level <level:0~100>
    ```

### Notes

-   Depending on your network settings your router might not provide native ipv6
    addresses to your devices (Border router / PC). If this is the case, you
    need to add a static ipv6 addresses on both device and then an ipv6 route to
    the border router on your PC
