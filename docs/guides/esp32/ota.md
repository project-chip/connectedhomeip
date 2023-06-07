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

## Encrypted OTA

ESP32 supports transferring encrypted OTA images. Currently, an application
image can be encrypted/decrypted using an RSA-3072 key pair.

### Firmware Changes

-   Enable configuration options for OTA requestor and Encrypted OTA:

    ```
    CONFIG_ENABLE_OTA_REQUESTOR=y
    CONFIG_ENABLE_ENCRYPTED_OTA=y
    ```

-   Applications need to provide the key pair to the OTA image processor using
    the `InitEncryptedOTA()` API to decrypt the received OTA image.

-   For testing purposes, in `examples/lighting-app/esp32`, there is a logic of
    embedding the private key in the firmware. To quickly test, please generate
    the key pair and rename it as `esp_image_encryption_public_key.pem` and copy
    it to directory `examples/lighting-app/esp32`.

Please follow the steps below to generate an application image for OTA upgrades:

1. Generate a new RSA-3072 key pair or use an existing one.

    - To generate a key pair, use the following command:

        ```
        openssl genrsa -out esp_image_encryption_key.pem 3072
        ```

    - Extract the public key from the key pair:
        ```
        openssl rsa -in esp_image_encryption_key.pem -pubout -out esp_image_encryption_public_key.pem
        ```

2. Encrypt the application binary using the
   [esp_enc_img_gen.py](https://github.com/espressif/idf-extra-components/blob/master/esp_encrypted_img/tools/esp_enc_img_gen.py)
   script.

    - Use the following command to encrypt the OTA image with the public key:

        ```
        python3 esp_enc_img_gen.py encrypt lighting-app.bin esp_image_encryption_public_key.pem lighting-app-encrypted.bin
        ```

    - Append the Matter OTA header:
        ```
        src/app/ota_image_tool.py create --vendor-id 0xFFF1 --product-id 0x8000 --version 2 --version-str "v2.0" -da sha256 lighting-app-encrypted.bin lighting-app-encrypted-ota.bin
        ```

3. Use the `lighting-app-encrypted-ota.bin` file with the OTA Provider app.
