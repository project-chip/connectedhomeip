# Build OTA image

Take build target `bouffalolab-bl602dk-light-wifi-littlefs` as example to
introduce OTA image building.

After example compiled, a python script `chip-bl602-lighting-example.flash.py`
will be generated out under `./out/bouffalolab-bl602dk-light-wifi-littlefs/` and
is used to download image to `Bouffalo Lab` SoC.

Type following command to generated OTA images:

```shell
./out/bouffalolab-bl602dk-light-wifi-littlefs/chip-bl602-lighting-example.flash.py --build-ota --vendor-id <vendor id> --product-id <product id> --version <version number> --version-str <version number string> --digest-algorithm <digest algorithm>
```

Please find `./src/app/ota_image_tool.py` for information on `vendor id`,
`product id`, `version number`, `version number string` and `digest algorithm`.

Here is an example command to generate an OTA image,

> Please change `CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION` and
> `CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING`in CHIPProjectConfig.h
> under example folder before to build a test OTA image.

```shell
./out/bouffalolab-bl602dk-light-wifi-littlefs/chip-bl602-lighting-example.flash.py --build-ota --vendor-id 0xFFF1 --product-id 0x8005 --version 10 --version-str "1.0" --digest-algorithm sha256
```

OTA images will generated under
`out/bouffalolab-bl602dk-light-wifi-littlefs/ota_images`:

> BL702 and BL702L have same OTA image format with BL602

-   `chip-bl602dk-lighting-example.bin.hash.matter`, OTA image packed with raw
    firmware image.
-   `chip-bl602dk-lighting-example.bin.xz.hash.matter`, OTA image packed with
    compressed firmware image.

BL616 SoC platform uses different OTA image format. Take BL616D lighting app
with Wi-Fi and `littlefs` supported as example:

-   `chip-bl616-lighting-example.bin.ota.matter`, OTA image packed with raw
    firmware image.
-   `chip-bl616-lighting-example.xz.ota.matter`, OTA image packed with
    compressed firmware image.

> Please contact `Bouffalo Lab` for more security requirements on firmware and
> OTA images.

## Test OTA software upgrade with chip-tool and ota-provider-app

Please follow
[chip_tool_guide](../../development_controllers/chip-tool/chip_tool_guide.md)
and [chip-tool guide](../../../examples/chip-tool/README.md) for chip-tool build
and usage and follow
[ota-provider-app guide](../../../examples/ota-provider-app/linux/README.md) for
ota-provider-app build and usage.

### Start ota-provider-app

-   Start ota-provider-app with OTA compressed image.

    ```shell
    $ rm -r /tmp/chip_*
    $ out/linux-x64-ota-provider/chip-ota-provider-app -f out/bouffalolab-bl602dk-light-wifi-littlefs/ota_images/chip-bl602-lighting-example.bin.xz.hash.matter
    ```

-   Provision ota-provider-app with node id assigned
    ```shell
    $ ./out/linux-x64-chip-tool/chip-tool pairing onnetwork <ota_provider_node_id> 20202021
    $ ./out/linux-x64-chip-tool/chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}, {"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": null, "targets": null}]' <ota_provider_node_id> 0
    ```

### Start ota software upgrade

-   BLE commission with node id assigned

    -   Wi-Fi

    ```shell
    ./out/linux-x64-chip-tool/chip-tool pairing ble-wifi <device_node_id> <wifi_ssid> <wifi_passwd> 20202021 3840
    ```

    -   Thread

        ```shell
        ./out/linux-x64-chip-tool/chip-tool pairing ble-thread <device_node_id> hex:<thread_operational_dataset> 20202021 3840
        ```

    -   Ethernet
        ```
        ./out/linux-x64-chip-tool/chip-tool pairing onnetwork <device_node_id> 20202021
        ```

-   Start OTA software upgrade process

    ```shell
    ./out/linux-x64-chip-tool/chip-tool otasoftwareupdaterequestor announce-otaprovider <ota_provider_node_id> 0 0 0 <device_node_id> 0
    ```

-   After the OTA software upgrade completes, the device will reboot
    automatically. Once rebooted, execute the following commands to verify if
    the new firmware is applied:

    ```
    ./out/linux-x64-chip-tool/chip-tool basicinformation read software-version <device_node_id> 0
    ./out/linux-x64-chip-tool/chip-tool basicinformation read software-version-string <device_node_id> 0
    ```
