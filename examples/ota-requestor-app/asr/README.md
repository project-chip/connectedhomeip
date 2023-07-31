# Matter ASR OTA Requestor Example

This example demonstrates the Matter OTA Requestor application on ASR platform.

---

-   [Matter ASR OTA Requestor Example](#matter-asr-ota-requestor-example)
    -   [Building and Commissioning](#building-and-commissioning)
    -   [Testing the example](#testing-the-example)

---

## Building and Commissioning

Please refer
[Building and Commissioning](../../../docs/guides/asr_getting_started_guide.md#building-the-example-application)
guides to get started

```
./scripts/build/build_examples.py --target asr-$ASR_BOARD-ota-requestor build
```

## Testing the example

-   After building a application, `*ota.bin` will generated automatically in the
    output directory.

-   Use
    [ota_image_tool](https://github.com/project-chip/connectedhomeip/blob/master/src/app/ota_image_tool.py)
    to generate the Matter OTA image. This tool can be used as follows, make
    sure the softwareVersion parameter must be greater than the
    `CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION` parameter set in the
    application's CHIPProjectConfig.h file.

    ```
    ./src/app/ota_image_tool.py create -v <Vendor ID> -p <Product ID> -vn 2 -vs "2.0" -da sha256 application_ota.bin matter_firmware_ota.bin
    ```

-   Run the Linux OTA Provider with OTA image.
    ```
    ./chip-ota-provider-app -f matter_firmware_ota.bin
    ```
-   OTA Provider commissioning in another Linux terminal.
    ```
    ./chip-tool pairing onnetwork 1 20202021
    ```
-   After OTA Provider commissioning is successful, use `chip-tool` to write ACL
    for OTA Provider.
    ```
    ./chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null },{"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": null, "targets": null }]' 1 0
    ```
-   Commission ota requestor device with node-id `OTA REQUESTOR APP NODE ID`
-   After OTA Requestor commissioning is successful, use `chip-tool` to inform
    OTA Provider to send OTA image to OTA Requestor.
    ```
    ./chip-tool otasoftwareupdaterequestor announce-otaprovider 1 0 0 0 <OTA REQUESTOR APP NODE ID> 0
    ```
