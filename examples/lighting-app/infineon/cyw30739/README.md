# Matter CYW30739 Lighting Example

An example showing the use of Matter on the Infineon CYW30739 platform.

---

## Table of Contents

-   [CHIP CYW30739 Lighting Example](#matter-cyw30739-lighting-example)
    -   [Table of Contents](#table-of-contents)
    -   [Introduction](#introduction)
    -   [Installing ModusToolbox™ Software](#installing-modustoolbox-software)
        -   [ModusToolbox™ tools package](#modustoolbox-tools-package)
            -   [Note for WSL (Windows Subsystem for Linux)](#note-for-wsl-windows-subsystem-for-linux)
        -   [Checkout Submodules and Bootstrap](#checkout-submodules-and-bootstrap)
    -   [Building](#building)
    -   [Factory Data](#factory-data)
        -   [Commissionable Data](#commissionable-data)
        -   [Device Information](#device-information)
        -   [DAC / DAC Key / PAI Certificate / Certificate Declaration](#dac--dac-key--pai-certificate--certificate-declaration)
        -   [Use Provisioned Optiga Trust M](#use-provisioned-optiga-trust-m)
    -   [Flashing the Application](#flashing-the-application)
        -   [Enter Recovery Mode](#enter-recovery-mode)
        -   [Run Flash Script](#run-flash-script)
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

## Installing [ModusToolbox™ Software](https://www.infineon.com/cms/en/design-support/tools/sdk/modustoolbox-software)

Follow the
[Installation Guide](https://www.infineon.com/ModusToolboxInstallguide) to
download and install the ModusToolbox™ Software.

### ModusToolbox™ tools package

ModusToolbox™ tools package should be automatically installed in the default
location if you followed the
[Installation Guide](https://www.infineon.com/ModusToolboxInstallguide). Please
refer to the Installation Guide for the default location for different operating
systems.

If you need to install the ModusToolbox™ tools package in a custom location, you
must set the environment variable `CY_TOOLS_PATHS` to the
`/path/to/ModusToolbox/tools_x.x` to specify the location of tools.

```bash
export CY_TOOLS_PATHS="/path/to/ModusToolbox/tools_x.x"
```

#### Note for WSL (Windows Subsystem for Linux)

If you are using WSL, please ensure you have installed the ModusToolbox™
Software for Linux. Running Windows tools directly from the WSL command line
would cause path resolution failure in the build process.

### Checkout Submodules and Bootstrap

Before building the example, check out the Matter repository and sync submodules
using the following command:

```bash
$ cd ~/connectedhomeip
$ scripts/checkout_submodules.py --platform infineon
$ bash scripts/bootstrap.sh -p all,infineon
```

## Building

-   Build the example application:

    ```bash
    $ cd ~/connectedhomeip
    $ scripts/examples/gn_build_example.sh examples/lighting-app/infineon/cyw30739 out/cyw30739-light
    ```

-   OR use GN/Ninja directly

    ```bash
    $ cd ~/connectedhomeip
    $ source scripts/activate.sh
    $ gn gen --root=examples/lighting-app/infineon/cyw30739 out/cyw30739-light
    $ ninja -C out/cyw30739-light [lighting_app-BOARD_NAME]
    ```

-   To delete generated executable, libraries and object files use:

    ```bash
    $ cd ~/connectedhomeip
    $ rm -rf out/cyw30739-light
    ```

## Factory Data

### Commissionable Data

Infineon CYW30739 examples use test passcode, discriminator and PAKE parameters
by default. For a production build, manufacturers should override commissionable
data by the following arguments:

-   `matter_passcode`, `matter_discriminator`, `matter_pake_iteration_count`,
    `matter_pake_salt`

    ```bash
    $ cd ~/connectedhomeip
    $ scripts/examples/gn_build_example.sh examples/lighting-app/infineon/cyw30739 out/cyw30739-light \
    'matter_passcode=20202021' \
    'matter_discriminator=3840' \
    'matter_pake_iteration_count=1000' \
    'matter_pake_salt="U1BBS0UyUCBLZXkgU2FsdA=="'
    ```

### Device Information

Infineon CYW30739 examples support overriding the default device information by
the following arguments:

-   matter_vendor_name
-   matter_vendor_id
-   matter_product_name
-   matter_product_id
-   matter_serial_number
-   matter_hardware_version
-   matter_hardware_version_string

To override the default device information, pass the desired values to the
`gn_build_example.sh` script as arguments.

    ```bash
    $ cd ~/connectedhomeip
    $ scripts/examples/gn_build_example.sh examples/lighting-app/infineon/cyw30739 out/cyw30739-light \
    'matter_vendor_name="Infineon"' \
    'matter_vendor_id="0x1388"' \
    'matter_product_name="TEST_PRODUCT"' \
    'matter_product_id="0x0001"' \
    'matter_serial_number="TEST_SN"' \
    'matter_hardware_version=30739' \
    'matter_hardware_version_string="30739"'
    ```

### DAC / DAC Key / PAI Certificate / Certificate Declaration

Infineon CYW30739 examples use development certifications, keys, and CD by
default. For a production build, manufacturers can provision certifications,
keys, and CD by the following arguments:

-   `matter_att_cert`, `matter_att_cert_password`, `matter_cd`

    ```bash
    $ cd ~/connectedhomeip
    $ scripts/examples/gn_build_example.sh examples/lighting-app/infineon/cyw30739 out/cyw30739-light \
    'matter_att_cert="/path/to/att_cert.p12"' \
    'matter_att_cert_password="password"' \
    'matter_cd="/path/to/cd.der"'
    ```

### Use Provisioned Optiga Trust M

For boards supported by Optiga Trust M, CYW30739 will provision factory data to
the Optiga Trust M by default for easy development.

The Optiga Trust M on a production board should come with provisioned factory
data. To ensure its optimal use, please configure the Optiga using the following
arguments:

-   `use_provisioned_optiga`, `optiga_dac_object_id`,
    `optiga_dac_key_object_id`, `optiga_pai_cert_object_id`

    ```bash
    $ cd ~/connectedhomeip
    $ scripts/examples/gn_build_example.sh examples/lighting-app/infineon/cyw30739 out/cyw30739-light \
    'optiga_dac_object_id="0xe0e0"' \
    'optiga_dac_key_object_id="0xe0f0"' \
    'optiga_pai_cert_object_id="0xe0e8"'
    ```

The developer must set the object IDs to corresponding values matching the
configurations used in the Optiga provisioning procedure.

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
    $ cd ~/connectedhomeip
    $ out/cyw30739-light/lighting_app-BOARD_NAME.flash.py --port <port>
    ```

## Running the Complete Example

-   It is assumed here that you already have an OpenThread border router
    configured and running. If not see the following guide
    [Openthread_border_router](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/openthread_border_router_pi.md)
    for more information on how to setup a border router on a raspberryPi.

-   You can provision and control the device using the Python controller REPL,
    chip-tool standalone, Android or iOS app

    [Python Controller](https://github.com/project-chip/connectedhomeip/blob/master/src/controller/python/README.md)
