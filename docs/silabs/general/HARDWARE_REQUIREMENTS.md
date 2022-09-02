# Matter Hardware Requirements

Whether you are running Matter over Thread or Matter over Wi-Fi you will need
some Silicon Labs hardware in order to run demos and do development. What
follows are the hardware requirements for both Thread and Wi-Fi use cases broken
down by platform and transport protocol.

## Matter Over Thread "Matter Hub" Requirements

If you are running Matter over Thread and do not have a platform on which to run
the Open Thread Border Router and Chip-Tool, Silicon Labs recommends that you
run them on a Raspberry Pi. This recommendation leads to the following
requirements:

-   **Raspberry Pi**

    -   Raspberry Pi 4 with an SD card with storage >= 64 GB

        > The Raspberry Pi 4 is used to run the Open Thread Border Router and
        > the Chip-Tool. In this documentation the combination of this software
        > on the Raspberry Pi is also called the 'Matter Hub' A software image
        > for the Raspberry Pi is provided on the
        > [Matter Artifacts page](./ARTIFACTS.md).

-   **Radio Co-Processor (RCP)**

    The Matter Hub needs a 15.4 Radio Co-Processor (RCP) in order to create and
    interact with the Thread network. The RCP can be any Silicon Labs
    development board that is capable of running the OpenThread RCP firmware.
    The RCP radio board is connected to the Raspberry Pi via USB.

    Over 60 Silicon Labs boards support the running of the RCP firmware. For a
    complete list of boards supported please download and build your image in
    Simplicity Studio. Pre-built OpenThread RCP firmware images are provided for
    the following boards on the [Matter Artifacts page](./ARTIFACTS.md):

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

    -   **MG24 boards:**
        -   BRD4186C / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@10dBm
            -   [XG24-RB4186C](https://www.silabs.com/development-tools/wireless/xg24-rb4186c-efr32xg24-wireless-gecko-radio-board)
        -   BRD4187C / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@20dBm
            -   [XG24-RB4187C](https://www.silabs.com/development-tools/wireless/xg24-rb4187c-efr32xg24-wireless-gecko-radio-board)

## Matter Over Thread Accessory Device Requirements

The Matter Accessory Device (MAD) is the actual device that the Matter
application firmware runs on. This is the Matter Light or the Matter Switch for
instance. Several different platforms for the Matter Accessory Device are
supported. Pre-built binary images for the Matter accessory devices are provided
on the [Matter Artifacts page](./ARTIFACTS.md). Silicon Labs supports
development of Matter Accessory Devices for Matter over Thread on the following
platforms:

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

## Matter over Wi-Fi Accessory Device Requirements

The Silicon Labs Matter over Wi-Fi demo and development requires two boards, the
Silicon Labs EFR32 Radio board to run the Matter code along with either the
RS9116 or the WF200 to run the Wi-Fi protocol stack. Pre-built images for both
the EFR32 and the RS9116 are provided on the
[Matter Artifacts page](./ARTIFACTS.md). The following boards are supported for
the Matter over Wi-Fi demos and development:

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

-   **MG24 boards:**

    -   BRD4186C / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@10dBm
        -   [XG24-RB4186C](https://www.silabs.com/development-tools/wireless/xg24-rb4186c-efr32xg24-wireless-gecko-radio-board)
    -   BRD4187C / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@20dBm -
        [XG24-RB4187C](https://www.silabs.com/development-tools/wireless/xg24-rb4187c-efr32xg24-wireless-gecko-radio-board)
        <br><br>

-   **Wi-Fi Dev Kit**

    -   RS9116
        -   [RS9116X-SB-EVK1](https://www.silabs.com/development-tools/wireless/wi-fi/rs9116x-sb-evk-development-kit)
    -   WF200
        -   [WF200](https://www.silabs.com/wireless/wi-fi/wf200-series-2-transceiver-ics)
    -   Interconnect board (included in the Wi-Fi kits)
    -   SPI Cable (included in the RS9116 kit)
    -   Jumper Cables (included in the RS9116 kit)

## Additional Matter over Wi-Fi Hardware Requirements

Matter over Wi-Fi has a few additional requirements for the working of the demo
and development. In addition to your Matter over Wi-Fi Accessory Device you will
need the following:

-   Access point with Internet access
-   microSD card (32GB) (if using Raspberry Pi)
-   **[Optional]** Android Mobile phone (If using the ChipTool on Android)
