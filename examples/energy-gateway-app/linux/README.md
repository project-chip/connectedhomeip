# Matter Linux Energy Gateway Example

The Energy Gateway app is intended to act as a `CommodityPrice` and
`Electrical Grid Conditions` cluster server that can share the current
electrical price of energy and grid conditions (how many Grams of CO2 per kWh)
to Energy Smart Appliances (ESAs).

This is primarily intended as an example reference application.

This document describes how to build and run CHIP Linux Energy Gateway Example
on Raspberry Pi. This doc is tested on **Ubuntu for Raspberry Pi Server 24.04
LTS (aarch64)**.

To cross-compile this example on x64 host and run on **NXP i.MX 8M Mini**
**EVK**, see the associated
[README document](../../../docs/platforms/nxp/nxp_imx8m_linux_examples.md) for
details.

<hr>

-   [Matter Linux Energy Gateway Example](#matter-linux-energy-gateway-example)
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
            -   [CommodityPrice cluster](#commodityprice-cluster)
            -   [ElectricalGridConditions cluster](#electricalgridconditions-cluster)

<hr>

## Building

-   Install tool chain

          $ sudo apt-get install git gcc g++ python pkg-config libssl-dev libdbus-1-dev libglib2.0-dev ninja-build python3-venv python3-dev unzip

-   Build the example application:

          $ cd ~/connectedhomeip/examples/energy-gateway-app/linux
          $ git submodule update --init
          $ source third_party/connectedhomeip/scripts/activate.sh
          $ gn gen out/debug
          $ ninja -C out/debug

-   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip/examples/energy-gateway-app/linux
          $ rm -rf out/

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

## Running the Complete Example on Raspberry Pi 4

-   Prerequisites

    1. A Raspberry Pi 4 board
    2. A USB Bluetooth Dongle, Ubuntu desktop will send Bluetooth advertisement,
       which will block CHIP from connecting via BLE. On Ubuntu server, you need
       to install `pi-bluetooth` via APT.
    3. Ubuntu 24.04 or newer image for ARM64 platform.

-   Building

    Follow [Building](#building) section of this document.

-   Running

    -   [Optional] Plug USB Bluetooth dongle

        -   Plug USB Bluetooth dongle and find its bluetooth controller selector
            as described in
            [Linux BLE Settings](/platforms/linux/ble_settings.md).

    -   Run Linux Energy Gateway Example App

              $ cd ~/connectedhomeip/examples/energy-gateway-app/linux
              $ sudo out/debug/chip-energy-gateway-app --ble-controller [bluetooth controller number]
              # In this example, the device we want to use is hci1
              $ sudo out/debug/chip-energy-gateway-app --ble-controller 1

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

## Python Test Cases

When you want to test this cluster you can use chip-repl or chip-tool by hand.
CHIP-REPL is slightly easier to interact with when dealing with some of the
complex structures.

There are several test scripts provided for Commodity Price cluster (in
[src/python_testing](/src/python_testing)):

-   `TC_SEPR_2_1`: This validates the primary functionality
-   \*`TC_SEPR_2_2`: This validates GetDetailedPriceRequest command
-   \*`TC_SEPR_2_3`: This validates the FORECASTING feature and
    GetDetailedForecastRequest command (NOTE requires TCP support - see below)

There are several test scripts provided for Electrical Grid Conditions cluster
(in [src/python_testing](/src/python_testing)):

-   `TC_EGC_2_1`: This validates the primary functionality
-   \*`TC_EGC_2_2`: This validates that when the CurrentConditions attribute
    changes, an event is sent
-   \*`TC_EGC_2_3`: This validates the FORECASTING feature using test event to
    simulate data

`*` - These scripts require the use of Test Event Triggers via the
GeneralDiagnostics cluster on Endpoint 0. This requires an `enableKey` (16
bytes) and a set of reserved int64_t test event trigger codes.

**NOTE: that some non Linux platforms may not support TCP (for large messages).
This means that the GetDetailedForecastRequest() command will not be supported
on these platforms.**

Once the application is built you also need to tell it at runtime what the
chosen enable key is using the `--enable-key` command line option.

       $ ./chip-energy-gateway-app --enable-key 000102030405060708090a0b0c0d0e0f

### Running the test cases:

From the top-level of the connectedhomeip repo type:

Start the chip-energy-gateway-app:

```bash
     rm -f /tmp/chip_*; out/debug/chip-energy-gateway-app --enable-key 000102030405060708090a0b0c0d0e0f
```

Then run the test:

```bash
     $ python src/python_testing/TC_SEPR_2_1.py --endpoint 1 -m on-network -n 1234 -p 20202021 -d 3840 --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
```

-   Note that the `--endpoint 1` must be used with the example, since the SEPR
    cluster is on endpoint 1. The `--hex-arg enableKey:<key>` value must match
    the `--enable-key <key>` used on chip-energy-gateway-app args.

The chip-energy-gateway-app will need to be stopped before running each test
script as each test commissions the chip-energy-gateway-app in the first step.
That is also why the `/tmp/chip_*` files are deleted before running
chip-energy-gateway-app as this is where the app stores the matter persistent
data (e.g. fabric info).

## CHIP-REPL Interaction

-   See chip-repl documentation in:
    -   [Working with Python CHIP Controller](../../../docs/development_controllers/chip-repl/python_chip_controller_building.md)
    -   [Matter_REPL_Intro](https://github.com/project-chip/connectedhomeip/blob/master/docs/development_controllers/chip-repl/Matter_REPL_Intro.ipynb)

### Building chip-repl:

```bash
    $ ./build_python.sh -i out/python
```

### Activating python virtual env

-   You need to repeat this step each time you start a new shell.

```bash
    $ source out/python/bin/activate
```

### Interacting with CHIP-REPL and the example app

-   Step 1: Launch the example app in shell 1

```bash
    $ ./chip-energy-gateway-app --enable-key 000102030405060708090a0b0c0d0e0f
```

-   Step 2: Launch CHIP-REPL in shell 2 (where you have previously run
    `source out/python/bin/activate`)

```bash
    $ chip-repl
```

-   Step 3: (In chip-repl) Commissioning OnNetwork

```python
    await devCtrl.CommissionOnNetwork(200,20202021)   # Commission with NodeID 200
    Out[1]: 200
```

#### CommodityPrice cluster

This allows you to get current and forecast energy prices.

-   Step 4: (In chip-repl) Read `Commodity Price` attributes

```python
    # Read from NodeID 200, Endpoint 1, all attributes on CommodityPrice cluster
    await devCtrl.ReadAttribute(200,[(1, chip.clusters.CommodityPrice)])
```

The response in the default app is a null `CurrentPrice` and an empty
`PriceForecast` attribute:

```
    Out[11]:

    {
    │   1: {
    │   │   <class 'chip.clusters.Objects.CommodityPrice'>: {
    │   │   │   <class 'chip.clusters.Attribute.DataVersion'>: 3672963490,
    │   │   │   <class 'chip.clusters.Objects.CommodityPrice.Attributes.AcceptedCommandList'>: [
    │   │   │   │   0,
    │   │   │   │   2
    │   │   │   ],
    │   │   │   <class 'chip.clusters.Objects.CommodityPrice.Attributes.Currency'>: CurrencyStruct(
    │   │   │   │   currency=826,
    │   │   │   │   decimalPoints=5
    │   │   │   ),
    │   │   │   <class 'chip.clusters.Objects.CommodityPrice.Attributes.GeneratedCommandList'>: [
    │   │   │   │   1,
    │   │   │   │   3
    │   │   │   ],
    │   │   │   <class 'chip.clusters.Objects.CommodityPrice.Attributes.AttributeList'>: [
    │   │   │   │   0,
    │   │   │   │   1,
    │   │   │   │   2,
    │   │   │   │   3,
    │   │   │   │   65532,
    │   │   │   │   65533,
    │   │   │   │   65528,
    │   │   │   │   65529,
    │   │   │   │   65531
    │   │   │   ],
    │   │   │   <class 'chip.clusters.Objects.CommodityPrice.Attributes.TariffUnit'>: <TariffUnitEnum.kKWh: 0>,
    │   │   │   <class 'chip.clusters.Objects.CommodityPrice.Attributes.PriceForecast'>: [],
    │   │   │   <class 'chip.clusters.Objects.CommodityPrice.Attributes.ClusterRevision'>: 4,
    │   │   │   <class 'chip.clusters.Objects.CommodityPrice.Attributes.CurrentPrice'>: Null,
    │   │   │   <class 'chip.clusters.Objects.CommodityPrice.Attributes.FeatureMap'>: 1
    │   │   }
    │   }
```

-   Step 5: (In chip-repl) Using TestEvent trigger
    `eventTrigger=0x0095000000000000` we can generate a test `CurrentPrice` with
    sample data

```python
    # Send a test event trigger NodeID 200, Endpoint 0, with eventTrigger=0x0095000000000000
    await devCtrl.SendCommand(200, 0, chip.clusters.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=bytes([b for b in range(16)]), eventTrigger=0x0095000000000000))
```

-   Step 6: (In chip-repl) Re-Read `CurrentPrice` attribute (see the values have
    changed)

```
    await devCtrl.ReadAttribute(200,[(1,chip.clusters.CommodityPrice.Attributes.CurrentPrice)])
    Out[18]:

    {
    │   1: {
    │   │   <class 'chip.clusters.Objects.CommodityPrice'>: {
    │   │   │   <class 'chip.clusters.Attribute.DataVersion'>: 3672963491,
    │   │   │   <class 'chip.clusters.Objects.CommodityPrice.Attributes.CurrentPrice'>: CommodityPriceStruct(
    │   │   │   │   periodStart=799150761,
    │   │   │   │   periodEnd=799152561,
    │   │   │   │   price=15916,
    │   │   │   │   priceLevel=3,
    │   │   │   │   description=None,
    │   │   │   │   components=None
    │   │   │   )
    │   │   }
    │   }
    }
```

**NOTE: that the description and components are not included in the attribute
READ.**

To get these we need to use the GetDetailedPriceRequest command: This takes a
`details` bitmap which requests the type of detail to be returned:

    -   b0 = Description
    -   b1 = Components

i.e.

    -   details = 1 (Description ONLY)
    -   details = 2 (Components ONLY)
    -   details = 3 (Description & Components)

-   Step 7: (In chip-repl) Send `GetDetailedPriceRequest()`

```
In [15]: await devCtrl.SendCommand(200, 1, chip.clusters.CommodityPrice.Commands.GetDetailedPriceRequest(3))
Out[15]:

GetDetailedPriceResponse(
│   currentPrice=CommodityPriceStruct(
│   │   periodStart=799150761,
│   │   periodEnd=799152561,
│   │   price=15916,
│   │   priceLevel=3,
│   │   description='Medium',
│   │   components=[
│   │   │   CommodityPriceComponentStruct(
│   │   │   │   price=15120,
│   │   │   │   source=<TariffPriceTypeEnum.kStandard: 0>,
│   │   │   │   description='ExVAT',
│   │   │   │   tariffComponentID=None
│   │   │   ),
│   │   │   CommodityPriceComponentStruct(
│   │   │   │   price=795,
│   │   │   │   source=<TariffPriceTypeEnum.kStandard: 0>,
│   │   │   │   description='VAT',
│   │   │   │   tariffComponentID=None
│   │   │   )
│   │   ]
│   )
)
```

-   Step 8: (In chip-repl) Using TestEvent trigger
    `eventTrigger=0x0095000000000001` we can generate a test `PriceForecast`
    containing the price values for the next few hours

```python
    # Send a test event trigger NodeID 200, Endpoint 0, with eventTrigger=0x0095000000000001
    await devCtrl.SendCommand(200, 0, chip.clusters.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=bytes([b for b in range(16)]), eventTrigger=0x0095000000000001))
```

-   Step 9: (In chip-repl) Read `PriceForecast` attributes (see the values have
    changed)

```
    await devCtrl.ReadAttribute(200,[(1,chip.clusters.CommodityPrice.Attributes.PriceForecast)])
    Out[21]:

    {
    │   1: {
    │   │   <class 'chip.clusters.Objects.CommodityPrice'>: {
    │   │   │   <class 'chip.clusters.Attribute.DataVersion'>: 3672963492,
    │   │   │   <class 'chip.clusters.Objects.CommodityPrice.Attributes.PriceForecast'>: [
    │   │   │   │   CommodityPriceStruct(
    │   │   │   │   │   periodStart=799152701,
    │   │   │   │   │   periodEnd=799154500,
    │   │   │   │   │   price=27935,
    │   │   │   │   │   priceLevel=3,
    │   │   │   │   │   description=None,
    │   │   │   │   │   components=None
    │   │   │   │   ),
    │   │   │   │   CommodityPriceStruct(
    │   │   │   │   │   periodStart=799154501,
    │   │   │   │   │   periodEnd=799156300,
    │   │   │   │   │   price=23687,
    │   │   │   │   │   priceLevel=2,
    │   │   │   │   │   description=None,
    │   │   │   │   │   components=None
    │   │   │   │   ),
    │   │   │   │   CommodityPriceStruct(
    │   │   │   │   │   periodStart=799156301,
    │   │   │   │   │   periodEnd=799158100,
    │   │   │   │   │   price=18636,
    │   │   │   │   │   priceLevel=2,
    │   │   │   │   │   description=None,
    │   │   │   │   │   components=None
    │   │   │   │   ),
    │   │   │   │   CommodityPriceStruct(
    │   │   │   │   │   periodStart=799158101,
    │   │   │   │   │   periodEnd=799159900,
    │   │   │   │   │   price=8889,
    │   │   │   │   │   priceLevel=1,
    │   │   │   │   │   description=None,
    │   │   │   │   │   components=None
    │   │   │   │   ),
    ...
    │   │   │   │   CommodityPriceStruct(
    │   │   │   │   │   periodStart=799251701,
    │   │   │   │   │   periodEnd=799253500,
    │   │   │   │   │   price=24450,
    │   │   │   │   │   priceLevel=3,
    │   │   │   │   │   description=None,
    │   │   │   │   │   components=None
    │   │   │   │   )
    │   │   │   ]
    │   │   }
    │   }
    }
```

**NOTE: that the description and components are not included in the attribute
READ.**

To get these we need to use the GetDetailedForecastRequest command (**USES
TCP**):

This takes a `details` bitmap which requests the type of detail to be returned:

b0 = Description b1 = Components

i.e. details = 1 (Description ONLY) details = 2 (Components ONLY) details = 3
(Description & Components)

-   Step 10: (In chip-repl) Send `GetDetailedForecastRequest()`
-   Step 10a: First we need to ensure we connect with TCP (only needed once):

```python
    dev = await devCtrl.GetConnectedDevice(200, allowPASE=False, timeoutMs=1000, payloadCapability=chip.ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD)
```

-   Step 10b: Send the `GetDetailedForecastRequest()` command:

```
    await devCtrl.SendCommand(200, 1, chip.clusters.CommodityPrice.Commands.GetDetailedForecastRequest(3))
    Out[24]:

    GetDetailedForecastResponse(
    │   priceForecast=[
    │   │   CommodityPriceStruct(
    │   │   │   periodStart=799152701,
    │   │   │   periodEnd=799154500,
    │   │   │   price=27935,
    │   │   │   priceLevel=3,
    │   │   │   description='High',
    │   │   │   components=[
    │   │   │   │   CommodityPriceComponentStruct(
    │   │   │   │   │   price=26538,
    │   │   │   │   │   source=<TariffPriceTypeEnum.kStandard: 0>,
    │   │   │   │   │   description='ExVAT',
    │   │   │   │   │   tariffComponentID=None
    │   │   │   │   ),
    │   │   │   │   CommodityPriceComponentStruct(
    │   │   │   │   │   price=1396,
    │   │   │   │   │   source=<TariffPriceTypeEnum.kStandard: 0>,
    │   │   │   │   │   description='VAT',
    │   │   │   │   │   tariffComponentID=None
    │   │   │   │   )
    │   │   │   ]
    │   │   ),
    │   │   CommodityPriceStruct(
    │   │   │   periodStart=799154501,
    │   │   │   periodEnd=799156300,
    │   │   │   price=23687,
    │   │   │   priceLevel=2,
    │   │   │   description='Medium',
    │   │   │   components=[
    │   │   │   │   CommodityPriceComponentStruct(
    │   │   │   │   │   price=22502,
    │   │   │   │   │   source=<TariffPriceTypeEnum.kStandard: 0>,
    │   │   │   │   │   description='ExVAT',
    │   │   │   │   │   tariffComponentID=None
    │   │   │   │   ),
    │   │   │   │   CommodityPriceComponentStruct(
    │   │   │   │   │   price=1184,
    │   │   │   │   │   source=<TariffPriceTypeEnum.kStandard: 0>,
    │   │   │   │   │   description='VAT',
    │   │   │   │   │   tariffComponentID=None
    │   │   │   │   )
    │   │   │   ]
    │   │   ),
    ...
    │   │   CommodityPriceStruct(
    │   │   │   periodStart=799251701,
    │   │   │   periodEnd=799253500,
    │   │   │   price=24450,
    │   │   │   priceLevel=3,
    │   │   │   description='High',
    │   │   │   components=[
    │   │   │   │   CommodityPriceComponentStruct(
    │   │   │   │   │   price=23227,
    │   │   │   │   │   source=<TariffPriceTypeEnum.kStandard: 0>,
    │   │   │   │   │   description='ExVAT',
    │   │   │   │   │   tariffComponentID=None
    │   │   │   │   ),
    │   │   │   │   CommodityPriceComponentStruct(
    │   │   │   │   │   price=1222,
    │   │   │   │   │   source=<TariffPriceTypeEnum.kStandard: 0>,
    │   │   │   │   │   description='VAT',
    │   │   │   │   │   tariffComponentID=None
    │   │   │   │   )
    │   │   │   ]
    │   │   )
    │   ]
    )

```

#### ElectricalGridConditions cluster

This allows you to get current and forecast electrical grid conditions. This
assumes you have already commissioned the app (see above).

-   Step 1: (In chip-repl) Read `Electrical Grid Conditions` attributes

```
    # Read from NodeID 200, Endpoint 1, all attributes on ElectricalGridConditions cluster
    await devCtrl.ReadAttribute(200,[(1, chip.clusters.ElectricalGridConditions)])
Out[2]:

{
│   1: {
│   │   <class 'chip.clusters.Objects.ElectricalGridConditions'>: {
│   │   │   <class 'chip.clusters.Attribute.DataVersion'>: 2998541776,
│   │   │   <class 'chip.clusters.Objects.ElectricalGridConditions.Attributes.LocalGenerationAvailable'>: True,
│   │   │   <class 'chip.clusters.Objects.ElectricalGridConditions.Attributes.ClusterRevision'>: 1,
│   │   │   <class 'chip.clusters.Objects.ElectricalGridConditions.Attributes.ForecastConditions'>: [],
│   │   │   <class 'chip.clusters.Objects.ElectricalGridConditions.Attributes.AcceptedCommandList'>: [],
│   │   │   <class 'chip.clusters.Objects.ElectricalGridConditions.Attributes.FeatureMap'>: 1,
│   │   │   <class 'chip.clusters.Objects.ElectricalGridConditions.Attributes.CurrentConditions'>: Null,
│   │   │   <class 'chip.clusters.Objects.ElectricalGridConditions.Attributes.GeneratedCommandList'>: [],
│   │   │   <class 'chip.clusters.Objects.ElectricalGridConditions.Attributes.AttributeList'>: [
│   │   │   │   0,
│   │   │   │   1,
│   │   │   │   2,
│   │   │   │   65532,
│   │   │   │   65533,
│   │   │   │   65528,
│   │   │   │   65529,
│   │   │   │   65531
│   │   │   ]
│   │   }
│   }
}

```

The response in the default app is a null `CurrentConditions` and an empty
`ForecastConditions` attribute.

-   Step 2: (In chip-repl) Using TestEvent trigger
    `eventTrigger=0x00A0000000000000` we can generate a test `CurrentConditions`
    with sample data

```python
    # Send a test event trigger NodeID 200, Endpoint 0, with eventTrigger=0x00A0000000000000
    await devCtrl.SendCommand(200, 0, chip.clusters.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=bytes([b for b in range(16)]), eventTrigger=0x00A0000000000000))
```

-   Step 3: (In chip-repl) Re-Read `CurrentConditions` attribute (see the values
    have changed)

```
    await devCtrl.ReadAttribute(200,[(1, chip.clusters.ElectricalGridConditions.Attributes.CurrentConditions)])
    Out[7]:

    {
    │   1: {
    │   │   <class 'chip.clusters.Objects.ElectricalGridConditions'>: {
    │   │   │   <class 'chip.clusters.Attribute.DataVersion'>: 488127616,
    │   │   │   <class 'chip.clusters.Objects.ElectricalGridConditions.Attributes.CurrentConditions'>: ElectricalGridConditionsStruct(
    │   │   │   │   periodStart=799154050,
    │   │   │   │   periodEnd=799155850,
    │   │   │   │   gridCarbonIntensity=230,
    │   │   │   │   gridCarbonLevel=<ThreeLevelEnum.kMedium: 1>,
    │   │   │   │   localCarbonIntensity=0,
    │   │   │   │   localCarbonLevel=<ThreeLevelEnum.kLow: 0>
    │   │   │   )
    │   │   }
    │   }
    }
```

-   Step 4: (In chip-repl) Using TestEvent trigger
    `eventTrigger=0x00A0000000000001` we can generate a test
    `ForecastConditions` with sample data

```python
    # Send a test event trigger NodeID 200, Endpoint 0, with eventTrigger=0x00A0000000000001
    await devCtrl.SendCommand(200, 0, chip.clusters.GeneralDiagnostics.Commands.TestEventTrigger(enableKey=bytes([b for b in range(16)]), eventTrigger=0x00A0000000000001))
```

-   Step 5: (In chip-repl) Re-Read `ForecastConditions` attribute (see the
    values have changed)

```
    await devCtrl.ReadAttribute(200,[(1, chip.clusters.ElectricalGridConditions.Attributes.ForecastConditions)])
    Out[9]:

    {
    │   1: {
    │   │   <class 'chip.clusters.Objects.ElectricalGridConditions'>: {
    │   │   │   <class 'chip.clusters.Attribute.DataVersion'>: 488127617,
    │   │   │   <class 'chip.clusters.Objects.ElectricalGridConditions.Attributes.ForecastConditions'>: [
    │   │   │   │   ElectricalGridConditionsStruct(
    │   │   │   │   │   periodStart=799154301,
    │   │   │   │   │   periodEnd=799156100,
    │   │   │   │   │   gridCarbonIntensity=18,
    │   │   │   │   │   gridCarbonLevel=<ThreeLevelEnum.kLow: 0>,
    │   │   │   │   │   localCarbonIntensity=18,
    │   │   │   │   │   localCarbonLevel=<ThreeLevelEnum.kLow: 0>
    │   │   │   │   ),
    │   │   │   │   ElectricalGridConditionsStruct(
    │   │   │   │   │   periodStart=799156101,
    │   │   │   │   │   periodEnd=799157900,
    │   │   │   │   │   gridCarbonIntensity=399,
    │   │   │   │   │   gridCarbonLevel=<ThreeLevelEnum.kHigh: 2>,
    │   │   │   │   │   localCarbonIntensity=399,
    │   │   │   │   │   localCarbonLevel=<ThreeLevelEnum.kHigh: 2>
    │   │   │   │   ),
    │   │   │   │   ElectricalGridConditionsStruct(
    │   │   │   │   │   periodStart=799157901,
    │   │   │   │   │   periodEnd=799159700,
    │   │   │   │   │   gridCarbonIntensity=165,
    │   │   │   │   │   gridCarbonLevel=<ThreeLevelEnum.kMedium: 1>,
    │   │   │   │   │   localCarbonIntensity=165,
    │   │   │   │   │   localCarbonLevel=<ThreeLevelEnum.kMedium: 1>
    │   │   │   │   ),
    ...
    │   │   │   │   ElectricalGridConditionsStruct(
    │   │   │   │   │   periodStart=799238901,
    │   │   │   │   │   periodEnd=799240700,
    │   │   │   │   │   gridCarbonIntensity=130,
    │   │   │   │   │   gridCarbonLevel=<ThreeLevelEnum.kMedium: 1>,
    │   │   │   │   │   localCarbonIntensity=130,
    │   │   │   │   │   localCarbonLevel=<ThreeLevelEnum.kMedium: 1>
    │   │   │   │   )
    │   │   │   ]
    │   │   }
    │   }
    }
```
