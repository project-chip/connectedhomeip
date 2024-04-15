# CHIP RT1170 All-clusters Application

The all-clusters example implements a server which can be accesed by a CHIP
controller and can accept basic cluster commands.

The example is based on
[Project CHIP](https://github.com/project-chip/connectedhomeip) and the NXP
RT1170 SDK, and provides a prototype application that demonstrates device
commissioning and different cluster control.

<hr>

- [CHIP RT1170 All-clusters Application](#chip-rt1170-all-clusters-application)
  - [Introduction](#introduction)
    - [Configuration(s) supported](#configurations-supported)
    - [Hardware requirements for RT1170 + IWX12](#hardware-requirements-for-rt1170--iwx12)
      - [Hardware rework for SPI support on EVKB-MIMXRT1170](#hardware-rework-for-spi-support-on-evkb-mimxrt1170)
      - [Board settings (Spinel over SPI, I2C, BLE over UART)](#board-settings-spinel-over-spi-i2c-ble-over-uart)
  - [Building](#building)
    - [Building with Matter over Wifi configuration on RT1170 + IWX12](#building-with-matter-over-wifi-configuration-on-rt1170--iwx12)
    - [Building with Matter over Thread configuration on RT1170 + IWX12](#building-with-matter-over-thread-configuration-on-rt1170--iwx12)
    - [Building with Matter over Wifi + OpenThread Border Router configuration on RT1170 + IWX12](#building-with-matter-over-wifi--openthread-border-router-configuration-on-rt1170--iwx12)
    - [General information](#general-information)
  - [Manufacturing data](#manufacturing-data)
  - [Flashing and debugging](#flashing-and-debugging)
  - [Testing the example](#testing-the-example)
      - [Matter over wifi configuration :](#matter-over-wifi-configuration-)
      - [Matter over thread configuration :](#matter-over-thread-configuration-)
      - [Matter over wifi with openthread border router configuration :](#matter-over-wifi-with-openthread-border-router-configuration-)
    - [Testing the all-clusters application without Matter CLI:](#testing-the-all-clusters-application-without-matter-cli)
    - [Testing the all-clusters application with Matter CLI enabled:](#testing-the-all-clusters-application-with-matter-cli-enabled)

<hr>

<a name="introduction"></a>

## Introduction

![RT1170 EVK](../../../../platform/nxp/rt/rt1170/doc/images/IMX-RT1170-EVK-TOP.jpg)

The RT1170 all-cluster application provides a working demonstration of the
RT1170 board integration, built using the Project CHIP codebase and the NXP
RT1170 SDK.

The example supports:

-   Matter over Wi-Fi
-   Matter over Openthread
-   Matter over Wi-Fi with Openthread Border Router support

The example targets the
[NXP MIMXRT1170-EVKB](https://www.nxp.com/part/MIMXRT1170-EVKB#/) board by
default.

### Configuration(s) supported

Here are listed configurations that allow to support Matter over Wi-Fi & Matter
over Thread on RT1170 :

-   RT1170 + IWX12 (Wi-Fi + BLE + 15.4)

<a name="hardware-requirements-for-rt1170-and-iwx12"></a>

### Hardware requirements for RT1170 + IWX12

Host part:

-   1 EVKB-MIMXRT1170

Transceiver part :

-   1
    [2EL M2 A1 IW612 Secure Module](https://www.nxp.com/products/wireless/wi-fi-plus-bluetooth-plus-802-15-4/2-4-5-ghz-dual-band-1x1-wi-fi-6-802-11ax-plus-bluetooth-5-2-plus-802-15-4-tri-radio-solution:IW612)

![](../../../../platform/nxp/rt/rt1170/doc/images/iwx612_2EL.jpg)

-   1
    [Murata uSD to M2 adapter revC](https://www.murata.com/en-eu/products/connectivitymodule/wi-fi-bluetooth/overview/lineup/usd-m2-adapter-2we-2wf)

![](../../../../platform/nxp/rt/rt1170/doc/images/murata_usd-M2_adapter.jpg)

-   Male to female Burg cables

#### Hardware rework for SPI support on EVKB-MIMXRT1170

To support SPI on the EVKB-MIMXRT1170 board, it is required to remove 0Ω
resistors R404,R406,R2015.

#### Board settings (Spinel over SPI, I2C, BLE over UART)

Plug IW612 Firecrest module to M.2 connector on Murata uSD to M2 adapter

The murata uSD-M2 adapter should be plugged to the RT1170 via SDIO.

The below tables explain pin settings (SPI settings) to connect the
evkbmimxrt1170 (host) to a IWX12 transceiver (rcp).

-   Murata uSD to M2 adapter connections description:

![](../../../../platform/nxp/rt/rt1170/doc/images/murata_usd-m2_connections_1.jpg)

![](../../../../platform/nxp/rt/rt1170/doc/images/murata_usd-m2_connections_2.jpg)

-   Jumpers positions on Murata uSD to M2 adapter:

    Use USB-C power supply | Jumper | Position| | :----: | :-----: | | J1 | 1-2
    | | J12 | 1-2 | | J13 | 1-2 | | J14 | 1-2 | | JP1.1 (back side)| ON |

-   Jumpers positions on MIMXRT1170-EVKB:

    | Jumper | Position |
    | :----: | :------: |
    |  J56   |   2-3    |

-   I2C connection to program IO-Expander on the IW612 module

    | MIMXRT1170-EVKB  | uSD-M2 adapter |
    | :--------------: | :------------: |
    | I2C_SDA (J10.18) |      J5.2      |
    | I2C_SDL (J10.20) |      J5.4      |

-   SPI connection between RT1170 and uSD-M2 adapter

    |  MIMXRT1170-EVKB  | uSD-M2 adapter |
    | :---------------: | :------------: |
    | SPI_MOSI (J10.8)  |     J5.10      |
    | SPI_MISO (J10.10) |      J9.7      |
    | SPI_CLK (J10.12)  |      J9.8      |
    |  SPI_CS (J10.6)   |      J5.8      |
    |  SPI_INT (J26.4)  |      J5.6      |
    |   GND (J10.14)    |     J5.15      |

-   UART BLE and Reset connections between RT1170 and uSD-M2 adapter

    |  MIMXRT1170-EVKB  | uSD-M2 adapter |
    | :---------------: | :------------: |
    |   RESET (J26.2)   |      J9.3      |
    | UART RXD (J25.13) |      J9.1      |
    | UART TXD (J25.15) |      J9.2      |
    | UART CTS (J25.9)  |      J8.4      |
    | UART RTS (J25.11) |      J8.3      |
    |    GND (J26.1)    |      J7.6      |

<a name="building"></a>

## Building

In order to build the Project CHIP example, we recommend using a Linux
distribution (the demo-application was compiled on Ubuntu 20.04).

-   Follow instruction in [BUILDING.md](../../../../../docs/guides/BUILDING.md)
    to setup the environement to be able to build Matter.

-   Download the NXP MCUXpresso git SDK and associated middleware from GitHub
    using the west tool.

```
user@ubuntu:~/Desktop/git/connectedhomeip$ scripts/checkout_submodules.py --shallow --platform nxp --recursive
user@ubuntu:~/Desktop/git/connectedhomeip$ source ./scripts/bootstrap.sh
user@ubuntu:~/Desktop/git/connectedhomeip$ source ./scripts/activate.sh
user@ubuntu:~/Desktop/git/connectedhomeip$ cd third_party/nxp/rt_sdk/repo
user@ubuntu:~/Desktop/git/connectedhomeip/third_party/nxp/rt_sdk/repo$ west init -l manifest --mf west.yml
user@ubuntu:~/Desktop/git/connectedhomeip/third_party/nxp/rt_sdk/repo$ west update
```

-   In case there are local modification to the already installed git NXP SDK.
    Use the west forall command instead of the west init to reset the west
    workspace before running the west update command. Warning: all local changes
    will be lost after running this command.

```
user@ubuntu:~/Desktop/git/connectedhomeip/third_party/nxp/rt_sdk/repo$ west forall -c "git reset --hard && git clean -xdf" -a
```

-   Start building the application

```
user@ubuntu:~/Desktop/git/connectedhomeip$ cd examples/all-cluster/nxp/rt/rt1170/
```

### Building with Matter over Wifi configuration on RT1170 + IWX12

-   Build the Wi-fi configuration for MIMXRT1170 board + IWX12 transceiver (with
    BLE for commissioning).

```
user@ubuntu:~/Desktop/git/connectedhomeip/examples/all-clusters-app/nxp/rt/rt1170$ gn gen --args="chip_enable_wifi=true iwx12_transceiver=true chip_config_network_layer_ble=true chip_enable_ble=true " out/debug
user@ubuntu:~/Desktop/git/connectedhomeip/examples/all-clusters-app/nxp/rt/rt1170$ ninja -C out/debug
```

### Building with Matter over Thread configuration on RT1170 + IWX12

-   Build the Openthread configuration for MIMXRT1170 board + IWX12 transceiver
    (with BLE for commissioning).

```
user@ubuntu:~/Desktop/git/connectedhomeip/examples/all-cluster/nxp/rt/rt1170$ gn gen --args="chip_enable_openthread=true iwx12_transceiver=true chip_inet_config_enable_ipv4=false chip_config_network_layer_ble=true" out/debug
user@ubuntu:~/Desktop/git/connectedhomeip/examples/all-cluster/nxp/rt/rt1170/$ ninja -C out/debug
```

### Building with Matter over Wifi + OpenThread Border Router configuration on RT1170 + IWX12

This configuration requires enabling the Matter CLI in order to control the
Thread network on the Border Router.

-   Build Matter with Border Router configuration with BLE commissioning
    (ble-wifi) :

```
user@ubuntu:~/Desktop/git/connectedhomeip/examples/all-clusters-app/nxp/rt/rt11170$ gn gen --args="chip_enable_wifi=true iwx12_transceiver=true chip_config_network_layer_ble=true chip_enable_ble=true chip_enable_openthread=true chip_enable_matter_cli=true openthread_root =\"//third_party/connectedhomeip/third_party/openthread/ot-nxp/openthread-br\"" out/debug
user@ubuntu:~/Desktop/git/connectedhomeip/examples/all-clusters-app/nxp/rt/rt1170$ ninja -C out/debug
```

### General information

The resulting output file can be found in
out/debug/chip-rt1170-all-cluster-example.

Optional GN options that can be added when building an application:

-   To enable the
    [matter CLI](#testing-the-all-clusters-application-with-matter-cli-enabled),
    the argument `chip_enable_matter_cli=true` must be added to the _gn gen_
    command.
-   To build the application in debug mode, the argument
    `is_debug=true optimize_debug=false` must be added to the _gn gen_ command.
-   To build with the option to have Matter certificates/keys pre-loaded in a
    specific flash area the argument `chip_with_factory_data=1` must be added to
    the _gn gen_ command. For more information, see
    [Guide for writing manufacturing data on NXP devices](../../../../../docs/guides/nxp_manufacturing_flow.md)

<a name="manufacturing-data"></a>

## Manufacturing data

See
[Guide for writing manufacturing data on NXP devices](../../../../../docs/guides/nxp_manufacturing_flow.md)

Other comments:

The all cluster app demonstrates the usage of encrypted Matter manufacturing
data storage. Matter manufacturing data should be encrypted using an AES 128
software key before flashing them to the device flash.

<a name="flashing-and-debugging"></a>

## Flashing and debugging

In order to flash the application we recommend using
[MCUXpresso IDE (version >= 11.5.0)](https://www.nxp.com/design/software/development-software/mcuxpresso-software-and-tools-/mcuxpresso-integrated-development-environment-ide:MCUXpresso-IDE).

-   Import the previously downloaded NXP SDK into MCUXpresso IDE. This can be
    done by drag-and-dropping the SDK archive into MCUXpresso IDE's "Installed
    SDKs" tab;
    ![Select SDK](../../../../platform/nxp/rt/rt1170/doc/images/select-sdk.png)
-   Import the connectedhomeip repo in MCUXpresso IDE as Makefile Project. Use
    _none_ as _Toolchain for Indexer Settings_:

```
File -> Import -> C/C++ -> Existing Code as Makefile Project
```

-   Configure MCU Settings:

```
Right click on the Project -> Properties -> C/C++ Build -> MCU Settings -> Select MIMXRT1170 -> Apply & Close
```

![MCU_Sett](../../../../platform/nxp/rt/rt1170/doc/images/mcu-set.png)

Sometimes when the MCU is selected it will not initialize all the memory regions
(usualy the BOARD_FLASH, BOARD_SDRAM and NCAHCE_REGION) so it is required that
this regions are added manualy like in the image above. In addition to that on
the BOARD_FLASH line, in the driver tab:

```
click inside the tab and on the right side a button with three horizontal dots will appear
click on the button and an window will show
form the dropdown menu select the MIMXRT1170_SFDP_QSPI driver
```

![flash_driver](../../../../platform/nxp/rt/rt1170/doc/images/flash_driver.png)

-   Configure the toolchain editor:

```
Right click on the Project -> C/C++ Build-> Tool Chain Editor -> NXP MCU Tools -> Apply & Close
```

![toolchain](../../../../platform/nxp/rt/rt1170/doc/images/toolchain.JPG)

-   Create a debug configuration:

    MIMXRT1170-EVKB board supports CMSIS-DAP debug protocol by default. It
    should be switched to JLink.

    -   download and install
        [MCU-LINK_INSTALLER 2.263](https://www.nxp.com/webapp/Download?colCode=MCU-LINK_INSTALLER_2.263_LIN)
    -   connect jumper JP3 to get board in ISP mode when powered
    -   connect board and run installed MCU-LINK, follow steps to flash JLINK
        firmware
    -   remove jumper JP3 and reboot board

```
Right click on the Project -> Debug -> As->SEGGER J-Link probes -> OK -> Select elf file
```

![debug_0](../../../../platform/nxp/rt/rt1170/doc/images/debug0.png)

-   Set the _Initialization Commands_ to:

```
Right click on the Project -> Debug As -> Debug configurations... -> Startup

set non-stop on
set pagination off
set mi-async
set remotetimeout 60000
##target_extended_remote##
set mem inaccessible-by-default ${mem.access}
mon ondisconnect ${ondisconnect}
set arm force-mode thumb
${load}
```

![init](../../../../platform/nxp/rt/rt1170/doc/images/startup.png)

-   Set the _vector.catch_ value to _false_ inside the .launch file:

```
Right click on the Project -> Utilities -> Open Directory Browser here -> edit *.launch file:

<booleanAttribute key="vector.catch" value="false"/>

```

-   Debug using the newly created configuration file:

<a name="testing-the-example"></a>

## Testing the example

To know how to commision a device over BLE, follow the instructions from
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
   [here](../../../../chip-tool/README.md#using-the-client-to-send-matter-commands).

<a name="testing-the-all-clusters-application-with-matter-cli-enabled"></a>

### Testing the all-clusters application with Matter CLI enabled:

Testing the all-clusters application with Matter CLI enabled:

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

3. The All-cluster example uses UART2 to print logs while running the server. To
   view raw UART output, a pin should be plugged to an USB to UART adapter
   (connector J9 pin 4), then start a terminal emulator like PuTTY and connect
   to the used COM port with the following UART settings:

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
