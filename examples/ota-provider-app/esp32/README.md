# CHIP ESP32 OTA Provider Example

A prototype application that demonstrates OTA provider capabilities.

## Supported Devices

-   This example supports ESP32 and ESP32C3. For details please check
    [here](https://github.com/project-chip/connectedhomeip/tree/master/examples/all-clusters-app/esp32#supported-devices).

## Copy the OTA image

-   Copy the binary file which you are going to send to the OTA Requestor to
    `./spiffs_image` path and make sure the `OTA_IMAGE_NAME` is the same as the
    name of the image file you placed in `./spiffs_image`.

```
idf.py menuconfig
```

-   Edit the `OTA_IMAGE_NAME` through `Demo`->`OTA image file name`.

## Building the Example Application

-   If you are building for the first time please check
    [Building the Example Application](https://github.com/project-chip/connectedhomeip/tree/master/examples/all-clusters-app/esp32#building-the-example-application)
    guide.
-   Otherwise, `idf.py build` works!

## Flashing the Example Application

```
idf.py -p <OTAProviderSerialPort> flash
```

## Commissioning over BLE using chip-tool

-   Please build the standalone chip-tool as described [here](../../chip-tool)
-   Commissioning the OTA Provider

```
./out/debug/chip-tool pairing ble-wifi 12345 <ssid> <passphrase> 20202021 3841
```

## Set delayed action time (Optional)

-   Set delayed action time in the Query Image Response and Apply Update
    Response, default is zero.

```
> matter OTAProvider delay <delay seconds>
```

---

Once OTA provider is commissioned then head over to
[OTA Requestor Example](../../ota-requestor-app/esp32).
