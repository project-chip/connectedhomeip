# CHIP nRF Connect Lock Example Application

The nRF Connect Lock Example demonstrates how to remotely control a door lock device with
one basic bolt. It uses buttons to test changing the lock and device states and LEDs
to show the state of these changes.
You can use this example as a reference for creating your own application.

The example is based on [CHIP](https://github.com/project-chip/connectedhomeip) and the nRF Connect
platform, and supports remote access and control of a simulated door lock over a low-power, 802.15.4
Thread network. It is capable of being paired into an existing CHIP network along with other
CHIP-enabled devices.

<hr>

-   [Overview](#overview)
    -   [Bluetooth LE Advertising](#bluetooth-le-advertising)
    -   [Bluetooth LE Rendezvous](#bluetooth-le-rendezvous)
    -   [Thread provisioning](#thread-provisioning)
-   [Requirements](#requirements)
-   [Device UI](#device-ui)
-   [Setting up environment](#setting-up-environment)
    -   [Using Docker container](#using-docker-container)
    -   [Using Native shell](#using-native-shell)
-   [Building](#building)
        -   [Building minimal binary](#building-minimal-binary)
-   [Flashing and debugging](#flashing-and-debugging)
-   [Testing the example](#testing-the-example)

<hr>

<a name="overview"></a>

## Overview

This example is running on the nRF Connect platform, which is based
on the [nRF Connect SDK](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/index.html)
and [Zephyr RTOS](https://zephyrproject.org/). Visit CHIP's
[nRF Connect Platform Overview](TODO:...) to read more information about
platform structure and dependencies.

The CHIP device that runs the lock application is controlled by the CHIP controller
device over the Thread protocol. By default, the CHIP device should have Thread disabled,
and it should be paired with CHIP controller and get configuration from it.
Some actions required before establishing full communication are described below.

The example also comes with a test mode, which allows to start Thread with the default settings
by pressing button manually. However, this mode does not guarantee that the device will be able
to communicate with the CHIP controller and other devices.

### Bluetooth LE Advertising

After powering up the device for the first time, it will start advertising over
Bluetooth LE to inform other devices about its presence. For security reasons,
Bluetooth LE advertising won't start automatically after powering up the device.
To make the device discoverable, you must press **Button 4**.

### Bluetooth LE Rendezvous

In CHIP, there is a commissioning procedure called Rendezvous, which is done over
Bluetooth LE between a CHIP device and the CHIP controller,
where the controller has the commissioner role.

To start the Rendezvous, the controller must get the onboarding information from the CHIP device.
The data payload is encoded within a QR code and typically presented on the device's display.
For this example, however, it is shared using **NFC**.

### Thread provisioning

Successfully finishing the Rendezvous procedure allows to perform Provisioning
operation, whose goal is to send the Thread network credentials from the CHIP controller
to the CHIP device. As a result, device is able to join the Thread network and
communicate with other Thread devices in the network.

<hr>

<a name="requirements"></a>

## Requirements

The application requires the nRF Connect SDK v1.4.0 to work correctly.

The example supports building and running on the following devices:

| Board name                                                                                | Board platform build name |
| ----------------------------------------------------------------------------------------- | ------------------------- |
| [nRF52840 DK](https://www.nordicsemi.com/Software-and-Tools/Development-Kits/nRF52840-DK) | `nrf52840dk_nrf52840`     |

<hr>

<a name="device-ui"></a>

## Device UI

This section lists the User Interface elements that you can use to control and monitor the state
of the device.
All these elements can be located on the following board picture:

![nrf52840 DK](../../platform/nrfconnect/doc/images/nrf52840-dk.png)

**LED 1** shows the overall state of the device and its connectivity.
The following states are possible:

- _Short Flash On (50 ms on/950 ms off)_ &mdash; The device is in the unprovisioned (unpaired) state
  and is waiting for a commissioning application to connect.

- _Rapid Even Flashing (100 ms on/100 ms off)_ &mdash; The device is in the unprovisioned state
  and a commissioning application is connected through Bluetooth LE.

- _Short Flash Off (950ms on/50ms off)_ &mdash; The device is fully provisioned, but does not yet
  have full Thread network or service connectivity.

- _Solid On_ &mdash; The device is fully provisioned and has full Thread
  network and service connectivity.

**LED 2** simulates the lock bolt and shows the state of the lock.
The following states are possible:

- _Solid On_ &mdash; The bolt is extended and the door is locked.

- _Off_ &mdash; The bolt is retracted and the door is unlocked.

- _Rapid Even Flashing (100 ms on/100 ms off during 2 s)_ &mdash; The simulated bolt is in motion
  from one position to another.

**Button 1** can be used for the following purposes:

- _Pressed for 6 s_ &mdash; Initiates the factory reset of the device.
  Releasing the button within the 6-second window cancels the factory reset procedure.
  **LEDs 1-4** blink in unison when the factory reset procedure is initiated.

- _Pressed for less than 3 s_ &mdash; Initiates the OTA software update process.
  This feature is not currently supported.

**Button 2** &mdash; Pressing the button once changes the lock state to the opposite one.

**Button 3** &mdash; Pressing the button once starts the Thread networking in the test mode
using the default configuration.

**Button 4** &mdash; Pressing the button once starts the Bluetooth LE advertising
for the predefined period of time.

**SEGGER J-Link USB port** can be used to get logs from the device or
communicate with it using the [command line interface](TODO:).

**NFC port with antenna attached** can be used to start the [Rendezvous](#bluetooth-le-rendezvous)
by providing the onboarding information from the CHIP device in a data payload that can be shared
[using NFC](TODO:).

<hr>

## Setting up the environment

Before building the example, check out the CHIP repository
and sync submodules using the following command:

        $ git submodule update --init

The example requires the nRF Connect SDK v1.4.0.
You can either install it along with the related tools directly on your system or use a Docker image
that has the tools pre-installed.

If you are a macOS user, you won't be able to use the Docker container
to flash the application onto a Nordic board due to
[certain limitations of Docker for macOS](https://docs.docker.com/docker-for-mac/faqs/#can-i-pass-through-a-usb-device-to-a-container).
Use the [native shell](#using-native-shell) for building instead.

### Using Docker container for setup

To use the Docker container for setup, complete the following steps:

1. If you do not have the nRF Connect SDK installed yet, create a directory for it by running the following command:

        $ mkdir ~/nrfconnect

2. Download the latest version of the nRF Connect SDK Docker image by running the following command:

        $ docker pull nordicsemi/nrfconnect-chip

3. Start Docker with the downloaded image by running the following command, customized to your needs
   as described below:

        $ docker run --rm -it -e RUNAS=$(id -u) -v ~/nrfconnect:/var/ncs -v ~/connectedhomeip:/var/chip \
            -v /dev/bus/usb:/dev/bus/usb --device-cgroup-rule "c 189:* rmw" nordicsemi/nrfconnect-chip

   In this command:

   -   *~/nrfconnect* can be replaced with an absolute path to the nRF Connect SDK
       source directory.
   -   *~/connectedhomeip* must be replaced with an absolute path to the CHIP source directory.
   -   *-v /dev/bus/usb:/dev/bus/usb --device-cgroup-rule "c 189:* rmw"*
       parameters can be omitted if you are not planning to flash the example onto
       hardware. These parameters give the container access to USB devices
       connected to your computer such as the nRF52840 DK.
   -   *--rm* can be omitted if you do not want the container to be
       auto-removed when you exit the container shell session.
   -   *-e RUNAS=$(id -u)* is needed to start the container session as the
       current user instead of root.

4. Check out or update the nRF Connect SDK to the recommended `v1.4.0` version by running
   the following command in the Docker container:

        $ setup --ncs v1.4.0
        /var/ncs repository is empty. Do you wish to check out nRF Connect SDK sources [v1.4.0]? [Y/N] y
        ...
        /var/chip repository is initialized, skipping...

Now you can proceed with the [Building](#building) instruction.

### Using native shell for setup

To use the native shell for setup, complete the following steps:

1. Download and install the following additional software:

   - [nRF Command Line Tools](https://www.nordicsemi.com/Software-and-Tools/Development-Tools/nRF-Command-Line-Tools)
   - [GN meta-build system](https://gn.googlesource.com/gn/)

2. Depending on whether you have the nRF Connect SDK installed:

   - Follow the
     [guide](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/gs_assistant.html#)
     in the nRF Connect SDK documentation to install the nRF Connect SDK v1.4.0.
     Since command-line tools will be used for building the example, installing SEGGER Embedded Studio is not required.

   - If you have an older version of the SDK installed, use the following
     commands to update it to the recommended version. Replace *nrfconnect-dir*
     with the path to your nRF Connect SDK installation directory.

            $ cd nrfconnect-dir/nrf
            $ git fetch origin
            $ git checkout v1.4.0
            $ west update

3. Initialize environment variables referred to by the CHIP and the nRF Connect SDK build
   scripts. Replace *nrfconnect-dir* with the path to your nRF Connect SDK
   installation directory, and *toolchain-dir* with the path to GNU Arm
   Embedded Toolchain.

        $ source nrfconnect-dir/zephyr/zephyr-env.sh
        $ export ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb
        $ export GNUARMEMB_TOOLCHAIN_PATH=toolchain-dir

Now you can proceed with the [Building](#building) instruction.

<hr>

<a name="building"></a>

## Building

Complete the following steps, regardless of the method used for setting up the environment:

1. Navigate to the example's directory:

        $ cd examples/lock-app/nrfconnect

2. Run the following command to build the example, with *board-name* replaced with the name
   of the Nordic Semiconductor's board you own, for example `nrf52840dk_nrf52840`:

        $ west build -b board-name

   You only need to specify the board name on the first build.
   See [Requirements](#requirements) for the names of compatible boards.

3. Remove all build artifacts by running the following command:

        $ rm -r build

4. Build the example with the release configuration that disables the diagnostic features
   like logs and command-line interface by running the following command:

        $ west build -b board-name -- -DOVERLAY_CONFIG=third_party/connectedhomeip/config/nrfconnect/release.conf

   Remember to replace *board-name* with the name of the Nordic Semiconductor's board you own.

The output `zephyr.hex` file will be available in the `/build/zephyr/` directory.

<hr>

### Building minimal binary

In order to build the example with no diagnostic features like UART console or
application logs, which should result in significantly smaller binary, run the
following commands:

        # Delete the build directory to make sure that no settings are cached
        $ rm -rf build/

        # Build the example using release config overlay
        $ west build -b nrf52840dk_nrf52840 -- -DOVERLAY_CONFIG=third_party/connectedhomeip/config/nrfconnect/release.conf

<a name="configuring"></a>

## Configuring the example

The Zephyr ecosystem is highly configurable and allows you to modify many
aspects of the application. The configuration system is based on Kconfig files and
the settings can be modified using the menuconfig utility.

To open the menuconfig utility, complete the following steps:

1. Go to the example directory by running the following command, with the *example-dir* directory
   name updated for your configuration:

        $ cd example-dir

2. Choose one of the following options:

   - If you are running the build for the first time, run the following command:

            $ west build -b nrf52840dk_nrf52840 -t menuconfig

   - If you are running a subsequent build, run the following command:

            $ west build -t menuconfig

   - If you are running menuconfig with ninja, run the following commands:

            $ cd example-dir/build
            $ ninja menuconfig

Changes done with menuconfig will be lost if the `build` directory is deleted.
To make them persistent, save the configuration options in the `prj.conf` file.

For more information, see the
[Configuring nRF Connect SDK examples](../../../docs/guides/nrfconnect_examples_configuration.md) page.

<hr>

<a name="flashing"></a>

## Flashing and debugging

To flash the application to the device, use the west tool and run the following commands,
with the *example-dir* directory name updated for your configuration:

        $ cd example-dir
        $ west flash

If you have multiple nRF52840 DK boards connected, west will prompt you to pick the correct one.

To debug the application on target, run the following commands:

        $ cd example-dir
        $ west debug

<hr>

## Testing the example

Check the [CLI tutorial](../../../docs/guides/nrfconnect_examples_cli.md) to
learn how to use command-line interface of the application.

TODO: mention Rendezvous tutorial here
