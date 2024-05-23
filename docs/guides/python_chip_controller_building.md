# Deprecation notice

chip-device-ctrl is no longer maintained and should not be used.

Matter-repl is the current python controller implementation.

# Working with Python CHIP Controller

The Python CHIP Controller is a tool that allows to commission a Matter device
into the network and to communicate with it using the Zigbee Cluster Library
(ZCL) messages.

> The chip-device-ctrl tool will be deprecated, and will be replaced by
> chip-repl. Continue reading to see how to do the same thing with chip-repl.

<hr>

-   [Source files](#source-files)
-   [Building Android CHIPTool](#building-and-installing)
-   [Running the tool](#running-the-tool)
-   [Using Python CHIP Controller for Matter accessory testing](#using-python-chip-controller-for-matter-accessory-testing)
-   [List of commands](#list-of-commands)

<hr>

## Source files

You can find source files of the Python CHIP Controller tool in the
`src/controller/python` directory.

The tool uses the generic CHIP Device Controller library, available in the
`src/controller` directory.

<hr>

## Building and installing

Before you can use the Python controller, you must compile it from the source on
Linux (amd64 / aarch64) or macOS.

> To ensure compatibility, build the Python CHIP controller and the Matter
> device from the same revision of the connectedhomeip repository.

To build and run the Python CHIP controller:

1. Install all necessary packages and prepare the build system. For more
   details, see the [Building Matter](BUILDING.md) documentation:

    ```
    sudo apt-get update
    sudo apt-get upgrade

    sudo apt-get install git gcc g++ python pkg-config libssl-dev libdbus-1-dev libglib2.0-dev libavahi-client-dev ninja-build python3-venv python3-dev python3-pip unzip libgirepository1.0-dev libcairo2-dev bluez
    ```

    If the Python CHIP controller is built on a Raspberry Pi, install additional
    packages and reboot the device:

    ```
    sudo apt-get install pi-bluetooth
    sudo reboot
    ```

2. Clone the Project CHIP repository:

    ```
    git clone https://github.com/project-chip/connectedhomeip.git
    ```

3. Enter the `connectedhomeip` directory:

    ```
    cd connectedhomeip
    ```

4. Initialize the git submodules:

    ```
    git submodule update --init
    ```

5. Build and install the Python CHIP controller:

    ```
    scripts/build_python.sh -m platform -i separate
    ```

    > Note: To get more details about available build configurations, run the
    > following command: `scripts/build_python.sh --help`

<hr>

## Running the tool

1. Activate the Python virtual environment:

    ```
    source out/python_env/bin/activate
    ```

2. Run the Python CHIP controller with root privileges, which is required to
   obtain access to the Bluetooth interface:

    ```
    sudo out/python_env/bin/chip-device-ctrl
    ```

    You can also select the Bluetooth LE interface using command line argument:

    ```
    sudo out/python_env/bin/chip-device-ctrl --bluetooth-adapter=hci2
    ```

<hr>

## Using Python CHIP Controller for Matter accessory testing

This section describes how to use Python CHIP controller to test the Matter
accessory. Below steps depend on the application clusters that you implemented
on the device side and may be different for your accessory.

### Step 1: Prepare the Matter accessory.

This tutorial is using the Matter Light Bulb example with the Bluetooth LE
commissioning. However, you can adapt this procedure to other available Matter
examples.

Build and program the device with the Matter accessory firmware by following the
example's documentation.

### Step 2: Enable Bluetooth LE advertising on Matter accessory device.

Some examples are configured to advertise automatically on boot. Other examples
require physical trigger, for example pushing a button. Follow the documentation
of the Matter accessory example to learn how Bluetooth LE advertising is enabled
for the given example.

### Step 3: Discover Matter accessory device over Bluetooth LE

An uncommissioned accessory device advertises over Bluetooth LE. Run the
following command to scan all advertised Matter devices:

```
chip-device-ctrl > ble-scan
```

### Step 4: Set network pairing credentials

You must provide the controller with network credentials that will be further
used in the device commissioning procedure to configure the device with a
network interface, such as Thread or Wi-Fi.

#### Setting Thread network credentials

1. Fetch and store the current Active Operational Dataset from the Thread Border
   Router. Depending on if the Thread Border Router is running on Docker or
   natively on Raspberry Pi, execute the following commands:

    - For Docker:

        ```
        sudo docker exec -it otbr sh -c "sudo ot-ctl dataset active -x"
        0e080000000000010000000300001335060004001fffe002084fe76e9a8b5edaf50708fde46f999f0698e20510d47f5027a414ffeebaefa92285cc84fa030f4f70656e5468726561642d653439630102e49c0410b92f8c7fbb4f9f3e08492ee3915fbd2f0c0402a0fff8
        Done
        ```

    - For native installation:

        ```
        sudo ot-ctl dataset active -x
        0e080000000000010000000300001335060004001fffe002084fe76e9a8b5edaf50708fde46f999f0698e20510d47f5027a414ffeebaefa92285cc84fa030f4f70656e5468726561642d653439630102e49c0410b92f8c7fbb4f9f3e08492ee3915fbd2f0c0402a0fff8
        Done
        ```

    Matter specification does not define how the Thread or Wi-Fi credentials are
    obtained by Controller. For example, for Thread, instead of fetching
    datasets directly from the Thread Border Router, you might also use a
    different out-of-band method.

2. Set the previously obtained Active Operational Dataset as a hex-encoded value
   using the following command:

    ```
    chip-device-ctrl > set-pairing-thread-credential 0e080000000000010000000300001335060004001fffe002084fe76e9a8b5edaf50708fde46f999f0698e20510d47f5027a414ffeebaefa92285cc84fa030f4f70656e5468726561642d653439630102e49c0410b92f8c7fbb4f9f3e08492ee3915fbd2f0c0402a0fff8
    ```

#### Setting Wi-Fi network credentials

Assuming your Wi-Fi SSID is _TESTSSID_, and your Wi-Fi password is _P455W4RD_,
set the credentials to the controller by executing the following command:

```
chip-device-ctrl > set-pairing-wifi-credential TESTSSID P455W4RD
```

**REPL Command**: `devCtrl.SetWiFiCredentials(<ssid>, <password>)`

### Step 5: Commission the Matter accessory device over Bluetooth LE

The controller uses a 12-bit value called **discriminator** to discern between
multiple commissionable device advertisements, as well as a 27-bit **setup PIN
code** to authenticate the device. You can find these values in the logging
terminal of the device (for example, UART). For example:

```
I: 254 [DL]Device Configuration:
I: 257 [DL] Serial Number: TEST_SN
I: 260 [DL] Vendor Id: 65521 (0xFFF1)
I: 263 [DL] Product Id: 32768 (0x8000)
I: 267 [DL] Hardware Version: 1
I: 270 [DL] Setup Pin Code: 20202021
I: 273 [DL] Setup Discriminator: 3840 (0xF00)
I: 278 [DL] Manufacturing Date: (not set)
I: 281 [DL] Device Type: 65535 (0xFFFF)
```

Run the following command to establish the secure connection over Bluetooth LE,
with the following assumptions for the Matter accessory device:

-   The discriminator of the device is _3840_
-   The setup pin code of the device is _20202021_
-   The temporary Node ID is _1234_

```
chip-device-ctrl > connect -ble 3840 20202021 1234
```

**REPL Command:**
`devCtrl.ConnectBLE(<discriminator>, <setup pincode>, <temporary node id>)`

You can skip the last parameter, the Node ID, in the command. If you skip it,
the controller will assign it randomly. In that case, note down the Node ID,
because it is required later in the configuration process.

After connecting the device over Bluetooth LE, the controller will go through
the following stages:

-   Establishing a secure connection that completes the PASE
    (Password-Authenticated Session Establishment) session using SPAKE2+
    protocol and results in printing the following log:

        ```
        Secure Session to Device Established
        ```

-   Providing the device with a network interface using ZCL Network
    Commissioning cluster commands, and the network pairing credentials set in
    the previous step.
-   Discovering the IPv6 address of the Matter accessory using the SRP (Service
    Registration Protocol) for Thread devices, or the mDNS (Multicast Domain
    Name System) protocol for Wi-Fi or Ethernet devices. It results in printing
    log that indicates that the node address has been updated. The IPv6 address
    of the device is cached in the controller for later usage.
-   Closing the Bluetooth LE connection, as the commissioning process is
    finished and the Python CHIP controller is now using only the IPv6 traffic
    to reach the device.

### Step 6: Control application ZCL clusters.

For the light bulb example, execute the following command to toggle the LED
state:

```
chip-device-ctrl > zcl OnOff Toggle 1234 1 0
```

**REPL Command:**
`await devCtrl.SendCommand(1234, 1, Clusters.OnOff.Commands.Toggle())`

To change the brightness of the LED, use the following command, with the level
value somewhere between 0 and 255.

```
chip-device-ctrl > zcl LevelControl MoveToLevel 1234 1 0 level=50
```

**REPL Command:**
`await devCtrl.SendCommand(1234, 1, LevelControl.Commands.MoveToLevel(level=50, transitionTime=Null, optionsMask=0, optionsOverride=0))`

### Step 7: Read basic information out of the accessory.

Every Matter accessory device supports a Basic Information Cluster, which
maintains collection of attributes that a controller can obtain from a device,
such as the vendor name, the product name, or software version. Use `zclread`
command to read those values from the device:

```
chip-device-ctrl > zclread BasicInformation VendorName 1234 1 0
chip-device-ctrl > zclread BasicInformation ProductName 1234 1 0
chip-device-ctrl > zclread BasicInformation SoftwareVersion 1234 1 0
```

**REPL Command:**
`await devCtrl.ReadAttribute(1234, [(1, Clusters.BasicInformation.Attributes.VendorName)])`

> Use the `zcl ? BasicInformation` command to list all available commands for
> Basic Information Cluster.
>
> In REPL, you can type `Clusters.BasicInformation.Attributes.` and then use the
> TAB key.

<hr>

## List of commands

### `ble-adapter-print`

> BLE adapter operations is not yet supported in REPL

Print the available Bluetooth adapters on device. Takes no arguments:

```
chip-device-ctrl > ble-adapter-print
2021-03-04 16:09:40,930 ChipBLEMgr   INFO     AdapterName: hci0   AdapterAddress: 00:AA:01:00:00:23
```

### `ble-debug-log`

> BLE adapter operations is not yet supported in REPL

Enable the Bluetooth LE debug logs.

```
chip-device-ctrl > ble-debug-log 1
```

### `ble-scan [-t <timeout>] [identifier]`

> BLE adapter operations is not yet supported in REPL

Start a scan action to search for valid CHIP devices over Bluetooth LE (for at
most _timeout_ seconds). Stop when the device is matching the identifier or the
counter times out.

```
chip-device-ctrl > ble-scan
2021-05-29 22:28:05,461 ChipBLEMgr   INFO     scanning started
2021-05-29 22:28:07,206 ChipBLEMgr   INFO     Name            = ChipLight
2021-05-29 22:28:07,206 ChipBLEMgr   INFO     ID              = f016e23d-0d00-35d5-93e7-588acdbc7e54
2021-05-29 22:28:07,207 ChipBLEMgr   INFO     RSSI            = -79
2021-05-29 22:28:07,207 ChipBLEMgr   INFO     Address         = E0:4D:84:3C:BB:C3
2021-05-29 22:28:07,209 ChipBLEMgr   INFO     Pairing State   = 0
2021-05-29 22:28:07,209 ChipBLEMgr   INFO     Discriminator   = 3840
2021-05-29 22:28:07,209 ChipBLEMgr   INFO     Vendor Id       = 9050
2021-05-29 22:28:07,209 ChipBLEMgr   INFO     Product Id      = 20044
2021-05-29 22:28:07,210 ChipBLEMgr   INFO     Adv UUID        = 0000fff6-0000-1000-8000-00805f9b34fb
2021-05-29 22:28:07,210 ChipBLEMgr   INFO     Adv Data        = 00000f5a234c4e
2021-05-29 22:28:07,210 ChipBLEMgr   INFO
2021-05-29 22:28:16,246 ChipBLEMgr   INFO     scanning stopped
```

### `set-pairing-thread-credential <threadOperationalDataset>`

Provides the controller with Thread network credentials that will be used in the
device commissioning procedure to configure the device with a Thread interface.

```
chip-device-ctrl > set-pairing-thread-credential 0e080000000000010000000300001335060004001fffe002084fe76e9a8b5edaf50708fde46f999f0698e20510d47f5027a414ffeebaefa92285cc84fa030f4f70656e5468726561642d653439630102e49c0410b92f8c7fbb4f9f3e08492ee3915fbd2f0c0402a0fff8
```

**REPL Commands:**
`devCtrl.SetThreadOperationalDataset(bytes.FromHex("0e080000000000010000000300001335060004001fffe002084fe76e9a8b5edaf50708fde46f999f0698e20510d47f5027a414ffeebaefa92285cc84fa030f4f70656e5468726561642d653439630102e49c0410b92f8c7fbb4f9f3e08492ee3915fbd2f0c0402a0fff8"))`

### `set-pairing-wifi-credential <ssid> <credentials>`

Provides the controller with Wi-Fi network credentials that will be used in the
device commissioning procedure to configure the device with a Wi-Fi interface.

```
chip-device-ctrl > set-pairing-wifi-credential TESTSSID P455W4RD
```

**REPL Commands:** `devCtrl.SetWiFiCredentials('TESTSSID', 'P455W4RD')`

### `connect -ip <address> <SetUpPinCode> [<nodeid>]`

Do key exchange and establish a secure session between controller and device
using IP transport.

The Node ID will be used by controller to distinguish multiple devices. This
does not match the spec and will be removed later. The nodeid will not be
persisted by controller / device.

If no nodeid given, a random Node ID will be used.

**REPL Commands:**
`devCtrl.CommissionIP(b'<ip address>', <setup pin code>, <nodeid>)`

### `connect -ble <discriminator> <SetUpPinCode> [<nodeid>]`

Do key exchange and establish a secure session between controller and device
using Bluetooth LE transport.

The Node ID will be used by controller to distinguish multiple devices. This
does not match the spec and will be removed later. The nodeid will not be
persisted by controller / device.

If no nodeid given, a random Node ID will be used.

**REPL Commands:**
`devCtrl.ConnectBLE(<discriminator>, <setup pin code>, <nodeid>)`

### `close-session <nodeid>`

If case there exists an open session (PASE or CASE) to the device with a given
Node ID, mark it as expired.

**REPL Commands:** `devCtrl.CloseSession(<nodeid>)`

### `discover`

> To be implemented in REPL

Discover available Matter accessory devices:

```
chip-device-ctrl > discover -all
```

### `resolve <node_id>`

> To be implemented in REPL

Resolve DNS-SD name corresponding with the given Node ID and update address of
the node in the device controller:

```
chip-device-ctrl > resolve 1234
```

### `setup-payload generate [-v <Vendor ID>] [-p <Product ID>] [-cf <Custom Flow>] [-dc <Discovery Capabilities>] [-dv <Discriminator Value>] [-ps <Passcode>]`

> To be implemented in REPL

Print the generated Onboarding Payload Contents in human-readable (Manual
Pairing Code) and machine-readable (QR Code) format:

```
chip-device-ctrl > setup-payload generate -v 9050 -p 65279 -cf 0 -dc 2 -dv 2976 -ps 34567890
Manual pairing code: [26318621095]
SetupQRCode: [MT:YNJV7VSC00CMVH7SR00]
```

### `setup-payload parse-manual <manual-pairing-code>`

> To be implemented in REPL

Print the commissioning information encoded in the Manual Pairing Code:

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

> To be implemented in REPL

Print the commissioning information encoded in the QR Code payload:

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

### `zcl <Cluster> <Command> <NodeId> <EndpointId> <GroupId> [arguments]`

Send a ZCL command to the device. For example:

```
chip-device-ctrl > zcl LevelControl MoveWithOnOff 12344321 1 0 moveMode=1 rate=2
```

**Format of arguments**

For any integer and char string (null terminated) types, just use `key=value`,
for example: `rate=2`, `string=123`, `string_2="123 456"`

For byte string type, use `key=encoding:value`, currently, we support `str` and
`hex` encoding, the `str` encoding will encode a NULL terminated string. For
example, `networkId=hex:0123456789abcdef` (for
`[0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef]`), `ssid=str:Test` (for
`['T', 'e', 's', 't', 0x00]`).

For boolean type, use `key=True` or `key=False`.

**REPL Commands:**

```python
# await devCtrl.SendCommand(<nodeid>, <endpoint>, Clusters.<cluster>.Commands.<command>(<arguments>))
# e.g.
await devCtrl.SendCommand(12344321, 1, Clusters.LevelControl.Commands.MoveWithOnOff(moveMode=1, rate=2, optionsMask=0, optionsOverride=0))
```

### `zcl ?`

List available clusters:

```
chip-device-ctrl > zcl ?
AccountLogin
ApplicationBasic
ApplicationLauncher
AudioOutput
BarrierControl
BasicInformation
Binding
BridgedDeviceBasicInformation
ColorControl
ContentLaunch
Descriptor
DoorLock
EthernetNetworkDiagnostics
FixedLabel
GeneralCommissioning
GeneralDiagnostics
GroupKeyManagement
Groups
Identify
KeypadInput
LevelControl
LowPower
MediaInput
MediaPlayback
NetworkCommissioning
OnOff
OperationalCredentials
PumpConfigurationAndControl
RelativeHumidityMeasurement
ScenesManagement
SoftwareDiagnostics
Switch
Channel
TargetNavigator
TemperatureMeasurement
TestCluster
Thermostat
TrustedRootCertificates
WakeOnLan
WindowCovering
```

**REPL Commands**

Type `Clusters.` and hit TAB

### `zcl ? <Cluster>`

List available commands in cluster. For example, for _Basic Information_
cluster:

```
chip-device-ctrl > zcl ? BasicInformation
DataModelRevision
VendorName
VendorID
ProductName
ProductID
UserLabel
Location
HardwareVersion
HardwareVersionString
SoftwareVersion
SoftwareVersionString
ManufacturingDate
PartNumber
ProductURL
ProductLabel
SerialNumber
LocalConfigDisabled
ClusterRevision
```

**REPL Commands**

Type `Clusters.(cluster name).Commands.` and hit TAB

### `zclread <Cluster> <Attribute> <NodeId> <EndpointId> <GroupId> [arguments]`

Read the value of ZCL attribute. For example:

```
chip-device-ctrl > zclread BasicInformation VendorName 1234 1 0
```

**REPL Commands**

```python
# devCtrl.ReadAttribute(<nodeid>, [(<endpoint id>, Clusters.<cluster>.Attributes.<attribute>)])
# e.g.
await devCtrl.ReadAttribute(1234, [(1, Clusters.BasicInformation.Attributes.VendorName)])
```

### `zclwrite <cluster> <attribute> <nodeid> <endpoint> <groupid> <value>`

Write the value to a ZCL attribute. For example:

```
chip-device-ctrl > zclwrite TestCluster Int8u 1 1 0 1
chip-device-ctrl > zclwrite TestCluster Boolean 1 1 0 True
chip-device-ctrl > zclwrite TestCluster OctetString 1 1 0 str:123123
chip-device-ctrl > zclwrite TestCluster CharString 1 1 0 233233
```

Note: The format of the value is the same as the format of argument values for
ZCL cluster commands.

**REPL Commands**

```python
# devCtrl.WriteAttribute(<nodeid>, [(<endpointid>, Clusters.<cluster>.Attributes.<attribute>(value=<attribute value>))])
# e.g.
await devCtrl.WriteAttribute(1, [(1, Clusters.UnitTesting.Attributes.Int8u(value=1))])
await devCtrl.WriteAttribute(1, [(1, Clusters.UnitTesting.Attributes.Boolean(value=True))])
await devCtrl.WriteAttribute(1, [(1, Clusters.UnitTesting.Attributes.OctetString(value=b'123123\x00'))])
await devCtrl.WriteAttribute(1, [(1, Clusters.UnitTesting.Attributes.CharString(value='233233'))])
```

### `zclsubscribe <Cluster> <Attribute> <Nodeid> <Endpoint> <MinInterval> <MaxInterval>`

Configure ZCL attribute reporting settings. For example:

```
chip-device-ctrl > zclsubscribe OccupancySensing Occupancy 1234 1 10 20
```

**REPL Commands**

```python
# devCtrl.ReadAttribute(<nodeid>, [(<endpoint>, Clusters.<cluster>.Attributes.<attribute>)], reportInterval=(<min interval>, <max interval>))
# e.g.
await devCtrl.ReadAttribute(1, [(1, Clusters.OccupancySensing.Attributes.Occupancy)], reportInterval=(10, 20))
```

### `zclsubscribe -shutdown <subscription id>`

Shutdown an existing attribute subscription.

```
chip-device-ctrl > zclsubscribe -shutdown 0xdeadbeefcafe
```

The subscription id can be obtained from previous subscription messages:

```
chip-device-ctrl > zclsubscribe OnOff OnOff 1 1 10 20
(omitted messages)
[1633922898.965587][1117858:1117866] CHIP:DMG: SubscribeResponse =
[1633922898.965599][1117858:1117866] CHIP:DMG: {
[1633922898.965610][1117858:1117866] CHIP:DMG:  SubscriptionId = 0xdeadbeefcafe,
[1633922898.965622][1117858:1117866] CHIP:DMG:  MinIntervalFloorSeconds = 0xa,
[1633922898.965633][1117858:1117866] CHIP:DMG:  MaxIntervalCeilingSeconds = 0x14,
[1633922898.965644][1117858:1117866] CHIP:DMG: }
[1633922898.965662][1117858:1117866] CHIP:ZCL: SubscribeResponse:
[1633922898.965673][1117858:1117866] CHIP:ZCL:   SubscriptionId:        0xdeadbeefcafe
[1633922898.965683][1117858:1117866] CHIP:ZCL:   ApplicationIdentifier: 0
[1633922898.965694][1117858:1117866] CHIP:ZCL:   status: EMBER_ZCL_STATUS_SUCCESS (0x00)
[1633922898.965709][1117858:1117866] CHIP:ZCL:   attributeValue: false
(omitted messages)
```

The subscription id is `0xdeadbeefcafe` in this case

**REPL Commands**

```python
# SubscriptionTransaction.Shutdown()
# e.g.
sub = await devCtrl.ReadAttribute(1, [(1, Clusters.OccupancySensing.Attributes.Occupancy)], reportInterval=(10, 20))
sub.Shutdown()
```
