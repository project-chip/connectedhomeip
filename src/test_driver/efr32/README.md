#CHIP EFR32 Test Driver

This builds and runs the NLUnitTest on the efr32 device

<hr>

-   [CHIP EFR32 Test Driver](#chip-efr32-test-driver)
    -   [Introduction](#introduction)
    -   [Building](#building)
    -   [Running The Tests](#running-the-tests)

<hr>

<a name="introduction"></a>

## Introduction

This builds a test binary which contains the NLUnitTests and can be flashed onto
a device. The device is controlled using the included RPCs, through the python
test runner.

<a name="building"></a>

## Building

-   Download the
    [Simplicity Commander](https://www.silabs.com/mcu/programming-options)
    command line tool, and ensure that `commander` is your shell search path.
    (For Mac OS X, `commander` is located inside
    `Commander.app/Contents/MacOS/`.)

-   Download and install a suitable ARM gcc tool chain:
    [GNU Arm Embedded Toolchain 9-2019-q4-major](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)

-   Install some additional tools(likely already present for CHIP developers):

#Linux \$ sudo apt-get install git libwebkitgtk-1.0-0 ninja-build

#Mac OS X \$ brew install ninja

-   Supported hardware:

    MG12 boards:

    -   BRD4161A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@19dBm
    -   BRD4164A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@19dBm
    -   BRD4166A / SLTB004A / Thunderboard Sense 2 / 2.4GHz@10dBm
    -   BRD4170A / SLWSTK6000B / Multiband Wireless Starter Kit / 2.4GHz@19dBm,
        915MHz@19dBm
    -   BRD4304A / SLWSTK6000B / MGM12P Module / 2.4GHz@19dBm

    MG21 boards:

    -   BRD4180A / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@20dBm

OR use GN/Ninja directly

          $ cd ~/connectedhomeip/src/test_driver/efr32/
          $ git submodule update --init
          $ source third_party/connectedhomeip/scripts/activate.sh
          $ export EFR32_BOARD=BRD4161A
          $ gn gen out/debug
          $ ninja -C out/debug

-   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip/src/test_driver/efr32/
          $ rm -rf out/

<a name="running-the-tests"></a>

## Running The Tests

The included python test runner will be installed as part of building.

-   To run the tests:

    \$ python -m nl_test_runner.nl_test_runner -d /dev/ttyACM1 -f
    out/debug/chip-efr32-device_tests.s37 -o out.log
