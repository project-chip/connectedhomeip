# Matter Linux Air Quality Example

An example showing the use of Matter on the Linux. The document will describe
how to build and run Matter Linux Air Quality Example on Raspberry Pi. This doc
is tested on **Ubuntu for Raspberry Pi Server 20.04 LTS (aarch64)** and **Ubuntu
for Raspberry Pi Desktop 20.10 (aarch64)**

To cross-compile this example on x64 host and run on **NXP i.MX 8M Mini**
**EVK**, see the associated
[README document](../../../docs/guides/nxp_imx8m_linux_examples.md) for details.

<hr>

-   [Matter Linux Air Quality Example](#matter-linux-air-quality-example)
    -   [Building](#building)
    -   [Commandline Arguments](#commandline-arguments)
    -   [Running the Complete Example on Raspberry Pi 4](#running-the-complete-example-on-raspberry-pi-4)
    -   [Trigger event using air-quality-sensor-app event named pipe](#trigger-event-using-air-quality-sensor-app-event-named-pipe)

<hr>

## Building

-   Install tool chain

          $ sudo apt-get install git gcc g++ python pkg-config libssl-dev libdbus-1-dev libglib2.0-dev ninja-build python3-venv python3-dev unzip

-   Build the example application:

          $ cd ~/connectedhomeip/examples/air-quality-sensor-app/linux
          $ git submodule update --init
          $ source third_party/connectedhomeip/scripts/activate.sh
          $ gn gen out/debug
          $ ninja -C out/debug

-   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip/examples/air-quality-sensor-app/linux
          $ rm -rf out/

-   Build the example with pigweed RPC

          $ cd ~/connectedhomeip/examples/air-quality-sensor-app/linux
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

## Running the Complete Example on Raspberry Pi 4

>     gn gen out/debug
>     ninja -C out/debug

-   Prerequisites

    1. A Raspberry Pi 4 board
    2. A USB Bluetooth Dongle, Ubuntu desktop will send Bluetooth advertisement,
       which will block Matter from connecting via BLE. On Ubuntu server, you
       need to install `pi-bluetooth` via APT.
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

        -   Run Linux Air Quality Example App

                  $ cd ~/connectedhomeip/examples/air-quality-sensor-app/linux
                  $ sudo out/debug/chip-air-quality-sensor-app --ble-device [bluetooth device number]
                  # In this example, the device we want to use is hci1
                  $ sudo out/debug/chip-air-quality-sensor-app --ble-device 1

        -   Test the device using ChipDeviceController on your laptop /
            workstation etc.

## Trigger event using air-quality-sensor-app event named pipe

You can send a command to air-quality-sensor-app to trigger specific event via
air-quality-sensor-app event named pipe /tmp/chip_air_quality_fifo\*<PID>.

### Trigger air quality change event

Generate event `AirQuality`, to change the air quality value.

```
$ echo '{"Name":"AirQuality","NewValue":3}' > /tmp/chip_air_quality_fifo_<PID>
```

### Trigger Temperature change event

Generate event `TemperatureMeasurement`, to change the temperate value.

```
$ echo '{"Name":"TemperatureMeasurement","NewValue":1800}' > /tmp/chip_air_quality_fifo_<PID>
```

### Trigger Humidity change event

Generate event `RelativeHumidityMeasurement`, to change the relative humidity
value (6000 for 60,0 %).

```
$ echo '{"Name":"RelativeHumidityMeasurement","NewValue":6000}' > /tmp/chip_air_quality_fifo_<PID>
```

### Trigger concentration change event

Concentration change events can be trigger on the concentration measurement
clusters.

Generate event `CarbonDioxideConcentrationMeasurement`, to change the CO2 value.

```
$ echo '{"Name":"CarbonDioxideConcentrationMeasurement","NewValue":400}' > /tmp/chip_air_quality_fifo_<PID>
```

Generate event `CarbonMonoxideConcentrationMeasurement`, to change the CO value.

```
$ echo '{"Name":"CarbonMonoxideConcentrationMeasurement","NewValue":1}' > /tmp/chip_air_quality_fifo_<PID>
```

Generate event `NitrogenDioxideConcentrationMeasurement`, to change the NO₂
value.

```
$ echo '{"Name":"NitrogenDioxideConcentrationMeasurement","NewValue":1}' > /tmp/chip_air_quality_fifo_<PID>
```

Generate event `Pm1ConcentrationMeasurement`, to change the PM1 value.

```
echo '{"Name":"Pm1ConcentrationMeasurement","NewValue":1}' > /tmp/chip_air_quality_fifo_<PID>
```

Generate event `Pm25ConcentrationMeasurement`, to change the PM2.5 value.

```
echo '{"Name":"Pm25ConcentrationMeasurement","NewValue":2.5}' > /tmp/chip_air_quality_fifo_<PID>
```

Generate event `Pm10ConcentrationMeasurement`, to change the PM10 value.

```
echo '{"Name":"Pm10ConcentrationMeasurement","NewValue":10}' > /tmp/chip_air_quality_fifo_<PID>
```

Generate event `TotalVolatileOrganicCompoundsConcentrationMeasurement`, to
change the TVOC value.

```
$ echo '{"Name":"TotalVolatileOrganicCompoundsConcentrationMeasurement","NewValue":100}' > /tmp/chip_air_quality_fifo_<PID>
```
