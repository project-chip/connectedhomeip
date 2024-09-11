# Matter Development and Testing with/on Apple Devices

### Overview

-   To develop a Matter Application, learn more about how to develop with Matter
    on Apple's platforms [here](https://developer.apple.com/apple-home/matter/)

    -   Additional documentation about the Matter Support API is
        [here](https://developer.apple.com/documentation/mattersupport)

### Filing Issues

-   An issues related to Apple Home, or Development on Apple's Platforms should
    be reported using Feedback Assistant as described
    [below](#providing-feedback-to-apple)

-   Any issues related to the Matter SDK should be reported to the project
    [here](https://github.com/project-chip/connectedhomeip/issues)

## Source Compatibility

Each release of an Apple operating system, or developer preview release may
bring a new SHA from this repository, which can be helpful for determining which
changes are present in the release for testing.

Listed are the Current SHAs:

-   Latest macOS/tvOS/iOS/iPadOS:
    [`2ee90eba27676950fa2f4ef96597d9696f510d5d`](https://github.com/project-chip/connectedhomeip/commits/2ee90eba27676950fa2f4ef96597d9696f510d5d)
-   Larger list of SHAs are [here](#release-to-sha-mappings)

## Supported Platforms for Matter Device Testing

-   Matter is supported by iOS/tvOS/iPadOS/watchOS/HomePod/AppleTV starting with
    16.1
-   To test your Matter device with Apple's platforms, all you need is a device
    with a supported OS
-   Pairing via QR Code or Setup Code is supported directly from the Home App,
    or any third party Application
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

##### Note: Developer mode is a great way to get logs from your device as well as enable other useful developer tools

To enable developer mode, please follow the instructions
[here](https://developer.apple.com/documentation/xcode/enabling-developer-mode-on-a-device)

## Using chip-tool on macOS, or CHIP Tool on iOS

Pairing Matter Accessory using Open Source Matter Darwin chip-tool and iOS
chip-tool will require installing the
[Bluetooth Central Matter Client Developer mode profile](https://developer.apple.com/services-account/download?path=/iOS/iOS_Logs/EnableBluetoothCentralMatterClientDeveloperMode.mobileconfig)
on MacOS or iOS/iPadOS.

-   Download the Bluetooth Central Matter Client Developer Mode profile and
    install it on a supported system.

    -   For _macOS_, Profile can be installed via Settings->Privacy &
        Security->Profiles
    -   For _iOS/iPadOS_, If necessary, email the profile or use AirDrop to
        transfer the profile to the _iOS/iPadOS_ device.

-   Restart your system
    -   For _iOS/iPadOS_, enable Developer Mode. Refer to
        [this developer page](https://developer.apple.com/documentation/xcode/enabling-developer-mode-on-a-device)
-   Compile chip-tool for macOS or CHIP Tool for iOS

## Supported Device Types in Apple Home

##### Note: Not exhaustive, and may be out of date, see [below](#apple-home-development-guide) for more information

| Type                    | Decimal | HEX  | Notes                                            |
| ----------------------- | ------- | ---- | ------------------------------------------------ |
| On/Off Light            | 256     | 0100 |                                                  |
| Dimming Light           | 257     | 0101 |                                                  |
| On/Off Plug-In Unit     | 266     | 010A |                                                  |
| Dimmable Plug-In Unit   | 267     | 010B |                                                  |
| On/Off Light Switch     | 259     | 0103 | Requires both On/Off Client and Server           |
| Dimmer Switch           | 260     | 0104 | Requires both On/Off Client and Server           |
| Generic Switch (button) | 15      | 000F | Supports momentary switch only, and not latching |
| Contact Sensor          | 21      | 0015 |                                                  |
| Light Sensor            | 262     | 0106 |                                                  |
| Occupancy Sensor        | 263     | 0107 |                                                  |
| Temperature Sensor      | 770     | 0302 |                                                  |
| Humidity Sensor         | 775     | 0307 |                                                  |
| Air Quality Sensor      | 44      | 002C |                                                  |
| Door Lock               | 10      | 000A |                                                  |
| Window Covering         | 514     | 0202 |                                                  |
| Heating/Cooling Unit    | 768     | 0300 |                                                  |
| Thermostat              | 769     | 0301 |                                                  |
| Fan                     | 43      | 002B |                                                  |
| Air Purifier            | 45      | 002D |                                                  |
| Temperature Color Light | 268     | 010C |                                                  |
| Enhanced Color Light    | 269     | 010D |                                                  |
| Bridges                 | 14      | 000E |                                                  |
| Robot Vacuum Cleaner    | 116     | 0074 | Announced, not yet supported                     |

## Additional Device Type Support on Apple's Platforms

Apple's platforms support all device types available in the Matter SDK, so
developers can use Matter.framework to develop their own applications that can
add devices to Apple Home, or create their own Fabric to manage devices as well.

Please see documentation about `-[HMAccessory matterNodeID]`
[here](https://developer.apple.com/documentation/homekit/hmaccessory/matternodeid-5zfqo)
which allows you to use Matter.framework to interact with Matter Devices
directly that are paired into Apple Home.

## Apple Home Development Guide

Please see [here](https://developer.apple.com/apple-home/) for more general
information about developing an Application or a Device for Apple Home. This
includes information about
[best practices](https://developer.apple.com/apple-home/downloads/Matter-Accessory-Best-Practices-for-Apple-Home.pdf),
[platform developer API](https://developer.apple.com/apple-home/matter/),
[OTA Updates](https://developer.apple.com/accessories/Apple-Matter-OTA-User-Guide.pdf),
general adoption Q&A, and the "Works with Apple Home" badge.

## General Matter Platform Development Guide

##### Getting the SDK Ready

##### Note: Most platforms have very similar, if not the same configuration requirements

1. Checkout and setup
   [Matter repo](https://github.com/project-chip/connectedhomeip.git) as per the
   instructions above.
2. Find and edit one of the platform
   [examples](https://github.com/project-chip/connectedhomeip/tree/master/examples)
   to support the fixed device types above.
3. Read the [platform guides](README.md) on how set up the hardware
    - There is a list of more detailed guides [here](#platform-guides)

## Testing your Matter Device with Apple Home

1. Clone the
   [Matter repository](https://github.com/project-chip/connectedhomeip.git)
2. Checkout the specific commit hash (from [above](#source-compatibility)) for
   maximum compatibility with your installed release:
    - Example command for SHA `2ee90eba27676950fa2f4ef96597d9696f510d5d`:
      `$ git checkout 2ee90eba27676950fa2f4ef96597d9696f510d5d`

In order to work with iOS/iPadOS/tvOS, device types as defined in the Matter
Device Library spec are used to determine accessory categories. Ensure the right
device type is set for each endpoint.

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

### Examples of how to setup devices

##### Case study 1: Configuring a development M5Stack, as a multi-device to work with iOS/iPadOS/tvOS

##### Note: These instructions are specific to getting started with the (Matter-provided) `all-clusters-app` on an ESP32-based M5Stack, however can be generalised to work on most platforms ([more listed below](#platform-guides))

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

### Platform Guides

-   [Bouffalo Lab](/examples/lighting-app/bouffalolab/README.md)
-   [EFR32 Window Covering](/examples/window-app/silabs/README.md)
-   [ESP32 All Clusters](/examples/all-clusters-app/esp32/README.md)
-   [ESP32 Lighting](/examples/lighting-app/esp32/README.md)
-   [ESP32 Temperature Sensor](/examples/temperature-measurement-app/esp32/README.md)
-   [mbedOS](/examples/all-clusters-app/mbed/README.md)
-   [nRF Connect All Clusters](./nrfconnect_examples_configuration.md)
-   [nRF Connect Pump](/examples/pump-app/nrfconnect/README.md)
-   [NXP Examples](./nxp/nxp_imx8m_linux_examples.md)
-   [NXP](/examples/all-clusters-app/nxp/mw320/README.md)
-   [Infineon CYW30739 Lighting](/examples/lighting-app/infineon/cyw30739/README.md)
-   [Infineon PSoC6](/examples/all-clusters-app/infineon/psoc6/README.md)
-   [Qorvo](/examples/lighting-app/qpg/README.md)
-   [Silicon Labs](./silabs_getting_started.md)
-   [Simulated Linux](./simulated_device_linux.md)
-   [Telink](/examples/lighting-app/telink/README.md)
-   [TI Platform](./ti/ti_matter_overview.md)
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
    -   Any Thread device:
        [Thread Profile](https://developer.apple.com/bug-reporting/profiles-and-logs/?name=HomeThread)

### Release to SHA Mappings

| Platform | Release Version | SHA                                        |
| -------- | --------------- | ------------------------------------------ |
| iOS      | 17.6.1          | `2ee90eba27676950fa2f4ef96597d9696f510d5d` |
| iPadOS   | 17.6.1          | `2ee90eba27676950fa2f4ef96597d9696f510d5d` |
| tvOS     | 17.6            | `2ee90eba27676950fa2f4ef96597d9696f510d5d` |
| macOS    | 14.6.1          | `2ee90eba27676950fa2f4ef96597d9696f510d5d` |
| iOS      | 17.5.1          | `d09b5ac98f4d7d8b9f2c307f55ab5462576623a5` |
| iPadOS   | 17.5.1          | `d09b5ac98f4d7d8b9f2c307f55ab5462576623a5` |
| tvOS     | 17.5.1          | `d09b5ac98f4d7d8b9f2c307f55ab5462576623a5` |
| macOS    | 14.5            | `d09b5ac98f4d7d8b9f2c307f55ab5462576623a5` |
| iOS      | 16.7.8          | `83f7a2fe136e0b746db09f1d19e36c693a634b66` |
| iPadOS   | 16.7.8          | `83f7a2fe136e0b746db09f1d19e36c693a634b66` |
