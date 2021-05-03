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
virtualenv out/python_env --clear
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

You can find the virtual interface by `hciconfig` command:

```
$ hciconfig

hci2:	Type: Primary  Bus: Virtual
	BD Address: 00:AA:01:01:00:24  ACL MTU: 192:1  SCO MTU: 0:0
	UP RUNNING
	RX bytes:0 acl:95 sco:0 events:205 errors:0
	TX bytes:2691 acl:95 sco:0 commands:98 errors:0

hci1:	Type: Primary  Bus: Virtual
	BD Address: 00:AA:01:00:00:23  ACL MTU: 192:1  SCO MTU: 0:0
	UP RUNNING
	RX bytes:0 acl:95 sco:0 events:208 errors:0
	TX bytes:3488 acl:95 sco:0 commands:110 errors:0
```

Then you can choose the adapter to use in command line arguments of the device
controller:

For example, add `--bluetooth-adapter=hci2` to use the virtual interface `hci2`
listed above.

```
chip-device-ctrl --bluetooth-adapter=hci2
```

## Usage / BLE Secure Session Establishment

1. Run CHIP Device Controller

> Running as root via `sudo` to ensure permissions to interface with the
> bluetooth adapter.

```
sudo chip-device-ctrl
```

or select the bluetooth interface by command line arguments.

```
sudo chip-device-ctrl --bluetooth-adapter=hci2
```

2. Scan BLE devices

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
2021-01-19 02:27:25,155 ChipBLEMgr   INFO     Adv UUID        = 0000fff6-0000-1000-8000-00805f9b34fb
2021-01-19 02:27:25,156 ChipBLEMgr   INFO     Adv Data        = 0067055a23fffe
2021-01-19 02:27:27,257 ChipBLEMgr   INFO
2021-01-19 02:27:34,213 ChipBLEMgr   INFO     scanning stopped
Connect to BLE device
```

3.  Set wifi credential

> Note: This command will be deprerated after the network provisioning cluster
> is ready.

```
chip-device-ctrl > set-pairing-wifi-credential TestAP TestPassword
```

4.  Connect to device using setup pin code

```
chip-device-ctrl > connect -ble 1383 20202021
```

## Thread provisioning

1. Configure Thread border router. For example, follow
   [Setup OpenThread Border Router on Raspberry Pi / ubuntu](../../../docs/guides/openthread_border_router_pi.md)
   instruction to configure OpenThread Border Router on a Linux workstation.

2. Run CHIP Device Controller

```
sudo chip-device-ctrl
```

3. Set Thread credentials

```
set-pairing-thread-credential <channel> <pan id[HEX]> <master_key>
```

4. BLE Connect to the device

```
connect -ble <discriminator> <setup pin code> [<nodeid>]
```

## IP Secure Session Establishment

1. Run CHIP Device Controller

```
chip-device-ctrl
```

> Note: SUDO is not required when use IP to connect device.

2. Connect to device using setup pin code

```
chip-device-ctrl > connect -ip <Device IP Address> 20202021
```

## Commands

**`[L]`** = Linux only / **`[D]`** = Deprecated / **`[W]`** = WIP / **`[T]`** =
For testing

### `setup-payload parse-manual <manual-pairing-code>`

Print the commissioning information encoded in the Manual Pairing Code.

```
chip-device-ctrl > setup-payload parse-manual 35767807533
Version: 0
VendorID: 0
ProductID: 0
RequiresCustomFlow: 0
RendezvousInformation: 0
Discriminator: 3840
SetUpPINCode: 20202021
```

### `setup-payload parse-qr <qr-code>`

Print the commissioning information encoded in the QR Code payload.

```
chip-device-ctrl > setup-payload parse-qr "VP:vendorpayload%CH:H34.GHY00 0C9SS0"
Version: 0
VendorID: 9050
ProductID: 20043
RequiresCustomFlow: 0
RendezvousInformation: 2 [BLE]
Discriminator: 3840
SetUpPINCode: 20202021
```

### **`[L]`** `ble-adapter-print`

Print the available Bluetooth adapters on device. Takes no arguments.

```
chip-device-ctrl > ble-adapter-print
2021-03-04 16:09:40,930 ChipBLEMgr   INFO     AdapterName: hci0   AdapterAddress: 00:AA:01:00:00:23
```

### **`[D]`** `ble-adapter-select <address>`

Select the Bluetooth adapter for device controller, takes adapter MAC address as
argument. This command only affects `ble-scan` command.

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
2021-01-19 02:27:25,155 ChipBLEMgr   INFO     Adv UUID        = 0000fff6-0000-1000-8000-00805f9b34fb
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

**Format of arguments**

For any integer and char string (null terminated) types, just use `key=value`,
for example: `rate=2`, `string=123`, `string_2="123 456"`

For byte string type, use `key=encoding:value`, currectly, we support `str` and
`hex` encoding, the `str` encoding will encode a NULL terminated string. For
example, `networkId=hex:0123456789abcdef` (for
`[0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef]`), `ssid=str:Test` (for
`['T', 'e', 's', 't', 0x00]`).

## Example Commissioning flow over

-   Assuming your WiFi ssid is `TESTSSID`, and your WiFi password is `P455W4RD`.

-   Assuming your Thread network has the following operational dataset (the
    extended pan id of this network is `577c1f5384d9e909`, thus the network id
    for this network is also `577c1f5384d9e909`):

    ```
    0e 08 0000000000010000
    00 03 000014
    35 06 0004001fffe0
    02 08 577c1f5384d9e909
    07 08 fdca4e253816ae9d
    05 10 bb53ac7bf2133f0f686759ad9969255c
    03 0f 4f70656e5468726561642d31343937
    01 02 1497
    04 10 420111ea791a892d28e3160f20eea396
    0c 03 0000ff
    ```

-   Assuming your device is on the same network, with IP address 192.168.0.1

-   The setup pincode is 20202021

-   You set the temporary node id to 4546

-   The discriminator of the device is 2333

> Establish PASE session over BLE
>
> ```
> chip-device-ctrl > connect -ble 2333 20202021 4546
> ```

> Establish PASE session over IP
>
> ```
> chip-device-ctrl > connect -ip 192.168.0.1 20202021 4546
> ```

> Skip this part if your device does not support WiFi.
>
> ```
> chip-device-ctrl > zcl NetworkCommissioning AddWiFiNetwork 4546 1 0 ssid=str:TESTSSID credentials=str:P455W4RD breadcrumb=0 timeoutMs=1000
>
> chip-device-ctrl > zcl NetworkCommissioning EnableNetwork 4546 1 0 networkID=str:TESTSSID breadcrumb=0 timeoutMs=1000
> ```

> Skip this part if your device does not support Thread.
>
> ```
> chip-device-ctrl > zcl NetworkCommissioning AddThreadNetwork 4546 1 0 operationalDataset=hex:0e080000000000010000000300001435060004001fffe00208577c1f5384d9e9090708fdca4e253816ae9d0510bb53ac7bf2133f0f686759ad9969255c030f4f70656e5468726561642d31343937010214970410420111ea791a892d28e3160f20eea3960c030000ff breadcrumb=0 timeoutMs=1000
>
> chip-device-ctrl > zcl NetworkCommissioning EnableNetwork 4546 1 0 networkID=hex:577c1f5384d9e909 breadcrumb=0 timeoutMs=1000
> ```

> If you are using BLE connection, release BLE connection
>
> ```
> chip-device-ctrl > close-ble
> ```
