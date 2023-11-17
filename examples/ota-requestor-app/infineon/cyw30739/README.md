# Matter CYW30739 OTA Requestor Example

An example showing the use of the Matter OTA Requestor functionality on the
Infineon CYW30739 platform.

---

## Table of Contents

-   [CHIP CYW30739 OTA Requestor Example](#matter-cyw30739-ota-requestor-example)
    -   [Introduction](#introduction)
    -   [Building](#building)
    -   [Flashing the Application](#flashing-the-application)
    -   [Running the Complete Example](#running-the-complete-example)

---

## Introduction

The CYW30739 OTA Requestor example provides a baseline demonstration the Matter
OTA Requestor functionality built with the Infineon Modustoolbox SDK. It can be
controlled by a Matter controller over Thread network.

The CYW30739 device can be commissioned over Bluetooth Low Energy where the
device and the Matter controller will exchange security information with the
Rendez-vous procedure. Target Thread Network information including the active
dataset and CASE credentials are then provided.

## Building

-   Build the example application:

    ```bash
    $ cd ~/connectedhomeip
    $ git submodule update --init
    $ ./scripts/examples/gn_build_example.sh examples/ota-requestor-app/infineon/cyw30739 out/ota-requestor-app
    ```

-   To delete generated executable, libraries and object files use:

    ```bash
    $ cd ~/connectedhomeip
    $ rm -rf ./out/
    ```

-   OR use GN/Ninja directly

    ```bash
    $ cd ~/connectedhomeip/examples/ota-requestor-app/infineon/cyw30739
    $ git submodule update --init
    $ source third_party/connectedhomeip/scripts/activate.sh
    $ gn gen out/debug
    $ ninja -C out/debug
    ```

-   To delete generated executable, libraries and object files use:

    ```bash
    $ cd ~/connectedhomeip/examples/ota-requestor-app/infineon/cyw30739
    $ rm -rf out/
    ```

## Building Options

### DAC / DAC Key / PAI Certificate / Certificate Declaration

Infineon CYW30739 examples use test certifications, keys, and CD by default. For
a production build, manufacturers can provision certifications, keys, and CD by
the following arguments:

-   `matter_dac`, `matter_dac_key`, `matter_pai`, `matter_cd`

    ```bash
    $ ./scripts/examples/gn_build_example.sh examples/lighting-app/infineon/cyw30739 out/lighting-app \
    'matter_dac="/path/to/dac.der"' \
    'matter_dac_key="/path/to/dac_key.der"' \
    'matter_pai="/path/to/pai.der"' \
    'matter_cd="/path/to/cd.der"'
    ```

## Flashing the Application

### Enter Recovery Mode

Put the CYW30739 in to the recovery mode before running the flash script.

1. Press and hold the `RECOVERY` button on the board.
2. Press and hold the `RESET` button on the board.
3. Release the `RESET` button.
4. After one second, release the `RECOVERY` button.

### Run Flash Script

-   On the command line:

    ```bash
    $ cd ~/connectedhomeip/examples/ota-requestor-app/infineon/cyw30739
    $ python3 out/debug/chip-cyw30739-ota-requestor-example.flash.py
    ```

## Running the Complete Example

-   It is assumed here that you already have an OpenThread border router
    configured and running. If not see the following guide
    [Openthread_border_router](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/openthread_border_router_pi.md)
    for more information on how to setup a border router on a raspberryPi.

    -   Get the active dataset hex for the chip-tool.
        ```bash
        ot-ctl dataset active -x
        ```

-   You can provision and control the Chip device using the python controller,
    Chip tool standalone, Android or iOS app

    [Chip tool](https://github.com/project-chip/connectedhomeip/blob/master/examples/chip-tool/README.md)

    Here is an example with the chip tool:

    -   Start a Linux OTA Provider.

        ```bash
        # Start the OTA provider server with an OTA binary file
        chip-ota-provider-app -f <path/to/ota/binary>
        ```

    -   Setup the CYW30739 OTA Requestor the the Linux OTA Provider by the
        controller.

        ```bash
        # Pair the OTA Requestor
        chip-tool pairing ble-thread 1234 hex:0e080000000000000000000300000b35060004001fffe00208dead00beef00cafe0708fddead00beef000005108e11d8ea8ffaa875713699f59e8807e0030a4f70656e5468726561640102c2980410edc641eb63b100b87e90a9980959befc0c0402a0fff8 20202021 3840

        # Pair the OTA Provider
        chip-tool pairing onnetwork-vendor 4321 20202021 9050

        # Announce the OTA provider to the requestor
        chip-tool otasoftwareupdaterequestor announce-otaprovider 4321 9 0 0 1234 0
        ```
