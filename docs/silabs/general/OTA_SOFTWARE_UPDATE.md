# Matter Software Update with Silicon Labs Example Applications

This page outlines the steps for a scenario that demonstrates the Over
The Air (OTA) Software Update functionality of Matter.

Over The Air (OTA) Software Update functionality is enabled by default for
all Silicon Labs example applications.

## Hardware Requirements
- To run matter ota on Silicon Labs Platform, refer to: [Hardware Requirements](../general/HARDWARE_REQUIREMENTS.md)

## Software Requirements
- To run matter ota on Silicon Labs Platform, refer to: [Software Requirements](../general/SOFTWARE_REQUIREMENTS.md)

## Setting up OTA Environment

- **OTA-A**, the running image: a regular application built with the default/older software version value. This application will be updated to the one with a higher software version. In the OTA Software Update process it acts as the OTA Requestor.
- **OTA-B**, the update image: a regular application built with a higher software version value.
- **Chip-tool**: the controller that announces the OTA-Provider's address to the application thus triggering the OTA Software Update.
- **OTA-Provider**: the server that carries the update image and from which the OTA Requestor will download the updated software.
- **Bootloader**: Silicon Labs Gecko Bootloader image that supports OTA; supports the external (SPI-flash) or the internal storage option.


### Setting up Chip-tool 

The chip-tool binary is a part of the Silicon Labs' Matter Hub Raspberry Pi Image available as a part of the Release Artifacts page. If you are planning to run chip-tool on the Matter Hub you may skip the rest of this section.

If you have not downloaded or cloned this repository, you can run the following 
commands on a Linux terminal running on either Linux machine, WSL or Virtual
Machine to clone the repository and run bootstrap to prepare to build the sample
application images.

