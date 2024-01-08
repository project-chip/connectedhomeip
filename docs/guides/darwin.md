# Testing with Apple Devices

### Matter is the foundation for connected things.

Learn more about Matter [here](https://buildwithmatter.com/)

## Source Compatibility

Each release of an Apple operating system, or developer preview release may
bring a new SHA from this repository, which can be helpful for determining which
changes are present in the release for testing.

Listed are the Current SHAs:

-   iOS/iPadOS/tvOS 16.1:
    [`33f6a910cd9a8a0cfdd7088e2f43efd2f7f566a7`](https://github.com/project-chip/connectedhomeip/commits/33f6a910cd9a8a0cfdd7088e2f43efd2f7f566a7)

-   iOS/iPadOS/tvOS 16.2 and 16.3:
    [`c279578c5bc37f117335aa96cec6c5552f070cc0`](https://github.com/project-chip/connectedhomeip/commits/c279578c5bc37f117335aa96cec6c5552f070cc0)

## Supported Platforms for Matter Device Testing

-   Matter is supported by iOS/tvOS/iPadOS/watchOS/HomePod/AppleTV starting with
    16.1

-   To test your Matter device with Apple's platforms, all you need is a device
    with a supported OS
-   Pairing via QR Code or Setup Code is supported directly from the Home App
-   Please proceed to
    [this section](#testing-your-matter-device-with-apple-home) if you're
    developing a new device
    -   If your device has already been certified, or already fully implemented
        and just awaiting testing, you can just try pairing to Apple Home
        directly
-   Please provide [feedback to Apple](#providing-feedback-to-apple) if you
    encounter any issues

## Supported Platforms for Application Development

For Context: This is a mapping of platform to OS

-   iPhone _(iOS)_
-   iPad _(iPadOS)_
-   mac _(macOS)_
-   AppleTV _(tvOS)_
-   HomePod _(tvOS)_
-   Apple Watch _(watchOS)_

### Overview

-   To develop a Matter Application, learn more about how to develop with Matter
    on Apple's platforms [here](https://developer.apple.com/apple-home/matter/)

    -   Additional documentation about the Matter Support API is
        [here](https://developer.apple.com/documentation/mattersupport)

### Setup Requirements for Application Development

-   Devices must support BLE pairing, and have it enabled
-   An iPhone or iPad running iOS/iPadOS at least 16.1.
-   An Apple TV or HomePod running at least tvOS 16.1.
-   The iPhone or iPad with the
    [Matter Developer Profile](#profile-installation) must belong to the same
    iCloud account as the Apple TV or HomePod with the Matter
    [Matter Developer Profile](#profile-installation).
-   A test iCloud account is highly recommended for Matter development to avoid
    interference with existing homes.
-   To pair and control Matter Thread devices, you'll need to have a HomePod
    Mini, Apple TV 4K, or another device that supports Thread
-   Enable Developer Mode during the development phase of your app by following
    the steps at
    [Enabling Developer Mode](https://developer.apple.com/documentation/xcode/enabling-developer-mode-on-a-device)
    on a device.

## Profile Installation

#### Reminder: This is only needed for Application Development

### Install the Matter Developer Profile on your iPhone or iPad

##### Note: For more detailed profile installation instructions, refer to [here](https://developer.apple.com/bug-reporting/profiles-and-logs)

##### Note: The profile will expire automatically after some time

1. Download the
   [Matter Developer Profile](https://developer.apple.com/services-account/download?path=/iOS/iOS_Logs/EnableBluetoothCentralMatterClientDeveloperMode.mobileconfig)
2. Email the profile to an account that is configured on your device running
   iOS/iPadOS 16.1 (or greater)
3. Selecting the profile in Mail on your iOS/iPadOS 16.1 (or greater) device
   will present an option to install the profile

    ##### Note: Make sure to head back to system preferences, and you'll see an option to verify and install the profile near the top

4. (Optional) You may be prompted if you want to install it on other devices.
   Follow the prompts to install on the HomePod, Apple TV, etc., where you have
   the described issue and reproduce it.

5. Restart the device

### Install the Matter Profile on your HomePod

##### Note: The profile will expire automatically after some time

1. Download the
   [Matter Developer Profile](https://developer.apple.com/services-account/download?path=/iOS/iOS_Logs/EnableBluetoothCentralMatterClientDeveloperMode.mobileconfig)
2. Email the profile to an account that is configured on your device running
   iOS/iPadOS 16.1 (or greater)
3. Selecting the profile in Mail on your iOS/iPadOS 16.1 (or greater) device
   will present an option to install the profile on the HomePod
4. Restart the device

### Install the Matter Profile on your Apple TV

##### Note: The profile will expire automatically after some time

1. Download the
   [profile](https://developer.apple.com/services-account/download?path=/iOS/iOS_Logs/EnableBluetoothCentralMatterClientDeveloperMode.mobileconfig)
2. Use
   [Apple Configurator](https://apps.apple.com/us/app/apple-configurator/id1037126344?mt=12)
   to transfer the profile to the Apple TV
3. Restart the device

### Enable Developer Mode on your Apple Device

##### Note: Developer mode is a great way to get logs from your device as well as

enables other useful developer tools

To enable developer mode, please follow the instructions
[here](https://developer.apple.com/documentation/xcode/enabling-developer-mode-on-a-device)

## Using chip-tool on macOS, or CHIP Tool on iOS

Pairing Matter Accessory using Open Source Matter Darwin chip-tool and iOS
chip-tool will require installing the
[Bluetooth Central Matter Client Developer mode profile](https://developer.apple.com/services-account/download?path=/iOS/iOS_Logs/EnableBluetoothCentralMatterClientDeveloperMode.mobileconfig)
on MacOS or iOS/iPadOS.

-   Download the Bluetooth Central Matter Client Developer Mode profile and
    install it on a iOS/iPadOS 16.1 beta 3 and MacOS 13.1 beta 3 or later
    system.

    -   For _macOS_, Profile can be installed via Settings->Privacy &
        Security->Profiles
    -   For _iOS/iPadOS_, If necessary, email the profile or use AirDrop to
        transfer the profile to the _iOS/iPadOS 16 beta 3_ device.

-   Restart your system
    -   For _iOS/iPadOS_, enable Developer Mode. Refer to
        [this developer page](https://developer.apple.com/documentation/xcode/enabling-developer-mode-on-a-device)
-   Compile chip-tool for macOS or CHIP Tool for iOS

## Testing your Matter Device with Apple Home

1. Clone the [Matter repo](https://github.com/project-chip/connectedhomeip.git)
2. Checkout the specific commit hash (from [above](#source-compatibility)) for
   maximum compatibility with your installed release:
    - Example command for SHA `c279578c5bc37f117335aa96cec6c5552f070cc0`:
      `$ git checkout c279578c5bc37f117335aa96cec6c5552f070cc0`

In order to work with iOS/iPadOS/tvOS 16.1 or greater, device types as defined
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

##### Note: These instructions are specific to getting started with the (Matter-provided) `all-clusters-app` on an ESP32-based M5Stack, however can be generalised to work on most platforms ([more listed below](#guides))

1. Checkout and setup
   [Matter repo](https://github.com/project-chip/connectedhomeip.git) as per the
   instructions above.
2. Follow
   [these](https://github.com/project-chip/connectedhomeip/tree/master/examples/all-clusters-app/esp32)
   instructions to initialize your development environment, compile the firmware
   and flash your hardware.

##### Case study 2: a development Nordic board (nRF52840), working as a light to work with iOS/iPadOS/tvOS

##### Note: These instructions are specific to getting started with the (Matter-provided) "lighting-app" on an Nordic-based NRF52840, however can be generalised to work on most platforms (more listed below)

1. Checkout and setup
   [Matter repo](https://github.com/project-chip/connectedhomeip.git) as per the
   instructions above.

2. Follow
   [these instructions](https://github.com/project-chip/connectedhomeip/tree/master/examples/lighting-app/nrfconnect#readme)
   initialize your development environment, compile the firmware and flash your
   hardware.

#### General Matter Platform Development Guide

##### Getting the SDK Ready

##### Note: Most platforms have very similar, if not the same configuration requirements

1. Checkout and setup
   [Matter repo](https://github.com/project-chip/connectedhomeip.git) as per the
   instructions above.
2. Find and edit one of the platform
   [examples](https://github.com/project-chip/connectedhomeip/tree/master/examples)
   to support the fixed device types above.
3. Read the [platform guides](README.md) on how set up the hardware

##### Guides

-   [Bouffalo Lab](/examples/lighting-app/bouffalolab/README.md)
-   [EFR32 Window Covering](/examples/window-app/silabs/README.md)
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
-   [Silicon Labs](./silabs_getting_started.md)
-   [Simulated Linux](./simulated_device_linux.md)
-   [Telink](/examples/lighting-app/telink/README.md)
-   [TI Platform](./ti/ti_platform_overview.md)
-   [TI All Clusters](/examples/all-clusters-app/cc13x4_26x4/README.md)
-   [Tizen](/examples/lighting-app/tizen/README.md)

## Providing Feedback to Apple

##### Note: For profile installation instructions, refer to [here](#profile-installation)

-   File a feedback assistant ticket.
    [About Feedback Assistant](https://developer.apple.com/bug-reporting/#feedback-assistant)
-   Click this link, and log in using your account:
    [File Feedback](https://feedbackassistant.apple.com/new-form-response)
-   Choose iOS/iPadOS to file an issue
-   Choose the area as Home App and fill in the details of the type of issue and
    the description
-   Please provide the following technical information:
    -   `sysdiagnose` logs for iPhone / iPad with the timestamp / time frame of
        the issue
    -   `sysdiagnose` logs for all the Home Hubs (HomePod mini / HomePod / Apple
        TV) with the timestamp / timeframe of the issue
    -   Accessory logs with the timestamp / timeframe of the issue
    -   mDNS network logs with timestamp / timeframe of the issue
    -   Video or screenshot describing the issue (optional but beneficial)
-   Please make sure you download and install the profiles for iOS and tvOS as
    provided here. The instructions are also in these links.
    -   iOS:
        -   [Apple Home Profile](https://developer.apple.com/bug-reporting/profiles-and-logs/?platform=ios&name=homekit)
        -   [Network Profile](https://developer.apple.com/bug-reporting/profiles-and-logs/?platform=ios&name=network)
        -   [mDNS Profile](https://developer.apple.com/bug-reporting/profiles-and-logs/?platform=ios&name=mdns)
    -   tvOS:
        -   [Apple Home Profile](https://developer.apple.com/bug-reporting/profiles-and-logs/?platform=tvos&name=homekit)
        -   [Network Profile](https://developer.apple.com/bug-reporting/profiles-and-logs/?platform=tvos&name=network)
        -   [mDNS Profile](https://developer.apple.com/bug-reporting/profiles-and-logs/?platform=tvos&name=mdns)

### Release Notes & Known Issues

-   Please refer to the iOS/iPadOS 16.1
    [Release Notes](https://developer.apple.com/documentation/ios-ipados-release-notes/ios-16_1-release-notes)
    for currently known issues.
-   Please refer to the iOS/iPadOS 16.2
    [Release Notes](https://developer.apple.com/documentation/ios-ipados-release-notes/ios-ipados-16_2-release-notes)
    for currently known issues
-   Please refer to the iOS & iPadOS 16.3
    [Release Notes](https://developer.apple.com/documentation/ios-ipados-release-notes/ios-ipados-16_3-release-notes)
    for currently known issues
-   Please refer to the iOS & iPadOS 16.4
    [Release Notes](https://developer.apple.com/documentation/ios-ipados-release-notes/ios-ipados-16_4-release-notes)
    for currently known issues
-   Please refer to the iOS & iPadOS 16.5
    [Release Notes](https://developer.apple.com/documentation/ios-ipados-release-notes/ios-ipados-16_5-release-notes)
    for currently known issues
-   An issues related to Apple Home integration should be reported
    [feedback](#providing-feedback-to-apple) as described in this section

-   Any issues related to the Matter SDK should be reported to the project
    [here](https://github.com/project-chip/connectedhomeip/issues)
