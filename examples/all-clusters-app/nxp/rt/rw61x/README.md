# CHIP RW61x All-clusters Application

The all-clusters example implements a server which can be accessed by a CHIP
controller and can accept basic cluster commands.

The example is based on
[Project CHIP](https://github.com/project-chip/connectedhomeip) and the NXP
RW612 SDK, and provides a prototype application that demonstrates device
commissioning and different cluster control.

<hr>

-   [Introduction](#introduction)
-   [Building](#building)
-   [Flashing and debugging](#flashing-and-debugging)
-   [Testing the example](#testing-the-example)
-   [Matter Shell](#testing-the-all-clusters-application-with-matter-cli-enabled)
-   [OTA Software Update](#ota-software-update)

<hr>

<a name="introduction"></a>

## Introduction

The RW61x all-cluster application provides a working demonstration of the
RW610/RW612 board integration, built using the Project CHIP codebase and the NXP
RW612 SDK.

The example supports:

-   Matter over Wi-Fi
-   Matter over Openthread
-   Matter over Wi-Fi with OpenThread Border Router support.

### Hardware requirements

For Matter over Thread configuration :

-   [`NXP RD-RW612-BGA`] board
-   BLE/15.4 antenna (to plug in Ant1)

For Matter over WiFi configuration :

-   [`NXP RD-RW612-BGA`] or [`NXP RD-RW610-BGA`] board
-   BLE antenna (to plug in Ant1)
-   Wi-Fi antenna (to plug in Ant2)

For Matter over Wi-Fi with OpenThread Border Router :

-   [`NXP RD-RW612-BGA`] board
-   BLE/15.4 antenna (to plug in Ant1)
-   Wi-Fi antenna (to plug in Ant2)

<a name="building"></a>

## Building

In order to build the Project CHIP example, we recommend using a Linux
distribution (the demo-application was compiled on Ubuntu 20.04).

-   Follow instruction in [BUILDING.md](../../../../../docs/guides/BUILDING.md)
    to setup the environment to be able to build Matter.

-   Download
    [RD-RW612 SDK for Project CHIP](https://mcuxpresso.nxp.com/en/select).
    Creating an nxp.com account is required before being able to download the
    SDK. Once the account is created, login and follow the steps for downloading
    SDK. The SDK Builder UI selection should be similar with the one from the
    image below.

    ![MCUXpresso SDK Download](../../../../platform/nxp/rt/rw61x/doc/images/mcux-sdk-download.PNG)

    (Note: All SDK components should be selected. If size is an issue Azure RTOS
    component can be omitted.)

    Please refer to Matter release notes for getting the latest released SDK.

-   Start building the application.

```
user@ubuntu:~/Desktop/git/connectedhomeip$ export NXP_SDK_ROOT=/home/user/Desktop/SDK_RW612/
user@ubuntu:~/Desktop/git/connectedhomeip$ scripts/checkout_submodules.py --shallow --platform nxp --recursive
user@ubuntu:~/Desktop/git/connectedhomeip$ source ./scripts/bootstrap.sh
user@ubuntu:~/Desktop/git/connectedhomeip$ source ./scripts/activate.sh
user@ubuntu:~/Desktop/git/connectedhomeip$ cd examples/all-clusters-app/nxp/rt/rw61x/
```

#### Building with Matter over Wifi configuration on RW61x

-   Build Matter-over-Wifi configuration with BLE commissioning (ble-wifi) :

```
user@ubuntu:~/Desktop/git/connectedhomeip/examples/all-clusters-app/nxp/rt/rw61x$ gn gen --args="chip_enable_wifi=true is_sdk_package=true" out/debug
user@ubuntu:~/Desktop/git/connectedhomeip/examples/all-clusters-app/nxp/rt/rw61x$ ninja -C out/debug
```

#### Building with Matter over Thread configuration on RW612

-   Build Matter-over-Thread configuration with BLE commissioning.

```
user@ubuntu:~/Desktop/git/connectedhomeip/examples/all-clusters-app/nxp/rt/rw61x$ gn gen --args="chip_enable_openthread=true chip_inet_config_enable_ipv4=false chip_config_network_layer_ble=true is_sdk_package=true" out/debug
user@ubuntu:~/Desktop/git/connectedhomeip/examples/all-clusters-app/nxp/rt/rw61x$ ninja -C out/debug
```

#### Building with Matter over Wifi + OpenThread Border Router configuration on RW612

This configuration requires enabling the Matter CLI in order to control the
Thread network on the Border Router.

-   Build Matter with Border Router configuration with BLE commissioning
    (ble-wifi) :

```
user@ubuntu:~/Desktop/git/connectedhomeip/examples/all-clusters-app/nxp/rt/rw610$ gn gen --args="chip_enable_wifi=true chip_enable_openthread=true chip_enable_matter_cli=true is_sdk_package=true openthread_root=\"//third_party/connectedhomeip/third_party/openthread/ot-nxp/openthread-br\"" out/debug
user@ubuntu:~/Desktop/git/connectedhomeip/examples/all-clusters-app/nxp/rt/rw610$ ninja -C out/debug
```

#### General information

The resulting output file can be found in
out/debug/chip-rw61x-all-cluster-example.

Optional GN options that can be added when building an application:

-   To enable the
    [matter CLI](README.md#testing-the-all-clusters-application-with-matter-cli-enabled),
    the argument `chip_enable_matter_cli=true` must be added to the _gn gen_
    command.
-   To switch the SDK type used, the argument `is_<sdk_type>=true` must be added
    to the _gn gen_ command (with <sdk_type> being either sdk_package or
    sdk_internal).
-   By default, the RW612 A1 board revision will be chosen. To switch to an A2
    revision, the argument `board_version=\"A2\"` must be added to the _gn gen_
    command.
-   To build the application in debug mode, the argument
    `is_debug=true optimize_debug=false` must be added to the _gn gen_ command.
-   To build with the option to have Matter certificates/keys pre-loaded in a
    specific flash area the argument `chip_with_factory_data=1` must be added to
    the _gn gen_ command. (for more information see
    [Guide for writing manufacturing data on NXP devices](../../../../../docs/guides/nxp_manufacturing_flow.md).
-   To build the application with the OTA Requestor enabled, the arguments
    `chip_enable_ota_requestor=true no_mcuboot=false` must be added to the _gn
    gen_ command. (More information about the OTA Requestor feature in
    [OTA Requestor README](../../../../../docs/guides/nxp_rw61x_ota_software_update.md)

## Manufacturing data

See
[Guide for writing manufacturing data on NXP devices](../../../../../docs/guides/nxp_manufacturing_flow.md)

Other comments:

The all cluster app demonstrates the usage of encrypted Matter manufacturing
data storage. Matter manufacturing data should be encrypted using an AES 128
software key before flashing them to the device flash.

Using DAC private key secure usage: Experimental feature, contain some
limitation: potential concurrent access issue during sign with dac key operation
due to the lack of protection between multiple access to `ELS` crypto module.
The argument `chip_enable_secure_dac_private_key_storage=1` must be added to the
_gn gen_ command to enable secure private DAC key usage with S50.
`chip_with_factory_data=1` must have been added to the _gn gen_ command

DAC private key generation: The argument `chip_convert_dac_private_key=1` must
be added to the _gn gen_ command to enable DAC private plain key conversion to
blob with S50. `chip_enable_secure_dac_private_key_storage=1` must have been
added to the _gn gen_ command

`ELS` contain concurrent access risks. They must be fixed before enabling it by
default.

<a name="flashing-and-debugging"></a>

## Flashing and debugging

### Flashing the All-Clusters application

In order to flash the application we recommend using
[MCUXpresso IDE (version >= 11.6.0)](https://www.nxp.com/design/software/development-software/mcuxpresso-software-and-tools-/mcuxpresso-integrated-development-environment-ide:MCUXpresso-IDE).

-   Import the previously downloaded NXP SDK into MCUXpresso IDE.

Right click the empty space in the MCUXpresso IDE "Installed SDKs" tab to show
the menu, select the "Import archive" (or "Import folder" if a folder is used)
menu item.

-   Import the connectedhomeip repo in MCUXpresso IDE as Makefile Project. Use
    _none_ as _Toolchain for Indexer Settings_:

```
File -> Import -> C/C++ -> Existing Code as Makefile Project
```

-   Configure MCU Settings:

```
Right click on the Project -> Properties -> C/C++ Build -> MCU Settings -> Select RW612 -> Apply & Close
```

![MCU_Set](../../../../platform/nxp/rt/rw61x/doc/images/mcu-set.PNG)

-   Configure the toolchain editor:

```
Right click on the Project -> C/C++ Build-> Tool Chain Editor -> NXP MCU Tools -> Apply & Close
```

![toolchain](../../../../platform/nxp/rt/rw61x/doc/images/toolchain.JPG)

-   Create a debug configuration :

```
Right click on the Project -> Debug -> As->SEGGER JLink probes -> OK -> Select elf file
```

(Note : if SDK package is used, a simpler way could be duplicating the debug
configuration from the SDK Hello World example after importing it.)

-   Debug using the newly created configuration file.

<a name="testing-the-example"></a>

## Testing the example

CHIP Tool is a Matter controller which can be used to commission a Matter device
into the network. For more information regarding how to use the CHIP Tool
controller, please refer to the
[CHIP Tool guide](../../../../../docs/guides/chip_tool_guide.md).

To know how to commission a device over BLE, follow the instructions from
[chip-tool's README.md 'Commission a device over
BLE'][readme_ble_commissioning_section].

[readme_ble_commissioning_section]:
    ../../../../chip-tool/README.md#commission-a-device-over-ble

To know how to commissioning a device over IP, follow the instructions from
[chip-tool's README.md 'Pair a device over
IP'][readme_pair_ip_commissioning_section]

[readme_pair_ip_commissioning_section]:
    ../../../../chip-tool/README.md#pair-a-device-over-ip

#### Matter over wifi configuration :

The "ble-wifi" pairing method can be used in order to commission the device.

#### Matter over thread configuration :

The "ble-thread" pairing method can be used in order to commission the device.

#### Matter over wifi with openthread border router configuration :

In order to create or join a Thread network on the Matter Border Router, the
`otcli` commands from the matter CLI can be used. For more information about
using the matter shell, follow instructions from
['Testing the all-clusters application with Matter CLI'](#testing-the-all-clusters-application-with-matter-cli-enabled).

In this configuration, the device can be commissioned over Wi-Fi with the
'ble-wifi' pairing method.

### Testing the all-clusters application without Matter CLI:

1. Prepare the board with the flashed `All-cluster application` (as shown
   above).
2. The All-cluster example uses UART1 (`FlexComm3`) to print logs while running
   the server. To view raw UART output, start a terminal emulator like PuTTY and
   connect to the used COM port with the following UART settings:

    - Baud rate: 115200
    - 8 data bits
    - 1 stop bit
    - No parity
    - No flow control

3. Open a terminal connection on the board and watch the printed logs.

4. On the client side, start sending commands using the chip-tool application as
   it is described
   [here](../../../../chip-tool/README.md#using-the-client-to-send-matter-commands).

<a name="testing-the-all-clusters-application-with-matter-cli-enabled"></a>

### Testing the all-clusters application with Matter CLI enabled:

The Matter CLI can be enabled with the all-clusters application.

For more information about the Matter CLI default commands, you can refer to the
dedicated [ReadMe](../../../../shell/README.md).

The All-clusters application supports additional commands :

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

Here are described steps to use the all-cluster-app with the Matter CLI enabled

1. Prepare the board with the flashed `All-cluster application` (as shown
   above).
2. The matter CLI is accessible in UART1. For that, start a terminal emulator
   like PuTTY and connect to the used COM port with the following UART settings:

    - Baud rate: 115200
    - 8 data bits
    - 1 stop bit
    - No parity
    - No flow control

3. The All-cluster example uses UART2 (`FlexComm0`) to print logs while running
   the server. To view raw UART output, a pin should be plugged to an USB to
   UART adapter (connector `HD2 pin 03`), then start a terminal emulator like
   PuTTY and connect to the used COM port with the following UART settings:

    - Baud rate: 115200
    - 8 data bits
    - 1 stop bit
    - No parity
    - No flow control

4. On the client side, start sending commands using the chip-tool application as
   it is described
   [here](../../../../chip-tool/README.md#using-the-client-to-send-matter-commands).

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

<a name="ota-software-update"></a>

## OTA Software Update

Over-The-Air software updates are supported with the RW61x all-clusters example.
The process to follow in order to perform a software update is described in the
dedicated guide
['Matter Over-The-Air Software Update with NXP RW61x example applications'](../../../../../docs/guides/nxp_rw61x_ota_software_update.md).
