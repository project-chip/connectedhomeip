# Matter Hardware Requirements

To run Matter over Thread or over Wi-Fi requires some Silicon Labs hardware in
order to run demos and do development. Following are the hardware requirements
for both Thread and Wi-Fi use cases broken down by platform and transport
protocol.

The following sections describe the hardware that may be used for Matter+OpenThread (Matter Hub and Accessory Device) and for Matter+Wi-Fi (Accessory Device). The EFRMG24 is the preferred starting point for Matter MCUs (including the Matter Hub RCP and both Accessory Devices). It provides Secure Vault and can use the internal flash of the device to store an upgrade image.

## Matter Over Thread "Matter Hub" Requirements

If you are running Matter over Thread and do not have a platform on which to run
the Open Thread Border Router and chip-tool, Silicon Labs recommends that you run
them on a Raspberry Pi. To do so you will need:

-   **Raspberry Pi**

    -   Raspberry Pi 4 with an SD card with storage >= 64 GB

        > The Raspberry Pi 4 is used to run the Open Thread Border Router and
        > the chip-tool. In this documentation the combination of this software
        > on the Raspberry Pi is also called the 'Matter Hub' A software image
        > for the Raspberry Pi is provided on the
        > [Matter Artifacts page](./ARTIFACTS.md).

