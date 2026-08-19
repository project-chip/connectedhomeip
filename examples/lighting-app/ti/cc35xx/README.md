# Matter Lighting Example Application

An example application showing the use of [Matter][matter] on the Texas
Instruments CC35XX family of Wireless MCUs.

This is an early, experimental release of Matter on the TI CC35xx platform. 

Features enabled in this release:
- Matter Lighting Application
- Ability to connect to AP (hardcoded credentials only)
    - Connectivity with AP is assumed to be stable
- Work based on Matter v1.5 specification

Features not currently enabled in this release:
- Support for unstable AP connections
- Persistent storage
- Over the Air (OTA) Update Support
- Intermittently Connected Devices (ICD) Support
- BLE based commissioning onto a Wi-Fi network

Limited testing has been performed. Matter Commissioning (onnetwork) and
the On/Off commands in the On/Off Cluster have been tested. Test coverage will
be improved for future releases.
---

-   [Matter Lighting Example Application](#matter-lighting-example-application)
    -   [Introduction](#introduction)
        -   [Device UI](#device-ui)
    -   [Building](#building)
        -   [Preparation](#preparation)
        -   [Compilation](#compilation)
    -   [Programming](#programming)
        -   [Code Composer Studio](#code-composer-studio)
        -   [Wi-Fi Toolbox](#wi-fi-toolbox)
    -   [Viewing Logging Output](#viewing-logging-output)
    -   [Running the Example](#running-the-example)
    -   [TI Support](#ti-support)

---

## Introduction

The CC35XX lighting example application provides a working demonstration of
a connected light device. This uses the open-source Matter implementation and
the Texas Instruments SimpleLink™ CC35XX software development kit.

This example is enabled to build for CC35XX devices.

The lighting example is intended to serve both as a means to explore the
workings of Matter, as well as a template for creating real products based on
the Texas Instruments devices.

## Device UI

| Action                                           | Functionality                          |
| ------------------------------------------------ | -------------------------------------- |
| Left Button (`BTN-1`) Press (less than 1000 ms)  | Turn the light on                      |
| Right Button (`BTN-2`) Press (less than 1000 ms) | Turn the light off                     |
| Red LED Solid On State                           | Light is turned on                     |
| Red LED Off State                                | Light is turned off                    |
| Green LED Blinking State                         | Identify Trigger Effect in progress    |
| Green LED Off State                              | No Identify Trigger Effect in progress |

## Prerequisites

**Hardware:**
- LP-EM-CC35X1 Rev A LaunchPad

**Software:**
- SimpleLink Wi-Fi SDK `10.10.00.18`
- SysConfig `1.26.3`
- SimpleLink Wi-Fi Toolbox `4.1.16`

> **Note:** SysConfig and SimpleLink Wi-Fi Toolbox are installed by the SimpleLink Wi-Fi SDK installer and do not need to be installed separately.

## Building

### Preparation

Some initial setup is necessary for preparing the build environment. This
section will need to be done when migrating to new versions of the SDK. This
guide assumes that the environment is linux based, and recommends Ubuntu 22.04.

> **Note:** Before building, complete the environment setup steps in the
> [CC35XX Matter Getting Started Guide][getting-started], including cloning the
> repository, bootstrapping, and copying the SDK into the Matter tree.

-   Download and install the [SimpleLink Wi-Fi SDK][simplelink-wifi-sdk] (`10.10.00.18`).
    SysConfig and the SimpleLink Wi-Fi Toolbox are included in the SDK installer.

    > **Note:** This example has been validated with SimpleLink Wi-Fi SDK `10.10.00.18`.

### Compilation

It is necessary to activate the environment in every new shell. Then run GN and
Ninja to build the executable.

-   Activate the build environment with the repository activate script.

    ```
    $ cd {matter-root}
    $ source ./scripts/activate.sh

    ```

-   Run the build to produce a default executable. By default on Linux both the
    TI SimpleLink SDK and Sysconfig are located in a `ti` folder in the user's
    home directory, and you must provide the absolute path to them. For example
    `/home/username/ti/sysconfig_1.26.3`. On Windows the default directory is
    `C:\ti`. Take note of this install path, as it will be used in the next
    step.

    ```
    $ cd {matter-root}/examples/lighting-app/ti/cc35xx
    $ gn gen out/debug --args="ti_sysconfig_root=\"$HOME/ti/sysconfig_1.26.3\" ti_simplelink_wifi_toolbox_root=\"$HOME/ti/simplelink_wifi_toolbox_lin_4_1_16\" ti_simplelink_wifi_sdk_root=\"$HOME/ti/simplelink_wifi_sdk_10_10_00_18\""
    $ ninja -C out/debug

    ```

    If you would like to define arguments on the command line you may add them
    to the GN call.

    ```
    gn gen out/debug --args="ti_sysconfig_root=\"$HOME/ti/sysconfig_1.26.3\" target_defines=[\"TI_ATTESTATION_CREDENTIALS=1\"] chip_generate_link_map_file=true ti_simplelink_wifi_toolbox_root=\"$HOME/ti/simplelink_wifi_toolbox_lin_4_1_16\" ti_simplelink_wifi_sdk_root=\"$HOME/ti/simplelink_wifi_sdk_10_10_00_18\""
    ```

    **Note:** `build_cc35xx_lighting_app.sh` can be used for building this example. Modify the paths for syscfg and others in the script if the default is not valid for your setup.

## Programming

Loading the built image onto a LaunchPad is supported via the SimpleLink Wi-Fi Toolbox. To program the Matter image using the toolbox, run the below command with the appropriate path to the `tool_settings.json` file in your setup:

```
cd {simplelink-wifi-toolbox-install-dir}
sudo ./simplelink-wifi-toolbox programmer -i XDS110 -param1 auto programming --tool_settings {matter-root}/examples/lighting-app/ti/cc35xx/out/debug/tool_settings.json --verbose
```

For more details on programming with the Wi-Fi Toolbox, refer to the [Programmer User Guide][programmer_user_guide].

### Code Composer Studio

CCS will allow for a full debug environment within the IDE.
This is accomplished by creating a target connection to the XDS110 debugger and
starting a project-less debug session. The CCS IDE will attempt to find the
source files on the local machine based on the debug information embedded within
the ELF. CCS may prompt you to find the source code if the image was built on
another machine or the source code is located in a different location than is
recorded within the ELF.

Download and install [Code Composer Studio][ccs].

First open CCS and create a new workspace.

Create a target connection (sometimes called the CCXML) for your target SoC and
debugger as described in the [Manual Method][ccs_manual_method] section of the
CCS User's Guide.

Next initiate a project-less debug session as described in the [Manual
Launch][ccs_manual_launch] section of the CCS User's Guide.

CCS should switch to the debug view described in the [After
Launch][ccs_after_launch] section of the User's Guide. The SoC core will likely
be disconnected and symbols will not be loaded. Connect to the core as described
in the [Debug View][ccs_debug_view] section of the User's Guide. Once the core
is connected, use the `Load` button, and `Load Symbols` option in the drop-down 
to load the ELF image.

Note that the default configuration of the CCXML uses 2-wire cJTAG instead of
the full 4-wire JTAG connection to match the default jumper configuration of the
LaunchPad.

### Wi-Fi Toolbox

Please refer to the QuickStart Guide on how to flash the CC35XX device.

## Viewing Logging Output

By default the log output will be sent to the Application/User UART. Open a
terminal emulator to that port to see the output with the following options:

| Parameter    | Value    |
| ------------ | -------- |
| Speed (baud) | `115200` |
| Data bits    | `8`      |
| Stop bits    | `1`      |
| Parity       | `None`   |
| Flow control | `None`   |

## Running the Example

### Provisioning

The first step to bring the Matter device onto the network is to provision it.

The SSID, Password, and WPA Security Type of the AP are listed as defines `AP_SSID`, `AP_PASSWORD`, and `WLAN_SEC_TYPE` at the top of `src/platform/ti/cc35xx/ConnectivityManagerImpl.cpp`. Please put in your AP credentials here. Upon device reset, the launchpad will attempt to connect to the AP specified. 

Once the device is connected to the local AP, commissioning can be triggered using "OnNetwork" configuration.

#### Bluetooth LE Provisioning

BLE provisioning is not supported currently.

### Commissioning

Once a device has been flashed with this example, it can now join and operate in
an existing Matter network. The following sections assume that a Matter network
is already active.

For insight into what other components are needed to run this example, please
refer to our [Matter Getting Started Guide][getting-started].

The steps below should be followed to commission the lighting device onto the
network and control it once it has been commissioned.

**Step 0**

Set up the CHIP tool by following the instructions outlined in the [CHIP Tool Guide][chip-tool-guide].

**Step 1**

Commission the light device onto the Matter network. Run the following command
on the CHIP tool:

```

./chip-tool pairing onnetwork <nodeID - e.g. 1> 20202021

```

**Note:** nodeID parameter can be set to a desired value and the same value should be used in subsequent commands

Once the device has been successfully commissioned, you will see the following
message on the CHIP tool output:

```

[1677648218.370754][39785:39790] CHIP:CTL: Received CommissioningComplete response, errorCode=0
[1677648218.370821][39785:39790] CHIP:CTL: Successfully finished commissioning step 'SendComplete'

```

An accompanying message will be seen from the device:

```

Commissioning completed successfully

```

**Step 2**

Send commands to the lighting app. Here are some example commands:

On

```
./chip-tool onoff on <nodeID> 1
./chip-tool onoff toggle <nodeID> 1 (assuming the light is off)

```

Off

```
./chip-tool onoff off <nodeID> 1
./chip-tool onoff toggle <nodeID> 1 (assuming the light is on)

```

Identify

```
./chip-tool identify identify <IdentifyTime> <nodeID> 1   e.g.  ./chip-tool identify identify 100 1 1

./chip-tool identify trigger-effect <EffectIdentifier> <EffectVariant> <nodeID> <endpointID>  e.g.  ./chip-tool identify trigger-effect 0x0000 0 1 1

```

## TI Support

For technical support, please consider creating a post on TI's [E2E forum][e2e].
Additionally, we welcome any feedback.

[matter]: https://csa-iot.org/all-solutions/matter/
[simplelink-wifi-sdk]: https://www.ti.com/tool/SIMPLELINK-WIFI-SDK
[ccs]: https://www.ti.com/tool/CCSTUDIO
[ccs_after_launch]:
    https://software-dl.ti.com/ccs/esd/documents/users_guide/ccs_debug-main.html?configuration#after-launch
[ccs_debug_view]:
    https://software-dl.ti.com/ccs/esd/documents/users_guide/ccs_debug-main.html?configuration#debug-view
[ccs_manual_launch]:
    https://software-dl.ti.com/ccs/esd/documents/users_guide/ccs_debug-main.html?configuration#manual-launch
[ccs_manual_method]:
    https://software-dl.ti.com/ccs/esd/documents/users_guide/ccs_debug-main.html?configuration#manual-method
[e2e]:
    https://e2e.ti.com/support/wireless-connectivity/wi-fi-group/wifi/f/wi-fi-forum
[getting-started]:
    ../../../../docs/platforms/ti/cc35xx_matter_v1.5_getting_started.md
[chip-tool-guide]:
    ../../../../docs/guides/chip_tool_guide.md
[programmer_user_guide]:
    https://software-dl.ti.com/simplelink/esd/simplelink_wifi_sdk/10.10.00.18/exports/docs/WiFi-toolbox/html/WiFi-toolbox/programmer_user_guide.html
