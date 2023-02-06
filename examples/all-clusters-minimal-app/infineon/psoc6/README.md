# CHIP PSoC6 All Clusters Example

An example showing the use of Matter on the Infineon CY8CKIT-062S2-43012 board.

<hr>

-   [Matter PSoC6 All Clusters Example](#chip-psoc6-all-clusters-example)
    -   [Introduction](#introduction)
    -   [Building](#building)
    -   [Flashing the Application](#flashing-the-application)
    -   [Commissioning and cluster control](#commissioning-and-cluster-control)
        -   [Setting up chip-tool](#setting-up-chip-tool)
        -   [Commissioning over BLE](#commissioning-over-ble)
            -   [Notes](#notes)
        -   [Factory Reset](#factory-reset)
    -   [OTA Software Update](#ota-software-update)

<hr>

## Introduction

The PSoC6 clusters example provides a baseline demonstration of a Cluster
control device, built using Matter and the Infineon Modustoolbox SDK. It can be
controlled by Matter controller over Wi-Fi network.

The PSoC6 device can be commissioned over Bluetooth Low Energy where the device
and the Matter controller will exchange security information with the Rendezvous
procedure. Wi-Fi Network credentials are then provided to the PSoC6 device which
will then join the network.

## Building

-   [Modustoolbox Software](https://www.cypress.com/products/modustoolbox)

    Refer to `integrations/docker/images/chip-build-infineon/Dockerfile` or
    `scripts/examples/gn_psoc6_example.sh` for downloading the Software and
    related tools.

-   Install some additional tools (likely already present for Matter
    developers):

    ```
    sudo apt install gcc g++ clang ninja-build python \
      python3-venv libssl-dev libavahi-client-dev libglib2.0-dev git cmake \
      python3-pip
    ```

-   Supported hardware:
    [CY8CKIT-062S2-43012](https://www.cypress.com/CY8CKIT-062S2-43012)

*   Build the example application:

          $ source scripts/activate.sh
          $ scripts/build/build_examples.py --no-log-timestamps --target 'infineon-psoc6-all-clusters-minimal' build

-   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip
          $ rm -rf out/

## Flashing the Application

-   Put CY8CKIT-062S2-43012 board on KitProg3 CMSIS-DAP Mode by pressing the
    `MODE SELECT` button. `KITPROG3 STATUS` LED is ON confirms board is in
    proper mode.

-   On the command line:

          $ cd ~/connectedhomeip
          $ python3 out/infineon-psoc6-all-clusters-minimal/chip-psoc6-clusters-minimal-example.flash.py

## Commissioning and cluster control

Commissioning can be carried out using BLE.

### Setting up Chip tool

Once PSoC6 is up and running, we need to set up chip-tool on Raspberry Pi 4 to
perform commissioning and cluster control.

-   Set up python controller.

           $ cd {path-to-connectedhomeip}
           $ ./scripts/examples/gn_build_example.sh examples/chip-tool out/debug

-   Execute the controller.

           $ ./out/debug/chip-tool

### Commissioning over BLE

Run the built executable and pass it the discriminator and pairing code of the
remote device, as well as the network credentials to use.

         $ ./out/debug/chip-tool pairing ble-wifi 1234 ${SSID} ${PASSWORD} 20202021 3840

         Parameters:
         1. Discriminator: 3840
         2. Setup-pin-code: 20202021
         3. Node ID: 1234 (you can assign any node id)
         4. SSID : Wi-Fi SSID
         5. PASSWORD : Wi-Fi Password

#### Notes

Raspberry Pi 4 BLE connection issues can be avoided by running the following
commands. These power cycle the BlueTooth hardware and disable BR/EDR mode.

          $ sudo btmgmt -i hci0 power off
          $ sudo btmgmt -i hci0 bredr off
          $ sudo btmgmt -i hci0 power on

### Factory Reset

-   Commissioned Wi-Fi Credentials can be cleared by pressing `USER_BTN2` button
    on the board. All the data configured on the device during the initial
    commissioning will be deleted and device will be ready for commissioning
    again.

-   Pressing the button again within 5 seconds will cancel the factory reset of
    the board.

## OTA Software Update

For the description of Software Update process with infineon PSoC6 example
applications see
[Infineon PSoC6 OTA Software Update](../../../../docs/guides/infineon_psoc6_software_update.md)
