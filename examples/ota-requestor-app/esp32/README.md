# CHIP ESP32 OTA Requestor Example

A prototype application that demonstrates OTA Requestor capabilities.

## Prerequisite

Before moving ahead, make sure you have
[OTA Provider](../../ota-provider-app/esp32) is commissioned and running.

## Supported Devices

-   This example supports ESP32 and ESP32C3. For details please check
    [here](https://github.com/project-chip/connectedhomeip/tree/master/examples/all-clusters-app/esp32#supported-devices).

## Building the Example Application

-   If you are building for the first time please check
    [Building the Example Application](https://github.com/project-chip/connectedhomeip/tree/master/examples/all-clusters-app/esp32#building-the-example-application)
    guide.
-   Otherwise, `idf.py build` works!

## Flashing the Example Application

```
idf.py -p <OTARequestorSerialPort> flash monitor
```

## Commissioning over BLE using chip-tool

-   Please build the standalone chip-tool as described [here](../../chip-tool)
-   Commissioning the OTA Requestor

```
./out/debug/chip-tool pairing ble-wifi 12346 <ssid> <passphrase> 20202021 3840
```

## Query for an OTA Image

After commissioning is successful, announce OTA provider's presence using
chip-tool. On receiving this command OTA requestor will query for OTA image.

```
./out/debug/chip-tool otasoftwareupdaterequestor announce-ota-provider 12345 0 0 0 12346 0
```

## Apply update

Once the transfer is complete, OTA requestor sends ApplyUpdateRequest command to
OTA provider for applying the image. Device will restart on successful
application of OTA image.

## ESP32 OTA Requestor with Linux OTA Provider

-   Build the [Linux OTA Provider](../../ota-provider-app/linux)
-   Run the Linux OTA Provider with OTA image.

```
./out/debug/chip-ota-provider-app -f hello-world.bin
```

-   Provision the Linux OTA Provider using chip-tool

```
./out/debug/chip-tool pairing onnetwork 12345 20202021
```

## Generate OTA image

User can generate the Matter OTA image by simply enabling
`CONFIG_CHIP_OTA_IMAGE_BUILD` config option. OTA image is generated in `build`
directory with name `<project name>-ota.bin`. This image then can be used with
OTA Provider Application.

Please make sure that version number is set to correct value. Use
`CONFIG_DEVICE_SOFTWARE_VERSION` and `CONFIG_DEVICE_SOFTWARE_VERSION_NUMBER`
config options for setting software version.

Matter OTA image can also be generated using
[ota_image_tool.py](https://github.com/project-chip/connectedhomeip/blob/master/src/app/ota_image_tool.py)
script.
