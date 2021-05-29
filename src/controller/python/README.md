# Python CHIP Device Controller

## Overview

The Python CHIP controller allows to commission the Matter device into
the network as well as communicate with it using the Zigbee Cluster Library (ZCL) messages. The tool utilizes the generic [Chip Device Controller](../) library.

Below instruction presents how to build and test Python CHIP controller. The sample test flow may vary depending on the application clusters implemented on the device side.

To learn more about advanced usage of the tool, check [extended documentation](ADVANCED_USAGE.md).

## Checkout / Build / Install

Before you can use the Python controller, you must compile it from the source on Linux (amd64 / aarch64) or macOS.

> To ensure compatibility, build the Python CHIP controller and the Matter device from the same revision of the connectedhomeip repository.

To build and run the Python CHIP controller:

1. Install all necessary packages and prepare the build system. For more details, see the [BUILDING.md](/docs/BUILDING.md) documentation.

   ```
   sudo apt-get update
   sudo apt-get upgrade

   sudo apt-get install git gcc g++ python pkg-config libssl-dev libdbus-1-dev libglib2.0-dev libavahi-client-dev ninja-build python3-venv python3-dev python3-pip unzip libgirepository1.0-dev libcairo2-dev bluez
   ```

   If the Python CHIP controller is built on a Raspberry Pi, install additional packages and reboot the device:

   ```
   sudo apt-get install pi-bluetooth
   sudo reboot
   ```

2. Clone the Project CHIP repository.

   ```
   git clone https://github.com/project-chip/connectedhomeip.git
   ```

3. Enter the `connectedhomeip` directory.

   ```
   cd connectedhomeip
   ```

4. Initialize the git submodules.

   ```
   git submodule update --init
   ```

5. Build and install the Python CHIP controller.

   ```
   scripts/build_python.sh -m platform
   ```

   > Note: To get more details about available build configurations, run the following command:
   > ```scripts/build_python.sh --help```

6. Activate the Python virtual environment.

   ```
   source out/python_env/bin/activate
   ```

7. Run the Python CHIP controller.

   > Running as root is necessary to obtain access to the Bluetooth interface.

   ```
   sudo out/python_env/bin/chip-device-ctrl
   ```

   You can also select the Bluetooth LE interface using command line argument:

   ```
   sudo out/python_env/bin/chip-device-ctrl --bluetooth-adapter=hci2
   ```

## Working with Python CHIP Controller

### 1. Prepare the Matter accessory.

In this tutorial, the [Matter: Light Bulb](/examples/lighting-app) example with Bluetooth LE commissioning will be used, but similar steps can be executed with the rest of the examples. Check [Matter examples](/examples) for the list of available samples.

Build and program the device with the Matter accessory firmware by following the example's documentation, for example [nRF Connect Lighting Example Application](/examples/lightigng-app/nrfconnect).

### 2. Enable Bluetooth LE advertising on Matter accessory device.

Some examples are configured to advertise automatically on boot, others require physical trigger like pushing the button. Follow the Matter accessory example's documentation to learn how Bluetooth LE advertising is enabled.

### 3. Discover Matter accessory device over Bluetooth LE

An uncommissioned accessory device advertises over Bluetooth LE. Run the following command to scan all advertised Matter devices:

```
chip-device-ctrl > ble-scan
```

### 4. Connect to Matter accessory device over Bluetooth LE

The controller uses a 12-bit value to discern between multiple commissionable device advertisements which is called **Discriminator**. The Matter accessory verifies the Controller by checking if the setup code is in his possession. You can find those values in the logging terminal of the device (e.g. UART), as listed below.

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

Run the following command to establish secure connection over Bluetooth LE, with the following assumptions for the Matter accessory device:
 - The discriminator of the device is 3840
 - The setup pin code of the device is 20202021
 - The temporary Node ID is 1234

```
chip-device-ctrl > connect -ble 3840 20202021 1234
```

