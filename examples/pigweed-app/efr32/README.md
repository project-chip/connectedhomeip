#CHIP EFR32 Pigweed Example Application

The EFR32 example demonstrates the usage of Pigweed module functionalities in an
application.

The example is based on [CHIP](https://github.com/project-chip/connectedhomeip),
the [Pigweed](https://pigweed.googlesource.com/pigweed/pigweed) module, which is
a collection of libraries that provide different functionalities for embedded
systems, and the EFR32 platform.

You can use this example as a training ground for making experiments, testing
Pigweed module features and checking what actions are necessary to fully
integrate Pigweed in a CHIP project.

Pigweed functionalities are being gradually integrated into CHIP. Currently, the
following features are available:

-   **Echo RPC** - Creates a Remote Procedure Call server and allows sending
    commands through the serial port to the device, which makes echo and sends
    the received commands back.

---

-   [CHIP EFR32 Pigweed Example Application](#chip-EFR32-pigweed-example-application)
    -   [Building the Example Application](#building-the-example-application)
        -   [To build the application, follow these steps:](#to-build-the-application-follow-these-steps)
    -   [Testing the Example Application](#testing-the-example-application)

---

## Building the Example Application

-   Download or clone the
    [sdk_support](https://github.com/SiliconLabs/sdk_support) from GitHub

            git clone https://github.com/SiliconLabs/sdk_support.git


    and export the path with :

            export EFR32_SDK_ROOT=<Path to cloned git repo>

-   Download the
    [Simplicity Commander](https://www.silabs.com/mcu/programming-options)
    command line tool, and ensure that `commander` is your shell search path.
    (For Mac OS X, `commander` is located inside
    `Commander.app/Contents/MacOS/`.)

            export PATH=/Applications/Commander.app/Contents/MacOS:$PATH

-   Download and install a suitable ARM gcc tool chain:
    [GNU Arm Embedded Toolchain 9-2019-q4-major](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)

-   Install some additional tools(likely already present for CHIP developers):

    -   Linux

              sudo apt-get install git libwebkitgtk-1.0-0 ninja-build

    -   Mac OS X

              brew install ninja

-   Supported hardware:

    MG12 boards:

    -   BRD4161A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@19dBm
    -   BRD4162A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@10dBm
    -   BRD4163A / SLWSTK6000B / Dual band Wireless Starter Kit / 2.4GHz@19dBm,
        868MHz@19dBm
    -   BRD4164A / SLWSTK6000B / Dual band Wireless Starter Kit / 2.4GHz@19dBm,
        915MHz@19dBm
    -   BRD4166A / SLTB004A / Thunderboard Sense 2 / 2.4GHz@10dBm
    -   BRD4170A / SLWSTK6000B / Multiband Wireless Starter Kit / 2.4GHz@19dBm,
        915MHz@19dBm
    -   BRD4304A / SLWSTK6000B / MGM12P Module / 2.4GHz@19dBm

    MG21 boards: Currently not supported due to RAM limitation.

    -   BRD4180A / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@20dBm

    MG24 boards :

    -   BRD2601B / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@10dBm
    -   BRD2703A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@10dBm
    -   BRD4186A / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@10dBm
    -   BRD4186C / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@10dBm
    -   BRD4187A / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@20dBm
    -   BRD4187C / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@20dBm

*   Build the example application:

        cd ~/connectedhomeip/examples/pigweed-app/efr32
        git submodule update --init
        source third_party/connectedhomeip/scripts/activate.sh
        export EFR32_BOARD=BRD4161A
        gn gen out/debug
        ninja -C out/debug

-   To delete generated executable, libraries and object files use:

          cd ~/connectedhomeip/examples/pigweed-app/efr32
          rm -rf out/

### Flashing the Application

-   On the command line:

        cd ~/connectedhomeip/examples/pigweed-app/efr32
        python3 out/debug/chip-efr32-pigweed-example.flash.py

-   With the Commander app, just load the .s37 file.
-   Or with the Ozone debugger, just load the .out file.

## Testing the Example Application

-   Determine the serial port name for the EFR device by checking /dev: ls
    /dev/tty\*

        It should look like this :
        - On Linux
            /dev/ttyACM0
        - On MAC
            /dev/tty.usbmodem0004401548451

-   Run the following command to start an interactive Python shell, where the
    Echo RPC commands can be invoked:

        python -m pw_hdlc.rpc_console --device /dev/tty.usbmodem0004401548451 -b 115200 <CHIP_ROOT>/third_party/pigweed/repo/pw_rpc/pw_rpc_protos/echo.proto -o /tmp/pw_rpc.out

-   To send an Echo RPC message, type the following command, where the actual
    message is the text in quotation marks after the `msg=` phrase:

        rpcs.pw.rpc.EchoService.Echo(msg="hi")

    Note: Some users might have to install the
    [VCP driver](https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers)
    before the device shows up on `/dev/tty`.

## Memory settings

While most of the RAM usage in CHIP is static, allowing easier debugging and
optimization with symbols analysis, we still need some HEAP for the crypto and
OpenThread. Size of the HEAP can be modified by changing the value of the
`SL_STACK_SIZE` define inside of the BUILD.gn file of this example. Please take
note that a HEAP size smaller than 5k can and will cause a Mbedtls failure
during the BLE rendez-vous.