-   **Radio Co-Processor (RCP)**

    The Matter Hub needs a 15.4 Radio Co-Processor (RCP) in order to create and
    interact with the Thread network. The RCP can be any Silicon Labs
    development board that is capable of running the OpenThread RCP firmware.
    The RCP radio board is connected to the Raspberry Pi via USB.

    Over 60 Silicon Labs boards support running the RCP firmware. To build an
    image for a board which is not listed here, download and build your image in
    Simplicity Studio. Pre-built OpenThread RCP firmware images are provided for
    the following boards on the [Matter Artifacts page](./ARTIFACTS.md):

    > **Note:** The EFR32MG24 is the preferred starting point for Matter MCUs.  It provides Secure Vault and can use the internal flash of the device to store an upgrade image. 

    -   **MG24 boards:**
        -   BRD4186C / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@10dBm
            -   [XG24-RB4186C](https://www.silabs.com/development-tools/wireless/xg24-rb4186c-efr32xg24-wireless-gecko-radio-board)
        -   BRD4187C / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@20dBm
            -   [XG24-RB4187C](https://www.silabs.com/development-tools/wireless/xg24-rb4187c-efr32xg24-wireless-gecko-radio-board)

    -   **MG12 boards:**

        -   BRD4161A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@19dBm
            -   [SLWRB4161A](https://www.silabs.com/development-tools/wireless/zigbee/slwrb4161a-efr32mg12-radio-board)
        -   BRD4162A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@10dBm
            -   [SLWRB4162A](https://www.silabs.com/development-tools/wireless/zigbee/slwrb4162a-efr32mg12-radio-board)
        -   BRD4163A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@19dBm
            -   [SLWRB4163A](https://www.silabs.com/development-tools/wireless/zigbee/slwrb4163a-efr32mg12-radio-board)
        -   BRD4164A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@19dBm
            -   [SLWRB4164A](https://www.silabs.com/development-tools/wireless/zigbee/slwrb4164a-efr32mg12-radio-board)

## Matter Over Thread Accessory Device Requirements

The Matter Accessory Device (MAD) is the actual device that the Matter
application firmware (such as the Matter Light or Matter Switch) runs on.
Several different platforms for the Matter Accessory Device are supported.
Pre-built binary images for the Matter accessory devices are provided on the
[Matter Artifacts page](./ARTIFACTS.md). Silicon Labs supports development of
Matter Accessory Devices for Matter over Thread on the following platforms:

> **Note:** The EFR32MG24 is the preferred starting point for Matter MCUs.  It provides Secure Vault and can use the internal flash of the device to store an upgrade image. 


-   **MG24 boards:**
    -   BRD4186C / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@10dBm
        -   [XG24-RB4186C](https://www.silabs.com/development-tools/wireless/xg24-rb4186c-efr32xg24-wireless-gecko-radio-board)
    -   BRD4187C / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@20dBm
        -   [XG24-RB4187C](https://www.silabs.com/development-tools/wireless/xg24-rb4187c-efr32xg24-wireless-gecko-radio-board)
    -   BRD2703A / MG24 Explorer Kit
        > Note: This board has yet to be released to the public, but it is
        > supported in the Silicon Labs build flow.
    -   BRD2601B / MG24 Explorer Kit
        -   [XG24-DK2601B](https://www.silabs.com/development-tools/wireless/efr32xg24-dev-kit?tab=overview)
    -   BRD4319A / SLWSTK6006A / Wireless Starter Kit/ 2.4GHz@20dBm
        > Note: This board has yet to be released to the public, but it is
        > supported in the Silicon Labs build flow.
    -   BRD4316A / SLWSTK6006A / Wireless Start Kit / 2.4GHz@10dBm
        - [XGM240-RB4316A](https://www.silabs.com/development-tools/wireless/xgm240-rb4316a-xgm240p-module-radio-board?tab=overview)
    -   BRD4317A / SLWSTK6006A / Wireless Starter Kit/ 2.4GHz@20dBm
        - [XGM240-RB4317A](https://www.silabs.com/development-tools/wireless/xgm240-rb4317a-xgm240p-module-radio-board?tab=overview)

-   **MG12 boards:**

    -   [EFR32MG12 Development Kit](https://www.silabs.com/development-tools/wireless/zigbee/efr32mg12-dual-band-starter-kit)
    -   BRD4161A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@19dBm
        -   [SLWRB4161A](https://www.silabs.com/development-tools/wireless/zigbee/slwrb4161a-efr32mg12-radio-board)
    -   BRD4162A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@10dBm
        -   [SLWRB4162A](https://www.silabs.com/development-tools/wireless/zigbee/slwrb4162a-efr32mg12-radio-board)
    -   BRD4163A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@19dBm
        -   [SLWRB4163A](https://www.silabs.com/development-tools/wireless/zigbee/slwrb4163a-efr32mg12-radio-board)
    -   BRD4164A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@19dBm
        -   [SLWRB4164A](https://www.silabs.com/development-tools/wireless/zigbee/slwrb4164a-efr32mg12-radio-board)
    -   BRD4166A / SLTB004A / Thunderboard Sense 2 / 2.4GHz@10dBm
        -   [Thunderboard Sense 2](https://www.silabs.com/development-tools/thunderboard/thunderboard-sense-two-kit)
    -   BRD4170A / SLWSTK6000B / Multiband Wireless Starter Kit / 2.4GHz@19dBm,
        915MHz@19dBm
        -   [SLWRB4170A](https://www.silabs.com/development-tools/wireless/zigbee/slwrb4170a-efr32mg12-radio-board)


## Matter over Wi-Fi Accessory Device Requirements for NCP mode

The Silicon Labs Matter over Wi-Fi NCP mode demo and development requires two boards: the
Silicon Labs EFR32 Radio board to run the Matter code and either the RS9116, SiWx917 or WF200 to run the Wi-Fi protocol stack. 
Pre-built images for the EFR32 are provided on the [Matter Artifacts page](./ARTIFACTS.md). Pre-built images for SiWx917 or RS9116 connectivity 
firmware are available as per the instructions on the [Software Requirements Page](./SOFTWARE_REQUIREMENTS.md). 

> **Note:** 
>    1. The EFR32MG24 is the preferred starting point for Matter MCUs.  It provides Secure Vault and can use the internal flash of the device to store an upgrade image.
>    2. The WF200 connectivity firmware image is included in the EFR32MG12 or EFR32MG24 images on the [Matter Artifacts page](./ARTIFACTS.md) for running with the WF200 in NCP mode. The 
Matter application downloads the connectivity firmware onto the WF200 on first-time startup.

The following boards are supported for the Matter over Wi-Fi demos and development:

-   **MG24 boards:**

    -   BRD4186C / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@10dBm
        -   [XG24-RB4186C](https://www.silabs.com/development-tools/wireless/xg24-rb4186c-efr32xg24-wireless-gecko-radio-board)
        -   MG24 with WSTK : [xG24-PK6009A](https://www.silabs.com/development-tools/wireless/efr32xg24-pro-kit-10-dbm?tab=overview)
    -   BRD4187C / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@20dBm
        - [XG24-RB4187C](https://www.silabs.com/development-tools/wireless/xg24-rb4187c-efr32xg24-wireless-gecko-radio-board)
        - MG24 with WSTK : [xG24-PK6010A](https://www.silabs.com/development-tools/wireless/efr32xg24-pro-kit-20-dbm?tab=overview)

-   **MG12 boards:**
    -   [EFR32MG12 Development Kit](https://www.silabs.com/development-tools/wireless/zigbee/efr32mg12-dual-band-starter-kit)
    -   BRD4161A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@19dBm
        -   [SLWRB4161A](https://www.silabs.com/development-tools/wireless/zigbee/slwrb4161a-efr32mg12-radio-board)
    -   BRD4162A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@10dBm
        -   [SLWRB4162A](https://www.silabs.com/development-tools/wireless/zigbee/slwrb4162a-efr32mg12-radio-board)
    -   BRD4163A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@19dBm
        -   [SLWRB4163A](https://www.silabs.com/development-tools/wireless/zigbee/slwrb4163a-efr32mg12-radio-board)
    -   BRD4164A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@19dBm
        -   [SLWRB4164A](https://www.silabs.com/development-tools/wireless/zigbee/slwrb4164a-efr32mg12-radio-board)
    -   BRD4170A / SLWSTK6000B / Multiband Wireless Starter Kit / 2.4GHz@19dBm, 915MHz@19dBm
        -   [SLWRB4170A](https://www.silabs.com/development-tools/wireless/zigbee/slwrb4170a-efr32mg12-radio-board)


-   **Wi-Fi NCP Dev Kits & boards**

    -   RS9116
        -   SB-EVK1 / Single Band Wi-Fi Development Kit / 2.4GHz
            -   [RS9116X-SB-EVK1](https://www.silabs.com/development-tools/wireless/wi-fi/rs9116x-sb-evk-development-kit)
        -   SB-EVK2 / Single Band Wi-Fi Development Kit / 2.4GHz
            -   [RS9116X-SB-EVK2](https://www.silabs.com/development-tools/wireless/wi-fi/rs9116x-sb-evk2-development-kit)
        -   DB-EVK1 / Dual Band Wi-Fi Development Kit / 2.4GHz & 5GHz
            -   [RS9116X-DB-EVK1](https://www.silabs.com/development-tools/wireless/wi-fi/rs9116x-db-evk-development-kit)
            > **Note:** Matter only supported over 2.4GHz on this Dev kit.
        -   Interconnect board (included in the Wi-Fi kits)
        -   SPI Cable (included in the RS9116 kit)
        -   Jumper Cables (included in the RS9116 kit)
    -   SiWx917
        -   SiWx917 NCP Mode / Wi-Fi Expansion Board / 2.4GHz
            -   BRD8036A (A0 Expansion v1.0)
            -   BRD8036A (A0 Expansion v1.1)
    -   WF200
        -   WF200 / Single Band Wi-Fi Expansion Board / 2.4GHz
            -   [SLEXP8022A](https://www.silabs.com/development-tools/wireless/wi-fi/wf200-wifi-expansion-kit)
        -   WFM200S / Single Band Wi-Fi Expansion Board / 2.4GHz
            -   [SLEXP8023A](https://www.silabs.com/development-tools/wireless/wi-fi/wfm200-wifi-expansion-kit)


## Matter over Wi-Fi Accessory Device Requirements for SoC mode

The Silicon Labs Matter over Wi-Fi demo and development for SoC mode requires the SiWx917 SoC board that 
supports Matter over Wi-Fi in a single-chip package - the integrated M4 processor is dedicated
for peripheral and application-related processing (Matter), while the ThreadArch® runs the wireless and networking protocol stacks. 

Pre-built images for the SiWx917 connectivity firmware are available as per the instructions on the [Software Requirements Page](./SOFTWARE_REQUIREMENTS.md). The following boards are supported for the Matter over Wi-Fi demos and development:

-   **Wi-Fi SoC boards:**

-   SiWx917 / BRD4002A / Wireless Starter Kit
-   SiWx917 SoC Mode
    -   SiWx917 SoC / Dual Flash Radio Board / 2.4GHz
        -   BRD4325B - A0 dual flash 1.1
        -   BRD4325B - A0 dual flash 1.2
    -   SiWx917 SoC / Common Flash Radio Board / 2.4GHz
        -   BRD4325C - B0 common flash v1.2
        -   BRD4325G - B0 Stacked Flash + External PSRAM v1.2
        -   BRD4338A - B0 common flash v2.0

    **Note:**
    Refer [SiWx917 SoC](https://www.silabs.com/wireless/wi-fi/siwx917-wireless-socs) for more details


## Additional Matter over Wi-Fi Hardware Requirements

In addition to your Matter over Wi-Fi Accessory Device you will need the
following for both running the demo and for development:

-   Access point with Internet access
-   microSD card (>=32GB) (if using Raspberry Pi)
-   **[Optional]** Android Mobile phone (If using the chip-tool on Android)
