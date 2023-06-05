# Matter ASR OTA Requestor Example

This example demonstrates the Matter OTA Requestor application on ASR platform.

---

-   [Matter ASR OTA Requestor Example](#matter-asr-ota-requestor-example)
    -   [Supported Chips](#supported-chips)
    -   [Building and Commissioning](#building-and-commissioning)
    -   [Testing the example](#testing-the-example)

---

## Supported Chips

The Matter demo application is supported on:

-   ASR582X
-   ASR595X

## Building and Commissioning

Please refer
[Building and Commissioning](../../../docs/guides/asr_getting_started_guide.md#building-the-example-application)
guides to get started

```
./scripts/build/build_examples.py --target asr-$ASR_BOARD-ota-requestor build
```

## Testing the example

-   Make OTA image: Taking lighting-app as an example, light project must
    compiled before this operation:

    ```
    third_party/asr/asr582x/asr_sdk/tools/otaImage/image_gen_header out/asr-asr582x-lighting/chip-asr-lighting-example.bin flash_remapping
    ```

    After that, `chip-asr-lighting-example.ota.bin` will generated in the
    directory `./out/asr-asr582x-lighting/`.

-   Run the Linux OTA Provider with OTA image.
    ```
    ./chip-ota-provider-app -f chip-asr-lighting-example.ota.bin
    ```
-   OTA Provider commissioning in another Linux terminal.
    ```
    ./chip-tool pairing onnetwork 1 20202021
    ```
-   After OTA Provider commissioning is successful, use `chip-tool` to write ACL
    for OAT Provider.
    ```
    ./chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null },{"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": null, "targets": null }]' <OTA REQUESTOR APP NODE ID> 0
    ```
-   Commission ota requestor device with node-id `OTA REQUESTOR APP NODE ID`
-   After OTA Requestor commissioning is successful, use `chip-tool` to inform
    OTA Provider to send OTA image to OTA Requestor.
    ```
    ./chip-tool otasoftwareupdaterequestor announce-ota-provider 1 0 0 0 <OTA REQUESTOR APP NODE ID> 0
    ```
