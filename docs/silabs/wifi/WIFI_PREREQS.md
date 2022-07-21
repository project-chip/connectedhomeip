[<< Back to TOC](../README.md)

# Matter Wifi Prerequisites

Silicon Labs Matter Wifi is supported on two different platforms, the
[WF200](https://www.silabs.com/wireless/wi-fi/wf200-series-2-transceiver-ics)
and the
[RS9116](https://www.silabs.com/development-tools/wireless/wi-fi/rs9116x-sb-evk-development-kit).
In both cases the Wifi part is attached to an EFR32MG12 development board via a
daughter card. The EFR32MG12 is used as a host processor for the application and
for it's Bluetooth capability which is necessary for Matter commissioning.

In addition to the EFR32MG12 and Wifi boards, you will need to run the Matter
ChipTool on some device. This can be build and run on a Linux or Mac laptop or
on a Raspberry Pi.

The hardware that you will need for Silicon Labs Matter Wifi development is as
follows:

## Hardware

-   Linux PC/Laptop **or** Raspberry Pi 4 (This is for running the ChipTool to
    commission and control the device)
-   Kits/Boards:
    -   SLWSTK6000B Wireless Starter Kit main board
    -   BRD4161A/BRD4186C daughter boards are supported
          - [SLWRB4161A](https://www.silabs.com/development-tools/wireless/zigbee/slwrb4161a-efr32mg12-radio-board)
          - [XG24-RB4186C](https://www.silabs.com/development-tools/wireless/xg24-rb4186c-efr32xg24-wireless-gecko-radio-board)
        > BLE and Matter code runs here
    -   Wifi Dev Kit, either
        [RS9116X-SB-EVK1](https://www.silabs.com/development-tools/wireless/wi-fi/rs9116x-sb-evk-development-kit)
        **or**
        [WF200](https://www.silabs.com/wireless/wi-fi/wf200-series-2-transceiver-ics)
    -   Interconnect board (included in the wifi kits)
    -   SPI Cable (included in the RS9116 kit)
    -   Jumper Cables (included in the RS9116 kit)
-   Access point with Internet access
-   microSD card (32GB) (If using Raspberry Pi)
-   **[Optional]** Android Mobile phone (If using the ChipTool on Android)

<br>

## Software

-   Ozone to flash the images generated
    -   Windows: [Download Ozone](https://www.segger.com/downloads/jlink/)
        > Search for "Ozone - The J-Link Debugger" and download the latest
        > Windows installer depending on 32/64bit and install it.
    -   Linux:
        [Download Ozone](https://www.segger.com/downloads/jlink/Ozone_Linux_x86_64.deb)
        and install it
-   [otional] ChipTool Android mobile application:
    [Download](https://confluence.silabs.com/download/attachments/240625466/chip-app.zip?version=1&modificationDate=1647837891300&api=v2),
    extract, and install to the Android mobile; [TODO: the link is a confluence
    link!]
-   Raspberry Pi imager tool: [Download](https://www.raspberrypi.com/software/)
-   Upgrade firmware on RS9116 EVK:
    [Reference](http://draft-docs.suds.silabs.net/rs9116-wiseconnect/2.5/wifibt-wc-getting-started-with-pc/update-evk-firmware)

<br>

## RS9116: Steps to Update Firmware

Pre-Built Rs9116 firmware is available on the
[Matter Artifacts page](../general/ARTIFACTS.md)

1. [Setting up TeraTerm](https://docs.silabs.com/rs9116/wiseconnect/2.0/tera-term-setup)
2. [Updating the RS9116 Firmware](https://docs.silabs.com/rs9116/wiseconnect/2.0/update-evk-firmware)
