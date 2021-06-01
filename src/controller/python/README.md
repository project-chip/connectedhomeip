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
chip-device-ctrl > setup-payload parse-manual 34970112332
Version: 0
VendorID: 0
ProductID: 0
CommissioningFlow: 0
RendezvousInformation: 0
Discriminator: 3840
SetUpPINCode: 20202021
```

### `setup-payload parse-qr <qr-code>`

Print the commissioning information encoded in the QR Code payload.

```
chip-device-ctrl > setup-payload parse-qr "VP:vendorpayload%MT:W0GU2OTB00KA0648G00"
Version: 0
VendorID: 9050
ProductID: 20043
CommissioningFlow: 0
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
> chip-device-ctrl > zcl NetworkCommissioning AddWiFiNetwork 4546 0 0 ssid=str:TESTSSID credentials=str:P455W4RD breadcrumb=0 timeoutMs=1000
>
> chip-device-ctrl > zcl NetworkCommissioning EnableNetwork 4546 0 0 networkID=str:TESTSSID breadcrumb=0 timeoutMs=1000
> ```

> Skip this part if your device does not support Thread.
>
> ```
> chip-device-ctrl > zcl NetworkCommissioning AddThreadNetwork 4546 0 0 operationalDataset=hex:0e080000000000010000000300001435060004001fffe00208577c1f5384d9e9090708fdca4e253816ae9d0510bb53ac7bf2133f0f686759ad9969255c030f4f70656e5468726561642d31343937010214970410420111ea791a892d28e3160f20eea3960c030000ff breadcrumb=0 timeoutMs=1000
>
> chip-device-ctrl > zcl NetworkCommissioning EnableNetwork 4546 0 0 networkID=hex:577c1f5384d9e909 breadcrumb=0 timeoutMs=1000
> ```

> If you are using BLE connection, release BLE connection
>
> ```
> chip-device-ctrl > close-ble
> ```

## Debugging with gdb

You can run the chip-device-ctrl under GDB for debugging, however, since the
CHIP core support library is a dynamic library, you cannot read the symbols
unless it is fully loaded.

The following block is a example debug session using GDB

```
# GDB cannot run scripts directly
# so you need to run Python3 with the path of device controller
# Here, we use the feature from bash to get the path of chip-device-ctrl without typing it.
$ gdb --args python3 `which chip-device-ctrl`
GNU gdb (Ubuntu 10.1-2ubuntu2) 10.1.90.20210411-git
Copyright (C) 2021 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
Type "show copying" and "show warranty" for details.
This GDB was configured as "aarch64-linux-gnu".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<https://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.

For help, type "help".
Type "apropos word" to search for commands related to "word"...
Reading symbols from python3...
(No debugging symbols found in python3)
(gdb)
```

The Python will create lots of threads due to main loop, so you may want to
supress thread related outputs first by `set print thread-events off`

```
(gdb) set print thread-events off
(gdb)
```

We cannot set breakpoints here, since the GDB knows nothing about the CHIP
library, let run the CHIP device controller first.

```
(gdb) run
Starting program: /usr/bin/python3 /home/ubuntu/.local/bin/chip-device-ctrl
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib/aarch64-linux-gnu/libthread_db.so.1".
CHIP:DIS: Init admin pairing table with server storage.
CHIP:IN: local node id is 0x000000000001b669
CHIP:DL: MDNS failed to join multicast group on wpan0 for address type IPv4: Inet Error 1016 (0x000003F8): Address not found
CHIP:ZCL: Using ZAP configuration...
CHIP:ZCL: deactivate report event
CHIP:CTL: Getting operational keys
CHIP:CTL: Generating operational certificate for the controller
CHIP:CTL: Getting root certificate for the controller from the issuer
CHIP:CTL: Generating credentials
CHIP:CTL: Loaded credentials successfully
CHIP:DL: Platform main loop started.
Chip Device Controller Shell

chip-device-ctrl >
```

The prompt `chip-device-ctrl >` indicates that the CHIP core library is loaded
by Python, you can browse the symbols in the CHIP core library, setting
breakpoints on functions and many other functions provided by GDB.

You can use `Ctrl-C` to send SIGINT to the controller anytime you want so you
can set breakpoints.

> (`Ctrl-C` pressed here.)

```
Thread 1 "python3" received signal SIGINT, Interrupt.
0x0000fffff7db79ec in __GI___select (nfds=<optimized out>, readfds=0xffffffffe760, writefds=0x0, exceptfds=0x0, timeout=<optimized out>) at ../sysdeps/unix/sysv/linux/select.c:49
49	../sysdeps/unix/sysv/linux/select.c: No such file or directory.
(gdb)
```

For example, you can break on `DeviceCommissioner::PairDevice` by using `break`
command in GDB (`b` for short)

```
(gdb) b DeviceCommissioner::PairDevice
Breakpoint 1 at 0xfffff5b0f6b4 (2 locations)
(gdb)
```

Type `continue` (`c` for short) to continue the device controller, you may need
another hit of `Enter` to see the prompt.

```
(gdb) c
Continuing.

