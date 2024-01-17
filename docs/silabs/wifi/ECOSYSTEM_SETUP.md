# Overview of Matter EcoSystem

- Silicon Labs Matter platforms integrates with the most recent versions of the Matter protocol and works with several commercial Matter ecosystems.
- Matter allows multiple ecosystems to work together and control the Device.
- Silicon Labs Matter devices can be controlled by various Matter enabled Ecosystems.

## Prerequisites

### Ecosystems
-  Devices which are compatible with official implementation of Matter in at least one commercial ecosystem
   - [Google Matter Hubs](https://developers.home.google.com/matter/supported-devices#choosing_a_device_type)
   - [Apple Matter](https://www.apple.com/home-app/accessories/)
   - [Samsung SmartThings Matter](https://support.smartthings.com/hc/en-us/articles/11219700390804-SmartThings-x-Matter-Integration-)
   - [Amazon Alexa Matter](https://developer.amazon.com/en-US/docs/alexa/smarthome/matter-support.html)

### Smartphone to Control Ecosystem
- Android smart phone installed with respective Ecosystem mobile apps **Amazon Alexa, Google Home, Samsung Smart Things**
- Apple iphone installed with mobile app **Apple Home**.

### Silicon Labs Development Boards
- In order to run Matter over Wi-Fi on compatible Ecosystem, at least below setup hardwares are required
 - Silicon Labs Wireless starter/development kit (WSTK)
 - Silicon Labs Wi-Fi development Kits & boards
    - For SoC variants, an SiWx917 SoC Dual flash boards or Common flash boards
    - For Network Co-Processor (NCP) variants,
      - Silicon Labs EFR32 - is used as a host processor and, with the WF200, provides Bluetooth LE capabilities
      - Silicon Labs Wi-Fi Processor
         - RS9116 development kit
         - WF200 expansion board
         - SiWx917 NCP expansion board

    -   **Wi-Fi SoC Dev kits and boards**
      - SiWx917 / BRD4002A / Wireless Starter Kit
      - SiWx917 SoC / Common Flash Radio Board / 2.4GHz
        - BRD4338A - B0 common flash v2.0

    -   **Wi-Fi NCP Dev kits and boards**
      -   **MG24 boards:**
            -   BRD4186C / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@10dBm
                -   [XG24-RB4186C](https://www.silabs.com/development-tools/wireless/xg24-rb4186c-efr32xg24-wireless-gecko-radio-board)
                -   MG24 with WSTK : [xG24-PK6009A](https://www.silabs.com/development-tools/wireless/efr32xg24-pro-kit-10-dbm?tab=overview)
            -   BRD4187C / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@20dBm
                -   [XG24-RB4187C](https://www.silabs.com/development-tools/wireless/xg24-rb4187c-efr32xg24-wireless-gecko-radio-board)
                -   MG24 with WSTK : [xG24-PK6010A](https://www.silabs.com/development-tools/wireless/efr32xg24-pro-kit-20-dbm?tab=overview)

      -   **Wi-Fi Dev Kits & boards**
            -   RS9116
                -   SB-EVK1 / Single Band Wi-Fi Development Kit / 2.4GHz
                    -   [RS9116X-SB-EVK1](https://www.silabs.com/development-tools/wireless/wi-fi/rs9116x-sb-evk-development-kit)
                -   SB-EVK2 / Single Band Wi-Fi Development Kit / 2.4GHz
                    -   [RS9116X-SB-EVK2](https://www.silabs.com/development-tools/wireless/wi-fi/rs9116x-sb-evk2-development-kit)
                -   DB-EVK1 / Dual Band Wi-Fi Development Kit / 2.4GHz & 5GHz
                    -   [RS9116X-DB-EVK1](https://www.silabs.com/development-tools/wireless/wi-fi/rs9116x-db-evk-development-kit)
                **Note:** Matter only supported over 2.4GHz on this Dev kit.
                -   Interconnect board (included in the Wi-Fi kits)
                -   SPI Cable (included in the RS9116 kit)
                -   Jumper Cables (included in the RS9116 kit)
            -   SiWx917
                -   SiWx917 NCP Mode / Wi-Fi Expansion Board / 2.4GHz
                    -   BRD4346A
            -   WF200
                -   WF200 / Single Band Wi-Fi Expansion Board / 2.4GHz
                    -   [SLEXP8022A](https://www.silabs.com/development-tools/wireless/wi-fi/wf200-wifi-expansion-kit)
                -   WFM200S / Single Band Wi-Fi Expansion Board / 2.4GHz
                    -   [SLEXP8023A](https://www.silabs.com/development-tools/wireless/wi-fi/wfm200-wifi-expansion-kit)

- Windows/Linux/MacOS computer with a USB port
- USB cable for connecting WSTK Board to Computer
- Raspberry Pi with a >32 GB SD Card
- Access Point with Internet Access
- Interconnect board (included in the Wi-Fi kits)
- SPI Cable (included in the RS9116 kit)
- Jumper Cables (included in the RS9116 kit)

For Detailed Information about Silicon Labs Development Board [Refer Hardware Requirements Page](../general/HARDWARE_REQUIREMENTS.md)

## Matter Setup with Ecosystem and Demo execution
Matter device can be controlled by single controller and interoperable with multiple controllers

- For Single Controller Setup and Procedure with matter compatible ecosystem, refer below links
  - [Google Ecosystem](./GOOGLE_ECOSYSTEM_SETUP.md)
  - [Amazon Ecosystem](./AMAZON_ECOSYSTEM_SETUP.md)
  - [Apple Ecosystem](./APPLE_ECOSYSTEM_SETUP.md)
  - [Samsung Aeotec Ecosystem](./SAMSUNG_ECOSYSTEM_SETUP.md)
- Follow [Multiple Controller Setup and Execution](./MULTICONTROLLER_ECOSYSTEM.md) to interoperate with multiple controllers