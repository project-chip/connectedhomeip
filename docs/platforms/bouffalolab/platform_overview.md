# `Bouffalo Lab` SoCs

-   BL602

    BL602/BL604 is combo chip-set for Wi-Fi 4 and BLE 5.0.

-   BL61X

    BL61X is combo chip-set for Wi-Fi 6, Classic Bluetooth, Bluetooth low energy
    5.3 and IEEE 802.15.4/ZigBee/Thread.

    BL61X has fully certified with all Thread 1.3 features, included Thread
    `SSED` and Thread Border Router.

-   BL70X

    BL70X is combo chip-set for BLE and IEEE 802.15.4/ZigBee/Thread.

    BL70X has fully certified with all Thread 1.3 features, included Thread
    `SSED` and Thread Border Router.

    -   BL702/BL706 has 14dbm tx power and is recommended for routing devices.
        SDK uses BL702 as a general name.
    -   BL702L/BL704L is designed for low power application. SDK uses BL702L as
        a general name.

# Solutions introduction

`Bouffalo Lab` has full connectivity supports for Matter Applications.

-   Wi-Fi 4/6 application, we have
    -   BL602, Wi-Fi 4 application.
    -   BL706 + BL602, Wi-Fi 4 application.
    -   BL616, Wi-Fi 6 application
-   Thread application, we have
    -   BL70X/BL616
    -   BL704L, Thread low power application.
-   Ethernet application, we have
    -   BL706/BL618
-   Openthread Border Router application based on FreeRTOS
    -   BL706 + BL602, Wi-Fi 4 as infrastructure network
    -   BL616, Wi-Fi 6 as infrastructure network
    -   BL706/BL616, Ethernet as as infrastructure network
-   Matter ZigBee Bridge application based on FreeRTOS - BL706 + BL602, Wi-Fi 4
    as infrastructure network - BL616, Wi-Fi 6 as infrastructure network -
    BL706/BL616, Ethernet as as infrastructure network
    > Please contact `Bouffalo Lab` for supports on OTBR and Matter ZigBee
    > Bridge application

# Current supported

-   Current supported boards:

    -   `BL602DK`

    -   `BL616DK`

        > Please contact `Bouffalo Lab` for BL616 SDK.

    -   `BL704LDK`

    -   `BL706DK`

-   Legacy supported boards:

    -   `BL602-IoT-Matter-V1`
    -   `BL602-NIGHT-LIGHT`
    -   `XT-ZB6-DevKit`
    -   `BL706-NIGHT-LIGHT`

-   Supported examples:

    -   [Lighting app](../../../examples/lighting-app/bouffalolab/README.md)

    -   [Contact sensor app](../../../examples/contact-sensor-app/bouffalolab/README.md)
