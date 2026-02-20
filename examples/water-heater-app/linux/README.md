# Matter Linux Water Heater Example

An example showing the use of CHIP on the Linux. The document will describe how
to build and run CHIP Linux Water Heater Example on Raspberry Pi. This doc is
tested on **Ubuntu for Raspberry Pi Server 20.04 LTS (aarch64)** and **Ubuntu
for Raspberry Pi Desktop 20.10 (aarch64)**

To cross-compile this example on x64 host and run on **NXP i.MX 8M Mini**
**EVK**, see the associated
[README document](../../../docs/platforms/nxp/nxp_imx8m_linux_examples.md) for
details.

<hr>

-   [Matter Linux Water Heater Example](#matter-linux-water-heater-example)
    -   [Building](#building)
    -   [Commandline arguments](#commandline-arguments)
    -   [Running the Complete Example on Raspberry Pi 4](#running-the-complete-example-on-raspberry-pi-4)
    -   [Device Tracing](#device-tracing)
    -   [Interaction using the chip-tool and TestEventTriggers](#interaction-using-the-chip-tool-and-testeventtriggers)
    -   [MATTER-REPL Interaction](#matter-repl-interaction)
        -   [Building matter-repl:](#building-matter-repl)
        -   [Activating python virtual env](#activating-python-virtual-env)
        -   [Interacting with matter-repl and the example app](#interacting-with-matter-repl-and-the-example-app)
        -   [Using matter-repl to Fake a water heater installation](#using-matter-repl-to-fake-a-water-heater-installation)

<hr>

## Building

-   Install tool chain

                              $ sudo apt-get install git gcc g++ python pkg-config libssl-dev libdbus-1-dev libglib2.0-dev ninja-build python3-venv python3-dev unzip

-   Build the example application:

                              $ cd ~/connectedhomeip/examples/water-heater-app/linux
                              $ git submodule update --init
                              $ source third_party/connectedhomeip/scripts/activate.sh
                              $ gn gen out/debug
                              $ ninja -C out/debug

-   To delete generated executable, libraries and object files use:

                              $ cd ~/connectedhomeip/examples/water-heater-app/linux
                              $ rm -rf out/

-   Build the example with pigweed RPC

                              $ cd ~/connectedhomeip/examples/water-heater-app/linux
                              $ git submodule update --init
                              $ source third_party/connectedhomeip/scripts/activate.sh
                              $ gn gen out/debug --args='import("//with_pw_rpc.gni")'
                              $ ninja -C out/debug

## Commandline arguments

-   `--wifi`

    Enables WiFi management feature. Required for WiFi commissioning.

-   `--thread`

    Enables Thread management feature, requires ot-br-posix dbus daemon running.
    Required for Thread commissioning.

-   `--ble-controller <selector>`

    Use the specific Bluetooth controller for BLE advertisement and connections.
    For details on controller selection refer to
    [Linux BLE Settings](/platforms/linux/ble_settings.md).

-   `--featureSet <feature map for Device Energy Management e.g. 0x7a>`

    Sets the run-time FeatureMap value for the Device Energy Management cluster.
    This allows the DEM cluster to support `PFR` or `SFR` so that the full range
    of TC_DEM_2.x test cases can be exercised with this application.

    See the test-runner headers in the respective test script in
    src/python_testing/TC_DEM_2.x.py which have recommended values to use.

## Running the Complete Example on Raspberry Pi 4

> If you want to test Echo protocol, please enable Echo handler
>
>     gn gen out/debug --args='chip_app_use_echo=true'
>     ninja -C out/debug

-   Prerequisites

    1. A Raspberry Pi 4 board
    2. A USB Bluetooth Dongle, Ubuntu desktop will send Bluetooth advertisement,
       which will block CHIP from connecting via BLE. On Ubuntu server, you need
       to install `pi-bluetooth` via APT.
    3. Ubuntu 20.04 or newer image for ARM64 platform.

-   Building

    Follow [Building](#building) section of this document.

-   Running

    -   [Optional] Plug USB Bluetooth dongle

        -   Plug USB Bluetooth dongle and find its bluetooth controller selector
            as described in
            [Linux BLE Settings](/platforms/linux/ble_settings.md).

    -   Run Linux Water Heater Example App

                                  $ cd ~/connectedhomeip/examples/water-heater-app/linux
                                  $ sudo out/debug/matter-water-heater-app --ble-controller [bluetooth controller number]
                                  # In this example, the device we want to use is hci1
                                  $ sudo out/debug/matter-water-heater-app --ble-controller 1

    -   Test the device using ChipDeviceController on your laptop / workstation
        etc.

## Device Tracing

Device tracing is available to analyze the device performance. To turn on
tracing, build with RPC enabled. See [Building with RPC enabled](#building).

Obtain tracing json file.

```
    $ ./{PIGWEED_REPO}/pw_trace_tokenized/py/pw_trace_tokenized/get_trace.py -s localhost:33000 \
     -o {OUTPUT_FILE} -t {ELF_FILE} {PIGWEED_REPO}/pw_trace_tokenized/pw_trace_protos/trace_rpc.proto
```

## Interaction using the chip-tool and TestEventTriggers

This section demonstrates how to interact with the Water Heater application
using the `chip-tool` and `TestEventTriggers`. By default (at the time of
writing), the Water Heater app does not configure some of its attributes with
simulated values (most default to 0). The steps below set the
[default](https://github.com/project-chip/connectedhomeip/blob/master/src/app/clusters/water-heater-management-server/WaterHeaterManagementTestEventTriggerHandler.h#L47)
`TestEventTrigger` which
`Simulate installation in a 100L tank full of water at 20C, with a target temperature of 60C, in OFF mode`.

Step-by-step:

1. Build the `water-heater-app` for linux (follow the [Building](#building)
   section above).

1. Run the Water Heater application:

    ```
    rm /tmp/chip_* && ./out/debug/matter-water-heater-app --trace-to json:log --enable-key 000102030405060708090a0b0c0d0e0f
    ```

1. Commission with chip-tool as node `0x12344321`:

    ```
    ./out/linux-x64-chip-tool-no-ble/chip-tool pairing code 0x12344321 MT:-24J0AFN00KA0648G00
    ```

1. Read the `TankVolume` attribute (expect 0 by default):

    ```
    ./out/linux-x64-chip-tool-no-ble/chip-tool waterheatermanagement read tank-volume 0x12344321 2 | grep TOO

    [1730306361.511] [2089549:2089552] [TOO]   TankVolume: 0
    ```

1. Set the default TestEventTrigger (`0x0094000000000000`):

-   `0x0094000000000000` corresponds to
    [`kBasicInstallationTestEvent`](https://github.com/project-chip/connectedhomeip/blob/5e3127f5ac61e13c572a968199280d90a9c19dce/src/app/clusters/water-heater-management-server/WaterHeaterManagementTestEventTriggerHandler.h#L47)
    from `WaterHeadermanagementTestEventTriggerHandler.h`
-   `hex:00010203...0e0f` is the `--enable-key` passed to the startup of
    chip-energy-management-app
-   `0x12344321` is the node-id that the app was commissioned on
-   final `0` is the endpoint on which the `GeneralDiagnostics` cluster exists
    to call the `TestEventTrigger` command
    ```
    ./out/linux-x64-chip-tool-no-ble/chip-tool generaldiagnostics test-event-trigger hex:000102030405060708090a0b0c0d0e0f 0x0094000000000000 0x12344321 0
    ```

1. Read TankVolume attribute again (now expect 100):

    ```
    ./out/linux-x64-chip-tool-no-ble/chip-tool waterheatermanagement read tank-volume 0x12344321 2 | grep TOO

    [1730312762.703] [2153606:2153609] [TOO]   TankVolume: 100
    ```

1. Set boost state:

    - `durationIndicates` the time period in seconds for which the BOOST state
      is activated before it automatically reverts to the previous mode (e.g.
      OFF, MANUAL or TIMED).

    ```
    ./out/linux-x64-chip-tool-no-ble/chip-tool waterheatermanagement boost '{ "duration": 1800 }' 0x12344321 2
    ```

1. Cancel boost state:

    ```
    ./out/linux-x64-chip-tool-no-ble/chip-tool waterheatermanagement cancel-boost 0x12344321 2
    ```

## MATTER-REPL Interaction

-   See matter-repl documentation in:
    -   [Working with Python CHIP Controller](../../../docs/development_controllers/matter-repl/python_chip_controller_building.md)
    -   [Matter_REPL_Intro](https://github.com/project-chip/connectedhomeip/blob/master/docs/development_controllers/matter-repl/Matter_REPL_Intro.ipynb)

### Building matter-repl:

```bash
    $ ./build_python.sh -i <path_to_out_folder>
```

### Activating python virtual env

-   You need to repeat this step each time you start a new shell.

```bash
    $ source <path_to_out_folder>/bin/activate
```

### Interacting with matter-repl and the example app

-   Step 1: Launch the example app

```bash
    $ ./out/debug/matter-water-heater-app --enable-key 000102030405060708090a0b0c0d0e0f
```

-   Step 2: Launch matter-repl

```bash
    $ matter-repl
```

-   Step 3: (In matter-repl) Commissioning OnNetwork

```python
    await devCtrl.CommissionOnNetwork(1234,20202021)   # Commission with NodeID 1234
Established secure session with Device
Commissioning complete
Out[2]: <matter.native.PyChipError object at 0x7f2432b16140>
```

-   Step 4: (In matter-repl) Read WaterHeaterManagement attributes

```python
    # Read from NodeID 1234, Endpoint 1, all attributes on WaterHeaterManagement cluster
    await devCtrl.ReadAttribute(1234,[(1, matter.clusters.WaterHeaterManagement)])
```

```
{
│   1: {
│   │   <class 'matter.clusters.Objects.WaterHeaterManagement'>: {
│   │   │   <class 'matter.clusters.Attribute.DataVersion'>: 1234567890,
│   │   │   <class 'matter.clusters.Objects.WaterHeaterManagement.Attributes.HeaterTypes'>: 1,
│   │   │   <class 'matter.clusters.Objects.WaterHeaterManagement.Attributes.HeatDemand'>: 0,
│   │   │   <class 'matter.clusters.Objects.WaterHeaterManagement.Attributes.TankVolume'>: 100,
│   │   │   <class 'matter.clusters.Objects.WaterHeaterManagement.Attributes.EstimatedHeatRequired'>: 16744,
│   │   │   <class 'matter.clusters.Objects.WaterHeaterManagement.Attributes.TankPercentage'>: 100,
│   │   │   <class 'matter.clusters.Objects.WaterHeaterManagement.Attributes.BoostState'>: <BoostStateEnum.kInactive: 0>,
│   │   │   <class 'matter.clusters.Objects.WaterHeaterManagement.Attributes.AcceptedCommandList'>: [
... │   │   ],
│   │   │   <class 'matter.clusters.Objects.WaterHeaterManagement.Attributes.GeneratedCommandList'>: [
... │   │   ],
│   │   │   <class 'matter.clusters.Objects.WaterHeaterManagement.Attributes.FeatureMap'>: 0,
│   │   │   <class 'matter.clusters.Objects.WaterHeaterManagement.Attributes.AttributeList'>: [
... │   │   ],
│   │   │   <class 'matter.clusters.Objects.WaterHeaterManagement.Attributes.ClusterRevision'>: 1
│   │   }
│   }
}

```

-   Step 5: Setting up a subscription so that attributes updates are sent
    automatically

```python
   reportingTimingParams = (3, 60) # MinInterval = 3s, MaxInterval = 60s
   subscription = await devCtrl.ReadAttribute(1234,[(1, matter.clusters.WaterHeaterManagement)], reportInterval=reportingTimingParams)
```

-   Step 6: Send a `Boost` command which activates boost mode for 1800 seconds
    (30 minutes). The `Boost` command takes a `duration` parameter which
    specifies how long the boost state should remain active before automatically
    reverting to the previous mode.

```python
   await devCtrl.SendCommand(1234, endpoint=1,
       payload=matter.clusters.WaterHeaterManagement.Commands.Boost(
           boostInfo=matter.clusters.WaterHeaterManagement.Structs.WaterHeaterBoostInfoStruct(
               duration=1800, targetPercentage=80)))
```

The output should look like:

```
Attribute Changed:
{
│   'Endpoint': 1,
│   'Attribute': <class 'matter.clusters.Objects.WaterHeaterManagement.Attributes.BoostState'>,
│   'Value': <BoostStateEnum.kActive: 1>
}
Attribute Changed:
{
│   'Endpoint': 1,
│   'Attribute': <class 'matter.clusters.Objects.WaterHeaterManagement.Attributes.HeatDemand'>,
│   'Value': 1
}
```

After 1800 seconds (30 minutes) the boost state should automatically revert:

```
Attribute Changed:
{
│   'Endpoint': 1,
│   'Attribute': <class 'matter.clusters.Objects.WaterHeaterManagement.Attributes.BoostState'>,
│   'Value': <BoostStateEnum.kInactive: 0>
}
Attribute Changed:
{
│   'Endpoint': 1,
│   'Attribute': <class 'matter.clusters.Objects.WaterHeaterManagement.Attributes.HeatDemand'>,
│   'Value': 0
}
```

Note that you can cancel the boost state early by sending the `CancelBoost`
command:

```python
   await devCtrl.SendCommand(1234, 1, matter.clusters.WaterHeaterManagement.Commands.CancelBoost())
```

result:

```
Attribute Changed:
{
│   'Endpoint': 1,
│   'Attribute': <class 'matter.clusters.Objects.WaterHeaterManagement.Attributes.BoostState'>,
│   'Value': <BoostStateEnum.kInactive: 0>
}
Attribute Changed:
{
│   'Endpoint': 1,
│   'Attribute': <class 'matter.clusters.Objects.WaterHeaterManagement.Attributes.HeatDemand'>,
│   'Value': 0
}
```

### Using matter-repl to Fake a water heater installation

If you haven't implemented a real water heater but want to simulate a water
heater installation and observe its behavior, you can use test event triggers to
simulate various scenarios.

The test event triggers values can be found in:
[WaterHeaterManagementTestEventTriggerHandler.h](../../../src/app/clusters/water-heater-management-server/WaterHeaterManagementTestEventTriggerHandler.h)

-   0x0094000000000000 - Simulates basic installation (100L tank at 20C, target
    60C, OFF mode)
-   0x0094000000000001 - Simulates drawing hot water from the tank
-   0x0094000000000002 - Simulates the tank heating up

To send a test event trigger to the app, use the following commands (in
matter-repl):

```python
    # send 1st event trigger to simulate basic installation
    await devCtrl.SendCommand(1234, 0, matter.clusters.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=bytes([b for b in range(16)]), eventTrigger=0x0094000000000000))

    # send 2nd event trigger to simulate drawing hot water
    await devCtrl.SendCommand(1234, 0, matter.clusters.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=bytes([b for b in range(16)]), eventTrigger=0x0094000000000001))
```

```
Attribute Changed:
{
│   'Endpoint': 1,
│   'Attribute': <class 'matter.clusters.Objects.WaterHeaterManagement.Attributes.TankVolume'>,
│   'Value': 100
}
Attribute Changed:
{
│   'Endpoint': 1,
│   'Attribute': <class 'matter.clusters.Objects.WaterHeaterManagement.Attributes.HeaterTypes'>,
│   'Value': 3
}
```

```python
    # send 2nd event trigger to simulate drawing hot water
    await devCtrl.SendCommand(1234, 0, matter.clusters.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=bytes([b for b in range(16)]), eventTrigger=0x0094000000000001))

```

Now you can read the updated attributes to see the changes:

```python
    # Read the WaterHeaterManagement attributes
    await devCtrl.ReadAttribute(1234,[(1, matter.clusters.WaterHeaterManagement)])
```

You should see changes in attributes like `TankPercentage`,
`EstimatedHeatRequired`, and `HeatDemand` reflecting the simulated water usage.
