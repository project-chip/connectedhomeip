# Matter NXP Examples Guide for FreeRTOS platforms

-   [Introduction](#introduction)
-   [Building](#building)
-   [Flashing and debugging](#flashing-and-debugging)
-   [Testing the example](#testing-the-example)
-   [Matter Shell](#testing-the-example-application-with-matter-cli-enabled)

## Introduction

This guide provides step-by-step instructions for working with CHIP applications
on NXP platforms that are FreeRTOS-based. It is intended to help users set up
their development environment, build the application, and test it effectively.

The NXP examples are based on
[Project CHIP](https://github.com/project-chip/connectedhomeip) and
[NXP MCUX SDK](https://mcuxpresso.nxp.com/mcuxsdk/latest/html/introduction/README.html),
and provide a prototype application that demonstrates device commissioning and
different cluster control.

### Supported platforms

Matter example applications are supported on the following NXP platforms :

| NXP platform | Dedicated readme                                              |
| ------------ | ------------------------------------------------------------- |
| RW61x        | [Matter NXP Guide for RW61x platform](./nxp_rw61x_guide.md)   |
| RT1170       | [Matter NXP Guide for RT1170 platform](./nxp_rt1170_guide.md) |
| RT1060       | [Matter NXP Guide for RT1060 platform](./nxp_rt1060_guide.md) |

## Building

In order to build the Project CHIP example, we recommend using a Linux
distribution (supported Operating Systems are listed in
[BUILDING.md](../../guides/BUILDING.md#prerequisites)).

-   Make sure that below prerequisites are correctly installed (as described in
    [BUILDING.md](../../guides/BUILDING.md#prerequisites)).

```
sudo apt-get install git gcc g++ pkg-config libssl-dev libdbus-1-dev \
     libglib2.0-dev libavahi-client-dev ninja-build python3-venv python3-dev \
     python3-pip unzip libgirepository1.0-dev libcairo2-dev libreadline-dev
```

### Set-up the build environment

NXP MCUX SDK provides a build and configuration system based on CMake and
Kconfig. For more information, please visit
[NXP MCUXPresso SDK Build And Configuration documentation](https://mcuxpresso.nxp.com/mcuxsdk/latest/html/develop/build_system/index.html).

Follow these steps to set-up your environment to build Matter application with
NXP MCUX SDK.

-   Step 1 : checkout NXP specific submodules only

```
user@ubuntu:~/Desktop/git/connectedhomeip$ scripts/checkout_submodules.py --shallow --platform nxp --recursive
```

-   Step 2 : Activate Matter environment :

If you are setting the Matter environment for the first time, or your local
environment is out of date, run the following script :

```
user@ubuntu:~/Desktop/git/connectedhomeip$ source scripts/bootstrap.sh -p all,nxp
```

Otherwise, you can run the following script to activate your environment :

```
user@ubuntu:~/Desktop/git/connectedhomeip$ source scripts/activate.sh
```

-   Step 3 : Install NXP MCUX SDK

```
user@ubuntu:~/Desktop/git$ third_party/nxp/nxp_matter_support/scripts/update_nxp_sdk.py --platform common
```

-   Step 4 : Source mcux-env.sh

```
user@ubuntu:~/Desktop/git/connectedhomeip$ source <path to mcux sdk>/mcux-env.sh
```

-   Step 5 : Export the `ARMGCC_DIR` environment variable (only applicable to
    CMake build)

The CMake build system requires the `ARMGCC_DIR` environment variable to point
to the root directory of the ARM GCC toolchain. You can either locate your
toolchain or use the one installed by Matter (during environment setup), and
export the path as follows :

```
user@ubuntu:~/Desktop/git/connectedhomeip$ export ARMGCC_DIR=/path/to/connectedhomeip/.environment/cipd/packages/arm
```

> Note : Steps 1 to 3 can be skipped if your environment is up to date. Steps 4
> and 5 should be repeated in each new terminal session to ensure the
> environment is correctly configured.

### Build the application

CHIP NXP examples support building with two different build systems :

-   `GN` : The GN build system is used to configure and build the entire
    project.
-   `CMake` : CMake build system is used to build the application and the NXP
    SDK, while the Matter stack is compiled with GN and linked to the
    application during the CMake build process.

> Please refer to the platform [dedicated readme](#supported-platforms) to check
> which build system(s) is supported by the platform.

#### CMake build system

The example application can be built with `west build` command, which can be
structured as follow :

```
user@ubuntu:~/Desktop/git/connectedhomeip$ west build -d <build_dir> -b <board> examples/<example_name>/nxp -DCONF_FILE_NAME=<prj_flavour.conf>
```

-   `build_dir` : Directory where the build output is stored.
-   `board` : Name of the board targeted for the build.
-   `examples/<example_name>/nxp` : Path to the application entry
    CMakeLists.txt. For example, to build the Thermostat application, the path
    `examples/thermostat/nxp` can be used.
-   `CONF_FILE_NAME` : Variable to provide the project configuration file name
    available under `examples/platform/nxp/config`. If this variable is provided
    in the build command, the build system will use
    `examples/platform/nxp/config/${CONF_FILE_NAME}`. (See table below for more
    information about the available project configuration files and their
    platform compatibility.)

Additional arguments that can be passed in the `west build` command line :

-   `--config release` : This parameter can be added to build in release mode.
    The default build type is `debug`.
-   `-DCONF_FILE` : This variable can be used to provide a custom project
    configuration file. When used, the specified configuration files will be
    merged and used as the application configuration. Note that this variable
    must be provided with an absolute path of the custom prj.conf.
-   `-Dcore_id` : If the board is multi-core, this parameter can be used to
    specify the core ID targeted for the build.
-   `-DCHIP_ROOT` : For an out-of-tree application, this variable can be used to
    provide the path of the connectedhomeip repository root. For in-tree
    applications, this parameter is not required, as the path is set by default.

> For additional information about platform-specific build options and
> configuration supported, please refer to the
> [dedicated readme](#supported-platforms) for the platform you are targeting.

##### Available project configuration files and platform compatibility

| Configuration File                         | Description                                            | Supported platforms   |
| ------------------------------------------ | ------------------------------------------------------ | --------------------- |
| `prj_wifi.conf`                            | Wi-Fi                                                  | RW61x, RT1060, RT1170 |
| `prj_wifi_fdata.conf`                      | Wi-Fi, factory data                                    | RW61x, RT1060, RT1170 |
| `prj_wifi_ota_fdata.conf`                  | Wi-Fi, factory data, OTA                               | RW61x, RT1060, RT1170 |
| `prj_wifi_ota_fdata_v2.conf`               | Wi-Fi, factory data, OTA, with SW v2                   | RW61x, RT1060, RT1170 |
| `prj_wifi_ota.conf`                        | Wi-Fi, OTA                                             | RW61x, RT1060, RT1170 |
| `prj_wifi_ota_v2.conf`                     | Wi-Fi, OTA, SW v2                                      | RW61x, RT1060, RT1170 |
| `prj_wifi_onnetwork.conf`                  | Wi-Fi onnetwork without BLE                            | RW61x, RT1060, RT1170 |
| `prj_thread_ftd.conf`                      | Thread FTD                                             | RW61x, RT1060, RT1170 |
| `prj_thread_ftd_ota.conf`                  | Thread FTD, OTA                                        | RW61x, RT1060, RT1170 |
| `prj_thread_ftd_ota_fdata.conf`            | Thread FTD, OTA, factory data                          | RW61x, RT1060, RT1170 |
| `prj_thread_ftd_wifi_br_ota.conf`          | Wi-Fi + Thread Border Router, OTA                      | RW61x, RT1060, RT1170 |
| `prj_thread_ftd_wifi_br_ota_fdata.conf`    | Wi-Fi + Thread Border Router, OTA, factory data        | RW61x, RT1060, RT1170 |
| `prj_thread_ftd_wifi_br_ota_fdata_v2.conf` | Wi-Fi + Thread Border Router, OTA SW v2 , factory data | RW61x, RT1060, RT1170 |
| `prj_thread_ftd_wifi_br_ota_v2.conf`       | Wi-Fi + Thread Border Router, OTA with SW v2           | RW61x, RT1060, RT1170 |
| `prj_eth.conf`                             | Ethernet                                               | RW61x `frdm`          |
| `prj_eth_fdata.conf`                       | Ethernet, factory data                                 | RW61x `frdm`          |
| `prj_eth_ota.conf`                         | Ethernet, OTA                                          | RW61x `frdm`          |
| `prj_eth_ota_fdata.conf`                   | Ethernet, factory data, OTA                            | RW61x `frdm`          |
| `prj_eth_ota_fdata_v2.conf`                | Ethernet, factory data, OTA, SW v2                     | RW61x `frdm`          |

The build output can be found under the `build_dir` specified in the
`west build` command, the binary can be found under the following name :
`build_dir/app.elf`.

##### How to customize the CMake build

If you would like to further configure your build, you can either edit the
application `prj.conf`, or provide your custom prj.conf in the build command
line using the `-DCONF_FILE`.

If needed, you can provide multiple custom prj.conf like follow :

```
user@ubuntu:~/Desktop/git/connectedhomeip$ west build -d <build_directory> -b <board> <path/to/examples> -DCONF_FILE="/path/to/prj_<custom1>.conf;/path/to/prj_<custom2>.conf;/path/to/prj_<custom3>.conf"
```

It is also supported to provide Kconfig symbols in the build command line with
`-DCONFIG_<symbol>=<value>`. For example, adding to the build command line
`-DCONFIG_CHIP_DEVICE_DISCRIMINATOR=3841` will set the discriminator Kconfig
`CONFIG_CHIP_DEVICE_DISCRIMINATOR` to `3841`.

#### GN build system

NXP CHIP examples support to build with GN build system. The configuration of
the application can be done by providing specific GN arguments in the build
command-line.

> The application configuration with GN can vary by platform, please make sure
> to follow the guidelines from the [dedicated readme](#supported-platforms) of
> the platform you are targeting.

##### General information

Here below is provided a list of supported GN options that can be added to the
_gn gen_ command when building an application.

| GN argument                                                                               | Description                                                                                                                                                                                                                                                                                                                                              |
| ----------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `chip_enable_wifi=true`                                                                   | Enable Wi-Fi in the application.                                                                                                                                                                                                                                                                                                                         |
| `chip_enable_openthread=true`                                                             | Enable OpenThread in the application.                                                                                                                                                                                                                                                                                                                    |
| `chip_enable_ble=false`                                                                   | This argument is used to disable BLE in the application. Note that BLE is enabled by default.                                                                                                                                                                                                                                                            |
| `chip_enable_secondary_nwk_if=true` and `chip_device_config_thread_network_endpoint_id=2` | These arguments can be added to enable the [secondary network commissioning interface](./nxp_otbr_guide.md#using-the-secondary-network-commissioning-interface). Note that this is only supported when building the Matter over Wifi + OpenThread Border Router configuration. Note that is only supported on the on the thermostat application for now. |
| `nxp_enable_matter_cli=true`                                                              | This argument enables the [Matter-CLI](./nxp_examples_freertos_platforms.md#testing-the-example-application-with-matter-cli-enabled).                                                                                                                                                                                                                    |
| `is_debug=true optimize_debug=false`                                                      | These arguments are used to build the application in debug mode.                                                                                                                                                                                                                                                                                         |
| `nxp_use_factory_data=true`                                                               | This argument can be used to build with the option to have Matter certificates/keys pre-loaded in a specific flash area. For more information see [Guide for writing manufacturing data on NXP devices](./nxp_manufacturing_flow.md).                                                                                                                    |
| `chip_enable_ota_requestor=true`                                                          | This argument can be used to enable the OTA Requestor feature. For RT1170, RT1060 nd RW61x, make sure to also add the argument `no_mcuboot=false` when enabling the OTA.                                                                                                                                                                                 |

> For more information about platform-specific GN options, please refer to the
> platform [dedicated readme](#supported-platforms).

## Flashing and debugging

For flashing and debugging the example application, follow detailed instructions
form the [dedicated readme](#supported-platforms) to the platform you are
targeting.

## Testing the example

CHIP Tool is a Matter controller which can be used to commission a Matter device
into the network. For more information regarding how to use the CHIP Tool
controller, please refer to the
[CHIP Tool guide](../../development_controllers/chip-tool/chip_tool_guide.md).

To know how to commission a device over BLE, follow the instructions from
[chip-tool's Readme 'Commissioning over Bluetooth LE'](../../development_controllers/chip-tool/chip_tool_guide.md#commissioning-over-bluetooth-le).

To know how to commissioning a device over IP, follow the instructions from
[chip-tool's Readme 'Commissioning into a network over IP'](../../development_controllers/chip-tool/chip_tool_guide.md#commissioning-into-a-network-over-ip)

#### Matter over wifi configuration :

The "ble-wifi" pairing method can be used in order to commission the device.

#### Matter over thread configuration :

The "ble-thread" pairing method can be used in order to commission the device.

#### Matter over wifi with openthread border router configuration :

In order to create or join a Thread network on the Matter Border Router, the TBR
management cluster or the `otcli` commands from the matter CLI can be used. For
more information about using the TBR management cluster follow instructions from
['Using the TBR management cluster'](../../platforms/nxp/nxp_otbr_guide.md#using-the-thread-border-router-management-cluster).
For more information about using the matter shell, follow instructions from
['Testing the all-clusters application with Matter CLI'](#testing-the-example-application-with-matter-cli-enabled).

In this configuration, the device can be commissioned over Wi-Fi with the
'ble-wifi' pairing method.

> For more information about supported configurations on the device you are
> testing, please refer to its [dedicated readme](#supported-platforms).

### Testing the example application without Matter CLI:

1. Prepare the board with the flashed example application.
2. The All-cluster example uses UART1 to print logs while running the server. To
   view raw UART output, start a terminal emulator like PuTTY and connect to the
   used COM port with the following UART settings:

    - Baud rate: 115200
    - 8 data bits
    - 1 stop bit
    - No parity
    - No flow control

3. Open a terminal connection on the board and watch the printed logs.

4. On the client side, start sending commands using the chip-tool application as
   it is described
   [here](../../development_controllers/chip-tool/chip_tool_guide.md#step-5-test-reception-of-commands).

<a name="testing-the-all-clusters-application-with-matter-cli-enabled"></a>

### Testing the example application with Matter CLI enabled:

The Matter CLI can be enabled with the NXP CHIP application.

For more information about the Matter CLI default commands, you can refer to the
dedicated [ReadMe](../../../examples/shell/README.md).

The NXP application supports additional commands :

```
> help
[...]
mattercommissioning     Open/close the commissioning window. Usage : mattercommissioning [on|off]
matterfactoryreset      Perform a factory reset on the device
matterreset             Reset the device
```

-   `matterfactoryreset` command erases the file system completely (all Matter
    settings are erased).
-   `matterreset` enables the device to reboot without erasing the settings.

To test the application with the CLI, you will need to connect to UART1 and
UART2 of your device. These `UART` are used for :

-   UART1 : Primary communication interface where the commands can be input.
-   UART2 : Secondary communication interface where Matter logs will be printed.

> _Important_ : The exact names, hardware connections, and pin configurations
> for UART1 and UART2 vary by platform. Please refer to the relevant
> platform-specific readme for detailed information.

1. Prepare the board with the flashed `All-cluster application` (as shown
   above).
2. The matter CLI is accessible in UART1. For that, start a terminal emulator
   like PuTTY and connect to the used COM port with the following UART settings:

    - Baud rate: 115200
    - 8 data bits
    - 1 stop bit
    - No parity
    - No flow control

3. The All-cluster example uses UART2 to print logs while running the server. To
   view these logs, start a terminal emulator like PuTTY and connect to the used
   COM port with the following UART settings:

    - Baud rate: 115200
    - 8 data bits
    - 1 stop bit
    - No parity
    - No flow control

4. On the client side, start sending commands using the chip-tool application as
   it is described
   [here](../../development_controllers/chip-tool/chip_tool_guide.md#step-5-test-reception-of-commands).

For Matter with OpenThread Border Router support, the matter CLI can be used to
start/join the Thread network, using the following ot-cli commands. (Note that
setting channel, panid, and network key is not enough anymore because of an Open
Thread stack update. We first need to initialize a new dataset.)

```
> otcli dataset init new
Done
> otcli dataset
Active Timestamp: 1
Channel: 25
Channel Mask: 0x07fff800
Ext PAN ID: 42af793f623aab54
Mesh Local Prefix: fd6e:c358:7078:5a8d::/64
Network Key: f824658f79d8ca033fbb85ecc3ca91cc
Network Name: OpenThread-b870
PAN ID: 0xb870
PSKc: f438a194a5e968cc43cc4b3a6f560ca4
Security Policy: 672 onrc 0
Done
> otcli dataset panid 0xabcd
Done
> otcli dataset channel 25
Done
> otcli dataset commit active
Done
> otcli ifconfig up
Done
> otcli thread start
Done
> otcli state
leader
Done
```
