# CHIP Ameba OTA Requestor Example

A prototype application that demonstrates OTA Requestor capabilities.

## Building the Example Application

-   Pull docker image:

          $ docker pull ghcr.io/project-chip/chip-build-ameba:119

-   Run docker container:

          $ docker run -it -v ${CHIP_DIR}:/root/chip ghcr.io/project-chip/chip-build-ameba:119

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

1.  Commission Ameba (ota-requestor-app) using chip-tool

          $ ./chip-tool pairing ble-wifi 1 <SSID> <PASSWORD> 20202021 3840

2.  Launch the linux [ota-provider-app](../../ota-provider-app/linux) and
    provide the SW_IMAGE_FILE

              $ ./chip-ota-provider-app -f ${SW_IMAGE_FILE}

3.  Commission the ota-provider-app using
    [chip-tool](https://github.com/project-chip/connectedhomeip/tree/master/examples/chip-tool)

              $ ./chip-tool pairing onnetwork 2 20202021

4.  Write the Default OTA providers into Ameba

          $ ./chip-tool otasoftwareupdaterequestor write default-otaproviders '[{"fabricIndex": 1, "providerNodeID": 2, "endpoint": 0}]' 1 0

5.  Configure the ACL of the ota-provider-app to allow access for Ameba

          $ ./chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": null, "targets": [{"cluster": 41, "endpoint": null, "deviceType": null}]}]' 1235 0

6.  Use the chip-tool to announce the ota-provider-app to start the OTA process

          $ ./chip-tool otasoftwareupdaterequestor announce-otaprovider 1 0 0 0 2 0

7.  The OTA process should include downloading the image, verification of image
    header, erasing upgraded flash partition, writing to flash and checksum
    verification.

8) Once OTA signature is updated, Ameba will reboot into the new image after 10
   seconds countdown.
