# Testing with Apple Devices

### Matter is the foundation for connected things.

Learn more about Matter on the [here](https://buildwithmatter.com/)

## Setup Requirements

Note: These steps are supported on:

-   iPhone _(iOS)_
-   iPad _(iPadOS)_
-   mac _(macOS)_
-   AppleTV _(tvOS)_
-   HomePod _(tvOS)_
-   Apple Watch _(watchOS)_

### Matter functionality is currently a Developer Preview, current requirements include:

-   An iPhone or iPad running iOS/iPadOS at least 15.6. You will need to install
    a profile on it (instructions below).
-   An Apple TV or HomePod running at least tvOS 15.6. You will need to install
    a profile on it (instructions below). If you have multiple Apple TVs and/or
    HomePods, you will need to install the profile on all of them.
-   The iPhone or iPad with the Matter profile must belong to the same iCloud
    account as the Apple TV or HomePod with the Matter profile.
-   A test iCloud account is highly recommended for Matter development to avoid
    interference with existing homes.
-   Devices must support BLE pairing, and have it enabled

## Source Compatibility

Each developer preview release is compatible with a certain SHA from this
repository.

-   iOS/iPadOS/tvOS 15.6 Developer Preview:
    [`cfc35951be66a664a6efdadea56d1b8ea6e63e96`](https://github.com/project-chip/connectedhomeip/commits/cfc35951be66a664a6efdadea56d1b8ea6e63e96)
-   iOS/iPadOS/tvOS 16.0 Developer Preview:
    [`cfc35951be66a664a6efdadea56d1b8ea6e63e96`](https://github.com/project-chip/connectedhomeip/commits/cfc35951be66a664a6efdadea56d1b8ea6e63e96)

## Profile Installation

### Install the Matter Profile on your iPhone or iPad

Note: The profile will expire automatically after some time

1. Download the
   [profile](https://developer.apple.com/services-account/download?path=/iOS/iOS_Logs/EnableMatter.mobileconfig)
   and install it on the iOS/iPadOS 15.6 device. If necessary, email the profile
   or use AirDrop to transfer the profile to the iOS/iPadOS 15.6 device.
2. Restart the device.

### Install the Matter Profile on your Apple TV or HomePod

Note: The profile will expire automatically after some time

1. Download the
   [profile](https://developer.apple.com/services-account/download?path=/iOS/iOS_Logs/EnableMatter.mobileconfig)
   and install it on the the Apple TV or HomePod running tvOS 15.6. If
   necessary, use
   [Apple Configurator 2](https://support.apple.com/guide/apple-configurator-2/connect-devices-to-your-mac-cad9d4b2211e/2.14/mac/11.3.1)
   to transfer the profile to the Apple TV or HomePod.
2. Restart the device.

### Ensuring Your Matter Accessory Works with iOS/iPadOS/tvOS

1. Clone the [Matter repo](https://github.com/project-chip/connectedhomeip.git)
2. Checkout the specific commit hash (from [above](#source-compatibility)) for
   maximum compatibility with your installed release:
    - Example command for SHA `cfc35951be66a664a6efdadea56d1b8ea6e63e96`:
      `$ git checkout cfc35951be66a664a6efdadea56d1b8ea6e63e96`

In order to work with iOS/iPadOS/tvOS 15.6 or greater, device types as defined
in the Matter Device Library spec are used to determine accessory categories.
Ensure the right device type is set for each endpoint.

-   For the `all-clusters-app` as an example, this can be set in
    `FIXED_DEVICE_TYPES` in `endpoint_config.h`

-   A light on Endpoint 1 can be set in `FIXED_DEVICE_TYPES` in
    `endpoint_config.h` for `all-clusters-app` like so:

```
// Array of device types
    #define FIXED_DEVICE_TYPES       \
    {                                \
        { 0x0016, 1 }, { 0x0100, 1 } \
    }
```

-   Supported device types are (not exhaustive):

| Type               | Decimal | HEX  |
| ------------------ | ------- | ---- |
| Lightbulb          | 256     | 0100 |
| Lightbulb + Dimmer | 257     | 0101 |
| Switch             | 259     | 0103 |
| Contact Sensor     | 21      | 0015 |
| Door Lock          | 10      | 000A |
| Light Sensor       | 262     | 0106 |
| Occupancy Sensor   | 263     | 0107 |
| Outlet             | 266     | 010A |
| Color Bulb         | 268     | 010C |
| Window Covering    | 514     | 0202 |
| Thermostat         | 769     | 0301 |
| Temperature Sensor | 770     | 0302 |
| Flow Sensor        | 774     | 0306 |

#### Examples of how to setup a couple apps

##### Case study 1: Configuring a development M5Stack, as a multi-device to work with iOS/iPadOS/tvOS

Note: These instructions are specific to getting started with the
(Matter-provided) `all-clusters-app` on an ESP32-based M5Stack, however can be
generalised to work on most platforms (more listed below)

1. Checkout and setup
   [Matter repo](https://github.com/project-chip/connectedhomeip.git) as per the
   instructions
   [above](#ensuring-your-matter-accessory-works-with-iosipadostvos)
2. Follow
   [these](https://github.com/project-chip/connectedhomeip/tree/master/examples/all-clusters-app/esp32)
   instructions to initialize your development environment, compile the firmware
   and flash your hardware.

##### Case study 2: a development Nordic board (nRF52840), working as a light to work with iOS/iPadOS/tvOS

Note: These instructions are specific to getting started with the
(Matter-provided) "lighting-app" on an Nordic-based NRF52840, however can be
generalised to work on most platforms (more listed below)

1. Checkout and setup
   [Matter repo](https://github.com/project-chip/connectedhomeip.git) as per the
   instructions
   [above](#ensuring-your-matter-accessory-works-with-iosipadostvos)

2. Follow
   [these instructions](https://github.com/project-chip/connectedhomeip/tree/master/examples/lighting-app/nrfconnect#readme)
   initialize your development environment, compile the firmware and flash your
   hardware.

#### General Platform Guides

##### Getting the SDK Ready

Note: Most platforms have very similar, if not the same configuration
requirements

1. Checkout and setup
   [Matter repo](https://github.com/project-chip/connectedhomeip.git) as per the
   instructions
   [above](#ensuring-your-matter-accessory-works-with-iosipadostvos)
2. Find and edit one of the platform
   [examples](https://github.com/project-chip/connectedhomeip/tree/master/examples)
   to support the fixed device types
   [above](#ensuring-your-matter-accessory-works-with-iosipadostvos)
3. Read the [platform guides](.) on how set up the hardware

##### Guides

-   [Bouffalo Lab](/examples/lighting-app/bouffalolab/bl602/README.md)
-   [EFR32 Window Covering](/examples/window-app/efr32/README.md)
-   [ESP32 All Clusters](/examples/all-clusters-app/esp32/README.md)
-   [ESP32 Lighting](/examples/lighting-app/esp32/README.md)
-   [ESP32 Temperature Sensor](/examples/temperature-measurement-app/esp32/README.md)
-   [mbedOS](/examples/all-clusters-app/mbed/README.md)
-   [nRF Connect All Clusters](./nrfconnect_examples_configuration.md)
-   [nRF Connect Pump](/examples/pump-app/nrfconnect/README.md)
-   [NXP Examples](./nxp_imx8m_linux_examples.md)
-   [NXP](/examples/all-clusters-app/nxp/mw320/README.md)
-   [P6](/examples/all-clusters-app/p6/README.md)
-   [Qorvo](/examples/lighting-app/qpg/README.md)
-   [SiliconLabs](./silabs_efr32_building.md)
-   [Simulated Linux](./simulated_device_linux.md)
-   [Telink](/examples/lighting-app/telink/README.md)
-   [TI Platform](./ti_platform_overview.md)
-   [TI All Clusters](/examples/all-clusters-app/cc13x2x7_26x2x7/README.md)
-   [Tizen](/examples/lighting-app/tizen/README.md)

### Release Notes & Known Issues

-   Please refer to the iOS/iPadOS 15.6
    [Release Notes](https://developer.apple.com/documentation/ios-ipados-release-notes/ios-ipados-15_6-release-notes)
    for currently known issues.
-   Please refer to the iOS/iPadOS 16.0
    [Release Notes](https://developer.apple.com/documentation/ios-ipados-release-notes/ios-ipados-16-release-notes)
    for currently known issues.
-   Further issues should be reported
    [here](https://github.com/project-chip/connectedhomeip/issues)
