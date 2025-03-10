# CHIP Ameba All Clusters Example

A prototype application that demonstrates device commissioning and cluster
control.

---

-   [CHIP Ameba All Clusters Example](#chip-ameba-all-clusters-example)
    -   [Supported Device](#supported-device)
    -   [Building the Example Application](#building-the-example-application)
    -   [Commissioning](#commissioning)
        -   [BLE mode](#ble-mode)
        -   [IP mode](#ip-mode)
    -   [Cluster control](#cluster-control)
    -   [Running RPC Console](#running-rpc-console)
    -   [Running Matter Shell](#running-matter-shell)
    -   [Binding and Controlling a Device](#binding-and-controlling-a-device)

---

## Supported Device

The CHIP demo application is supported on
[Ameba RTL8722DM Board](https://www.amebaiot.com/en/amebad).

## Building the Example Application

-   Pull docker image:

          $ docker pull ghcr.io/project-chip/chip-build-ameba:119

-   Run docker container:

          $ docker run -it -v ${CHIP_DIR}:/root/chip ghcr.io/project-chip/chip-build-ameba:119

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

## Commissioning

There are two commissioning modes supported by Ameba platform:

### BLE mode

1. Build and Flash
2. The all-clusters example will run automatically after booting the Ameba
   board.
3. Test with
   [Chip-Tool](https://github.com/project-chip/connectedhomeip/tree/master/examples/chip-tool)

### IP mode

1. Build and Flash
2. The all-clusters example will run automatically after booting the Ameba
   board.
3. Connect to AP using `ATW0, ATW1, ATWC` commands
4. Test with
   [Chip-Tool](https://github.com/project-chip/connectedhomeip/tree/master/examples/chip-tool)

## Cluster Control

After successful commissioning, use the OnOff cluster command to control the
OnOff attribute. This allows you to toggle a parameter implemented by the device
to be On or Off.

-   Via
    [Chip-Tool](https://github.com/project-chip/connectedhomeip/tree/master/examples/chip-tool#using-the-client-to-send-matter-commands)

          $ ./chip-tool onoff on <nodeID> 1
          $ ./chip-tool onoff off <nodeID> 1

## Running RPC Console

-   Connect a USB-TTL adapter as shown below
-   For AmebaD

            Ameba         USB-TTL
            A19           TX
            A18           RX
            GND           GND

*   For AmebaZ2

            Ameba         USB-TTL
            A13           TX
            A14           RX
            GND           GND

-   Build the
    [chip-rpc console](https://github.com/project-chip/connectedhomeip/tree/master/examples/common/pigweed/rpc_console)

-   As part of building the example with RPCs enabled the chip_rpc python
    interactive console is installed into your venv. The python wheel files are
    also created in the output folder: out/debug/chip_rpc_console_wheels. To
    install the wheel files without rebuilding:

            $ pip3 install out/debug/chip_rpc_console_wheels/*.whl

*   Launch the chip-rpc console after resetting Ameba board

            $ chip-console --device /dev/tty<port connected to USB-TTL adapter> -b 115200

-   Get and Set lighting directly using the RPC console

            python
            rpcs.chip.rpc.Lighting.Get()
            rpcs.chip.rpc.Lighting.Set(on=True, level=128, color=protos.chip.rpc.LightingColor(hue=5, saturation=5))

## Running Matter Shell

-   Matter Shell is enabled whenever RPC is disabled.

-   RPC console and Matter Shell cannot be enabled at the same time as they use
    the same UART port.

-   Connect Ameba to the USB-TTL adapter as shown in the RPC section.

-   Open the USB-TTL serial port and type `help` to view the available commands

-   To know what are the available subcommands are there, enter `switch` command
    in the shell

## Binding and Controlling a Device

-   This example shows how to bind a Switch Device to a Controllee Device and
    control it through the Matter Shell. One binding client (Switch Device) and
    one binding server (Controllee) is required.

-   Commission the switch (nodeID 1) and controllee device (nodeID 2) using
    chip-tool.

              $ ./chip-tool pairing ble-wifi 1 <SSID> <PASSWORD> 20202021 3840
              $ ./chip-tool pairing ble-wifi 2 <SSID> <PASSWORD> 20202021 3840

-   After successful commissioning, configure the ACL in the controllee device
    to allow access from switch device and chip-tool.

              $ ./chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null },{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [1], "targets": null }]' 2 0

-   Bind the endpoint 1 OnOff cluster of the controllee device to the switch
    device.

              $ ./chip-tool binding write binding '[{"fabricIndex": 1, "node":2, "endpoint":1, "cluster":6}]' 1 1

-   Send OnOff command to the device through the switch device's Matter Shell

    `switch onoff on`

    `switch onoff off`

*   You may also bind more than one cluster to the switch device. Below command
    binds the Identify, OnOff, LevelControl, ColorControl and Thermostat
    clusters to the switch device.

              $ ./chip-tool binding write binding '[{"fabricIndex": 1, "node":2, "endpoint":1, "cluster":3}, {"fabricIndex": 1, "node":2, "endpoint":1, "cluster":6}, {"fabricIndex": 1, "node":2, "endpoint":1, "cluster":8}, {"fabricIndex": 1, "node":2, "endpoint":1, "cluster":768}, {"fabricIndex": 1, "node":2, "endpoint":1, "cluster":513}]' 1 1

-   After binding the clusters, you may send these cluster commands to the
    controllee device through the switch device's Matter Shell. Follow the
    format shown in the description of the commands.

    `switch onoff on`

    `switch levelcontrol movetolevel 100 0 0 0`

    `switch colorcontrol movetohue 100 0 0 0 0`

    `switch thermostat SPRL 0 0`

*   You may also request to read cluster attributes from Matter Shell

    `switch <cluster> read <attribute>`
