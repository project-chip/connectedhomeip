# CHIP ESP32 OTA Provider Example

A prototype application that demonstrates OTA provider capabilities.

## Supported Devices

-   This example supports ESP32 and ESP32C3. For details please check
    [here](https://github.com/shubhamdp/connectedhomeip/tree/shubhamdp-patch-1/examples/all-clusters-app/esp32#supported-devices).

## Building the Example Application

-   If you are building for the first time please check
    [Building the Example Application](https://github.com/shubhamdp/connectedhomeip/tree/shubhamdp-patch-1/examples/all-clusters-app/esp32#building-the-example-application)
    guide.
-   Otherwise, `idf.py build` works!

## Flashing the Example Application

```
idf.py -p <OTAProviderSerialPort> flash
```

## Flashing the hello-world.bin OTA image

Flash hello-world OTA image on OTA Provider's "ota_data" flash partition. Please
find hello-world.bin
[here](http://shubhamdp.github.io/esp_ota/esp32/hello-world-flash-in-ota-provider-partition.bin).
This OTA image is built for ESP32, it will not work on other devices. This is
the OTA upgrade image and will be sent to OTA requestor.

```
esptool.py -p <ProviderSerialPort> write_flash 0x206400 hello-world-flash-in-ota-provider-partition.bin
```

NOTE: This is a modified binary which contains the size of OTA image at first 4
bytes.

Run the idf monitor

```
idf.py -p <OTAProviderSerialPort> monitor
```

## Commissioning over BLE using chip-tool

-   Please build the standalone chip-tool as described [here](../../chip-tool)
-   Commissioning the OTA Provider

```
./out/debug/chip-tool pairing ble-wifi 12345 <ssid> <passphrase> 0 20202021 3841
```

---

Please note down the IP Address and Node ID of OTA Provider, these are required
for [OTA Requestor Example](../../ota-requestor-app/esp32). Once OTA provider is
commissioned then head over to
[OTA Requestor Example](../../ota-requestor-app/esp32).

---

## Features

-   Can complete full BDX transfer
-   Provide the full OTA image to Requestor

## Limitations

-   Synchronous BDX transfer only
-   Does not check VID/PID
-   Only one transfer at a time
-   Does not check incoming UpdateTokens
-   Does not support the header defined in Matter Specification.
