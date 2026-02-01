# Matter NXP MCXW71 Applications Guide

-   [Introduction](#introduction)
-   [Building](#building)
-   [Manufacturing Data](#manufacturing-data)
-   [Flashing and debugging](#flashing-and-debugging)
-   [Testing the example](#testing-the-example)
-   [OTA Software Update](#ota-software-update)

<hr>

<a name="introduction"></a>

## Introduction

The Matter MCXW71 example applications provide a working demonstration of the
FRDM-MCXW71 board integration, built using the Project CHIP codebase and the NXP
MCUX SDK.

### Supported configurations

The examples support:

-   Matter over Openthread

### Supported build systems

MCXW71 platform supports only one build system to generate the application :

-   `CMake`

<a name="building"></a>

## Building

Make sure to follow shared build instructions from
[CHIP NXP Examples Guide for FreeRTOS platforms](./nxp_examples_freertos_platforms.md)
to set-up your environment.

> In the following steps, the "lighting-app" is used as an example.

The examples support configuration and build using the CMake build system. You
can find more information in
[CMake Build System](./nxp_examples_freertos_platforms.md#cmake-build-system)
section which explains how to further configure your application build.

In the `west build` command, the `board` option should be replaced with
`frdmmcxw71`. The build option
`-DCONFIG_MCUX_COMPONENT_middleware.freertos-kernel.config=n` must be used in
the build command line for the MCXW71 platform.

Example of build command to build the Lighting app with Matter-over-Thread (FTD
device type) configuration on MCXW71 platform :

```
user@ubuntu:~/Desktop/git/connectedhomeip$ west build -d bin/lighting-app/ftd -b frdmmcxw71 examples/lighting-app/nxp -DCONF_FILE_NAME=prj_thread_ftd.conf -DCONFIG_MCUX_COMPONENT_middleware.freertos-kernel.config=n
```

Note that the MCXW71 example supports various configurations that can be
provided to the `CONF_FILE_NAME` variable, you can refer to the
[table of available project configuration files and platform compatibility](./nxp_examples_freertos_platforms.md#build-config-files)
to check all the supported configurations.

<a name="manufacturing-data"></a>

## Manufacturing data

See
[Guide for writing manufacturing data on NXP devices](./nxp_manufacturing_flow.md).

<a name="flashing-and-debugging"></a>

## Flashing and debugging

### Flashing the NBU

Normally the `NBU` image should be written/updated only when migrating to a new
NXP SDK or to a new Matter release altogether. The procedure below can be used
to upload/refresh the board's NBU firmware.

1. Install the Bootloader Host Application (`blhost`) from the
   [MCU Bootloader for NXP Microcontrollers downloads page](https://www.nxp.com/design/design-center/software/development-software/mcuxpresso-software-and-tools-/mcu-bootloader-for-nxp-microcontrollers:MCUBOOT).

2. The NBU image is available in your Matter source code tree in

`third_party/nxp/nxp_matter_support/github_sdk/sdk_next/repo/mcuxsdk/middleware/wireless/ieee-802.15.4/bin/k32w1_mcxw71/k32w1_mcxw71_nbu_ble_15_4_dyn_matter.sb3`

3. Initialize the board in `ISP` (In-System Programming) mode, which allows the
   SoC to accept commands to erase/write/update internal flash memories via the
   serial port. In order to do that disconnect the board from your PC and hold
   down the SW3 button while reconnecting it back. When you release the SW3
   button the board should be operating in ISP mode.

4. Erase the host and NBU flash memories using the `blhost` tool

    ```
    blhost -p <assigned_port> flash-erase-all
    ```

5. Use the same `blhost` tool to write the NBU image on the board:

    ```
    blhost -p <assigned_port> -- receive-sb-file k32w1_mcxw71_nbu_ble_15_4_dyn_matter.sb3
    ```

6. Return the board to normal operating mode by disconnecting and then
   reconnecting it back to your PC.

### Flashing the example application

We recommend using `JLink Commander` from Segger to flash the example
application. It can be downloaded and installed from
https://www.segger.com/downloads/jlink. Once installed, JLink Commander can be
run to flash the application using the following steps :

```
$ JLink
```

```
J-Link > connect
Device> ? # you will be presented with a dialog -> select `MCXW716`
Please specify target interface:
J) JTAG (Default)
S) SWD
T) cJTAG
TIF> S
Specify target interface speed [kHz]. <Default>: 4000 kHz
Speed> # <enter>
```

Program the application executable :

```
J-Link > loadfile <application_binary> (.elf format)
```

<a name="testing-the-example"></a>

## Testing the example

To test the example, please make sure to check the `Testing the example` section
from the common readme
[CHIP NXP Examples Guide for FreeRTOS platforms](./nxp_examples_freertos_platforms.md#testing-the-example).

### UART Ports

Testing the example with the CLI enabled will require connecting to both UART0
and UART1 serial ports. For applications where CLI is _not_ available,
connecting to the default UART1 is enough.

| Interface       | Role                                                                  |
| --------------- | --------------------------------------------------------------------- |
| UART0           | Used for logs only when `matter-cli` is enabled                       |
| UART1 (default) | Used either as `matter-cli` or for logs when `matter-cli` is disabled |

You need a `USB-UART` bridge to make use of the UART0 interface. The pin
configuration is the following:

-   `J5` `pin 3` - UART0 RX
-   `J5` `pin 4` - UART0 TX
-   `J5` `pin 8` - GND

### Device UI

The device state feedback is provided through the on-board LEDs:

| Widget  | Effect                              | Description                                                                                           |
| ------- | ----------------------------------- | ----------------------------------------------------------------------------------------------------- |
| LED2    | short flash on (50ms on/950ms off)  | The device is in an unprovisioned (unpaired) state and is waiting for a commissioner to connect.      |
| LED2    | rapid even flashing (100ms period)  | The device is in an unprovisioned state and a commissioner is connected via BLE.                      |
| LED2    | short flash off (950ms on/50ms off) | The device is fully provisioned, but does not yet have full network (Thread) or service connectivity. |
| LED2    | solid on                            | The device is fully provisioned and has full network and service connectivity.                        |
| RGB LED | on                                  | The `StateValue` attribute of the `BooleanState` cluster is `true` (simulating detection).            |
| RGB LED | off                                 | The `StateValue` attribute of the `BooleanState` cluster is `false` (simulating no detection).        |

NOTE: `LED2` will be disabled when OTA is used. On `FRDM-MCXW71` board, `PTB0`
is wired to both `LED2` and `CS` (Chip Select) of the External Flash Memory.
Since the OTA image is stored in external memory, `LED2` operations will affect
OTA operation by corrupting packages and OTA will not work.

The user can control the device using the on-board buttons. The actions are
summarized below:

| Button | Action       | State                                        | Output                                                                                                                                 |
| ------ | ------------ | -------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------- |
| SW2    | short press  | not commissioned                             | Enable/disable BLE advertising                                                                                                         |
| SW2    | short press  | commissioned + device is LIT                 | Enable Active Mode                                                                                                                     |
| SW2    | long press   | any                                          | Initiate a factory reset (can be cancelled by pressing the button again within the factory reset timeout limit - 6 seconds by default) |
| SW2    | double press | commissioned + device is LIT + supports DSLS | Enable / Disable SIT Mode                                                                                                              |
| SW4    | short press  | any                                          | Toggle attribute `StateValue` value                                                                                                    |
| SW4    | long press   | any                                          | Clean soft reset of the device (takes into account proper Matter shutdown procedure)                                                   |

<a name="ota-software-update"></a>

## OTA Software Update

Over-The-Air software updates are supported with the MCXW71 examples. The
process to follow in order to perform a software update is described in the
dedicated guide
['Matter Over-The-Air Software Update with NXP MCXW example applications'](./nxp_mcxw_ota_guide.md).
