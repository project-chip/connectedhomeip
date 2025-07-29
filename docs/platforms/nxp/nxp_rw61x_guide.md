# Matter NXP RW61x Applications Guide targeting FreeRTOS OS

-   [Introduction](#introduction)
-   [Building](#building)
-   [Flashing and debugging](#flashing-and-debugging)
-   [Testing the example](#testing-the-example)
-   [OTA Software Update](#ota-software-update)
-   [Thread Border Router overview](#thread-border-router-overview)

<hr>

<a name="introduction"></a>

## Introduction

The Matter RW61x example applications provide a working demonstration of the
RW610/RW612 board integration, built using the Project CHIP codebase and the NXP
MCUX SDK.

### Supported configurations

The examples support:

-   Matter over Wi-Fi
-   Matter over Openthread
-   Matter over Wi-Fi with OpenThread Border Router support.

### Supported build systems

RW61x platform supports two different build systems to generate the application
:

-   `GN`
-   `CMake`

### Hardware requirements

For Matter over Thread configuration :

-   For [`NXP RD-RW612-BGA`] board: BLE/15.4 antenna (to plug in Ant2)
-   For [`NXP FRDM-RW612`] board: no external antenna needed (embedded PCB
    antenna)

For Matter over WiFi configuration :

-   For [`NXP RD-RW612-BGA`] or [`NXP RD-RW610-BGA`] board: BLE antenna (to plug
    in Ant2) + Wi-Fi antenna (to plug in Ant1)
-   For [`NXP FRDM-RW612`] board: no external antenna needed (embedded PCB
    antenna)

For Matter over Wi-Fi with OpenThread Border Router :

-   For [`NXP RD-RW612-BGA`] board: BLE/15.4 antenna (to plug in Ant2) + Wi-Fi
    antenna (to plug in Ant1)
-   For [`NXP FRDM-RW612`] board: no external antenna needed (embedded PCB
    antenna)

For Matter over Ethernet configuration :

-   For [`NXP FRDM-RW612`] board: nothing to do (embedded PCB ethernet
    connector)
-   [`NXP RD-RW612-BGA`] board is not supported, recommendation: use `frdm`
    board

<a name="building"></a>

## Building

Make sure to follow common build instructions from
[CHIP NXP Examples Guide for FreeRTOS platforms](./nxp_examples_freertos_platforms.md#set-up-the-build-environment)
to set-up your environment.

> In the following steps, the "all-clusters-app" is used as an example.

### CMake Build System

The example supports configuration and build with CMake build system. You can
find more information in
[CMake Build System](./nxp_examples_freertos_platforms.md#cmake-build-system)
section which explains how to further configure your application build.

In the `west build` command, the `board` option can be replaced with
`rdrw612bga` if using `NXP RD-RW612-BGA` board, or `frdmrw612` if using
`NXP FRDM-RW612` board.

Example of build command to build the All-Clusters app with Matter-over-WiFi
configuration on RW61x platform :

```
user@ubuntu:~/Desktop/git/connectedhomeip$ west build -d build_matter -b frdmrw612 examples/all-clusters-app/nxp -DCONF_FILE_NAME=prj_wifi.conf
```

Note that the RW61x example supports various configurations that can be provided
to the `CONF_FILE_NAME` variable, you can refer to the
[table of available project configuration files and platform compatibility](./nxp_examples_freertos_platforms.md#available-project-configuration-files-and-platform-compatibility)
to check all the supported configurations.

> Note : BLE and Matter-CLI are enabled by default in Matter applications built
> with CMake. To disable them, you can refer to the
> [How to customize the CMake build](./nxp_examples_freertos_platforms.md#how-to-customize-the-cmake-build)
> section.

Ethernet cmake build command example:

```
user@ubuntu:~/Desktop/git/connectedhomeip$ west build -d build_matter -b frdmrw612 examples/thermostat/nxp -DCONF_FILE_NAME=prj_eth.conf
```

### GN Build System

#### Building with Matter over Wifi configuration on RW61x

-   Build Matter-over-Wifi configuration with BLE commissioning (ble-wifi) :

```
user@ubuntu:~/Desktop/git/connectedhomeip/examples/all-clusters-app/nxp/rt/rw61x$ gn gen --args="chip_enable_wifi=true" out/debug
user@ubuntu:~/Desktop/git/connectedhomeip/examples/all-clusters-app/nxp/rt/rw61x$ ninja -C out/debug
```

#### Building with Matter over Thread configuration on RW612

-   Build Matter-over-Thread configuration with BLE commissioning.

```
user@ubuntu:~/Desktop/git/connectedhomeip/examples/all-clusters-app/nxp/rt/rw61x$ gn gen --args="chip_enable_openthread=true chip_inet_config_enable_ipv4=false chip_config_network_layer_ble=true" out/debug
user@ubuntu:~/Desktop/git/connectedhomeip/examples/all-clusters-app/nxp/rt/rw61x$ ninja -C out/debug
```

#### Building with Matter over Wifi + OpenThread Border Router configuration on RW612

This configuration supports the Thread Border Router management cluster to
provision the Thread credentials. Enabling the Matter CLI in order to control
the Thread network on the Border Router is optional but recommended for other
features like the Thread credential sharing.

Note that the Thread Border Router management cluster is only supported on the
thermostat application for now.

-   Build Matter with Border Router configuration with BLE commissioning
    (ble-wifi) :

```
user@ubuntu:~/Desktop/git/connectedhomeip/examples/all-clusters-app/nxp/rt/rw610$ gn gen --args="chip_enable_wifi=true chip_enable_openthread=true nxp_enable_matter_cli=true" out/debug
user@ubuntu:~/Desktop/git/connectedhomeip/examples/all-clusters-app/nxp/rt/rw610$ ninja -C out/debug
```

#### General information

The resulting output file can be found in
out/debug/chip-rw61x-all-cluster-example.

Additional GN options can be added when building the application. You can check
[Common GN options to FreeRTOS platforms](./nxp_examples_freertos_platforms.md#general-information)
for the full list. Below is the list of RW61x specific GN options :

-   `board_version` is a GN option used to select the board variant. By default,
    the `NXP RD-RW612-BGA` board will be chosen by the application. To switch to
    `NXP FRDM-RW612` board variant, the GN argument `board_version=\"frdm\"`
    must be added to the _gn gen_ command.

## Manufacturing data

See
[Guide for writing manufacturing data on NXP devices](./nxp_manufacturing_flow.md)

Other comments:

The NXP RW61x application demonstrates the usage of encrypted Matter
manufacturing data storage. Matter manufacturing data should be encrypted using
an AES 128 software key before flashing them to the device flash.

Using DAC private key secure usage: Experimental feature, contain some
limitation: potential concurrent access issue during sign with dac key operation
due to the lack of protection between multiple access to `ELS` crypto module.
The argument `nxp_enable_secure_dac_private_key_storage=true` must be added to
the _gn gen_ command to enable secure private DAC key usage with S50.
`nxp_use_factory_data=true` must have been added to the _gn gen_ command

DAC private key generation: The argument `chip_convert_dac_private_key=1` must
be added to the _gn gen_ command to enable DAC private plain key conversion to
blob with S50. `nxp_enable_secure_dac_private_key_storage=1` must have been
added to the _gn gen_ command

`ELS` contain concurrent access risks. They must be fixed before enabling it by
default.

<a name="flashing-and-debugging"></a>

## Flashing and debugging

### Flashing the example application

We recommend using `JLink` from Segger to flash the example application. It can
be downloaded and installed from
https://www.segger.com/products/debug-probes/j-link. Once installed, JLink can
be run to flash the application using the following steps :

```
$ JLink
```

```
J-Link > connect
Device> ? # you will be presented with a dialog -> select `RW612`
Please specify target interface:
J) JTAG (Default)
S) SWD
T) cJTAG
TIF> S
Specify target interface speed [kHz]. <Default>: 4000 kHz
Speed> # <enter>

J-Link > exec EnableEraseAllFlashBanks
J-Link > erase 0x8000000, 0x88a0000
```

Program the application executable :

```
J-Link > loadfile <application_binary>
```

To program an application in binary format you can use the following command
instead :

```
J-Link > loadbin <application_binary>.bin 0x8000400
```

<a name="testing-the-example"></a>

## Testing the example

To test the example, please make sure to check the `Testing the example` section
from the common readme
[CHIP NXP Examples Guide for FreeRTOS platforms](./nxp_examples_freertos_platforms.md#testing-the-example).

### UART details

Testing the example with the CLI enabled will require connecting to UART1 and
UART2, here are more details to follow for RW61x platform :

-   UART1 : `Flexcomm3`. To view output on this UART, a USB cable could be
    plugged in.
-   UART2 : `Flexcomm0`. To view output on this UART, a pin should be plugged to
    an `USB to UART adapter`.
    -   For [`NXP RD-RW612-BGA`] board, use connector `HD2 pin 03`.
    -   For [`NXP FRDM-RW612`] board, use `J5 pin 4` (`mikroBUS`: TX).

<a name="ota-software-update"></a>

## OTA Software Update

Over-The-Air software updates are supported with the RW61x examples. The process
to follow in order to perform a software update is described in the dedicated
guide
['Matter Over-The-Air Software Update with NXP RW61x example applications'](./nxp_RTs_ota_software_update.md).

<a name="thread-border-router-overview"></a>

## Thread Border Router overview

To enable Thread Border Router support see the [build](#building) section.

The complete Border Router guide is located [here](./nxp_otbr_guide.md).
