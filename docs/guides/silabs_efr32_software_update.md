# Matter Software Update with EFR32 example applications

The Over The Air (OTA) Software Update functionality can be added to any of the
EFR32 example applications by passing the `chip_enable_ota_requestor=true`
option to the build script. This option is supposed to be enabled by default for
all of the EFR32 example applications.

## Running the OTA Download scenario

-   For Matter with OpenThread: Bring up the OpenThread Border Router as
    discussed in examples/lighting-app/efr32/README.md and get its operational
    dataset.

-   On a Linux or Darwin platform build the chip-tool and the ota-provider-app
    as follows:

           ```
           scripts/examples/gn_build_example.sh examples/chip-tool out/
           scripts/examples/gn_build_example.sh examples/ota-provider-app/linux out/debug chip_config_network_layer_ble=false
           ```

-   Build or download the Gecko Bootloader binary. Bootloader should be built
    with the Gecko SDK version 3.2.1 or earlier. For the bootloader using the
    external flash select the "external SPI" bootloader type configured with a
    single slot of at least 1000 KB. For the bootloader using the internal flash
    (supported on MG24 boards only) select the "internal storage" bootloader
    type. Follow the instructions in "UG266: Silicon Labs Gecko Bootloader
    User’s Guide". Pre-built binaries for some configurations should be
    available in

           ```
           third_party/efr32_sdk/repo/platform/bootloader/sample-apps/bootloader-storage-spiflash-single
           ```

-   Using the commander tool upload the bootloader to the device running the
    application.

-   Create a bootable image file (using the Lighting application image as an
    example):

           ```
           commander gbl create chip-efr32-lighting-example.gbl --app chip-efr32-lighting-example.s37
           ```

-   Create the Matter OTA file from the bootable image file:

           ```
           ./src/app/ota_image_tool.py create -v 0xFFF1 -p 0x8005 -vn 1 -vs "1.0" -da sha256 chip-efr32-lighting-example.gbl chip-efr32-lighting-example.ota
           ```

-   In a terminal start the Provider app passing to it the path to the Matter
    OTA file created in the previous step:

           ```
           rm -r /tmp/chip_*
           ./out/debug/chip-ota-provider-app -f chip-efr32-lighting-example.ota
           ```

-   In a separate terminal run the chip-tool commands to provision the Provider:

           ```
           ./out/chip-tool pairing onnetwork 1 20202021
           ./out/chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}, {"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": null, "targets": null}]' 1 0
           ```

-   If the application device had been previously commissioned hold Button 0 for
    six seconds to factory-reset the device.

-   In the chip-tool terminal enter:

           ```
           ./out/chip-tool pairing ble-thread 2 hex:<operationalDataset> 20202021 3840
           ```

where operationalDataset is obtained from the OpenThread Border Router.

-   Once the commissioning process completes enter:

           ```
           ./out/chip-tool otasoftwareupdaterequestor announce-ota-provider 1 0 0 0 2 0
           ```

-   The application device will connect to the Provider and start the image
    download. Once the image is downloaded the device will reboot into the
    downloaded image.

## Managing the Software Version, Vendor and Product ID

Starting the ota-provider-app with the --otaImageList command line option allows
the user to supply a JSON file specifying the Software Version, Vendor and
Product ID that identify the image served by the Provider, see
[ota-provider-app](../../examples/ota-provider-app/linux/README.md)

Example provider configuration file:

```
{ "foo": 1, // ignored by parser
  "deviceSoftwareVersionModel":
  [
      { "vendorId": 65521, "productId": 32773, "softwareVersion": 1, "softwareVersionString": "1.0.0", "cDVersionNumber": 18, "softwareVersionValid": true, "minApplicableSoftwareVersion": 0, "maxApplicableSoftwareVersion": 100, "otaURL": "chip-efr32-lighting-example.ota" }
  ]
}
```

In order for the Provider to successfully serve the image to a device during the
OTA Software Update process the softwareVersion parameter in the Provider config
file must be greater than the CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION
parameter set in the application's CHIPProjectConfig.h file.
