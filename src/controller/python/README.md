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

2. Build CHIP

Follow [BUILDING.md](/docs/BUILDING.md) to build CHIP on your platform.

3. Install Chip Device Controller

> Note: Python device controller is not versioned, so you need to uninstall the
> old device controller before install the new one.

**For Linux (Raspberry Pi 4)**

```
cd out/debug/controller/python
sudo pip3 install chip-0.0-cp38-cp38-linux_aarch64.whl
```

> Note: for linux, sudo is necessary since it need to interact with bluetoothd
> via dbus with sudo permission

**For macOS**

```
cd out/debug/mac_x64_clang/controller/python
pip3 install chip-0.0-cp38-cp38-mac_aarch64.whl
```

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

```
sudo chip-device-ctrl
```

2. [Required when there are multiple BLE adapters] Select BLE adapter (Linux
   only)

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

**Using device discriminator**

```
chip-device-ctrl > ble-connect 1383
2020-11-23 17:36:41,894 ChipBLEMgr   INFO     trying to connect to 1383
2020-11-23 17:36:44,571 ChipBLEMgr   INFO     BLE connecting
2020-11-23 17:36:44,572 ChipBLEMgr   INFO     Discovering services
2020-11-23 17:36:45,939 ChipBLEMgr   INFO     Service discovering success
2020-11-23 17:36:45,971 ChipBLEMgr   INFO     connect success
```

> Note, they will be replaced by using discriminator.

5.  Set wifi credential

> Note: This command will be deprerated after the network provisioning cluster
> is ready.

```
chip-device-ctrl > set-pairing-wifi-credential TestAP TestPassword
```

6.  Connect to device using setup pin code

```
chip-device-ctrl > connect -ble 12345678
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

### **`[L]`** `ble-adapter-print`

Print the available Bluetooth adapters on device. Takes no arguments.

```
chip-device-ctrl > ble-adapter-print
2021-01-19 02:14:16,766 ChipBLEMgr   INFO     adapter 0 = DC:A6:32:9E:2E:A7
```

### **`[L]`** `ble-adapter-select <address>`

Select the Bluetooth adapter for device controller, takes adapter MAC address as
argument.

```
chip-device-ctrl > ble-adapter-select DC:A6:32:9E:2E:A7
(no output)
```

### `ble-connect <identifier>`

Connect to a device using the selected identifier, the identifier can be device
name or device discriminator (preferred). You **MUST** use `ble-scan` command to
build device list, or use `ble-scan-connect`.

### **`[T]`** `ble-debug-log <0|1>`

Use `ble-debug-log 1` to set logging level to debug and use `ble-debug-log 0` to
set logging level to info.

```
chip-device-ctrl > ble-debug-log 0
2021-01-19 02:23:32,092 ChipBLEMgr   INFO     current logging level is info
chip-device-ctrl > ble-debug-log 1
2021-01-19 02:23:33,964 ChipBLEMgr   DEBUG    current logging level is debug
```

### `ble-disconnect`

Disconnect current BLE connection.

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

### `ble-scan-connect [-t <timeout>] [identifier]`

Equals to run `ble-scan [-t <timeout>] [identifier]` and then
`ble-connect [identifier]`

### **`[T]`** `btp-connect`

Used to test Python bluetooth binding.

### **`[D]`** `close`

Clost rendezvous session.

### `connect -ip <address> <SetUpPinCode>`

Do key exchange and establish a secure session between controller and device
using IP transport.

### `connect -ble <SetUpPinCode>`

Do key exchange and establish a secure session between controller and device
using BLE transport.

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
