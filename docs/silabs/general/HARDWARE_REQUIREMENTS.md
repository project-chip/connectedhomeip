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
<br><br>
    -   **MG12 boards:**

        -   BRD4161A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@19dBm
            -   [SLWRB4161A](https://www.silabs.com/development-tools/wireless/zigbee/slwrb4161a-efr32mg12-radio-board)
        -   BRD4162A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@10dBm
            -   [SLWRB4162A](https://www.silabs.com/development-tools/wireless/zigbee/slwrb4162a-efr32mg12-radio-board)
        -   BRD4163A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@19dBm
            -   [SLWRB4163A](https://www.silabs.com/development-tools/wireless/zigbee/slwrb4163a-efr32mg12-radio-board)
        -   BRD4164A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@19dBm
            -   [SLWRB4164A](https://www.silabs.com/development-tools/wireless/zigbee/slwrb4164a-efr32mg12-radio-board)
                <br><br>

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
<br><br>
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
            <br><br>


## Matter over Wi-Fi Accessory Device Requirements

The Silicon Labs Matter over Wi-Fi demo and development requires two boards: the
Silicon Labs EFR32 Radio board to run the Matter code and the RS9116
or the SiWx917 or
the WF200 to run the Wi-Fi protocol stack. Pre-built images for both the EFR32
and the RS9116 are provided on the [Matter Artifacts page](./ARTIFACTS.md). The
following boards are supported for the Matter over Wi-Fi demos and development:

> **Note:** The EFR32MG24 is the preferred starting point for Matter MCUs.  It provides Secure Vault and can use the internal flash of the device to store an upgrade image. 


-   **MG24 boards:**

    -   BRD4186C / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@10dBm
        -   [XG24-RB4186C](https://www.silabs.com/development-tools/wireless/xg24-rb4186c-efr32xg24-wireless-gecko-radio-board)
    -   BRD4187C / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@20dBm
        - [XG24-RB4187C](https://www.silabs.com/development-tools/wireless/xg24-rb4187c-efr32xg24-wireless-gecko-radio-board)
<br><br>
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
            <br><br>


-   **Wi-Fi Dev Kit**

    -   RS9116
        -   [RS9116X-SB-EVK1](https://www.silabs.com/development-tools/wireless/wi-fi/rs9116x-sb-evk-development-kit)
    -   SiWx917
        -   [SiWx917](https://www.silabs.com/wireless/wi-fi/siwx917-wireless-socs)
    -   WF200
        -   [WF200](https://www.silabs.com/wireless/wi-fi/wf200-series-2-transceiver-ics)
    -   Interconnect board (included in the Wi-Fi kits)
    -   SPI Cable (included in the RS9116 kit)
    -   Jumper Cables (included in the RS9116 kit)

## Additional Matter over Wi-Fi Hardware Requirements

In addition to your Matter over Wi-Fi Accessory Device you will need the
following for both running the demo and for development:

-   Access point with Internet access
-   microSD card (>=32GB) (if using Raspberry Pi)
-   **[Optional]** Android Mobile phone (If using the chip-tool on Android)