chip-device-ctrl >
```

Let do pairing over IP to see the effect of the breakpoint we just set.

```
chip-device-ctrl > connect -ip 192.168.50.5 20202021 1
Device is assigned with nodeid = 1

Thread 1 "python3" hit Breakpoint 1, 0x0000fffff5b0f6b4 in chip::Controller::DeviceCommissioner::PairDevice(unsigned long, chip::RendezvousParameters&)@plt ()
   from /home/ubuntu/.local/lib/python3.9/site-packages/chip/_ChipDeviceCtrl.so
(gdb)
```

The `@plt` symbol means it is a symbol used by dynamic library loader, type `c`
(for `continue`) and it will break on the real function.

```
(gdb) c
Continuing.

Thread 1 "python3" hit Breakpoint 1, chip::Controller::DeviceCommissioner::PairDevice (this=0xd28540, remoteDeviceId=1, params=...) at ../../src/controller/CHIPDeviceController.cpp:827
827	{
(gdb)
```

You can find the `this` pointer, and value of arguments passed to this function,
then you can use `bt` (for `backtrace`) to see the backtrace of the call stack.

```
(gdb) bt
#0  chip::Controller::DeviceCommissioner::PairDevice(unsigned long, chip::RendezvousParameters&) (this=0xd28540, remoteDeviceId=1, params=...)
    at ../../src/controller/CHIPDeviceController.cpp:827
#1  0x0000fffff5b3095c in pychip_DeviceController_ConnectIP(chip::Controller::DeviceCommissioner*, char const*, uint32_t, chip::NodeId)
    (devCtrl=0xd28540, peerAddrStr=0xfffff467ace0 "192.168.50.5", setupPINCode=20202021, nodeid=1) at ../../src/controller/python/ChipDeviceController-ScriptBinding.cpp:234
#2  0x0000fffff7639148 in  () at /lib/aarch64-linux-gnu/libffi.so.8
#3  0x0000fffff7638750 in  () at /lib/aarch64-linux-gnu/libffi.so.8
#4  0x0000fffff7665a44 in  () at /usr/lib/python3.9/lib-dynload/_ctypes.cpython-39-aarch64-linux-gnu.so
#5  0x0000fffff7664c7c in  () at /usr/lib/python3.9/lib-dynload/_ctypes.cpython-39-aarch64-linux-gnu.so
#6  0x00000000004a54f0 in _PyObject_MakeTpCall ()
#7  0x000000000049cb10 in _PyEval_EvalFrameDefault ()
#8  0x0000000000496d1c in  ()
#9  0x00000000004b1eb0 in _PyFunction_Vectorcall ()
#10 0x0000000000498264 in _PyEval_EvalFrameDefault ()
#11 0x00000000004b1cb8 in _PyFunction_Vectorcall ()
#12 0x0000000000498418 in _PyEval_EvalFrameDefault ()
#13 0x0000000000496d1c in  ()
#14 0x00000000004b1eb0 in _PyFunction_Vectorcall ()
#15 0x0000000000498418 in _PyEval_EvalFrameDefault ()
#16 0x00000000004b1cb8 in _PyFunction_Vectorcall ()
#17 0x00000000004c6bc8 in  ()
#18 0x0000000000498264 in _PyEval_EvalFrameDefault ()
#19 0x00000000004b1cb8 in _PyFunction_Vectorcall ()
#20 0x0000000000498418 in _PyEval_EvalFrameDefault ()
#21 0x00000000004966f8 in  ()
#22 0x00000000004b1f18 in _PyFunction_Vectorcall ()
#23 0x0000000000498418 in _PyEval_EvalFrameDefault ()
#24 0x00000000004b1cb8 in _PyFunction_Vectorcall ()
#25 0x0000000000498264 in _PyEval_EvalFrameDefault ()
#26 0x00000000004966f8 in  ()
#27 0x0000000000496490 in _PyEval_EvalCodeWithName ()
#28 0x0000000000595b7c in PyEval_EvalCode ()
#29 0x00000000005c6a5c in  ()
#30 0x00000000005c0a70 in  ()
#31 0x00000000005c69a8 in  ()
#32 0x00000000005c6148 in PyRun_SimpleFileExFlags ()
#33 0x00000000005b60bc in Py_RunMain ()
#34 0x0000000000585a08 in Py_BytesMain ()
#35 0x0000fffff7d0c9d4 in __libc_start_main (main=
    0x5858fc <_start+60>, argc=2, argv=0xfffffffff498, init=<optimized out>, fini=<optimized out>, rtld_fini=<optimized out>, stack_end=<optimized out>) at ../csu/libc-start.c:332
#36 0x00000000005858f8 in _start ()
(gdb)
```

The frame #0 and frame #1 are the function frame in the CHIP C++ library, the
other frames lives in the Python intepreter so you can ignore it.
