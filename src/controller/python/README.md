# CHIP Device Controller - Python Binding & Command line interface

## Overview

Chip Device Controller is a general library to control devices and manage
commissioner functionality, it has Python, Java (for Android) and ObjC (for iOS)
bindings for building tests and CHIP mobile applications. This document will
focus on its python binding interface on Linux and mac os.

## Checkout / Build / Install

> Note: The CHIP can be built on Linux (amd64 / aarch64) and macOS

1. Clone Project CHIP repo.

```
git clone https://github.com/project-chip/connectedhomeip.git
```

2. Build the CHIP python package

Follow [BUILDING.md](/docs/BUILDING.md) to setup CHIP on your platform.

Genrally, once build dependencies are satisfied you can build the `python`
target.

Use `scripts/build_python.sh` or run something equivalent to:

```sh
gn gen out/python_lib
ninja -C out/python_lib python
```

3. Install Chip Device Controller

> Note: Python device controller is not versioned, so you need to uninstall the
> old device controller before install the new one.

> It is recommended to setup a separate clean virtual environment The
> `scripts/build_python.sh` script sets up a python environment and installs the
> WHL file.

```sh
virtualenv out/python_env --clean
source out/python_env/bin/activate
pip install out/python_lib/controller/python/chip*.whl
```

The WHL file installation will:

-   Install the 'chip' module
-   create a `ENV/bin/chip-device-ctrl` script that provides an interactive
    shell for the chip library

## BLE Virtualization on Linux (Optional)

If we would like to setup virtual BLE central and peripheral in the same
machine, then Bluez setup

```
cd third_party/bluez/repo
./bootstrap
third_party/bluez/repo/configure --prefix=/usr --mandir=/usr/share/man --sysconfdir=/etc --localstatedir=/var --enable-experimental --with-systemdsystemunitdir=/lib/systemd/system --with-systemduserunitdir=/usr/lib/systemd --enable-deprecated --enable-testing --enable-tools
make
```

Note: You also need to install several packages on RPi if you want to build
bluez

```
sudo apt-get install libtool m4 automake autotools-dev libudev-dev libical-dev libreadline-dev
```

Run bluetoothd:

```
sudo third_party/bluez/repo/src/bluetoothd --experimental --debug &
```

Bring up two virtual ble interface:

```
sudo third_party/bluez/repo/emulator/btvirt -L -l2
```

## Usage / BLE Secure Session Establishment

1. Run CHIP Device Controller

> Running as root via `sudo` to ensure permissions to interface with the
> bluetooth adapter.

```
sudo chip-device-ctrl
```

2. [WIP][required when there are multiple ble adapters] Select BLE adapter
   (Linux only)

```
chip-device-ctrl > ble-adapter-print
2020-11-23 17:41:53,116 ChipBLEMgr   INFO     adapter 0 = DE:AD:BE:EF:00:00
2020-11-23 17:41:53,116 ChipBLEMgr   INFO     adapter 1 = DE:AD:BE:EF:01:01
2020-11-23 17:41:53,116 ChipBLEMgr   INFO     adapter 2 = DE:AD:BE:EF:02:02

chip-device-ctrl > ble-adapter-select DE:AD:BE:EF:00:00
```

3. Scan BLE devices

```
chip-device-ctrl > ble-scan
2021-01-19 02:27:23,653 ChipBLEMgr   INFO     scanning started
2021-01-19 02:27:25,144 ChipBLEMgr   INFO     Name            = CHIP-1383
2021-01-19 02:27:25,144 ChipBLEMgr   INFO     ID              = ae0125dc-e621-3e05-9166-70ca7ea07985
2021-01-19 02:27:25,146 ChipBLEMgr   INFO     RSSI            = -32
2021-01-19 02:27:25,147 ChipBLEMgr   INFO     Address         = DC:A6:32:A5:4C:56
2021-01-19 02:27:25,151 ChipBLEMgr   INFO     Pairing State   = 0
2021-01-19 02:27:25,151 ChipBLEMgr   INFO     Discriminator   = 1383
2021-01-19 02:27:25,152 ChipBLEMgr   INFO     Vendor Id       = 9050
2021-01-19 02:27:25,152 ChipBLEMgr   INFO     Product Id      = 65279
2021-01-19 02:27:25,155 ChipBLEMgr   INFO     Adv UUID        = 0000feaf-0000-1000-8000-00805f9b34fb
2021-01-19 02:27:25,156 ChipBLEMgr   INFO     Adv Data        = 0067055a23fffe
2021-01-19 02:27:27,257 ChipBLEMgr   INFO
2021-01-19 02:27:34,213 ChipBLEMgr   INFO     scanning stopped
Connect to BLE device
```

4.  Set wifi credential

> Note: This command will be deprerated after the network provisioning cluster
> is ready.

```
chip-device-ctrl > set-pairing-wifi-credential TestAP TestPassword
```

5.  Connect to device using setup pin code

