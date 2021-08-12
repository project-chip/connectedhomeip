# Working with Python CHIP Controller

The Python CHIP controller is a tool that allows to commission a Matter device
into the network and to communicate with it using the Zigbee Cluster Library
(ZCL) messages.

<hr>

-   [Source files](#source)
-   [Building Android CHIPTool](#building)
-   [Running the tool](#running)
-   [Using Python CHIP Controller for Matter accessory testing](#using)
-   [List of commands](#commands)

<hr>

<a name="source"></a>

## Source files

You can find source files of the Python CHIP Controller tool in the
`src/controller/python` directory.

The tool uses the generic CHIP Device Controller library, available in the
`src/controller` directory.

<hr>

<a name="building"></a>

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
    scripts/build_python.sh -m platform
    ```

    > Note: To get more details about available build configurations, run the
    > following command: `scripts/build_python.sh --help`

<hr>

<a name="running"></a>

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

<a name="using"></a>

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

### Step 4: Connect to Matter accessory device over Bluetooth LE

The controller uses a 12-bit value called **discriminator** to discern between
multiple commissionable device advertisements. Moreover, a 27-bit **PIN code**
is used by the controller to authenticate in the device. You can find those
values in the logging terminal of the device (for example, UART). For example:

```
I: 254 [DL]Device Configuration:
I: 257 [DL] Serial Number: TEST_SN
I: 260 [DL] Vendor Id: 9050 (0x235A)
I: 263 [DL] Product Id: 20043 (0x4E4B)
I: 267 [DL] Product Revision: 1
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

You can skip the last parameter, that is the Node ID. If you skip it, the
controller will assign it randomly. However, note the Node ID down, because it
is required later in the configuration process.

At the end of the secure connection establishment, the Python controller prints
the following log:

```
Secure Session to Device Established
```

This means that the PASE (Password-Authenticated Session Establishment) session
using SPAKE2+ protocol is completed.

### Step 5: Commission Matter accessory to the underlying network

The main goal of the network commissioning step is to configure the device with
a network interface, such as Thread or Wi-Fi. This process provides the device
with network credentials.

#### Commissioning a Thread device

1. Fetch and store the current Active Operational Dataset and Extended PAN ID
   from the Thread Border Router. Depending if Thread Border Router is running
   on Docker or natively on Raspberry Pi, execute the following commands:

    - For Docker:

        ```
        sudo docker exec -it otbr sh -c "sudo ot-ctl dataset active -x"
        0e080000000000010000000300001335060004001fffe002084fe76e9a8b5edaf50708fde46f999f0698e20510d47f5027a414ffeebaefa92285cc84fa030f4f70656e5468726561642d653439630102e49c0410b92f8c7fbb4f9f3e08492ee3915fbd2f0c0402a0fff8
        Done

        sudo docker exec -it otbr sh -c "sudo ot-ctl dataset extpanid”
        4fe76e9a8b5edaf5
        Done
        ```

    - For native installation:

        ```
        sudo ot-ctl dataset active -x
        0e080000000000010000000300001335060004001fffe002084fe76e9a8b5edaf50708fde46f999f0698e20510d47f5027a414ffeebaefa92285cc84fa030f4f70656e5468726561642d653439630102e49c0410b92f8c7fbb4f9f3e08492ee3915fbd2f0c0402a0fff8
        Done

        sudo ot-ctl dataset extpanid
        4fe76e9a8b5edaf5
        Done
        ```

    Matter specifiction does not define how the Thread or Wi-Fi credentials are
    obtained by Controller. For example, for Thread, instead of fetching
    datasets directly from the Thread Border Router, you might also use a
    different out-of-band method.

2. Inject the previously obtained Active Operational Dataset as hex-encoded
   value using ZCL Network Commissioning cluster:

    > Each ZCL command has a following format:
    > `zcl <Cluster> <Command> <Node Id> <Endpoint Id> <Group Id> [arguments]`

    ```
    chip-device-ctrl > zcl NetworkCommissioning AddThreadNetwork 1234 0 0 operationalDataset=hex:0e080000000000010000000300001335060004001fffe002084fe76e9a8b5edaf50708fde46f999f0698e20510d47f5027a414ffeebaefa92285cc84fa030f4f70656e5468726561642d653439630102e49c0410b92f8c7fbb4f9f3e08492ee3915fbd2f0c0402a0fff8 breadcrumb=0 timeoutMs=3000
    ```

3. Enable Thread interface on the device by executing the following command with
   `networkID` equal to Extended PAN Id of the Thread network:

    ```
    chip-device-ctrl > zcl NetworkCommissioning EnableNetwork 1234 0 0 networkID=hex:4fe76e9a8b5edaf5 breadcrumb=0 timeoutMs=3000
    ```

#### Commissioning a Wi-Fi device

1. Assuming your Wi-Fi SSID is _TESTSSID_, and your Wi-Fi password is
   _P455W4RD_, inject the credentials to the device by excuting the following
   command:

    ```
    chip-device-ctrl > zcl NetworkCommissioning AddWiFiNetwork 1234 0 0 ssid=str:TESTSSID credentials=str:P455W4RD breadcrumb=0 timeoutMs=1000
    ```

2. Enable the Wi-Fi interface on the device by executing the following command:

    ```
    chip-device-ctrl > zcl NetworkCommissioning EnableNetwork 1234 0 0 networkID=str:TESTSSID breadcrumb=0 timeoutMs=1000
    ```

### Step 6: Close Bluetooth LE connection.

After the Matter accessory device was provisioned with Thread or Wi-Fi
credentials (or both), the commissioning process is finished. The Python CHIP
controller is now using only the IPv6 traffic to reach the device, so you can
close the Bluetooth LE connection. To close the connection, run the following
command:

```
chip-device-ctrl > close-ble
```

### Step 7: Discover IPv6 address of the Matter accessory.

The Matter controller must discover the IPv6 address of the node that it
previously commissioned. Depending on the network type:

-   For Thread, the Matter accessory uses SRP (Service Registration Protocol) to
    register its presence on the Thread Border Router’s SRP Server.
-   For Wi-Fi or Ethernet devices, the Matter accessory uses the mDNS (Multicast
    Domain Name System) protocol.

Assuming your Fabric ID is _5544332211_ and Node ID is _1234_ (use the Node ID
you noted down when you established the secure connection over Bluetooth LE)),
run the following command:

```
chip-device-ctrl > resolve 5544332211 1234
```

A notification in the log indicates that the node address has been updated. The
IPv6 address of the device is cached in the controller for later usage.

### Step 8: Control application ZCL clusters.

For the light bulb example, execute the following command to toggle the LED
state:

```
chip-device-ctrl > zcl OnOff Toggle 1234 1 0
```

To change the brightness of the LED, use the following command, with the level
value somewhere between 0 and 255.

```
chip-device-ctrl > zcl LevelControl MoveToLevel 1234 1 0 level=50
```

### Step 9: Read basic information out of the accessory.

Every Matter accessory device supports a Basic Cluster, which maintains
collection of attributes that a controller can obtain from a device, such as the
vendor name, the product name, or software version. Use `zclread` command to
read those values from the device:

```
chip-device-ctrl > zclread Basic VendorName 1234 1 0
chip-device-ctrl > zclread Basic ProductName 1234 1 0
chip-device-ctrl > zclread Basic SoftwareVersion 1234 1 0
```

> Use the `zcl ? Basic` command to list all available commands for Basic
> Cluster.

<hr>

<a name="commands"></a>

## List of commands

### `ble-adapter-print`

Print the available Bluetooth adapters on device. Takes no arguments:

```
chip-device-ctrl > ble-adapter-print
2021-03-04 16:09:40,930 ChipBLEMgr   INFO     AdapterName: hci0   AdapterAddress: 00:AA:01:00:00:23
```

### `ble-debug-log`

Enable the Bluetooth LE debug logs.

```
chip-device-ctrl > ble-debug-log 1
```

### `ble-scan [-t <timeout>] [identifier]`

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

### `connect -ip <address> <SetUpPinCode> [<nodeid>]`

Do key exchange and establish a secure session between controller and device
using IP transport.

The Node ID will be used by controller to distinguish multiple devices. This
does not match the spec and will be removed later. The nodeid will not be
persisted by controller / device.

If no nodeid given, a random Node ID will be used.

### `connect -ble <discriminator> <SetUpPinCode> [<nodeid>]`

Do key exchange and establish a secure session between controller and device
using Bluetooth LE transport.

The Node ID will be used by controller to distinguish multiple devices. This
does not match the spec and will be removed later. The nodeid will not be
persisted by controller / device.

If no nodeid given, a random Node ID will be used.

### `close-session <nodeid>`

If case there eixsts an open session (PASE or CASE) to the device with a given
Node ID, mark it as expired.

### `discover`

Discover available Matter accessory devices:

```
chip-device-ctrl > discover -all
```

### `resolve <fabric_id> <node_id>`

Resolve DNS-SD name corresponding with the given fabric and Node IDs and update
address of the node in the device controller:

```
chip-device-ctrl > resolve 5544332211 1234
```

### `setup-payload generate [-v <Vendor ID>] [-p <Product ID>] [-cf <Custom Flow>] [-dc <Discovery Capabilities>] [-dv <Discriminator Value>] [-ps <Passcode>]`

Print the generated Onboarding Payload Contents in human-readable (Manual
Pairing Code) and machine-readable (QR Code) format:

```
chip-device-ctrl > setup-payload generate -v 9050 -p 65279 -cf 0 -dc 2 -dv 2976 -ps 34567890
Manual pairing code: [26318621095]
SetupQRCode: [MT:YNJV7VSC00CMVH7SR00]
```

### `setup-payload parse-manual <manual-pairing-code>`

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

For byte string type, use `key=encoding:value`, currectly, we support `str` and
`hex` encoding, the `str` encoding will encode a NULL terminated string. For
example, `networkId=hex:0123456789abcdef` (for
`[0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef]`), `ssid=str:Test` (for
`['T', 'e', 's', 't', 0x00]`).

For boolean type, use `key=True` or `key=False`.

### `zcl ?`

List available clusters:

```
chip-device-ctrl > zcl ?
AccountLogin
ApplicationBasic
ApplicationLauncher
AudioOutput
BarrierControl
Basic
Binding
BridgedDeviceBasic
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
Scenes
SoftwareDiagnostics
Switch
TvChannel
TargetNavigator
TemperatureMeasurement
TestCluster
Thermostat
TrustedRootCertificates
WakeOnLan
WindowCovering
```

### `zcl ? <Cluster>`

List available commands in cluster. For example, for _Basic_ cluster:

```
chip-device-ctrl > zcl ? Basic
InteractionModelVersion
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

### `zclread <Cluster> <Attribute> <NodeId> <EndpointId> <GroupId> [arguments]`

Read the value of ZCL attribute. For example:

```
chip-device-ctrl > zclread Basic VendorName 1234 1 0
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

### `zclconfigure <Cluster> <Attribute> <Nodeid> <Endpoint> <MinInterval> <MaxInterval> <Change>`

Configure ZCL attribute reporting settings. For example:

```
chip-device-ctrl > zclconfigure OccupancySensing Occupancy 1234 1 0 1000 2000 1
```
