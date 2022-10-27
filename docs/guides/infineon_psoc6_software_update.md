# Matter Software Update with Infineon PSoC6 example applications

The Over The Air (OTA) Software Update functionality can be added to any of the
Infineon PSoC6 example applications by passing the
`chip_enable_ota_requestor=true` option to the build script.

## Running the OTA Download scenario

-   An OTA Provider is a node that can respond to the OTA Requestors' queries
    about available software updates and share the update packages with them. An
    OTA Requestor is any node that needs to be updated and can communicate with
    the OTA Provider to fetch applicable software updates. In the procedure
    described below, the OTA Provider will be a Linux application and the
    example running on the Infineon PSoC6 board will work as the OTA Requestor.

-   On a Linux or Darwin platform build the chip-tool and the ota-provider-app
    as follows:

           ```
           scripts/examples/gn_build_example.sh examples/chip-tool out/
           scripts/examples/gn_build_example.sh examples/ota-provider-app/linux out/debug chip_config_network_layer_ble=false
           ```

-   Build the PSoC6 OTA Requestor application from the chip root dir:

           ```
           $ scripts/build/build_examples.py --enable-flashbundle --target infineon-psoc6-lock-ota build
           $ third_party/infineon/psoc6/psoc6_sdk/ota/ota_base_build.sh out/infineon-psoc6-lock-ota chip-psoc6-lock-example
           ```

-   Build the PSoC6 OTA Update application from the chip root dir and create OTA
    file

           ```
           $ scripts/build/build_examples.py --enable-flashbundle --no-log-timestamps --target infineon-psoc6-lock-ota-updateimage build
           $ third_party/infineon/psoc6/psoc6_sdk/ota/ota_update_build.sh out/infineon-psoc6-lock-ota-updateimage chip-psoc6-lock-example
           ```

*   Additionally a pre-compiled bootloader must be flashed to the board using
    [Cypress Programmer](https://softwaretools.infineon.com/tools/com.ifx.tb.tool.cypressprogrammer).
    This image can be found at:

          $ ./third_party/infineon/psoc6/psoc6_sdk/ota/matter-psoc6-mcuboot-bootloader.hex

*   In a terminal start the Provider app passing to it the path to the Matter
    OTA file created in the previous step:(output of ota_update_build step)

           ```
           rm -r /tmp/chip_*
           ./out/debug/chip-ota-provider-app -f chip-psoc6-lock-example.ota
           ```

*   In a separate terminal run the chip-tool commands to provision the Provider:

           ```
           ./out/chip-tool pairing onnetwork 1 20202021
           ./out/chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}, {"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": null, "targets": null}]' 1 0
           ```

*   If the application device had been previously commissioned press USER Button
    2 to factory-reset the device.

*   In the chip-tool terminal enter:

           ```
           ./out/chip-tool pairing ble-wifi 2 <WIFI_SSID> <WIFI_PASSWORD> 20202021 3840
           ```

*   Once the commissioning process completes enter:

           ```
           ./out/chip-tool otasoftwareupdaterequestor announce-ota-provider 1 0 0 0 2 0
           ```

*   The application device will connect to the Provider and start the image
    download. Status of the transfer can be monitored in the OTA Provider
    terminal. Once the image is downloaded the device will reboot into the
    downloaded image.

In order for the Provider to successfully serve the image to a device during the
OTA Software Update process the softwareVersion parameter in the Provider config
file must be greater than the CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION
parameter set in the application's CHIPProjectConfig.h file.
