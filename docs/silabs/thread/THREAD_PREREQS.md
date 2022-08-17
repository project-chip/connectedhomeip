# Matter 15.4 Hardware and Software Prerequisites

## Software Requirements

### System Agnostic Software Requirements (Mac/ Linux/ Windows):

1. SSH Client ([PuTTY](https://www.putty.org/) or similar):
    > SSH client is used to communicate with the Raspberry Pi over a secure
    > shell.
1. [Raspberry Pi Disk Imager](https://www.raspberrypi.com/software/)
    > Raspberry Pi Disk Imager is used to flash the SD Card that contains the
    > operating system for the Raspberry Pi. Note that, for the
    Thread demo, the operating system is the demo image.
1. [Flash tool](../general/FLASH_SILABS_DEVICE.md)
    > Simplicity Commander standalone or Simplicity Studio is used to flash
    > Silicon Labs hardware with firmware images for the RCP and the Matter
    > Accessory Device.
1. Git [only required for building images and development]
    > Make sure Git is installed on the local machine that will be used to flash
    > the devices so that the necessary repositories can be cloned locally and
    > used as needed.
1. If you are using an EFR32MG2x device you will require a bootloader to run the demo applications. When you flash your application image be sure to include a bootloader as well (if one is not already present). Bootloader images are provided on the [Matter Artifacts page](../general/ARTIFACTS.md).

<br>

### Windows-Specific Software Requirements

&emsp; These requirements are in addition to those mentioned above, for Windows only. <br>

&emsp; A Unix-like command line:

-   [Windows Subsystem for Linux (WSL)](https://docs.microsoft.com/en-us/windows/wsl/about)
    **OR**
-   [VirtualBox](https://www.virtualbox.org/) with
    [Ubuntu 20.04.x LTS](https://ubuntu.com/download/desktop)

    > WSL or Virtual Box are used to emulate a virtual Linux machine, which is
    > useful for accessing the Linux command line tools

<br><br>

## Hardware Requirements

1. Matter Hub (OTBR + ChipTool)

    &emsp;Raspberry Pi 4 with an SD card with storage $\geq$ 64 GB

    > The Raspberry Pi 4 is used to run the Open Thread Border Router and the
    > Chip Tool. In this documentation the combination of this software on the
    > Raspberry Pi is also called the 'Matter Hub'

<br>

2. Radio Co-Processor (RCP)

    The RCP is a Silicon Labs development board or ThunderBoard Sense 2 running
    the Radio Co-Processor firmware for Thread. The following Silicon Labs
    devices are supported:

    - EFR32MG based on EFR32 Mighty Gecko Wireless Starter Kit
        - [EFR32MG12 Development Kit](https://www.silabs.com/development-tools/wireless/zigbee/efr32mg12-dual-band-starter-kit)

    **or**

    - Silicon Labs Thunderboard Sense 2
        - [Thunderboard Sense 2](https://www.silabs.com/development-tools/thunderboard/thunderboard-sense-two-kit)

<br>

3. Matter Accessory Device (MAD)

    The following Silicon Labs devices are supported to function as a MAD:

    - **MG12 boards:**

        - [EFR32MG12 Development Kit](https://www.silabs.com/development-tools/wireless/zigbee/efr32mg12-dual-band-starter-kit)
        - BRD4161A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@19dBm
            - [SLWRB4161A](https://www.silabs.com/development-tools/wireless/zigbee/slwrb4161a-efr32mg12-radio-board)
        - BRD4163A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@19dBm
            - [SLWRB4163A](https://www.silabs.com/development-tools/wireless/zigbee/slwrb4163a-efr32mg12-radio-board)
        - BRD4164A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@19dBm
            - [SLWRB4164A](https://www.silabs.com/development-tools/wireless/zigbee/slwrb4164a-efr32mg12-radio-board)
        - BRD4166A / SLTB004A / Thunderboard Sense 2 / 2.4GHz@10dBm
            - [Thunderboard Sense 2](https://www.silabs.com/development-tools/thunderboard/thunderboard-sense-two-kit)
        - BRD4170A / SLWSTK6000B / Multiband Wireless Starter Kit /
          2.4GHz@19dBm, 915MHz@19dBm
            - [SLWRB4170A](https://www.silabs.com/development-tools/wireless/zigbee/slwrb4170a-efr32mg12-radio-board)
              <br><br>

    - **MG24 boards:**
        - BRD4186C / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@10dBm
            - [XG24-RB4186C](https://www.silabs.com/development-tools/wireless/xg24-rb4186c-efr32xg24-wireless-gecko-radio-board)
        - BRD4187C / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@20dBm
            - [XG24-RB4187C](https://www.silabs.com/development-tools/wireless/xg24-rb4187c-efr32xg24-wireless-gecko-radio-board)

----
[Table of Contents](../README.md) | [Thread Demo](./DEMO_OVERVIEW.md) | [Wi-Fi Demo](../wifi/DEMO_OVERVIEW.md)