You may skip providing the last parameter which is a Node ID. In such case, the controller will randomly assign it. However, note the Node ID down, because it is required later in the configuration process.

At the end of the secure connection establishment, the Python controller will indicate this by printing the following log:

```
Secure Session to Device Established
```

This means that the PASE (Password-Authenticated Session Establishment) session using SPAKE2+ protocol is completed.

### 5. Commission Matter accessory to the underlying network

One of the commissioning steps is Network Commissioning. The main goal of this step is to configure network interface, such as Thread or Wifi, and to provide network credentials.

#### Commissioning of Thread device

1. Fetch and store the current Active Operational Dataset and Extended PAN ID from the Thread Border Router.

   Depending if Thread Border Router is running on Docker or natively on Raspberry Pi, execute the following commands:

   For Docker:
   ```
   sudo docker exec -it otbr sh -c "sudo ot-ctl dataset active -x"
   0e080000000000010000000300001335060004001fffe002084fe76e9a8b5edaf50708fde46f999f0698e20510d47f5027a414ffeebaefa92285cc84fa030f4f70656e5468726561642d653439630102e49c0410b92f8c7fbb4f9f3e08492ee3915fbd2f0c0402a0fff8
   Done

   sudo docker exec -it otbr sh -c "sudo ot-ctl dataset extpanid”
   4fe76e9a8b5edaf5
   Done
   ```

   For native installation:

   ```
   sudo ot-ctl dataset active -x
   0e080000000000010000000300001335060004001fffe002084fe76e9a8b5edaf50708fde46f999f0698e20510d47f5027a414ffeebaefa92285cc84fa030f4f70656e5468726561642d653439630102e49c0410b92f8c7fbb4f9f3e08492ee3915fbd2f0c0402a0fff8
   Done

   sudo ot-ctl dataset extpanid
   4fe76e9a8b5edaf5
   Done
   ```

   Matter specifiction does not define how the Thread or Wi-Fi credentials are obtained by Controller. For example, for Thread, instead of fetching datasets directly from the Thread Border Router, you may also use different out-of-band method.

2. Inject the previously obtained Active Operational Dataset as hex-encoded value using ZCL Network Commissioning cluster.

   > Each ZCL command has a following format:
      `zcl <Cluster> <Command> <Node Id> <Endpoint Id> <Group Id> [arguments]`
   >
   > Use the `zcl ? <Cluster>` command to list all available commands for given ZCL cluster.

   ```
   chip-device-ctrl > zcl NetworkCommissioning AddThreadNetwork 1234 0 0 operationalDataset=hex:0e080000000000010000000300001335060004001fffe002084fe76e9a8b5edaf50708fde46f999f0698e20510d47f5027a414ffeebaefa92285cc84fa030f4f70656e5468726561642d653439630102e49c0410b92f8c7fbb4f9f3e08492ee3915fbd2f0c0402a0fff8 breadcrumb=0 timeoutMs=3000
   ```

3. Enable Thread interface in the device by executing the following command with `networkID` equal to Extended PAN Id of the Thread network.

   ```
   chip-device-ctrl > zcl NetworkCommissioning EnableNetwork 1234 0 0 networkID=hex:4fe76e9a8b5edaf5 breadcrumb=0 timeoutMs=3000
   ```

#### Commissioning of Wi-Fi device

1. Assuming your Wi-Fi SSID is *TESTSSID*, and your Wi-Fi password is *P455W4RD*, inject the credentials to the device by excuting the following command.

   ```
   chip-device-ctrl > zcl NetworkCommissioning AddWiFiNetwork 1234 0 0 ssid=str:TESTSSID credentials=str:P455W4RD breadcrumb=0 timeoutMs=1000
   ```

2. Enable Wi-Fi interface in the device by executing the following command:

   ```
   chip-device-ctrl > zcl NetworkCommissioning EnableNetwork 1234 0 0 networkID=str:TESTSSID breadcrumb=0 timeoutMs=1000
   ```

