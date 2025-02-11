# Matter Linux Energy Management Example

An example showing the use of CHIP on the Linux. The document will describe how
to build and run CHIP Linux Energy Management Example on Raspberry Pi. This doc
is tested on **Ubuntu for Raspberry Pi Server 20.04 LTS (aarch64)** and **Ubuntu
for Raspberry Pi Desktop 20.10 (aarch64)**

To cross-compile this example on x64 host and run on **NXP i.MX 8M Mini**
**EVK**, see the associated
[README document](../../../docs/platforms/nxp/nxp_imx8m_linux_examples.md) for
details.

<hr>

-   [Matter Linux Energy Management Example](#matter-linux-energy-management-example)
    -   [Building](#building)
    -   [Commandline arguments](#commandline-arguments)
    -   [Running the Complete Example on Raspberry Pi 4](#running-the-complete-example-on-raspberry-pi-4)
    -   [Device Tracing](#device-tracing)
    -   [Python Test Cases](#python-test-cases)
        -   [Running the test cases:](#running-the-test-cases)
    -   [CHIP-REPL Interaction](#chip-repl-interaction)
        -   [Building chip-repl:](#building-chip-repl)
        -   [Activating python virtual env](#activating-python-virtual-env)
        -   [Interacting with CHIP-REPL and the example app](#interacting-with-chip-repl-and-the-example-app)
        -   [Using chip-repl to Fake a charging session](#using-chip-repl-to-fake-a-charging-session)
    -   [Water Heater App: Interaction using the chip-tool and TestEventTriggers](#water-heater-app-interaction-using-the-chip-tool-and-testeventtriggers)

<hr>

## Building

-   Install tool chain

          $ sudo apt-get install git gcc g++ python pkg-config libssl-dev libdbus-1-dev libglib2.0-dev ninja-build python3-venv python3-dev unzip

-   Build the example application:

          $ cd ~/connectedhomeip/examples/energy-management-app/linux
          $ git submodule update --init
          $ source third_party/connectedhomeip/scripts/activate.sh
          $ gn gen out/debug
          $ ninja -C out/debug

-   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip/examples/energy-management-app/linux
          $ rm -rf out/

-   Build the example with pigweed RPC

          $ cd ~/connectedhomeip/examples/energy-management-app/linux
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

-   `--ble-device <interface id>`

    Use specific bluetooth interface for BLE advertisement and connections.

    `interface id`: the number after `hci` when listing BLE interfaces by
    `hciconfig` command, for example, `--ble-device 1` means using `hci1`
    interface. Default: `0`.

-   `--application <evse | water-heater>`

    Emulate either an EVSE or Water Heater example.

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

        -   Plug USB Bluetooth dongle and find its bluetooth device number. The
            number after `hci` is the bluetooth device number, `1` in this
            example.

                  $ hciconfig
                  hci1:	Type: Primary  Bus: USB
                      BD Address: 00:1A:7D:AA:BB:CC  ACL MTU: 310:10  SCO MTU: 64:8
                      UP RUNNING PSCAN ISCAN
                      RX bytes:20942 acl:1023 sco:0 events:1140 errors:0
                      TX bytes:16559 acl:1011 sco:0 commands:121 errors:0

                  hci0:	Type: Primary  Bus: UART
                      BD Address: B8:27:EB:AA:BB:CC  ACL MTU: 1021:8  SCO MTU: 64:1
                      UP RUNNING PSCAN ISCAN
                      RX bytes:8609495 acl:14 sco:0 events:217484 errors:0
                      TX bytes:92185 acl:20 sco:0 commands:5259 errors:0

        -   Run Linux Energy Management Example App

                  $ cd ~/connectedhomeip/examples/energy-management-app/linux
                  $ sudo out/debug/chip-energy-management-app --ble-device [bluetooth device number]
                  # In this example, the device we want to use is hci1
                  $ sudo out/debug/chip-energy-management-app --ble-device 1

        -   Test the device using ChipDeviceController on your laptop /
            workstation etc.

## Device Tracing

Device tracing is available to analyze the device performance. To turn on
tracing, build with RPC enabled. See [Building with RPC enabled](#building).

Obtain tracing json file.

```
    $ ./{PIGWEED_REPO}/pw_trace_tokenized/py/pw_trace_tokenized/get_trace.py -s localhost:33000 \
     -o {OUTPUT_FILE} -t {ELF_FILE} {PIGWEED_REPO}/pw_trace_tokenized/pw_trace_protos/trace_rpc.proto
```

## Python Test Cases

When you want to test this cluster you can use chip-repl or chip-tool by hand.
CHIP-REPL is slightly easier to interact with when dealing with some of the
complex structures.

There are several test scripts provided for EVSE (in
[src/python_testing](/src/python_testing)):

-   `TC_EEVSE_2_2`: This validates the primary functionality
-   `TC_EEVSE_2_3`: This validates Get/Set/Clear target commands
-   `TC_EEVSE_2_4`: This validates Faults
-   `TC_EEVSE_2_5`: This validates EVSE diagnostic command (optional)
-   `TC_EEVSE_2_6`: This validates EVSE Forecast Adjustment with State Forecast
    Reporting feature functionality
-   `TC_EEVSE_2_7`: This validates EVSE Constraints-based Adjustment with Power
    Forecast Reporting feature functionality
-   `TC_EEVSE_2_8`: This validates EVSE Constraints-based Adjustment with State
    Forecast Reporting feature functionality
-   `TC_EEVSE_2_9`: This validates EVSE Power or State Forecast Reporting
    feature functionality

These scripts require the use of Test Event Triggers via the GeneralDiagnostics
cluster on Endpoint 0. This requires an `enableKey` (16 bytes) and a set of
reserved int64_t test event trigger codes.

By default the test event support is not enabled, and when compiling the example
app you need to add `chip_enable_energy_evse_trigger=true` to the gn args.

          $ gn gen out/debug --args='chip_enable_energy_evse_trigger=true'
          $ ninja -C out/debug

Once the application is built you also need to tell it at runtime what the
chosen enable key is using the `--enable-key` command line option.

          $ ./chip-energy-management-app --enable-key 000102030405060708090a0b0c0d0e0f --application evse

### Running the test cases:

From the top-level of the connectedhomeip repo type:

Start the chip-energy-management-app:

```bash
rm -f evse.bin; out/debug/chip-energy-management-app --enable-key 000102030405060708090a0b0c0d0e0f --KVS evse.bin --featureSet $featureSet --application evse
```

where the \$featureSet depends on the test being run:

```
TC_DEM_2_2.py: 0x01  // PA
TC_DEM_2_3.py: 0x3b  // STA, PAU, FA, CON + (PFR | SFR)
TC_DEM_2_4.py: 0x3b  // STA, PAU, FA, CON + (PFR | SFR)
TC_DEM_2_5.py: 0x3b  // STA, PAU, FA, CON + PFR
TC_DEM_2_6.py: 0x3d  // STA, PAU, FA, CON + SFR
TC_DEM_2_7.py: 0x3b  // STA, PAU, FA, CON + PFR
TC_DEM_2_8.py: 0x3d  // STA, PAU, FA, CON + SFR
TC_DEM_2_9.py: 0x3f  // STA, PAU, FA, CON + PFR + SFR
```

where

```
PA  - DEM.S.F00(PowerAdjustment)
PFR - DEM.S.F01(PowerForecastReporting)
SFR - DEM.S.F02(StateForecastReporting)
STA - DEM.S.F03(StartTimeAdjustment)
PAU - DEM.S.F04(Pausable)
FA  - DEM.S.F05(ForecastAdjustment)
CON  -DEM.S.F06(ConstraintBasedAdjustment)
```

Then run the test:

```bash
     $ python src/python_testing/TC_EEVSE_2_2.py --endpoint 1 -m on-network -n 1234 -p 20202021 -d 3840 --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
```

-   Note that the `--endpoint 1` must be used with the example, since the EVSE
    cluster is on endpoint 1. The `--hex-arg enableKey:<key>` value must match
    the `--enable-key <key>` used on chip-energy-management-app args.

The chip-energy-management-app will need to be stopped before running each test
script as each test commissions the chip-energy-management-app in the first
step. That is also why the evse.bin is deleted before running
chip-energy-management-app as this is where the app stores the matter persistent
data (e.g. fabric info).

## CHIP-REPL Interaction

-   See chip-repl documentation in:
    -   [Working with Python CHIP Controller](../../../docs/development_controllers/chip-repl/python_chip_controller_building.md)
    -   [Matter_REPL_Intro](../../../docs/development_controllers/chip-repl/Matter_REPL_Intro.ipynb)

### Building chip-repl:

```bash
    $ ./build_python.sh -i <path_to_out_folder>
```

### Activating python virtual env

-   You need to repeat this step each time you start a new shell.

```bash
    $ source <path_to_out_folder>/bin/activate
```

### Interacting with CHIP-REPL and the example app

-   Step 1: Launch the example app

```bash
    $ ./chip-energy-management-app --enable-key 000102030405060708090a0b0c0d0e0f --application evse
```

-   Step 2: Launch CHIP-REPL

```bash
    $ chip-repl
```

-   Step 3: (In chip-repl) Commissioning OnNetwork

```python
    await devCtrl.CommissionOnNetwork(1234,20202021)   # Commission with NodeID 1234
Established secure session with Device
Commissioning complete
Out[2]: <chip.native.PyChipError object at 0x7f2432b16140>
```

-   Step 4: (In chip-repl) Read EVSE attributes

```python
    # Read from NodeID 1234, Endpoint 1, all attributes on EnergyEvse cluster
    await devCtrl.ReadAttribute(1234,[(1, chip.clusters.EnergyEvse)])
```

```
{
│   1: {
│   │   <class 'chip.clusters.Objects.EnergyEvse'>: {
│   │   │   <class 'chip.clusters.Attribute.DataVersion'>: 3790455237,
│   │   │   <class 'chip.clusters.Objects.EnergyEvse.Attributes.ChargingEnabledUntil'>: Null,
│   │   │   <class 'chip.clusters.Objects.EnergyEvse.Attributes.FaultState'>: <FaultStateEnum.kNoError: 0>,
│   │   │   <class 'chip.clusters.Objects.EnergyEvse.Attributes.NextChargeStartTime'>: Null,
│   │   │   <class 'chip.clusters.Objects.EnergyEvse.Attributes.StateOfCharge'>: Null,
│   │   │   <class 'chip.clusters.Objects.EnergyEvse.Attributes.MaximumChargeCurrent'>: 0,
│   │   │   <class 'chip.clusters.Objects.EnergyEvse.Attributes.ApproximateEVEfficiency'>: Null,
│   │   │   <class 'chip.clusters.Objects.EnergyEvse.Attributes.BatteryCapacity'>: Null,
│   │   │   <class 'chip.clusters.Objects.EnergyEvse.Attributes.AcceptedCommandList'>: [
... │   │   ],
│   │   │   <class 'chip.clusters.Objects.EnergyEvse.Attributes.MinimumChargeCurrent'>: 6000,
│   │   │   <class 'chip.clusters.Objects.EnergyEvse.Attributes.NextChargeTargetSoC'>: Null,
│   │   │   <class 'chip.clusters.Objects.EnergyEvse.Attributes.SessionDuration'>: 758415333,
│   │   │   <class 'chip.clusters.Objects.EnergyEvse.Attributes.NumberOfWeeklyTargets'>: 0,
│   │   │   <class 'chip.clusters.Objects.EnergyEvse.Attributes.FeatureMap'>: 1,
│   │   │   <class 'chip.clusters.Objects.EnergyEvse.Attributes.GeneratedCommandList'>: [
...
│   │   │   ],
│   │   │   <class 'chip.clusters.Objects.EnergyEvse.Attributes.State'>: <StateEnum.kNotPluggedIn: 0>,
│   │   │   <class 'chip.clusters.Objects.EnergyEvse.Attributes.SessionID'>: Null,
│   │   │   <class 'chip.clusters.Objects.EnergyEvse.Attributes.SessionEnergyCharged'>: Null,
│   │   │   <class 'chip.clusters.Objects.EnergyEvse.Attributes.VehicleID'>: Null,
│   │   │   <class 'chip.clusters.Objects.EnergyEvse.Attributes.NextChargeRequiredEnergy'>: Null,
│   │   │   <class 'chip.clusters.Objects.EnergyEvse.Attributes.SessionEnergyDischarged'>: Null,
│   │   │   <class 'chip.clusters.Objects.EnergyEvse.Attributes.AttributeList'>: [
... │   │   ],
│   │   │   <class 'chip.clusters.Objects.EnergyEvse.Attributes.NextChargeTargetTime'>: Null,
│   │   │   <class 'chip.clusters.Objects.EnergyEvse.Attributes.CircuitCapacity'>: 0,
│   │   │   <class 'chip.clusters.Objects.EnergyEvse.Attributes.DischargingEnabledUntil'>: Null,
│   │   │   <class 'chip.clusters.Objects.EnergyEvse.Attributes.SupplyState'>: <SupplyStateEnum.kDisabled: 0>,
│   │   │   <class 'chip.clusters.Objects.EnergyEvse.Attributes.RandomizationDelayWindow'>: 600,
│   │   │   <class 'chip.clusters.Objects.EnergyEvse.Attributes.MaximumDischargeCurrent'>: 0,
│   │   │   <class 'chip.clusters.Objects.EnergyEvse.Attributes.NumberOfDailyTargets'>: 1,
│   │   │   <class 'chip.clusters.Objects.EnergyEvse.Attributes.UserMaximumChargeCurrent'>: 80000,
│   │   │   <class 'chip.clusters.Objects.EnergyEvse.Attributes.ClusterRevision'>: 2
│   │   }
│   }
}

```

-   Step 5: Setting up a subscription so that attributes updates are sent
    automatically

```python
   reportingTimingParams = (3, 60) # MinInterval = 3s, MaxInterval = 60s
   subscription = await devCtrl.ReadAttribute(1234,[(1, chip.clusters.EnergyEvse)], reportInterval=reportingTimingParams)
```

-   Step 6: Send an `EnableCharging` command which lasts for 60 seconds The
    `EnableCharging` takes an optional `chargingEnabledUntil` parameter which
    allows the charger to automatically disable itself at some preset time in
    the future. Note that it uses Epoch_s (which is from Jan 1 2000) which is a
    uint32_t in seconds.

```python
   from datetime import datetime, timezone, timedelta
   epoch_end = int((datetime.now(tz=timezone.utc) + timedelta(seconds=60) - datetime(2000, 1, 1, 0, 0, 0, 0, timezone.utc)).total_seconds())

   await devCtrl.SendCommand(1234, 1, chip.clusters.EnergyEvse.Commands.EnableCharging(chargingEnabledUntil=epoch_end,minimumChargeCurrent=2000,maximumChargeCurrent=25000),timedRequestTimeoutMs=3000)
```

The output should look like:

```
Attribute Changed:
{
│   'Endpoint': 1,
│   'Attribute': <class 'chip.clusters.Objects.EnergyEvse.Attributes.SupplyState'>,
│   'Value': <SupplyStateEnum.kChargingEnabled: 1>
}
Attribute Changed:
{
│   'Endpoint': 1,
│   'Attribute': <class 'chip.clusters.Objects.EnergyEvse.Attributes.MinimumChargeCurrent'>,
│   'Value': 2000
}
Attribute Changed:
{
│   'Endpoint': 1,
│   'Attribute': <class 'chip.clusters.Objects.EnergyEvse.Attributes.ChargingEnabledUntil'>,
│   'Value': 758416066
}
```

After 60 seconds the charging should automatically become disabled:

```
Attribute Changed:
{
│   'Endpoint': 1,
│   'Attribute': <class 'chip.clusters.Objects.EnergyEvse.Attributes.SupplyState'>,
│   'Value': <SupplyStateEnum.kDisabled: 0>
}
Attribute Changed:
{
│   'Endpoint': 1,
│   'Attribute': <class 'chip.clusters.Objects.EnergyEvse.Attributes.DischargingEnabledUntil'>,
│   'Value': 0
}
Attribute Changed:
{
│   'Endpoint': 1,
│   'Attribute': <class 'chip.clusters.Objects.EnergyEvse.Attributes.MinimumChargeCurrent'>,
│   'Value': 0
}
Attribute Changed:
{
│   'Endpoint': 1,
│   'Attribute': <class 'chip.clusters.Objects.EnergyEvse.Attributes.ChargingEnabledUntil'>,
│   'Value': 0
}
```

Note that you can omit the `chargingEnabledUntil` argument and it will charge
indefinitely.

### Using chip-repl to Fake a charging session

If you haven't implemented a real EVSE but want to simulate plugging in an EV
then you can use a few of the test event triggers to simulate these scenarios.

The test event triggers values can be found in:
[EnergyEvseTestEventTriggerHandler.h](../../../src/app/clusters/energy-evse-server/EnergyEvseTestEventTriggerHandler.h)

-   0x0099000000000000 - Simulates the EVSE being installed on a 32A supply
-   0x0099000000000002 - Simulates the EVSE being plugged in (this should
    generate an `EVConnected` event)
-   0x0099000000000004 - Simulates the EVSE requesting power

To send a test event trigger to the app, use the following commands (in
chip-repl):

```python
    # send 1st event trigger to 'install' the EVSE on a 32A supply
    await devCtrl.SendCommand(1234, 0, chip.clusters.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=bytes([b for b in range(16)]), eventTrigger=0x0099000000000000))

    # send 2nd event trigger to plug the EV in
    await devCtrl.SendCommand(1234, 0, chip.clusters.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=bytes([b for b in range(16)]), eventTrigger=0x0099000000000002))

```

Now send the enable charging command (omit the `chargingEnabledUntil` arg this
time):

```python
    await devCtrl.SendCommand(1234, 1, chip.clusters.EnergyEvse.Commands.EnableCharging(minimumChargeCurrent=2000,maximumChargeCurrent=25000),timedRequestTimeoutMs=3000)
```

Now send the test event trigger to simulate the EV asking for demand:

```python
    # send 2nd event trigger to plug the EV in
    await devCtrl.SendCommand(1234, 0, chip.clusters.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=bytes([b for b in range(16)]), eventTrigger=0x0099000000000004))

    # Read the events
    await devCtrl.ReadEvent(1234,[(1, chip.clusters.EnergyEvse,1)])
```

```
[
│   EventReadResult(
│   │   Header=EventHeader(
│   │   │   EndpointId=1,
│   │   │   ClusterId=153,
│   │   │   EventId=0,
│   │   │   EventNumber=65538,
│   │   │   Priority=<EventPriority.INFO: 1>,
│   │   │   Timestamp=1705102500069,
│   │   │   TimestampType=<EventTimestampType.EPOCH: 1>
│   │   ),
│   │   Status=<Status.Success: 0>,
│   │   Data=EVConnected(
│   │   │   sessionID=0
│   │   )
│   ),
│   EventReadResult(
│   │   Header=EventHeader(
│   │   │   EndpointId=1,
│   │   │   ClusterId=153,
│   │   │   EventId=2,
│   │   │   EventNumber=65539,
│   │   │   Priority=<EventPriority.INFO: 1>,
│   │   │   Timestamp=1705102801764,
│   │   │   TimestampType=<EventTimestampType.EPOCH: 1>
│   │   ),
│   │   Status=<Status.Success: 0>,
│   │   Data=EnergyTransferStarted(
│   │   │   sessionID=0,
│   │   │   state=<StateEnum.kPluggedInCharging: 3>,
│   │   │   maximumCurrent=25000
│   │   )
│   )
]
```

-   We can see that the `EventNumber 65538` was sent when the vehicle was
    plugged in, and a new `sessionID=0` was created.
-   We can also see that the `EnergyTransferStarted` was sent in
    `EventNumber 65539`

What happens when we unplug the vehicle?

```python
    await devCtrl.SendCommand(1234, 0, chip.clusters.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=bytes([b for b in range(16)]), eventTrigger=0x0099000000000001))
```

When we re-read the events:

```
[
│   EventReadResult(
│   │   Header=EventHeader(
│   │   │   EndpointId=1,
│   │   │   ClusterId=153,
│   │   │   EventId=3,
│   │   │   EventNumber=65540,
│   │   │   Priority=<EventPriority.INFO: 1>,
│   │   │   Timestamp=1705102996749,
│   │   │   TimestampType=<EventTimestampType.EPOCH: 1>
│   │   ),
│   │   Status=<Status.Success: 0>,
│   │   Data=EnergyTransferStopped(
│   │   │   sessionID=0,
│   │   │   state=<StateEnum.kPluggedInCharging: 3>,
│   │   │   reason=<EnergyTransferStoppedReasonEnum.kOther: 2>,
│   │   │   energyTransferred=0
│   │   )
│   ),
│   EventReadResult(
│   │   Header=EventHeader(
│   │   │   EndpointId=1,
│   │   │   ClusterId=153,
│   │   │   EventId=1,
│   │   │   EventNumber=65541,
│   │   │   Priority=<EventPriority.INFO: 1>,
│   │   │   Timestamp=1705102996749,
│   │   │   TimestampType=<EventTimestampType.EPOCH: 1>
│   │   ),
│   │   Status=<Status.Success: 0>,
│   │   Data=EVNotDetected(
│   │   │   sessionID=0,
│   │   │   state=<StateEnum.kPluggedInCharging: 3>,
│   │   │   sessionDuration=0,
│   │   │   sessionEnergyCharged=0,
│   │   │   sessionEnergyDischarged=0
│   │   )
│   )
]

```

-   In `EventNumber 65540` we had an `EnergyTransferStopped` event with reason
    `kOther`.

    This was a rather abrupt end to a charging session (normally we would see
    the EVSE or EV decide to stop charging), but this demonstrates the cable
    being pulled out without a graceful charging shutdown.

-   In `EventNumber 65541` we had an `EvNotDetected` event showing that the
    state was `kPluggedInCharging` prior to the EV being not detected (normally
    in a graceful shutdown this would be `kPluggedInNoDemand` or
    `kPluggedInDemand`).

## Water Heater App: Interaction using the chip-tool and TestEventTriggers

This section demonstrates how to run the Water Heater application and interact
with it using the `chip-tool` and `TestEventTriggers`. By default (at the time
of writing), the WaterHeater app does not configure some of its attributes with
simulated values (most default to 0). The steps below set the
[default](https://github.com/project-chip/connectedhomeip/blob/master/src/app/clusters/water-heater-management-server/WaterHeaterManagementTestEventTriggerHandler.h#L47)
`TestEventTrigger` which
`Simulate installation in a 100L tank full of water at 20C, with a target temperature of 60C, in OFF mode`.

Step-by-step:

1. Build the `energy-management-app` for linux:

    ```
    ./scripts/build/build_examples.py --target linux-x64-energy-management-no-ble build
    ```

1. Run the Water Heater application:

    ```
    rm /tmp/chip_* && ./out/linux-x64-energy-management-no-ble/chip-energy-management-app --application water-heater --trace-to json:log --enable-key 000102030405060708090a0b0c0d0e0f
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
