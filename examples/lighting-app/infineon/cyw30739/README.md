# Matter CYW30739 Lighting Example

An example showing the use of Matter on the Infineon CYW30739 platform.

---

## Table of Contents

-   [CHIP CYW30739 Lighting Example](#matter-cyw30739-lighting-example)
    -   [Introduction](#introduction)
    -   [Building](#building)
    -   [Flashing the Application](#flashing-the-application)
    -   [Running the Complete Example](#running-the-complete-example)

---

## Introduction

The CYW30739 lighting example provides a baseline demonstration of a Light
control device, built using Matter and the Infineon Modustoolbox SDK. It can be
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
    $ ./scripts/examples/gn_build_example.sh examples/lighting-app/infineon/cyw30739 out/lighting-app
    ```

-   To delete generated executable, libraries and object files use:

    ```bash
    $ cd ~/connectedhomeip
    $ rm -rf ./out/
    ```

-   OR use GN/Ninja directly

    ```bash
    $ cd ~/connectedhomeip/examples/lighting-app/infineon/cyw30739
    $ git submodule update --init
    $ source third_party/connectedhomeip/scripts/activate.sh
    $ gn gen out/debug
    $ ninja -C out/debug
    ```

-   To delete generated executable, libraries and object files use:

    ```bash
    $ cd ~/connectedhomeip/examples/lighting-app/infineon/cyw30739
    $ rm -rf out/
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
    $ cd ~/connectedhomeip/examples/lighting-app/infineon/cyw30739
    $ python3 out/debug/chip-cyw30739-lighting-example.flash.py
    ```

## Running the Complete Example

-   It is assumed here that you already have an OpenThread border router
    configured and running. If not see the following guide
    [Openthread_border_router](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/openthread_border_router_pi.md)
    for more information on how to setup a border router on a raspberryPi.

-   You can provision and control the Chip device using the python controller,
    Chip tool standalone, Android or iOS app

    [Python Controller](https://github.com/project-chip/connectedhomeip/blob/master/src/controller/python/README.md)

    Here is an example with the Python controller:

    ```bash
    $ chip-device-ctrl
    chip-device-ctrl > connect -ble 3840 20202021 1234
    chip-device-ctrl > zcl NetworkCommissioning AddThreadNetwork 1234 0 0 operationalDataset=hex:0e080000000000000000000300000b35060004001fffe00208dead00beef00cafe0708fddead00beef000005108e11d8ea8ffaa875713699f59e8807e0030a4f70656e5468726561640102c2980410edc641eb63b100b87e90a9980959befc0c0402a0fff8 breadcrumb=0 timeoutMs=1000
    chip-device-ctrl > zcl NetworkCommissioning EnableNetwork 1234 0 0 networkID=hex:dead00beef00cafe breadcrumb=0 timeoutMs=1000
    chip-device-ctrl > close-ble
    chip-device-ctrl > resolve 1234
    chip-device-ctrl > zcl OnOff Toggle 1234 1 0
    ```
