# Matter Silabs dishwasher Example

An example showing the use of Matter on the Silicon Labs EFR32 MG24 boards.

<hr>

-   [Matter Silabs dishwasher Example](#matter-silabs-dishwasher-example)
    -   [Introduction](#introduction)
    -   [Building](#building)
    -   [Flashing the Application](#flashing-the-application)
    -   [Viewing Logging Output](#viewing-logging-output)
    -   [Running the Complete Example](#running-the-complete-example)
        -   [Commissioning](#commissioning)

<hr>

> **NOTE:** Silicon Laboratories now maintains a public matter GitHub repo with
> frequent releases thoroughly tested and validated. Developers looking to
> develop matter products with silabs hardware are encouraged to use our latest
> release with added tools and documentation.
> [Silabs Matter Github](https://github.com/SiliconLabs/matter/releases)

## Introduction

The Silabs dishwasher example provides a baseline demonstration of a dishwasher
control device, built using Matter and the Silicon Labs gecko SDK. It can be
controlled by a Chip controller over an Openthread or Wifi network..

The Silabs device can be commissioned over Bluetooth Low Energy where the device
and the Chip controller will exchange security information with the Rendez-vous
procedure. If using Thread, Thread Network credentials are then provided to the
Silabs device which will then join the Thread network.

If the LCD is enabled, the LCD on the Silabs WSTK shows a QR Code containing the
needed commissioning information for the BLE connection and starting the
Rendez-vous procedure.

The dishwasher example is intended to serve both as a means to explore the
workings of Matter as well as a template for creating real products based on the
Silicon Labs platform.

## Building

-   Download the
    [Simplicity Commander](https://www.silabs.com/mcu/programming-options)
    command line tool, and ensure that `commander` is your shell search path.
    (For Mac OS X, `commander` is located inside
    `Commander.app/Contents/MacOS/`.)

-   Download and install a suitable ARM gcc tool chain:
    [GNU Arm Embedded Toolchain 9-2019-q4-major](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)

-   Install some additional tools (likely already present for CHIP developers):

    -   Linux: `sudo apt-get install git ninja-build`

    -   Mac OS X: `brew install ninja`

-   Supported hardware:

    -   > For the latest supported hardware please refer to the
        > [Hardware Requirements](https://github.com/SiliconLabs/matter/blob/latest/docs/silabs/general/HARDWARE_REQUIREMENTS.md)
        > in the Silicon Labs Matter Github Repo

    MG21 boards: Currently not supported due to RAM limitation.

    -   BRD4180A / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@20dBm

    MG24 boards :

    -   BRD2601B / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@10dBm
    -   BRD2703A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@10dBm
    -   BRD4186C / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@10dBm
    -   BRD4187C / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@20dBm
    -   BRD2703A / MG24 Explorer Kit
    -   BRD2704A / SparkFun Thing Plus MGM240P board

-   Build the example application:

          cd ~/connectedhomeip
          ./scripts/examples/gn_silabs_example.sh ./silabs_examples/dishwasher-app/silabs/ ./out/dishwasher-app BRD4187C

*   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip
          $ rm -rf ./out/

    OR use GN/Ninja directly

          $ cd ~/connectedhomeip/silabs_examples/dishwasher-app/silabs
          $ git submodule update --init
          $ source third_party/connectedhomeip/scripts/activate.sh
          $ export SILABS_BOARD=BRD4187C
          $ gn gen out/dishwasher-app
          $ ninja -C out/dishwasher-app

*   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip/silabs_examples/dishwasher-app/silabs
          $ rm -rf out/

For more build options, help is provided when running the build script without
arguments

         ./scripts/examples/gn_silabs_example.sh

## Flashing the Application

-   On the command line:

          $ python3 out/dishwasher-app/matter-silabs-dishwasher-example.flash.py

-   Or with the Ozone debugger, just load the .out file.

All Silabs boards require a bootloader, see Silicon Labs documentation for more
info. Pre-built bootloader binaries are available in the Assets section of the
Releases page on
[Silabs Matter Github](https://github.com/SiliconLabs/matter/releases) .

## Viewing Logging Output

The example application is built to use the SEGGER Real Time Transfer (RTT)
facility for log output. RTT is a feature built-in to the J-Link Interface MCU
on the WSTK development board. It allows bi-directional communication with an
embedded application without the need for a dedicated UART.

Using the RTT facility requires downloading and installing the _SEGGER J-Link
Software and Documentation Pack_
([web site](https://www.segger.com/downloads/jlink#J-LinkSoftwareAndDocumentationPack)).

Alternatively, SEGGER Ozone J-Link debugger can be used to view RTT logs too
after flashing the .out file.

-   Download the J-Link installer by navigating to the appropriate URL and
    agreeing to the license agreement.

-   [JLink_Linux_x86_64.deb](https://www.segger.com/downloads/jlink/JLink_Linux_x86_64.deb)
-   [JLink_MacOSX.pkg](https://www.segger.com/downloads/jlink/JLink_MacOSX.pkg)

*   Install the J-Link software

          $ cd ~/Downloads
          $ sudo dpkg -i JLink_Linux_V*_x86_64.deb

*   In Linux, grant the logged in user the ability to talk to the development
    hardware via the linux tty device (/dev/ttyACMx) by adding them to the
    dialout group.

          $ sudo usermod -a -G dialout ${USER}

Once the above is complete, log output can be viewed using the JLinkExe tool in
combination with JLinkRTTClient as follows:

-   Run the JLinkExe tool with arguments to autoconnect to the WSTK board:

    For MG24 use:

          $ JLinkExe -device EFR32MG24AXXXF1024 -if SWD -speed 4000 -autoconnect 1

-   In a second terminal, run the JLinkRTTClient to view logs:

          $ JLinkRTTClient

## Running the Complete Example

-   It is assumed here that you already have an OpenThread border router
    configured and running. If not see the following guide
    [Openthread_border_router](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/openthread_border_router_pi.md)
    for more information on how to setup a border router on a raspberryPi.

    Take note that the RCP code is available directly through
    [Simplicity Studio 5](https://www.silabs.com/products/development-tools/software/simplicity-studio/simplicity-studio-5)
    under File->New->Project Wizard->Examples->Thread : ot-rcp

-   User interface : **LCD** The LCD on Silabs WSTK shows a QR Code. This QR
    Code is be scanned by the CHIP Tool app For the Rendez-vous procedure over
    BLE

        * On devices that do not have or support the LCD Display like the BRD4166A Thunderboard Sense 2,
          a URL can be found in the RTT logs.

          <info  > [SVR] Copy/paste the below URL in a browser to see the QR Code:
          <info  > [SVR] https://project-chip.github.io/connectedhomeip/qrcode.html?data=CH%3AI34NM%20-00%200C9SS0

    **LED 0** shows the overall state of the device and its connectivity. The
    following states are possible:

        -   _Short Flash On (50 ms on/950 ms off)_ ; The device is in the
            unprovisioned (unpaired) state and is waiting for a commissioning
            application to connect.

        -   _Rapid Even Flashing_ ; (100 ms on/100 ms off)_ &mdash; The device is in the
            unprovisioned state and a commissioning application is connected through
            Bluetooth LE.

        -   _Short Flash Off_ ; (950ms on/50ms off)_ &mdash; The device is fully
            provisioned, but does not yet have full Thread network or service
            connectivity.

        -   _Solid On_ ; The device is fully provisioned and has full Thread
            network and service connectivity.

    **LED 1** Shows the dishwasher working state following states are possible:

        -   _Solid On_ ; dishwasher is running
        -   _Slow_Blink_ ; dishwasher is paused
        -   _Off_ ; dishwasher is stopped
        -   _Fast_Blink_ ; dishwasher has encountered an error

    **Push Button 0**

        -   _Press and Release_ : Start, or restart, BLE advertisement in fast mode. It will advertise in this mode
            for 30 seconds. The device will then switch to a slower interval advertisement.
            After 15 minutes, the advertisement stops.
            Additionally, it will cycle through the QR code, application status screen and device status screen, respectively.

        -   _Pressed and hold for 6 s_ : Initiates the factory reset of the device.
            Releasing the button within the 6-second window cancels the factory reset
            procedure. **LEDs** blink in unison when the factory reset procedure is
            initiated.

    **Push Button 1** Cycle the dishwasher operational states
    Running/Paused/Stopped

### Commissioning

You can provision and control the Matter device using the python controller,
`chip-tool` standalone, Android, or iOS app.

Silabs provides `chip-tool` as a wrapper function and more user-friendly method
of using [chip-tool](../../chip-tool/README.md) within the pre-built Raspberry
Pi image. For more info on using `chip-tool`, see
[Chiptool](../../../docs/development_controllers/chip-tool/chip_tool_guide.md).

Here is an example using `chip-tool`:

    $ chip-tool pairing ble-thread 1 hex:0e080000000000010000000300001335060004001fffe002084fe76e9a8b5edaf50708fde46f999f0698e20510d47f5027a414ffeebaefa92285cc84fa030f4f70656e5468726561642d653439630102e49c0410b92f8c7fbb4f9f3e08492ee3915fbd2f0c0402a0fff8 20202021 3840 --ble-adapter 0
    $ chip-tool operationalstate start 1 1
