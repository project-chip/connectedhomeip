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

**For Linux**

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

## Usage

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
2020-11-23 17:36:18,012 ChipBLEMgr   INFO     use default adapter
2020-11-23 17:36:18,102 ChipBLEMgr   INFO     scanning started
2020-11-23 17:36:18,837 ChipBLEMgr   INFO     Name =    CHIP-0000
2020-11-23 17:36:18,837 ChipBLEMgr   INFO     ID =      f8cc7d32-c110-398b-984b-e43732c9afa1
2020-11-23 17:36:18,839 ChipBLEMgr   INFO     RSSI =    -56
2020-11-23 17:36:18,840 ChipBLEMgr   INFO     address = C3:A7:6C:A5:E3:ED
2020-11-23 17:36:18,843 ChipBLEMgr   INFO     ADV data: 0000feaf-0000-1000-8000-00805f9b34fb
Connect to BLE device
```

**Using device name**

```
chip-device-ctrl > ble-connect CHIP-0000
2020-11-23 17:36:41,894 ChipBLEMgr   INFO     trying to connect to CHIP-0000
2020-11-23 17:36:44,571 ChipBLEMgr   INFO     BLE connecting
2020-11-23 17:36:44,572 ChipBLEMgr   INFO     Discovering services
2020-11-23 17:36:45,939 ChipBLEMgr   INFO     Service discovering success
2020-11-23 17:36:45,971 ChipBLEMgr   INFO     connect success
```

**Using device address (on Linux)**

```
chip-device-ctrl > ble-connect C3:A7:6C:A5:E3:ED
2020-11-23 17:36:41,894 ChipBLEMgr   INFO     trying to connect to C3:A7:6C:A5:E3:ED
2020-11-23 17:36:44,571 ChipBLEMgr   INFO     BLE connecting
2020-11-23 17:36:44,572 ChipBLEMgr   INFO     Discovering services
2020-11-23 17:36:45,939 ChipBLEMgr   INFO     Service discovering success
2020-11-23 17:36:45,971 ChipBLEMgr   INFO     connect success
```

**Using uuid (on macOS)**

```
chip-device-ctrl > ble-connect f8cc7d32-c110-398b-984b-e43732c9afa1
2020-11-23 17:36:41,894 ChipBLEMgr   INFO     trying to connect to f8cc7d32-c110-398b-984b-e43732c9afa1
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
chip-device-ctrl > connect 12345678
```

## Commands

// TODO: For now, the CHIP device controller does not have any interaction model
(cluster) or service provisioning commands, they will be added to the device
controller as soon as they are implemented.
