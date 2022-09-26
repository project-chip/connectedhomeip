# Matter nRF Connect Lighting Example Application

The nRF Connect Lighting Example demonstrates how to remotely control a white
dimmable light bulb. It uses buttons to test changing the lighting and device
states and LEDs to show the state of these changes. You can use this example as
a reference for creating your own application.

![Nordic Smiconductor logo](../../platform/nrfconnect/doc/images/Logo_RGB_H-small.png)
![nRF52840 DK](../../platform/nrfconnect/doc/images/nRF52840-DK-small.png)

The example is based on
[Matter](https://github.com/project-chip/connectedhomeip) and Nordic
Semiconductor's nRF Connect SDK, and supports remote access and control of a
lighting over a low-power, 802.15.4 Thread network.

The example behaves as a Matter accessory, that is a device that can be paired
into an existing Matter network and can be controlled by this network. The
device works as a Thread Router.

<hr>

-   [Overview](#overview)
    -   [Bluetooth LE advertising](#bluetooth-le-advertising)
    -   [Bluetooth LE rendezvous](#bluetooth-le-rendezvous)
    -   [Device Firmware Upgrade](#device-firmware-upgrade)
-   [Requirements](#requirements)
    -   [Supported devices](#supported_devices)
-   [Device UI](#device-ui)
-   [Setting up the environment](#setting-up-the-environment)
    -   [Using Docker container for setup](#using-docker-container-for-setup)
    -   [Using native shell for setup](#using-native-shell-for-setup)
-   [Building](#building)
    -   [Removing build artifacts](#removing-build-artifacts)
    -   [Building with release configuration](#building-with-release-configuration)
    -   [Building with Pigweed RPCs](#building-with-pigweed-rpcs)
    -   [Building with Device Firmware Upgrade support](#building-with-device-firmware-upgrade-support)
-   [Configuring the example](#configuring-the-example)
    -   [Example build types](#example-build-types)
-   [Flashing and debugging](#flashing-and-debugging)
    -   [Flashing on the development kits](#nrfdks_flashing)
    -   [Flashing on the nRF52840 Dongle](#nrf52840dongle_flashing)
-   [Testing the example](#testing-the-example)
    -   [Testing using CHIPTool](#testing-using-chiptool)
    -   [Testing Device Firmware Upgrade](#testing-device-firmware-upgrade)
    -   [Device Tracing](#device-tracing)

<hr>

<a name="overview"></a>

## Overview

This example is running on the nRF Connect platform, which is based on Nordic
Semiconductor's
[nRF Connect SDK](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/index.html)
and [Zephyr RTOS](https://zephyrproject.org/). Visit Matter's
[nRF Connect platform overview](../../../docs/guides/nrfconnect_platform_overview.md)
to read more about the platform structure and dependencies.

The Matter device that runs the lighting application is controlled by the Matter
controller device over the Thread protocol. By default, the Matter device has
Thread disabled, and it should be paired with Matter controller and get
configuration from it. Some actions required before establishing full
communication are described below.

The example can be configured to use the secure bootloader and utilize it for
performing over-the-air Device Firmware Upgrade using Bluetooth LE.

### Bluetooth LE advertising

To commission the device onto a Matter network, the device must be discoverable
over Bluetooth LE that starts automatically upon the device startup, but only
for a predefined period of time (15 minutes by default). If the Bluetooth LE
advertising times out, you can re-enable it manually using **Button 4**.

### Bluetooth LE rendezvous

In Matter, the commissioning procedure is done over Bluetooth LE between a
Matter device and the Matter controller, where the controller has the
commissioner role.

To start the rendezvous, the controller must get the commissioning information
from the Matter device. The data payload is encoded within a QR code, printed to
the UART console, and shared using an NFC tag. NFC tag emulation starts
automatically when Bluetooth LE advertising is started and stays enabled until
Bluetooth LE advertising timeout expires.

#### Thread provisioning

Last part of the rendezvous procedure, the provisioning operation involves
sending the Thread network credentials from the Matter controller to the Matter
device. As a result, device is able to join the Thread network and communicate
with other Thread devices in the network.

### Device Firmware Upgrade

The example supports over-the-air (OTA) device firmware upgrade (DFU) using one
of the two available methods:

-   Matter OTA update that is mandatory for Matter-compliant devices and enabled
    by default
-   [Simple Management Protocol](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/guides/device_mgmt/index.html#device-mgmt)
    over Bluetooth LE, an optional proprietary method that can be enabled to
    work alongside the default Matter OTA update. Note that this protocol is not
    a part of the Matter specification.

For both methods, the
[MCUboot](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/mcuboot/index.html)
bootloader solution is used to replace the old firmware image with the new one.

#### Matter Over-the-Air Update

The Matter over-the-air update distinguishes two types of nodes: OTA Provider
and OTA Requestor.

An OTA Provider is a node that hosts a new firmware image and is able to respond
on an OTA Requestor's queries regarding availability of new firmware images or
requests to start sending the update packages.

An OTA Requestor is a node that wants to download a new firmware image and sends
requests to an OTA Provider to start the update process.

#### Simple Management Protocol

Simple Management Protocol (SMP) is a basic transfer encoding that is used for
device management purposes, including application image management. SMP supports
using different transports, such as Bluetooth LE, UDP, or serial USB/UART.

In this example, the Matter device runs the SMP Server to download the
application update image using the Bluetooth LE transport.

See the
[Building with Device Firmware Upgrade support](#building-with-device-firmware-upgrade-support)
section to learn how to enable SMP and use it for the DFU purpose in this
example.

#### Bootloader

MCUboot is a secure bootloader used for swapping firmware images of different
versions and generating proper build output files that can be used in the device
firmware upgrade process.

The bootloader solution requires an area of flash memory to swap application
images during the firmware upgrade. Nordic Semiconductor devices use an external
memory chip for this purpose. The memory chip communicates with the
microcontroller through the QSPI bus.

See the
[Building with Device Firmware Upgrade support](#building-with-device-firmware-upgrade-support)
section to learn how to change MCUboot and flash configuration in this example.

<hr>

<a name="requirements"></a>

## Requirements

The application requires a specific revision of the nRF Connect SDK to work
correctly. See [Setting up the environment](#setting-up-the-environment) for
more information.

<a name="supported_devices"></a>

### Supported devices

The example supports building and running on the following devices:

| Hardware platform                                                                                 | Build target               | Platform image                                                                                                                                          |
| ------------------------------------------------------------------------------------------------- | -------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [nRF52840 DK](https://www.nordicsemi.com/Software-and-Tools/Development-Kits/nRF52840-DK)         | `nrf52840dk_nrf52840`      | <details><summary>nRF52840 DK</summary><img src="../../platform/nrfconnect/doc/images/nRF52840_DK_info-medium.jpg" alt="nRF52840 DK"/></details>        |
| [nRF5340 DK](https://www.nordicsemi.com/Software-and-Tools/Development-Kits/nRF5340-DK)           | `nrf5340dk_nrf5340_cpuapp` | <details><summary>nRF5340 DK</summary><img src="../../platform/nrfconnect/doc/images/nRF5340_DK_info-medium.jpg" alt="nRF5340 DK"/></details>           |
| [nRF52840 Dongle](https://www.nordicsemi.com/Software-and-Tools/Development-Kits/nRF52840-Dongle) | `nrf52840dongle_nrf52840`  | <details><summary>nRF52840 Dongle</summary><img src="../../platform/nrfconnect/doc/images/nRF52840_Dongle-medium.jpg" alt="nRF52840 Dongle"/></details> |

<hr>

<a name="device-ui"></a>

## Device UI

This section lists the User Interface elements that you can use to control and
monitor the state of the device. These correspond to PCB components on the
platform image.

> **Note**:
>
> The following Device UI elements are missing on the nRF52840 Dongle: **Button
> 2**, **Button 3**, **Button 4**, **SEGGER J-Link USB port**, and **NFC port
> with antenna attached**. You can collect logs from the nRF52840 Dongle using
> the **nRF USB port** instead of the **SEGGER J-Link USB port**.
> Functionalities associated with the remaining missing elements are
> inaccessible.

**LED 1** shows the overall state of the device and its connectivity. The
following states are possible:

-   _Short Flash On (50 ms on/950 ms off)_ &mdash; The device is in the
    unprovisioned (unpaired) state and is waiting for a commissioning
    application to connect.

-   _Rapid Even Flashing (100 ms on/100 ms off)_ &mdash; The device is in the
    unprovisioned state and a commissioning application is connected through
    Bluetooth LE.

-   _Short Flash Off (950ms on/50ms off)_ &mdash; The device is fully
    provisioned, but does not yet have full Thread network or service
    connectivity.

-   _Solid On_ &mdash; The device is fully provisioned and has full Thread
    network and service connectivity.

**LED 2** simulates the light bulb and shows the state of the lighting. The
following states are possible:

-   _Solid On_ &mdash; The light bulb is on.

-   _Off_ &mdash; The light bulb is off.

**LED 3** can be used to identify the device. The LED starts blinking evenly
(500 ms on/500 ms off) when the Identify command of the Identify cluster is
received. The command's argument can be used to specify the duration of the
effect.

**Button 1** can be used for the following purposes:

-   _Pressed for 6 s_ &mdash; Initiates the factory reset of the device.
    Releasing the button within the 6-second window cancels the factory reset
    procedure. **LEDs 1-4** blink in unison when the factory reset procedure is
    initiated.

-   _Pressed for less than 3 s_ &mdash; Initiates the OTA software update over
    Bluetooth LE process. This feature is disabled by default, but can be
    enabled by following the
    [Building with Device Firmware Upgrade support](#building-with-device-firmware-upgrade-support)
    instruction.

**Button 2** &mdash; Pressing the button once changes the lighting state to the
opposite one.

**Button 4** &mdash; Pressing the button once starts the NFC tag emulation and
enables Bluetooth LE advertising for the predefined period of time (15 minutes
by default).

**SEGGER J-Link USB port** can be used to get logs from the device or
communicate with it using the
[command line interface](../../../docs/guides/nrfconnect_examples_cli.md).

**NFC port with antenna attached** can be used to start the
[rendezvous](#bluetooth-le-rendezvous) by providing the commissioning
information from the Matter device in a data payload that can be shared using
NFC.

<hr>

## Setting up the environment

Before building the example, check out the Matter repository and sync submodules
using the following command:

        ```
        $ git submodule update --init
        ```

The example requires a specific revision of the nRF Connect SDK. You can either
install it along with the related tools directly on your system or use a Docker
image that has the tools pre-installed.

If you are a macOS user, you won't be able to use the Docker container to flash
the application onto a Nordic development kit due to
[certain limitations of Docker for macOS](https://docs.docker.com/docker-for-mac/faqs/#can-i-pass-through-a-usb-device-to-a-container).
Use the [native shell](#using-native-shell) for building instead.

### Using Docker container for setup

To use the Docker container for setup, complete the following steps:

1.  If you do not have the nRF Connect SDK installed yet, create a directory for
    it by running the following command:

        ```
        $ mkdir ~/nrfconnect
        ```

2.  Download the latest version of the nRF Connect SDK Docker image by running
    the following command:

        ```
        $ docker pull nordicsemi/nrfconnect-chip
        ```

3.  Start Docker with the downloaded image by running the following command,
    customized to your needs as described below:

         ```
         $ docker run --rm -it -e RUNAS=$(id -u) -v ~/nrfconnect:/var/ncs -v ~/connectedhomeip:/var/chip \
             -v /dev/bus/usb:/dev/bus/usb --device-cgroup-rule "c 189:* rmw" nordicsemi/nrfconnect-chip
         ```

    In this command:

    -   _~/nrfconnect_ can be replaced with an absolute path to the nRF Connect
        SDK source directory.
    -   _~/connectedhomeip_ must be replaced with an absolute path to the Matter
        source directory.
    -   _-v /dev/bus/usb:/dev/bus/usb --device-cgroup-rule "c 189:_ rmw"\*
        parameters can be omitted if you are not planning to flash the example
        onto hardware. These parameters give the container access to USB devices
        connected to your computer such as the nRF52840 DK.
    -   _--rm_ can be omitted if you do not want the container to be
        auto-removed when you exit the container shell session.
    -   _-e RUNAS=\$(id -u)_ is needed to start the container session as the
        current user instead of root.

4.  Update the nRF Connect SDK to the most recent supported revision, by running
    the following command:

         ```
         $ cd /var/chip
         $ python3 scripts/setup/nrfconnect/update_ncs.py --update
         ```

Now you can proceed with the [Building](#building) instruction.

### Using native shell for setup

To use the native shell for setup, complete the following steps:

1.  Download and install the following additional software:

    -   [nRF Command Line Tools](https://www.nordicsemi.com/Software-and-Tools/Development-Tools/nRF-Command-Line-Tools)
    -   [GN meta-build system](https://gn.googlesource.com/gn/)

2.  If you do not have the nRF Connect SDK installed, follow the
    [guide](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/gs_assistant.html#)
    in the nRF Connect SDK documentation to install the latest stable nRF
    Connect SDK version. Since command-line tools will be used for building the
    example, installing SEGGER Embedded Studio is not required.

    If you have the SDK already installed, continue to the next step and update
    the nRF Connect SDK after initializing environment variables.

3.  Initialize environment variables referred to by the Matter and the nRF
    Connect SDK build scripts. Replace _nrfconnect-dir_ with the path to your
    nRF Connect SDK installation directory, and _toolchain-dir_ with the path to
    GNU Arm Embedded Toolchain.

         ```
         $ source nrfconnect-dir/zephyr/zephyr-env.sh
         $ export ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb
         $ export GNUARMEMB_TOOLCHAIN_PATH=toolchain-dir
         ```

4.  Update the nRF Connect SDK to the most recent supported revision by running
    the following command (replace _matter-dir_ with the path to Matter
    repository directory):

         ```
         $ cd matter-dir
         $ python3 scripts/setup/nrfconnect/update_ncs.py --update
         ```

Now you can proceed with the [Building](#building) instruction.

<hr>

<a name="building"></a>

## Building

Complete the following steps, regardless of the method used for setting up the
environment:

1.  Navigate to the example's directory:

        ```
        $ cd examples/lighting-app/nrfconnect
        ```

2.  Run the following command to build the example, with _build-target_ replaced
    with the build target name of the Nordic Semiconductor's kit you own, for
    example `nrf52840dk_nrf52840`:

        ```
        $ west build -b build-target
        ```

    You only need to specify the build target on the first build. See
    [Requirements](#requirements) for the build target names of compatible kits.

The output `zephyr.hex` file will be available in the `build/zephyr/` directory.

### Removing build artifacts

If you're planning to build the example for a different kit or make changes to
the configuration, remove all build artifacts before building. To do so, use the
following command:

    ```
    $ rm -r build
    ```

### Building with release configuration

To build the example with release configuration that disables the diagnostic
features like logs and command-line interface, run the following command:

    ```
    $ west build -b build-target -- -DCONF_FILE=prj_release.conf
    ```

Remember to replace _build-target_ with the build target name of the Nordic
Semiconductor's kit you own.

### Building with Pigweed RPCs

The RPCs in `lighting-common/lighting_service/lighting_service.proto` can be
used to control various functionalities of the lighting app from a USB-connected
host computer. To build the example with the RPC server, run the following
command with _build-target_ replaced with the build target name of the Nordic
Semiconductor's kit you own:

    ```
    $ west build -b build-target -- -DOVERLAY_CONFIG=rpc.overlay
    ```

### Building with Device Firmware Upgrade support

Support for DFU using Matter OTA is enabled by default.

To enable DFU over Bluetooth LE, run the following command with _build-target_
replaced with the build target name of the Nordic Semiconductor kit you are
using (for example `nrf52840dk_nrf52840`):

    ```
    $ west build -b build-target -- -DCONFIG_CHIP_DFU_OVER_BT_SMP=y
    ```

To completely disable support for both DFU methods, run the following command
with _build-target_ replaced with the build target name of the Nordic
Semiconductor kit you are using (for example `nrf52840dk_nrf52840`):

    ```
    $ west build -b build-target -- -DCONF_FILE=prj_no_dfu.conf
    ```

> **Note**:
>
> There are two types of Device Firmware Upgrade modes: single-image DFU and
> multi-image DFU. Single-image mode supports upgrading only one firmware image,
> the application image, and should be used for single-core nRF52840 DK devices.
> Multi-image mode allows to upgrade more firmware images and is suitable for
> upgrading the application core and network core firmware in two-core nRF5340
> DK devices.
>
> Currently the multi-image mode is only available for the DFU over Bluetooth LE
> method.

#### Changing bootloader configuration

To change the default MCUboot configuration, edit the `prj.conf` file located in
the `child_image/mcuboot` directory.

Make sure to keep the configuration consistent with changes made to the
application configuration. This is necessary for the configuration to work, as
the bootloader image is a separate application from the user application and it
has its own configuration file.

#### Changing flash memory settings

In the default configuration, the MCUboot uses the
[Partition Manager](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/scripts/partition_manager/partition_manager.html#partition-manager)
to configure flash partitions used for the bootloader application image slot
purposes. You can change these settings by defining
[static partitions](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/scripts/partition_manager/partition_manager.html#ug-pm-static).
This example uses this option to define using an external flash.

To modify the flash settings of your board (that is, your _build-target_, for
example `nrf52840dk_nrf52840`), edit the `pm_static_dfu.yml` file located in the
`configuration/build-target/` directory.

<hr>

<a name="configuring"></a>

## Configuring the example

The Zephyr ecosystem is based on Kconfig files and the settings can be modified
using the menuconfig utility.

To open the menuconfig utility, run the following command from the example
directory:

    ```
    $ west build -b build-target -t menuconfig
    ```

Remember to replace _build-target_ with the build target name of the Nordic
Semiconductor's kit you own.

Changes done with menuconfig will be lost if the `build` directory is deleted.
To make them persistent, save the configuration options in the `prj.conf` file.

### Example build types

The example uses different configuration files depending on the supported
features. Configuration files are provided for different build types and they
are located in the application root directory.

The `prj.conf` file represents a debug build type. Other build types are covered
by dedicated files with the build type added as a suffix to the prj part, as per
the following list. For example, the release build type file name is
`prj_release.conf`. If a board has other configuration files, for example
associated with partition layout or child image configuration, these follow the
same pattern.

Before you start testing the application, you can select one of the build types
supported by the sample. This sample supports the following build types,
depending on the selected board:

-   debug -- Debug version of the application - can be used to enable additional
    features for verifying the application behavior, such as logs or
    command-line shell. It can be used only for the nRF52840 DK and nRF5340 DK,
    as those platforms have DFU enabled by default.
-   release -- Release version of the application - can be used to enable only
    the necessary application functionalities to optimize its performance. It
    can be used only for the nRF52840 DK and nRF5340 DK, as those platforms have
    DFU enabled by default.
-   no_dfu -- Debug version of the application without Device Firmware Upgrade
    feature support - can be used for the nRF52840 DK, nRF5340 DK and nRF52840
    Dongle.

For more information, see the
[Configuring nRF Connect SDK examples](../../../docs/guides/nrfconnect_examples_configuration.md)
page.

<hr>

<a name="flashing"></a>

## Flashing and debugging

The flashing and debugging procedure is different for the development kits and
the nRF52840 Dongle.

<a name="nrfdks_flashing"></a>

### Flashing on the development kits

To flash the application to the device, use the west tool and run the following
command from the example directory:

        ```
        $ west flash --erase
        ```

If you have multiple development kits connected, west will prompt you to pick
the correct one.

To debug the application on target, run the following command from the example
directory:

        ```
        $ west debug
        ```

<a name="nrf52840dongle_flashing"></a>

### Flashing on the nRF52840 Dongle

Visit
[Programming and Debugging nRF52840 Dongle](https://docs.zephyrproject.org/latest/boards/arm/nrf52840dongle_nrf52840/doc/index.html#programming-and-debugging)
to read more about flashing on the nRF52840 Dongle.

<hr>

## Testing the example

Check the [CLI tutorial](../../../docs/guides/nrfconnect_examples_cli.md) to
learn how to use command-line interface of the application.

### Testing using CHIPTool

Read the
[Android commissioning guide](../../../docs/guides/nrfconnect_android_commissioning.md)
to see how to use [CHIPTool](../../../src/android/CHIPTool/README.md) for
Android smartphones to commission and control the application within a
Matter-enabled Thread network.

### Testing Device Firmware Upgrade

Read the
[DFU tutorial](../../../docs/guides/nrfconnect_examples_software_update.md) to
see how to upgrade your device firmware.

## Testing using the RPC console

If the flashed device has been built with the pigweed RPCs, the RPC console can
be used to interact with the device.

Build or install the [rpc console](../../common/pigweed/rpc_console/README.md)

Start the console

        ```
        $ chip-console --device /dev/ttyUSB0
        ```

From within the console you can then invoke rpcs:

        ```python
        rpcs.chip.rpc.Lighting.Get()

        rpcs.chip.rpc.Lighting.Set(on=True, level=128, color=protos.chip.rpc.LightingColor(hue=5, saturation=5))
        ```

## Device Tracing

Device tracing is available to analyze the device performance. To turn on
tracing, build with RPC enabled. See
[Using the RPC console](#building-with-pigweed-rpcs).

Obtain tracing json file.

```
    $ ./{PIGWEED_REPO}/pw_trace_tokenized/py/pw_trace_tokenized/get_trace.py -d {PORT} -o {OUTPUT_FILE} \
    -t {ELF_FILE} {PIGWEED_REPO}/pw_trace_tokenized/pw_trace_protos/trace_rpc.proto
```
