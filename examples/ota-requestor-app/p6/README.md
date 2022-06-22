#CHIP P6 OTA Requestor Example

An example demonstrating the OTA Requestor cluster on a Infineon
CY8CKIT-062S2-43012 board.

<hr>

-   [Matter P6 OTA Requestor Example](#chip-p6-ota-requestor-example)
    -   [Introduction](#introduction)
    -   [Building](#building)
    -   [Flashing the Application](#flashing-the-application)
    -   [Running OTA Update Process](#running-ota-update-process)
    -   [Notes](#notes)

<hr>

<a name="introduction"></a>

## Introduction

The P6 OTA Requestor example provides a baseline demonstration of a OTA
requestor device, built using Matter and the Infineon Modustoolbox SDK. It can
be controlled by Matter controller over Wi-Fi network.

The P6 device can be commissioned over Bluetooth Low Energy where the device and
the Matter controller will exchange security information with the Rendezvous
procedure. Wi-Fi Network credentials are then provided to the P6 device which
will then join the network.

<a name="building"></a>

## Building

-   [Modustoolbox Software](https://www.cypress.com/products/modustoolbox)

    Refer to `integrations/docker/images/chip-build-infineon/Dockerfile` or
    `scripts/examples/gn_p6_example.sh` for downloading the Software and related
    tools.

-   Install some additional tools (likely already present for Matter
    developers): \$ sudo apt install gcc g++ clang ninja-build python
    python3-venv libssl-dev libavahi-client-dev libglib2.0-dev git cmake
    python3-pip

-   Supported hardware:
    [CY8CKIT-062S2-43012](https://www.cypress.com/CY8CKIT-062S2-43012)

*   The following applications must be built to demonstrate the OTA process:

          - The P6 OTA Requestor App
          - The Updated P6 OTA Requestor App (or other app)
          - An OTA Provider App (the Linux ota-provider app is used here)
          - chip-tool

*   Build the P6 OTA Requestor application from the chip root dir:

          $ ./examples/ota-requestor-app/p6/ota_base_build.sh

*   Build the P6 OTA Update application from the chip root dir:

          $ ./examples/ota-requestor-app/p6/ota_update_build.sh

*   On a RPi4: Build the Linux OTA Provider application from the chip root dir:

          $ ./scripts/examples/gn_build_example.sh examples/ota-provider-app/linux/ out/ota_provider_debug/ chip_config_network_layer_ble=false

*   On a RPi4: Build chip-tool:

          $ ./scripts/examples/gn_build_example.sh examples/chip-tool/ out/chip-tool/

*   Additionally a pre-compiled bootloader must be flashed to the board. This
    can be found at:

          $ ./third_party/p6/p6_sdk/ota/matter-psoc6-mcuboot-bootloader.hex

<a name="flashing-the-application"></a>

## Flashing the Application

-   Flash the bootloader by first putting the CY8CKIT-062S2-43012 board into
    KitProg3 DAPLINK Mode by pressing the `MODE SELECT` button.
    `KITPROG3 STATUS` LED will blink to indicate the the board is in the proper
    mode. A drive named 'DAPLINK' should be automatically mounted. To flash
    drag-and-drop matter-psoc6-mcuboot-bootloader.hex into that drive.

-   Put CY8CKIT-062S2-43012 board back into KitProg3 CMSIS-DAP Mode by pressing
    the `MODE SELECT` button. `KITPROG3 STATUS` LED is ON confirms board is in
    proper mode.

-   On the command line:

          $ cd ~/connectedhomeip
          $ python3 out/ota_requestor_debug/chip-p6-ota-requestor-example.flash.py

<a name="running-ota-update-process"></a>

### Running OTA Update Process

-   Make sure the ota-requestor-app is flashed and booting on the
    CY8CKIT-062S2-43012.

-   Transfer out/ota_requestor_update_debug/chip-p6-ota-requestor-example.bin to
    a RPi4.

-   On the RPi: In terminal 1 run the Linux ota-provider-app as follows:

        $ ./out/ota_provider_debug/chip-ota-provider-app -f chip-p6-ota-requestor-example.bin

-   On the RPi: In terminal 2 run the following chip-tool commands

        $ ./out/chip-tool/chip-tool pairing ble-wifi 2 "<SSID>" "<PASSWORD>" 20202021 3840

        $ ./out/chip-tool/chip-tool pairing onnetwork 1 20202021

        $ ./chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}, {"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": [1, 2], "targets": [{"cluster": null, "endpoint": 0, "deviceType": null}]}]' 1 0

        $ ./chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}, {"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": [1, 2], "targets": [{"cluster": null, "endpoint": 0, "deviceType": null}]}]' 2 0

        $ ./chip-tool otasoftwareupdaterequestor write default-ota-providers '[{"fabricIndex": 1, "providerNodeID": 1, "endpoint": 0}]' 2 0

-   Press user button 1 on the CY8CKIT-062S2-43012. This will trigger a
    query-image call from the board using the default ota provider list written
    in the above commands.

-   Using a serial emulator reading from the CY8CKIT-062S2-43012, you should
    observe the updated application being transferred to the board, written to
    flash, and, when completed, booted into.

<a name="notes"></a>

#### Notes

Raspberry Pi 4 BLE connection issues can be avoided by running the following
commands. These power cycle the BlueTooth hardware and disable BR/EDR mode.

          $ sudo btmgmt -i hci0 power off
          $ sudo btmgmt -i hci0 bredr off
          $ sudo btmgmt -i hci0 power on
