#CHIP EFR32 OTA Requestor Example

An example showing the use of the Matter OTA Requestor functionality on the
Silicon Labs EFR32 MG12.

<a name="intro"></a>

## Introduction

The EFR32 OTA Requestor example provides a baseline demonstration the Matter OTA
Requestor functionality built with the Silicon Labs gecko SDK. It can be
controlled by a Chip controller over Openthread network..

<a name="building"></a>

## Building

For initial setup steps please see the CHIP EFR32 Lighting Example README at
examples/lighting-app/efr32/README.md

-   Supported hardware:

    MG12 boards:

    -   BRD4161A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@19dBm
    -   BRD4164A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@19dBm
    -   BRD4166A / SLTB004A / Thunderboard Sense 2 / 2.4GHz@10dBm
    -   BRD4170A / SLWSTK6000B / Multiband Wireless Starter Kit / 2.4GHz@19dBm,
        915MHz@19dBm
    -   BRD4304A / SLWSTK6000B / MGM12P Module / 2.4GHz@19dBm

    MG21 boards: Currently not supported due to RAM limitation.

    -   BRD4180A / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@20dBm

    MG24 boards :

    -   BRD4186A / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@10dBm
    -   BRD4187A / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@20dBm

*   Build the example application:

          cd ~/connectedhomeip
          ./scripts/examples/gn_efr32_example.sh  ./examples/ota-requestor-app/efr32/ ./out/ota-requestor-app BRD4161A

-   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip
          $ rm -rf ./out/

## Flashing the Application

-   On the command line:

          $ cd ~/connectedhomeip/out/ota-requestor-app/BRD4161A
          $ python3 chip-efr32-ota-requestor-example.flash.py

-   Or with the Ozone debugger, just load the .out file.

<a name="view-logging"></a>

## Viewing Logging Output

See examples/lighting-app/efr32/README.md

## Running the OTA Download scenario

-   Bring up the Open Thread Border Router as discussed in
    examples/lighting-app/efr32/README.md and get its operational dataset.

-   On a Linux or Darwin platform build the chip-tool and the ota-provider-app
    as follows:

           scripts/examples/gn_build_example.sh examples/chip-tool out/
           scripts/examples/gn_build_example.sh examples/ota-provider-app/linux out/debug chip_config_network_layer_ble=false

-   In a terminal start the provider app passing to it the path to the image
    file that the Requestor is supposed to download (for example /tmp/ota.out):

                ./out/debug/chip-ota-provider-app -f /tmp/ota.out

-   In a separate terminal run the chip-tool commands to provision the Provider:

             rm -r /tmp/chip_*
             ./out/chip-tool pairing onnetwork 1 20202021

-   If the Requestor had been previously commissioned hold Button 0 for six
    seconds to factory-reset the device.

-   In the chip-tool terminal enter:

            ./out/chip-tool pairing ble-thread 2 hex:<operationalDataset> 73141520   3840

where operationalDataset is obtained from the Open Thread Border Router.

-   Once the commissioning process completes enter:

            ./out/chip-tool otasoftwareupdaterequestor announce-ota-provider 1 0 0 0 2 0

-   The Requestor will connect to the Provider and start the image download.
