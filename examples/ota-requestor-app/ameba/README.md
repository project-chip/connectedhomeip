# CHIP Ameba OTA Requestor Example

A prototype application that demonstrates OTA Requestor capabilities.

## Building the Example Application

-   Pull docker image:

          $ docker pull connectedhomeip/chip-build-ameba:latest

-   Run docker container:

          $ docker run -it -v ${CHIP_DIR}:/root/chip connectedhomeip/chip-build-ameba:latest

-   Setup build environment:

          $ source ./scripts/bootstrap.sh

-   To build the demo application:

          $ ./scripts/build/build_examples.py --target ameba-amebad-ota-requestor build

    The output image files are stored in
    `out/ameba-amebad-ota-requestor/asdk/image` folder.

    The bootloader image files are stored in
    `out/ameba-amebad-ota-requestor/asdk/bootloader` folder.

-   After building the application, **Ameba Image Tool** is used to flash it to
    Ameba board.

1. Connect your device via USB and open Ameba Image Tool.
2. Select correct serial port and set baudrate as **115200**.
3. Browse and add the corresponding image files in the Flash Download list to
   the correct locations
4. Click **Download** button.

## Testing the Example Application

Launch the linux [ota-provider-app](../../ota-provider-app/linux) and provide
the SW_IMAGE_FILE

    $ ./chip-ota-provider-app -f ${SW_IMAGE_FILE}

Commission the ota-provider-app using
[chip-tool](https://github.com/project-chip/connectedhomeip/tree/master/examples/chip-tool)

    $ ./chip-tool pairing onnetwork 1 20202021

Input `ATS$` command to start the CHIP ota-requestor task, then use chip-tool to
commission it

    $ ./chip-tool pairing ble-wifi 2 <SSID> <PASSWORD> 20202021 3840

After commissioning the ota-requestor, use the chip-tool to announce the
ota-provider-app to start the OTA process

    $ ./chip-tool otasoftwareupdaterequestor announce-ota-provider 1 0 0 0 2 0

The OTA process should include downloading the image, verification of image
header, erasing upgraded flash partition, writing to flash and checksum
verification
