# Matter PSoC6 Lock Example

An example showing the use of Matter on the Infineon CY8CKIT-062S2-43012 board.

<hr>

-   [Matter PSoC6 Lock Example](#matter-psoc6-lock-example)
    -   [Introduction](#introduction)
    -   [Building](#building)
    -   [Flashing the Application](#flashing-the-application)
    -   [Commissioning and cluster control](#commissioning-and-cluster-control)
        -   [Setting up chip-tool](#setting-up-chip-tool)
        -   [Commissioning over BLE](#commissioning-over-ble)
            -   [Notes](#notes)
        -   [Cluster control](#cluster-control)
        -   [Factory Reset](#factory-reset)
    -   [Building with Optiga Trust M as HSM](#building-with-optiga-trust-m-as-hsm)
        -   [Optiga Trust M Provisioning](#optiga-trust-m-provisioning)
    -   [OTA Software Update](#ota-software-update)

<hr>

## Introduction

The PSoC6 lock example provides a demonstration of a door lock control device,
built using Matter and the Infineon Modustoolbox SDK. It can be controlled by a
Matter controller over Wi-Fi network.

The PSoC6 device can be commissioned over Bluetooth Low Energy where the device
and the Matter controller will exchange security information with the Rendezvous
procedure. Wi-Fi Network credentials are then provided to the PSoC6 device which
will then join the network.

## Building

-   Download and install
    [Modustoolbox Software v3.2](https://www.infineon.com/modustoolbox)

    Refer to `integrations/docker/images/stage-2/chip-build-infineon/Dockerfile`
    or `scripts/examples/gn_psoc6_example.sh` for downloading the Software and
    related tools.

-   Install some additional tools (likely already present for Matter
    developers): \$ sudo apt install gcc g++ clang ninja-build python
    python3-venv libssl-dev libavahi-client-dev libglib2.0-dev git cmake
    python3-pip

-   Supported hardware:
    [CY8CKIT-062S2-43012](https://www.infineon.com/CY8CKIT-062S2-43012)

*   Build the example application:

          $ source scripts/activate.sh
          $ scripts/build/build_examples.py --no-log-timestamps --target 'infineon-psoc6-lock' build

-   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip
          $ rm -rf out/

_To build with Infineon Hardware Security Module-OPTIGA™ Trust M for Device
attestation and other security use cases, please refer to the
[Building with OPTIGA™ Trust M as HSM](#building-with-optiga-trust-m-as-hsm) for
more instructions_

## Flashing the Application

-   Put CY8CKIT-062S2-43012 board on KitProg3 CMSIS-DAP Mode by pressing the
    `MODE SELECT` button. `KITPROG3 STATUS` LED is ON confirms board is in
    proper mode. (Modustoolbox Software needs to be installed)

-   On the command line:

          $ cd ~/connectedhomeip
          $ export CY_TOOLS_PATHS=<Modustoolbox install location>/tools_3.2
          $ python3 out/infineon-psoc6-lock/chip-psoc6-lock-example.flash.py

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

### Cluster control

-   After successful commissioning, use the doorlock cluster command to toggle
    device between lock or Unlock states.

    `$ ./out/debug/chip-tool doorlock lock-door 1234 1 --timedInteractionTimeoutMs 100`

    `$ ./out/debug/chip-tool doorlock unlock-door 1234 1 --timedInteractionTimeoutMs 100`

-   Cluster OnOff can also be done using the `USER_BTN1` button on the board.
    This button is configured with `APP_LOCK_BUTTON` in `include/AppConfig.h`.
    Press `USER_BTN1` on the board to toggle between lock and unlock states. The
    Lock/Unlock status of door can be observed with 'LED9' on the board. This
    LED is configured with `LOCK_STATE_LED` in `include/AppConfig.h`.

### Factory Reset

-   Commissioned Wi-Fi Credentials can be cleared by pressing `USER_BTN2` button
    on the board. All the data configured on the device during the initial
    commissioning will be deleted and device will be ready for commissioning
    again.
-   Pressing the button again within 5 seconds will cancel the factory reset of
    the board.

## Building with Optiga Trust M as HSM

Infineon Hardware Security Module-OPTIGA™ Trust M is a high-end security
solution that provides an anchor of trust for connecting IoT devices to the
cloud, giving every IoT device its own unique identity.

-   Supported hardware setup:
    [CY8CKIT-062S2-43012](https://www.cypress.com/CY8CKIT-062S2-43012)

    [OPTIGA™ Trust M MTR](https://www.infineon.com/cms/en/product/evaluation-boards/trust-m-mtr-shield/)

    [OPTIGA™ Trust Adapter](https://www.infineon.com/cms/en/product/evaluation-boards/optiga-trust-adapter/)

-   Building:

    Follow the steps to build with OPTIGA™ Trust M for device attestation use
    case:

    ```
      $ source scripts/activate.sh
      $ scripts/build/build_examples.py --no-log-timestamps --target 'infineon-psoc6-lock-trustm' build
    ```

-   To delete generated executable, libraries and object files use:

        $ cd ~/connectedhomeip
        $ rm -rf out/

-   Proceed to OPTIGA™ Trust M Provisioning section to complete the credential
    storage into HSM.

### Optiga Trust M Provisioning

For the description of OPTIGA™ Trust M Provisioning with test DAC generation and
PAI and CD storage, please refer to
[Infineon OPTIGA™ Trust M Provisioning](../../../../docs/guides/infineon_trustm_provisioning.md)

After completing OPTIGA™ Trust M Provisioning, proceed to
[Flashing the Application](#flashing-the-application) section to continue with
subsequent steps.

## OTA Software Update

For the description of Software Update process with infineon PSoC6 example
applications see
[Infineon PSoC6 OTA Software Update](../../../../docs/guides/infineon_psoc6_software_update.md)
