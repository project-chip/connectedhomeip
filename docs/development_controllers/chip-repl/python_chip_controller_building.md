# Working with Python CHIP Controller

The Python CHIP controller is a library that allows to create a Matter fabric
and commission Matter devices with it.

The `chip-repl` is a REPl which sets up a Python CHIP Controller and allows to
explore the Python CHIP Controller API and communicate with devices from the
command line.

<hr>

-   [Source files](#source-files)
-   [Building Python CHIP Controller](#building-and-installing)
-   [Running the CHIP REPL](#running-the-chip-repl)
-   [Using Python CHIP Controller REPL for Matter accessory testing](#using-python-chip-controller-repl-for-matter-accessory-testing)
-   [Example usage of the Python CHIP Controller REPL](#example-usage-of-the-python-chip-controller-repl)
-   [Explore Clusters, Attributes and Commands](#explore-clusters-attributes-and-commands)

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
   details, see the [Building Matter](../../guides/BUILDING.md) documentation:

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
    scripts/build_python.sh -m platform -i out/python_env
    source out/python_env/bin/activate
    ```

    > Note: This builds the Python CHIP Controller along with the CHIP REPL as
    > Python wheels and installs it into a separate Python virtual environment.
    > To get more details about available build configurations, run the
    > following command: `scripts/build_python.sh --help`

<hr>

## Running the CHIP REPL

1. Activate the Python virtual environment with the Python CHIP Controller
   installed:

    ```
    source out/python_env/bin/activate
    ```

2. Run the CHIP REPL to explore the API of the Python CHIP controller:

    ```
    chip-repl
    ```

NOTE: To get more verbose logs, pass the debug flag: `chip-repl --debug`

<hr>

## Using Python CHIP Controller REPL for Matter accessory testing

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

### Step 3: Discover commissionable Matter accessory device

An uncommissioned accessory device advertises over Bluetooth LE or via mDNS if
already on the network. Run the following command to scan all advertised Matter
devices:

```python
await devCtrl.DiscoverCommissionableNodes()
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

2. Set the previously obtained Active Operational Dataset as a byte array using
   the following command:

    ```python
    thread_dataset = bytes.fromhex("0e080000000000010000000300001335060004001fffe002084fe76e9a8b5edaf50708fde46f999f0698e20510d47f5027a414ffeebaefa92285cc84fa030f4f70656e5468726561642d653439630102e49c0410b92f8c7fbb4f9f3e08492ee3915fbd2f0c0402a0fff8")
    devCtrl.SetThreadOperationalDataset(thread_dataset)
    ```

#### Setting Wi-Fi network credentials

Assuming your Wi-Fi SSID is _TESTSSID_, and your Wi-Fi password is _P455W4RD_,
set the credentials to the controller by executing the following command:

```python
devCtrl.SetWiFiCredentials(<ssid>, <password>)
```

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

```python
await devCtrl.ConnectBLE(3840, 20202021, 1234)
```

You can skip the last parameter, the Node ID, in the command. If you skip it,
the controller will assign it randomly. In that case, note down the Node ID,
because it is required later in the configuration process.

It is also possible to use the QR setup code instead. It typically is shown on
the terminal of the device as well. For example:

```
CHIP:SVR: SetupQRCode: [MT:-24J0AFN00KA0648G00]
```

Use the following command to commission the device with the QR code:

```python
await devCtrl.CommissionWithCode("MT:-24J0AFN00KA0648G00", 1234)
```

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

### Step 6: Control application clusters.

For the light bulb example, execute the following command to toggle the LED
state:

```python
await devCtrl.SendCommand(1234, 1, Clusters.OnOff.Commands.Toggle())
```

To change the brightness of the LED, use the following command, with the level
value somewhere between 0 and 255.

```python
commandToSend = LevelControl.Commands.MoveToLevel(level=50, transitionTime=Null, optionsMask=0, optionsOverride=0)
await devCtrl.SendCommand(1234, 1, commandToSend)
```

### Step 7: Read basic information out of the accessory.

Every Matter accessory device supports a Basic Information Cluster, which
maintains collection of attributes that a controller can obtain from a device,
such as the vendor name, the product name, or software version. Use
`ReadAttribute()` command to read those values from the device:

```python
attributes = [
    (0, Clusters.BasicInformation.Attributes.VendorName),
    (0, Clusters.BasicInformation.Attributes.ProductName),
    (0, Clusters.BasicInformation.Attributes.SoftwareVersion),
]
await devCtrl.ReadAttribute(1234, attributes)
```

> In REPL, you can type `Clusters.BasicInformation.Attributes.` and then use the
> TAB key.

<hr>

## Example usage of the Python CHIP Controller REPL

These section covers a few useful commands of the Python CHIP Controller along
with examples demonstrating how they can be called from the REPL.

The
[CHIP Device Controller API documentation offer](https://project-chip.github.io/connectedhomeip-doc/testing/ChipDeviceCtrlAPI.html#chip-chipdevicectrl)
the full list of available commands.

### `SetThreadOperationalDataset(<thread-dataset>)`

Provides the controller with Thread network credentials that will be used in the
device commissioning procedure to configure the device with a Thread interface.

```python
thread_dataset = bytes.fromhex("0e080000000000010000000300001335060004001fffe002084fe76e9a8b5edaf50708fde46f999f0698e20510d47f5027a414ffeebaefa92285cc84fa030f4f70656e5468726561642d653439630102e49c0410b92f8c7fbb4f9f3e08492ee3915fbd2f0c0402a0fff8")
devCtrl.SetThreadOperationalDataset(thread_dataset)
```

### `SetWiFiCredentials(<ssid>: str, <password>: str)`

Provides the controller with Wi-Fi network credentials that will be used in the
device commissioning procedure to configure the device with a Wi-Fi interface.

```python
devCtrl.SetWiFiCredentials('TESTSSID', 'P455W4RD')
```

### `CommissionWithCode(<setupPayload>: str, <nodeid>: int, <discoveryType>: DiscoveryType)`

Commission with the given nodeid from the setupPayload. setupPayload may be a QR
or the manual setup code.

```python
await devCtrl.CommissionWithCode("MT:-24J0AFN00KA0648G00", 1234)
```

### `SendCommand(<nodeid>: int, <endpoint>: int, Clusters.<cluster>.Commands.<command>(<arguments>))`

Send a Matter command to the device. For example:

```python
commandToSend = Clusters.LevelControl.Commands.MoveWithOnOff(moveMode=1, rate=2, optionsMask=0, optionsOverride=0)
await devCtrl.SendCommand(1234, 1, commandToSend)
```

To see available arguments just create a command object without argument:

```
Clusters.LevelControl.Commands.MoveWithOnOff()
```

Shows which arguments are available:

```
MoveWithOnOff(
│   moveMode=0,
│   rate=Null,
│   optionsMask=0,
│   optionsOverride=0
)
```

### `ReadAttribute(<nodeid>: int, [(<endpoint id>: int, Clusters.<cluster>.Attributes.<attribute>)])`

Read the value of an attribute. For example:

```python
await devCtrl.ReadAttribute(1234, [(0, Clusters.BasicInformation.Attributes.VendorName)])
```

### `WriteAttribute(<nodeid>: int, [(<endpointid>: int, Clusters.<cluster>.Attributes.<attribute>(value=<attribute value>))])`

Write a value to an attribute. For example:

```python
await devCtrl.WriteAttribute(1234, [(1, Clusters.UnitTesting.Attributes.Int8u(value=1))])
await devCtrl.WriteAttribute(1234, [(1, Clusters.UnitTesting.Attributes.Boolean(value=True))])
await devCtrl.WriteAttribute(1234, [(1, Clusters.UnitTesting.Attributes.OctetString(value=b'123123\x00'))])
await devCtrl.WriteAttribute(1234, [(1, Clusters.UnitTesting.Attributes.CharString(value='233233'))])
```

### `ReadAttribute(<nodeid>: int, [(<endpoint>: int, Clusters.<cluster>.Attributes.<attribute>)], reportInterval=(<min interval>: int, <max interval>: int))`

Configure Matter attribute reporting settings. For example:

```python
await devCtrl.ReadAttribute(1234, [(1, Clusters.OccupancySensing.Attributes.Occupancy)], reportInterval=(10, 20))
```

To shutdown an existing attribute subscription use the `Shutdown()` function on
the returned subscription object:

```python
sub = await devCtrl.ReadAttribute(1234, [(1, Clusters.OccupancySensing.Attributes.Occupancy)], reportInterval=(10, 20))
sub.Shutdown()
```

## Explore Clusters, Attributes and Commands

In the Python REPL the Clusters and Attributes are classes. The `Clusters`
module contains all clusters. Tab completion can be used to explore available
clusters, attributes and commands.

For example, to get a list of Clusters, type `Clusters.` and hit tab. Continue
to hit tab to cycle through the available Clusters. Pressing return will select
the Cluster.

To explore Attributes, use the same technique but with the Attributes sub-class
of the Clusters class, for example, type `Clusters.(cluster name).Attributes.`
and hit tab.

The same is true for Commands, use the Commands sub-class. type
`Clusters.(cluster name).Commands.` and hit tab.
