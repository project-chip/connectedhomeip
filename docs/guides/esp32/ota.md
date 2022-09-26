# Matter OTA

## Generate CHIP OTA image

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

## Enabling OTA Requestor

-   Please make sure `CONFIG_ENABLE_OTA_REQUESTOR` config option is enabled for
    enabling OTA requestor feature.
-   Currently all-clusters-app, lighting-app, and ota-requestor-app supports OTA
    requestor functionality.
-   Build and flash any supported app, and commission it.

## Setup OTA Provider app

Setup any of the OTA Provider, commission it and install the appropriate access
control list.

-   [Linux OTA Provider](../../../examples/ota-provider-app/linux)
-   [ESP32 OTA Provider](../../../examples/ota-provider-app/esp32)

## Query for an OTA Image

### Using Console

After commissioning is successful, press Enter in requestor device console and
type below query.

```
>matter ota query 1 <PROVIDER NODE ID> 0
```

Once the transfer is complete, OTA requestor sends ApplyUpdateRequest command to
OTA provider for applying the image. Device will restart on successful
application of OTA image.

### Using chip-tool

After commissioning is successful, announce OTA provider's presence using
chip-tool. On receiving this command OTA requestor will query for OTA image.

```
./out/debug/chip-tool otasoftwareupdaterequestor announce-ota-provider <PROVIDER NODE ID> 0 0 0 <REQUESTOR NODE ID> 0
```