### 6. Close Bluetooth LE connection.

After Matter accessory was provisioned with Thread and/or Wi-Fi credentials, the commissioning process is finished. Python CHIP controller will now use only IPv6 traffic to reach the device, so Bluetooth LE connection can be closed.

```
chip-device-ctrl > close-ble
```

### 7. Discover IPv6 address of the Matter accessory.

The Matter controller needs to discover IPv6 address of the node that it previously commissioned. For Thread, the Matter accessory uses SRP (Service Registration Protocol) to register its presence on the Thread Border Router’s SRP Server, for Wi-Fi or Ethernet devices, the mDNS (Multicast Domain Name System) protocol is used instead.

Run the following command, assuming that:
 - The Fabric ID of the device is 5544332211
 - The Node ID is 1234

```
chip-device-ctrl > resolve 5544332211 1234
```

After successful resolution, you should see the log indicating that node address has been updated. The IPv6 address of the device will be cached in the controller for later usage.

### 8. Control application ZCL clusters.

For Light Bulb example, execute the following command to toggle the LED state:

```
chip-device-ctrl > zcl OnOff Toggle 1234 1 0
```

To change the brightness of the LED, use the following command, with level changed to the value between 0 and 255.

```
chip-device-ctrl > zcl LevelControl MoveToLevel 1234 1 0 level=50
```

### 9. Read basic information out of the accessory.

Every Matter accessory device supports Basic Cluster which maintains collection of attributes that a controller may obtain from a device, such as the vendor name, the product name, or software version. Use `zclread` command to read those values out of the device:

```
chip-device-ctrl > zclread Basic VendorName 1234 1 0
chip-device-ctrl > zclread Basic ProductName 1234 1 0
chip-device-ctrl > zclread Basic SoftwareVersion 1234 1 0
```

## List of commands

### `ble-adapter-print`

Print the available Bluetooth adapters on device. Takes no arguments.

```
chip-device-ctrl > ble-adapter-print
2021-03-04 16:09:40,930 ChipBLEMgr   INFO     AdapterName: hci0   AdapterAddress: 00:AA:01:00:00:23
```

### `ble-debug-log`

Enable Bluetooth LE debug logs.

```
chip-device-ctrl > ble-debug-log 1
```

### `ble-scan [-t <timeout>] [identifier]`

Start a ble-scan action for searching valid CHIP devices over BLE [for at most
*timeout* seconds], stop when device matching the identifier or timeout.

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

### `discover`

Discover available Matter accessory devices.

```
chip-device-ctrl > discover -all
```

### `resolve <fabric_id> <node_id>`

Resolve DNS-SD name corresponding with the given fabric and node IDs and update address of the node in the device controller.

```
chip-device-ctrl > resolve 5544332211 1234
```

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

### `zcl <Cluster> <Command> <NodeId> <EndpointId> <GroupId> [arguments]`

Send a ZCL command the device. For example:

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

### `zcl ?`

List available clusters.

```
chip-device-ctrl > zcl ?
dict_keys(['BarrierControl', 'Basic', 'ColorControl', 'DoorLock', 'Groups', 'IasZone', 'Identify', 'LevelControl', 'NetworkProvisioning', 'OnOff', 'Scenes', 'TemperatureMeasurement'])
```

### `zcl ? <Cluster>`

List available commands in cluster. For example, for *LevlControl* cluster:

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

### `zclread <Cluster> <Attribute> <NodeId> <EndpointId> <GroupId> [arguments]`

Read the value of ZCL attribute. For example:

```
chip-device-ctrl > zclread Basic VendorName 1234 1 0
```

#### `zclconfigure <Cluster> <Attribute> <Nodeid> <Endpoint> <MinInterval> <MaxInterval> <Change>`

Configure ZCL attribute reporting settings. For example:

```
chip-device-ctrl > zclconfigure OccupancySensing Occupancy 1234 1 0 1000 2000 1
```
