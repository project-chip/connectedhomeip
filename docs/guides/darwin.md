# Testing with Apple Devices

### Matter is the foundation for connected things.

Learn more about Matter [here](https://buildwithmatter.com/)

## Setup Requirements

Note: These steps are supported on:

-   iPhone _(iOS)_
-   iPad _(iPadOS)_
-   mac _(macOS)_
-   AppleTV _(tvOS)_
-   HomePod _(tvOS)_
-   Apple Watch _(watchOS)_

### Current requires for testing with Apple Devices

##### Note: Matter functionality is currently a Developer Preview

-   Devices must support BLE pairing, and have it enabled
-   An iPhone or iPad running iOS/iPadOS at least 15.6. You will need to install
    a [profile](#profile-installation) on it.
-   An Apple TV or HomePod running at least tvOS 15.6. You will need to install
    a [profile](#profile-installation) on it. If you have multiple Apple TVs
    and/or HomePods, you will need to install the
    [profile](#profile-installation) on all of them.
-   The iPhone or iPad with the Matter [profile](#profile-installation) must
    belong to the same iCloud account as the Apple TV or HomePod with the Matter
    [profile](#profile-installation).
-   A test iCloud account is highly recommended for Matter development to avoid
    interference with existing homes.
-   To pair and control Matter Thread devices, you'll need to have a HomePod
    Mini or Apple TV 4K
-   To pair to devices via BLE, you must either use the Matter Pairing API
    documented
    [here](https://developer.apple.com/documentation/homekit/hmmatterhome), or
    enable [Developer mode](#enable-developer-mode-on-your-apple-device).

## Source Compatibility

Each developer preview release is compatible with a certain SHA from this
repository.

-   iOS/iPadOS/tvOS 16.0 Developer Preview:
    [`aa9457e6b94b735076dff6297176183bf9780177`](https://github.com/project-chip/connectedhomeip/commits/aa9457e6b94b735076dff6297176183bf9780177)

## Profile Installation

### Install the Matter Profile on your iPhone or iPad

Note: The profile will expire automatically after some time

1. Download the
   [profile](https://developer.apple.com/services-account/download?path=/iOS/iOS_Logs/EnableMatter.mobileconfig)
2. Email the profile to an account that is configured on your device running
   iOS/iPadOS 15.6 (or greater)
3. Selecting the profile in Mail on your iOS/iPadOS 15.6 (or greater) device
   will present an option to install the profile

    ##### Note: Make sure to head back to system preferences, and you'll see an option to verify and install the profile near the top

4. Restart the device

### Install the Matter Profile on your HomePod

Note: The profile will expire automatically after some time

1. Download the
   [profile](https://developer.apple.com/services-account/download?path=/iOS/iOS_Logs/EnableMatter.mobileconfig)
2. Email the profile to an account that is configured on your device running
   iOS/iPadOS 15.6 (or greater)
3. Selecting the profile in Mail on your iOS/iPadOS 15.6 (or greater) device
   will present an option to install the profile on the HomePod
4. Restart the device

### Install the Matter Profile on your Apple TV

Note: The profile will expire automatically after some time

1. Download the
   [profile](https://developer.apple.com/services-account/download?path=/iOS/iOS_Logs/EnableMatter.mobileconfig)
2. Use
   [Apple Configurator](https://apps.apple.com/us/app/apple-configurator/id1037126344?mt=12)
   to transfer the profile to the Apple TV
3. Restart the device

### Enable Developer Mode on your Apple Device

Note: Developer mode is a great way to get logs from your device as well as
enables other useful developer tools

To enable developer mode, please follow the instructions
[here](https://developer.apple.com/documentation/xcode/enabling-developer-mode-on-a-device)

### Ensuring Your Matter Accessory Works with iOS/iPadOS/tvOS

1. Clone the [Matter repo](https://github.com/project-chip/connectedhomeip.git)
2. Checkout the specific commit hash (from [above](#source-compatibility)) for
   maximum compatibility with your installed release:
    - Example command for SHA `aa9457e6b94b735076dff6297176183bf9780177`:
      `$ git checkout aa9457e6b94b735076dff6297176183bf9780177`

In order to work with iOS/iPadOS/tvOS 15.6 or greater, device types as defined
in the Matter Device Library spec are used to determine accessory categories.
Ensure the right device type is set for each endpoint.

-   For the `all-clusters-app` as an example, this can be set in
    `FIXED_DEVICE_TYPES`, `FIXED_DEVICE_TYPE_OFFSETS`, and
    `FIXED_DEVICE_TYPE_LENGTHS` in `endpoint_config.h`
-   Here's an example from `all-clusters-app` (feel free to search for more in
    the tree by looking for the above keys)

Example:

```
// Array of device types
#define FIXED_DEVICE_TYPES
    {
        { 0x0016, 1 }, { 0x0100, 1 }, { 0x0100, 1 }, { 0xF002, 1 }
    }

// Array of device type offsets
#define FIXED_DEVICE_TYPE_OFFSETS
    {
        0, 1, 2, 3
    }

// Array of device type lengths
#define FIXED_DEVICE_TYPE_LENGTHS
    {
        1, 1, 1, 1
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

#### Examples of how to setup devices

##### Case study 1: Configuring a development M5Stack, as a multi-device to work with iOS/iPadOS/tvOS

Note: These instructions are specific to getting started with the
(Matter-provided) `all-clusters-app` on an ESP32-based M5Stack, however can be
generalised to work on most platforms ([more listed below](#guides))

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
-   [Infineon CYW30739 Lighting](/examples/lighting-app/infineon/cyw30739/README.md)
-   [Infineon PSoC6](/examples/all-clusters-app/infineon/psoc6/README.md)
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
