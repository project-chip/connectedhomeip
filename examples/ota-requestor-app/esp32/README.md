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
./out/debug/chip-tool pairing ble-wifi 12346 <ssid> <passphrase> 0 20202021 3840
```

## Query for an OTA Image

After commissioning is successful, query for OTA image. Head over to ESP32
console and fire the following command. This command creates a CASE session with
OTA Provider and start the OTA image transfer using BDX protocol.

```
esp32> QueryImage <OTAProviderIpAddress> <OTAProviderNodeId>
```

OTA image transfer takes some time and once done it prints following log

```
I (322620) OTARequesterImpl: Transfer complete!
```

## Apply update request

Once transfer is complete OTA Requestor should take permission from the OTA
Provider for applying the OTA image.

```
esp32> ApplyUpdateRequest <OTAProviderIpAddress> <OTAProviderNodeId>
```

After this step device should reboot and start running hello world example.

## ESP32 OTA Requestor with Linux OTA Provider

-   Build the [Linux OTA Provider](../../ota-provider-app/linux)
-   Run the Linux OTA Provider with
    [hello world OTA image](http://shubhamdp.github.io/esp_ota/esp32/hello-world-for-linux-provider.bin).
    This OTA image is built for ESP32, it will not work on other devices.

```
./out/debug/chip-ota-provider-app -f hello-world-for-linux-provider.bin
```

-   Provision the Linux OTA Provider using chip-tool

```
./out/debug/chip-tool pairing 12345 20202021
```

-   Note down the OTA Provider IP address and Node Id and repeat the steps for
    ESP32 OTA Requestor app

---

## Features

-   Can perform the actual OTA if the image provided is valid
-   Code for running a full BDX download exists in BDX
-   Can send QueryImage and ApplyUpdateRequest commands triggered from console
-   Downloads a file over BDX served by an OTA Provider server

## Limitations

-   Does not verify QueryImageResponse status
-   Does not support AnnounceOTAProvider command or OTA Requestor attributes
-   Does not support the header defined in Matter Specification.