```
chip-device-ctrl > connect -ble 1383 12345678
```

## IP Secure Session Establishment

1. Run CHIP Device Controller

```
chip-device-ctrl
```

> Note: SUDO is not required when use IP to connect device.

2. Connect to device using setup pin code

```
chip-device-ctrl > connect -ip <Device IP Address> 12345678
```

## Commands

**`[L]`** = Linux only / **`[D]`** = Deprecated / **`[W]`** = WIP / **`[T]`** =
For testing

### **`[W][L]`** `ble-adapter-print`

Print the available Bluetooth adapters on device. Takes no arguments.

```
chip-device-ctrl > ble-adapter-print
2021-01-19 02:14:16,766 ChipBLEMgr   INFO     adapter 0 = DC:A6:32:9E:2E:A7
```

### **`[W][L]`** `ble-adapter-select <address>`

Select the Bluetooth adapter for device controller, takes adapter MAC address as
argument.

```
chip-device-ctrl > ble-adapter-select DC:A6:32:9E:2E:A7
(no output)
```

### `ble-scan [-t <timeout>] [identifier]`

Start a ble-scan action for searching valid CHIP devices over BLE [for at most
*timeout* seconds], stop when device matching the identifier or timeout.

```
chip-device-ctrl > ble-scan
2021-01-19 02:27:23,653 ChipBLEMgr   INFO     scanning started
2021-01-19 02:27:25,144 ChipBLEMgr   INFO     Name            = CHIP-1383
2021-01-19 02:27:25,144 ChipBLEMgr   INFO     ID              = ae0125dc-e621-3e05-9166-70ca7ea07985
2021-01-19 02:27:25,146 ChipBLEMgr   INFO     RSSI            = -32
2021-01-19 02:27:25,147 ChipBLEMgr   INFO     Address         = DC:A6:32:A5:4C:56
2021-01-19 02:27:25,151 ChipBLEMgr   INFO     Pairing State   = 0
2021-01-19 02:27:25,151 ChipBLEMgr   INFO     Discriminator   = 1383
2021-01-19 02:27:25,152 ChipBLEMgr   INFO     Vendor Id       = 9050
2021-01-19 02:27:25,152 ChipBLEMgr   INFO     Product Id      = 65279
2021-01-19 02:27:25,155 ChipBLEMgr   INFO     Adv UUID        = 0000feaf-0000-1000-8000-00805f9b34fb
2021-01-19 02:27:25,156 ChipBLEMgr   INFO     Adv Data        = 0067055a23fffe
2021-01-19 02:27:27,257 ChipBLEMgr   INFO
2021-01-19 02:27:34,213 ChipBLEMgr   INFO     scanning stopped
```

### `connect -ip <address> <SetUpPinCode> [<nodeid>]`

Do key exchange and establish a secure session between controller and device
using IP transport.

The node id will be used by controller to distinguish multiple devices. This
does not match the spec and will be removed later. The nodeid will not be
persisted by controller / device.

If no nodeid given, a random node id will be used.

### `connect -ble <discriminator> <SetUpPinCode> [<nodeid>]`

Do key exchange and establish a secure session between controller and device
using BLE transport.

The node id will be used by controller to distinguish multiple devices. This
does not match the spec and will be removed later. The nodeid will not be
persisted by controller / device.

If no nodeid given, a random node id will be used.

### **`[D]`** `set-pairing-wifi-credential <ssid> <password>`

Set WiFi credential for **_deprecated_** network provisioning precedure.

### **`[W]`** `zcl`

Sending ZCL commands.

#### `zcl ?`

List available clusters.

```
chip-device-ctrl > zcl ?
dict_keys(['BarrierControl', 'Basic', 'ColorControl', 'DoorLock', 'Groups', 'IasZone', 'Identify', 'LevelControl', 'NetworkProvisioning', 'OnOff', 'Scenes', 'TemperatureMeasurement'])
```

#### `zcl ? <Cluster>`

List available commands in cluster.

```
chip-device-ctrl > zcl ? LevelControl
Move
   moveMode: int, rate: int, optionMask: int, optionOverride: int
MoveToLevel
   level: int, transitionTime: int, optionMask: int, optionOverride: int
MoveToLevelWithOnOff
   level: int, transitionTime: int
MoveWithOnOff
   moveMode: int, rate: int
Step
   stepMode: int, stepSize: int, transitionTime: int, optionMask: int, optionOverride: int
StepWithOnOff
   stepMode: int, stepSize: int, transitionTime: int
Stop
   optionMask: int, optionOverride: int
StopWithOnOff
  <no arguments>
```

#### `zcl <Cluster> <Command> <NodeId> <EndpointId> <GroupId> [arguments]`

Send a ZCL command to `EndpointId` on device (`NodeId`).

Example:

```
chip-device-ctrl > zcl LevelControl MoveWithOnOff 12344321 1 0 moveMode=1 rate=2
```
