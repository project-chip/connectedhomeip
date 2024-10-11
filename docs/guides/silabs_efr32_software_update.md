# Matter Software Update with EFR32 example applications

The Over The Air (OTA) Software Update functionality can be added to any of the
EFR32 example applications by passing the `chip_enable_ota_requestor=true`
option to the build script. This option is supposed to be enabled by default for
all of the EFR32 example applications.

## Running the OTA Download scenario

-   For Matter with OpenThread: Bring up the OpenThread Border Router as
    discussed in examples/lighting-app/silabs/README.md and get its operational
    dataset.

-   On a Linux or Darwin platform build the chip-tool and the ota-provider-app
    as follows:

           scripts/examples/gn_build_example.sh examples/chip-tool out/
           scripts/examples/gn_build_example.sh examples/ota-provider-app/linux out/debug chip_config_network_layer_ble=false

-   Build or download the Gecko Bootloader binary. Follow the instructions in
    "UG266: Silicon Labs Gecko Bootloader User’s Guide". For the bootloader
    using the external flash select the "external SPI" bootloader type
    configured with a single slot of at least 1000 KB. For the bootloader using
    the internal flash see the Internal Storage Bootloader section below.
    Pre-built binaries for some configurations are available at the following
    location, see README.md for details

           third_party/silabs/matter_support/matter/efr32/bootloader_binaries

-   Using the commander tool upload the bootloader to the device running the
    application.

-   Create a bootable image file (using the Lighting application image as an
    example):

           commander gbl create chip-efr32-lighting-example.gbl --app chip-efr32-lighting-example.s37

-   Create the Matter OTA file from the bootable image file:

           ./src/app/ota_image_tool.py create -v 0xFFF1 -p 0x8005 -vn 2 -vs "2.0" -da sha256 chip-efr32-lighting-example.gbl chip-efr32-lighting-example.ota

-   In a terminal start the Provider app passing to it the path to the Matter
    OTA file created in the previous step:

           rm -r /tmp/chip_kvs_provider
           ./out/debug/chip-ota-provider-app --KVS /tmp/chip_kvs_provider -f chip-efr32-lighting-example.ota

-   In a separate terminal run the chip-tool commands to provision the Provider:

           ./out/chip-tool pairing onnetwork 1 20202021
           ./out/chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}, {"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": null, "targets": null}]' 1 0

-   If the application device had been previously commissioned hold Button 0 for
    six seconds to factory-reset the device.

-   In the chip-tool terminal enter:

           ./out/chip-tool pairing ble-thread 2 hex:<operationalDataset> 20202021 3840

where operationalDataset is obtained from the OpenThread Border Router.

-   Once the commissioning process completes enter:

           ./out/chip-tool otasoftwareupdaterequestor announce-otaprovider 1 0 0 0 2 0

-   The application device will connect to the Provider and start the image
    download. Once the image is downloaded the device will reboot into the
    downloaded image.

## Internal Storage Bootloader

Internal storage bootloader for Matter OTA software update is supported on MG24
boards only. In this use case both the running image and the downloadable update
image must fit on the internal flash at the same time. This in turn requires
that both images are built with a reduced feature set such as disabled logging
and Matter shell. The following set of compile flags leaves out all the optional
features and results in the minimal image size:

           chip_detail_logging=false chip_automation_logging=false chip_progress_logging=false is_debug=false show_qr_code=false chip_build_libshell=false enable_openthread_cli=false chip_openthread_ftd=true

Using LZMA compression when building the .gbl file ( passing `--compress lzma`
parameter to the `commander gbl create` command) further reduces the downloaded
image size.

When building an internal storage bootloader the two key configuration
parameters are the Slot Start Address and Slot Size in the Bootloader Storage
Slot component. The storage slot must not overlap with the running image and the
NVM section of the flash. In other words, the slot start address must be greater
than the end of the running image address and the sum of the start address and
the slot size must be less than the address of the NVM section. The simplest way
to get the relevant addresses for the running image and NVM would be by using
the Silicon Labs `commander` tool (Device Info->Main Flash->Flash Map).

The pre-built bootloader binaries are configured with slot start address of
0x080EC000 and slot size of 548864

## Managing the Software Version

In order for the Provider to successfully serve the image to a device during the
OTA Software Update process the Software Version parameter that the .ota file
was built with must be greater than the
CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION parameter set in the application's
`CHIPProjectConfig.h` file. The Software Version parameter is set by the `-vn`
parameter passed to the `ota_image_tool.py create` command. For example, if the
application's running image was built with
CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION set to 1 and if the `.ota` file is
built with `-vn 2` then the Provider will serve the update image when requested.

In order for the OTA Software Update subsystem to consider an update to be
successful and for the NotifyUpdateApplied command to be transmitted the
CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION in the updated image must exceed the
software version of the running image (continuing the above example, the image
for the update must be built with CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION set
to 2).

## Managing the Vendor and Product ID

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

## Additional Info

Developers can find more resources on
[Silicon Labs Matter Community Page](https://community.silabs.com/s/article/connected-home-over-ip-chip-faq?language=en_US)
.