1. To download the [SiliconLabs Matter codebase](https://github.com/SiliconLabs/matter.git), run the following commands.

    ```shell
     $ git clone https://github.com/SiliconLabs/matter.git
    ```

2. Bootstrapping:

    ```shell
    $ cd matter
    $ ./scripts/checkout_submodules.py --shallow --recursive --platform efr32
    $ . scripts/bootstrap.sh
    # Create a directory where binaries will be updated/compiled called `out`
    $ mkdir out
    ```

    To control the  Matter application you will have to compile and run the chip-tool on either a Linux, Mac, or Raspberry Pi. The chip-tool builds faster on the Mac and Linux machines so that is recommended, but if you have access to a Raspberry Pi that will work as well.


3. Build the chip-tool

    ```shell
    $ ./scripts/examples/gn_build_example.sh examples/chip-tool out/standalone
    ```

This will build chip-tool in `out/standalone`.


### Setting up OTA-Provider

The chip-ota-provider-app binary for a Raspberry Pi is a part of the Artifacts package available with the Matter Extension release. If you are planning to run the OTA-Provider on a Raspberry Pi there is no need to build it. 

- To Build OTA-Provider app for Linux Platform, run below command in the matter repository.

```shell
    scripts/examples/gn_build_example.sh examples/ota-provider-app/linux out/debug chip_config_network_layer_ble=false
```

## Matter OTA Images
- To create and Build matter OTA from matter code, need two different applications to build mentioned below:
  - OTA A
  - OTA B

### Build OTA A Application
 - Matter OTA-A Application will be used to flash on the matter device.
 - In matter cloned directory run below commands to build an OTA A application.
```
For RS9116: 

./scripts/examples/gn_silabs_example.sh examples/lighting-app/silabs/ out/rs911x_lighting_A BRD41xxx disable_lcd=true use_external_flash=false chip_enable_ble_rs911x=true --wifi rs9116

For WF200

./scripts/examples/gn_silabs_example.sh examples/lighting-app/silabs/ out/wf200_lighting_A BRD41xxx chip_build_libshell=false --wifi wf200

For SiWx917 NCP

./scripts/examples/gn_silabs_example.sh examples/lighting-app/silabs/ out/siwx917_lighting BRD41xxx disable_lcd=true use_external_flash=false --wifi SiWx917

For SiWx917 SOC

./scripts/examples/gn_silabs_example.sh examples/lighting-app/silabs/ out/SiWx917_lighting BRD4388A

```
**Note:** Matter OTA-A application will be having software version as 1 by default in **third_party/silabs/BUILD.gn** file.

### Build OTA B Application
- Matter OTA-B application will be used to create gbl & OTA file.
- Open the file **third_party/silabs/BUILD.gn** from matter code cloned directory and modify **sl_matter_version = 1** to any number.

**Note:** Make sure always **sl_matter_version** should be greater than **sl_hardware_version**
- After Modifying software version build OTA B application using below commands for EFR boards

```
For RS9116: 

./scripts/examples/gn_silabs_example.sh examples/lighting-app/silabs/ out/rs911x_lighting_B BRD41xxx use_external_flash=false chip_enable_ble_rs911x=true --wifi rs9116

For WF200

./scripts/examples/gn_silabs_example.sh examples/lighting-app/silabs/ out/wf200_lighting_B BRD41xxx chip_build_libshell=false --wifi wf200

For SiWx917 NCP

./scripts/examples/gn_silabs_example.sh examples/lighting-app/silabs/ out/siwx917_lighting BRD41xxx disable_lcd=true use_external_flash=false --wifi SiWx917

For SiWx917 SOC

./scripts/examples/gn_silabs_example.sh examples/lighting-app/silabs/ out/SiWx917_lighting BRD4388A

```

## Running the OTA Download Scenario

- Create a bootable image file (using the Lighting application image as an
    example):

    ```shell
    commander gbl create chip-efr32-lighting-example.gbl --compress lzma --app chip-efr32-lighting-example.s37
    ```
**Note**:- Using LZMA compression when building the .gbl file ( passing `--compress lzma` parameter to the `commander gbl create` command) further reduces the downloaded image size.

- Create the Matter OTA file from the bootable image file:

    ```shell
    $ commander ota create --type matter --input chip-efr32-lighting-example.gbl --vendorid 0xFFF1 --productid 0x8005 --swstring "2.0" --swversion 2 --digest sha256 -o chip-efr32-lighting-example.ota
    ```

### Running OTA Provider

- Locate **ota-provider** terminal and start the Provider app passing to it the path to the Matter OTA file created in the previous step:

- In a terminal start the Provider app and pass to it the path to the Matter
    OTA file created in the previous step:

    ```shell
    $ rm -r /tmp/chip_kvs_provider
    ```

    ```shell
    ./out/chip-ota-provider-app  --KVS /tmp/chip_kvs_provider -f chip-efr32-lighting-example.ota
    ```

### Setting up OTA-Requestor

- Before running **ota-requestor** app flash the bootloader binary images for Silicon Labs Devices.

#### Flash Bootloader Images

- Bootloader binaries will be flashed using Simplicity Commander only. It will support EFR32 NCP Boards only.
- Silicon Labs Devices will supports below Bootloader variants for EFR32 Boards.
    - Internal Storage Bootloader
    - External Storage Bootloader
- To Flash the Bootloader Binary along with the application for Silicon Labs Device, refer [Flashing Binaries](../general/FLASHING_USING_COMMANDER.md)

### Running OTA-Requestor

- In a separate terminal run the chip-tool commands to provision the Provider:

    ```shell
    $ ./out/chip-tool pairing onnetwork 1 20202021
    ```

    ```shell
    $ ./out/chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}, {"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": null, "targets": null}]' 1 0
    ```
- For Matter over OpenThread, bring up the OpenThread Border Router and get its operational
    dataset, for Matter over WiFi bring up the AP. 
    
- If the application device had been previously commissioned, hold Button 0
    for six seconds to factory-reset the device.

- Commission the device. 
  For Matter over OpenThread:
   
    ```shell
    $ ./out/chip-tool pairing ble-thread 2 hex:<operationalDataset> 20202021 3840
    ```
    where operationalDataset is obtained from the OpenThread Border Router.

    For Matter over WiFi:
    ```shell
      ./out/chip-tool pairing ble-wifi "node_id" "SSID" "PSK" 20202021 3840
    ```

- Once the commissioning process completes enter:

    ```shell
    $ ./out/chip-tool otasoftwareupdaterequestor announce-otaprovider 1 0 0 0 2 0
    ```
- The application device will connect to the Provider and start the image download. Once the image is downloaded the device will reboot into the downloaded image.

## Additional Info

Developers can find more resources on
[Silicon Labs Matter Community Page](https://community.silabs.com/s/article/connected-home-over-ip-chip-faq?language=en_US).
