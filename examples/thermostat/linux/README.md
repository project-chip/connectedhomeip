# Matter Linux Thermostat Example

An example showing the use of Matter on the Linux. The document will describe
how to build and run Matter Linux Thermostat Example on Raspberry Pi. This doc
is tested on **Ubuntu for Raspberry Pi Server 20.04 LTS (aarch64)** and **Ubuntu
for Raspberry Pi Desktop 20.10 (aarch64)**

To cross-compile this example on x64 host and run on **NXP i.MX 8M Mini**
**EVK**, see the associated
[README document](../../../docs/guides/nxp/nxp_imx8m_linux_examples.md) for
details.

<hr>

-   [Matter Linux Thermostat Example](#matter-linux-thermostat-example)
    -   [Building](#building)
    -   [Commandline Arguments](#commandline-arguments)
    -   [Running the Complete Example on Raspberry Pi 4](#running-the-complete-example-on-raspberry-pi-4)
    -   [Presets](#presets)

<hr>

## Building

-   Install tool chain

          $ sudo apt-get install git gcc g++ python pkg-config libssl-dev libdbus-1-dev libglib2.0-dev ninja-build python3-venv python3-dev unzip

-   Build the example application:

          $ cd ~/connectedhomeip/examples/thermostat/linux
          $ git submodule update --init
          $ source third_party/connectedhomeip/scripts/activate.sh
          $ gn gen out/debug
          $ ninja -C out/debug

-   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip/examples/thermostat/linux
          $ rm -rf out/

## Commandline arguments

-   `--wifi`

    Enables WiFi management feature. Required for WiFi commissioning.

-   `--thread`

    Enables Thread management feature, requires ot-br-posix dbus daemon running.
    Required for Thread commissioning.

-   `--ble-device <interface id>`

    Use specific bluetooth interface for BLE advertisement and connections.

    `interface id`: the number after `hci` when listing BLE interfaces by
    `hciconfig` command, for example, `--ble-device 1` means using `hci1`
    interface. Default: `0`.

## Running the Complete Example on Raspberry Pi 4

> If you want to test Echo protocol, please enable Echo handler
>
>     gn gen out/debug --args='chip_app_use_echo=true'
>     ninja -C out/debug

-   Prerequisites

    1. A Raspberry Pi 4 board
    2. A USB Bluetooth Dongle, Ubuntu desktop will send Bluetooth advertisement,
       which will block Matter from connecting via BLE. On Ubuntu server, you
       need to install `pi-bluetooth` via APT.
    3. Ubuntu 20.04 or newer image for ARM64 platform.

-   Building

    Follow [Building](#building) section of this document.

-   Running

    -   [Optional] Plug USB Bluetooth dongle

        -   Plug USB Bluetooth dongle and find its bluetooth device number. The
            number after `hci` is the bluetooth device number, `1` in this
            example.

                  $ hciconfig
                  hci1:	Type: Primary  Bus: USB
                      BD Address: 00:1A:7D:AA:BB:CC  ACL MTU: 310:10  SCO MTU: 64:8
                      UP RUNNING PSCAN ISCAN
                      RX bytes:20942 acl:1023 sco:0 events:1140 errors:0
                      TX bytes:16559 acl:1011 sco:0 commands:121 errors:0

                  hci0:	Type: Primary  Bus: UART
                      BD Address: B8:27:EB:AA:BB:CC  ACL MTU: 1021:8  SCO MTU: 64:1
                      UP RUNNING PSCAN ISCAN
                      RX bytes:8609495 acl:14 sco:0 events:217484 errors:0
                      TX bytes:92185 acl:20 sco:0 commands:5259 errors:0

        -   Run Linux Thermostat Example App

                  $ cd ~/connectedhomeip/examples/thermostat/linux
                  $ sudo out/debug/thermostat-app --ble-device [bluetooth device number]
                  # In this example, the device we want to use is hci1
                  $ sudo out/debug/thermostat-app --ble-device 1

        -   Test the device using ChipDeviceController on your laptop /
            workstation etc.

## Presets

### Read presets

```shell
chip-tool-x86-64 thermostat read presets <nodeID> 1 | grep TOO
[1758985235.117] [4022:4024] [TOO] Sending command to node 0x1e
[1758985235.347] [4022:4024] [TOO] Sending ReadAttribute to:
[1758985235.347] [4022:4024] [TOO]      cluster 0x0000_0201, attribute: 0x0000_0050, endpoint 1
[1758985235.350] [4022:4024] [TOO] Endpoint: 1 Cluster: 0x0000_0201 Attribute 0x0000_0050 DataVersion: 1113257472
[1758985235.351] [4022:4024] [TOO]   Presets: 2 entries
[1758985235.351] [4022:4024] [TOO]     [1]: {
[1758985235.351] [4022:4024] [TOO]       PresetHandle: 01
[1758985235.351] [4022:4024] [TOO]       PresetScenario: 1
[1758985235.351] [4022:4024] [TOO]       CoolingSetpoint: 2500
[1758985235.351] [4022:4024] [TOO]       HeatingSetpoint: 2100
[1758985235.351] [4022:4024] [TOO]       BuiltIn: TRUE
[1758985235.351] [4022:4024] [TOO]      }
[1758985235.351] [4022:4024] [TOO]     [2]: {
[1758985235.351] [4022:4024] [TOO]       PresetHandle: 02
[1758985235.351] [4022:4024] [TOO]       PresetScenario: 2
[1758985235.351] [4022:4024] [TOO]       CoolingSetpoint: 2600
[1758985235.351] [4022:4024] [TOO]       HeatingSetpoint: 2000
[1758985235.351] [4022:4024] [TOO]       BuiltIn: TRUE
[1758985235.351] [4022:4024] [TOO]      }
```

### Write presets

Write the 3-presets value using the atomic-request:

```json
[
    {
        "presetHandle": "01",
        "presetScenario": 1,
        "coolingSetpoint": 2500,
        "heatingSetpoint": 2100,
        "builtIn": true
    },
    {
        "presetHandle": "02",
        "presetScenario": 2,
        "coolingSetpoint": 2600,
        "heatingSetpoint": 2000,
        "builtIn": true
    },
    {
        "presetHandle": null,
        "presetScenario": 3,
        "coolingSetpoint": 2700,
        "heatingSetpoint": 2000,
        "builtIn": false
    }
]
```

**Atomic-request**

All three commands have to be executed within 3 seconds (timeout):

```shell
chip-tool-x86-64 thermostat atomic-request 0 '[80]' <nodeID> 1 --Timeout 3000
chip-tool-x86-64 thermostat write presets '[ { "presetHandle": "01", "presetScenario": 1, "coolingSetpoint": 2500, "heatingSetpoint": 2100, "builtIn": true }, {"presetHandle": "02", "presetScenario": 2, "coolingSetpoint": 2600, "heatingSetpoint": 2000, "builtIn": true }, {"presetHandle": null, "presetScenario": 3, "coolingSetpoint": 2700, "heatingSetpoint": 2000, "builtIn": false } ]' <nodeID> 1
chip-tool-x86-64 thermostat atomic-request 1 '[80]' <nodeID> 1
```

### Set active preset

Select PresetHandle `02`:

```shell
chip-tool-x86-64 thermostat set-active-preset-request PresetHandle destination-id endpoint-id-ignored-for-group-commands
chip-tool-x86-64 thermostat set-active-preset-request hex:02 <nodeID> 1 | grep TOO
[1758995147.916] [4607:4609] [TOO] Sending command to node 0x1e
[1758995148.195] [4607:4609] [TOO] Sending cluster (0x00000201) command (0x00000006) on endpoint 1
```

### Read active PresetHandle

```shell
chip-tool-x86-64 thermostat read active-preset-handle <nodeID> 1 | grep TOO
[1758995260.093] [4613:4615] [TOO] Sending command to node 0x1e
[1758995260.387] [4613:4615] [TOO] Sending ReadAttribute to:
[1758995260.387] [4613:4615] [TOO]      cluster 0x0000_0201, attribute: 0x0000_004E, endpoint 1
[1758995260.396] [4613:4615] [TOO] Endpoint: 1 Cluster: 0x0000_0201 Attribute 0x0000_004E DataVersion: 1113257472
[1758995260.396] [4613:4615] [TOO]   ActivePresetHandle: 02
```
