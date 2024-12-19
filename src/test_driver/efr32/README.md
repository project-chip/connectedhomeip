This is a test driver for the Matter unit tests to be run on an EFR32 device. It
builds a set of test binaries, each of which contains one directory of unit
tests that can be flashed onto a device and run. The device is controlled using
the included RPCs through the python test runner.

-   [Prerequisites](#prerequisites)
-   [Building The Unit Tests](#building-the-unit-tests)
-   [Running The Unit Tests](#running-the-unit-tests)

## Prerequisites

-   Download the
    [Simplicity Commander](https://www.silabs.com/mcu/programming-options)
    command line tool, and ensure that `commander` is in your shell search path.
    (For Mac OS X, `commander` is located inside
    `Commander.app/Contents/MacOS/`.)

-   Download and install a suitable ARM gcc tool chain (For most Host, the
    bootstrap already installs the toolchain):
    [GNU Arm Embedded Toolchain 12.2 Rel1](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)

-   Install some additional tools:

    For Linux: `sudo apt-get install git libwebkitgtk-1.0-0 ninja-build`

    For Mac OS: `brew install ninja`

-   Supported hardware:

    MG24 boards :

    -   BRD2601B / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@10dBm
    -   BRD2703A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@10dBm
    -   BRD4186A / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@10dBm
    -   BRD4186C / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@10dBm
    -   BRD4187A / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@20dBm
    -   BRD4187C / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@20dBm

## Building The Unit Tests

The unit tests can be built using gn and ninja:

    source scripts/activate.sh
    cd src/test_driver/efr32
    export SILABS_BOARD=BRD2703A
    gn gen out/debug
    ninja -C out/debug

## Building and Installing the Runner

The python wheels for the runner can be built and installed like this:

    ninja -C out/debug runner
    pip3 install out/debug/chip_pw_test_runner_wheels/*.whl --force-reinstall

## Running The Unit Tests

The unit tests can be run using the test runner python script:

    python -m py.pw_test_runner.pw_test_runner -d /dev/ttyACM1 -f out/debug/tests -o out.log
