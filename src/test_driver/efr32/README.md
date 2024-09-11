#CHIP EFR32 Test Driver

This builds and runs the unit tests on the efr32 device.

<hr>

-   [Introduction](#introduction)
-   [Building](#building)
-   [Running The Tests](#running-the-tests)

<hr>

<a name="introduction"></a>

## Introduction

This builds a set of test binaries which contain the unit tests and can be
flashed onto a device. The device is controlled using the included RPCs, through
the python test runner.

<a name="building"></a>

## Building

-   Download the
    [Simplicity Commander](https://www.silabs.com/mcu/programming-options)
    command line tool, and ensure that `commander` is your shell search path.
    (For Mac OS X, `commander` is located inside
    `Commander.app/Contents/MacOS/`.)

-   Download and install a suitable ARM gcc tool chain (For most Host, the
    bootstrap already installs the toolchain):
    [GNU Arm Embedded Toolchain 12.2 Rel1](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)

-   Install some additional tools(likely already present for CHIP developers):

#Linux `sudo apt-get install git libwebkitgtk-1.0-0 ninja-build`

#Mac OS X `brew install ninja`

-   Supported hardware:

    MG24 boards :

    -   BRD2601B / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@10dBm
    -   BRD2703A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@10dBm
    -   BRD4186A / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@10dBm
    -   BRD4186C / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@10dBm
    -   BRD4187A / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@20dBm
    -   BRD4187C / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@20dBm

OR use GN/Ninja directly

          ```
          cd ~/connectedhomeip/src/test_driver/efr32/
          git submodule update --init
          source third_party/connectedhomeip/scripts/activate.sh
          export SILABS_BOARD=BRD4187C
          gn gen out/debug
          ninja -C out/debug
          ```

-   To delete generated executable, libraries and object files use:

          ```
          cd ~/connectedhomeip/src/test_driver/efr32/
          rm -rf out/
          ```

<a name="running-the-tests"></a>

## Running The Tests

Build the runner using gn:

    ```
    cd <connectedhomeip>/src/test_driver/efr32
    gn gen out/debug
    ninja -C out/debug runner
    ```

Or build using build script from the root

    ```
    cd <connectedhomeip>
    ./scripts/build/build_examples.py --target linux-x64-pw-test-runner build
    ```

The runner will be installed into the venv and python wheels will be packaged in
the output folder for deploying.

Then the python wheels need to installed using pip3.

    ```
    pip3 install out/debug/chip_pw_test_runner_wheels/*.whl
    ```

Other python libraries may need to be installed such as

    ```
    pip3 install pyserial
    ```

-   To run all tests:

    ```
    python -m pw_test_runner.pw_test_runner -d /dev/ttyACM1 -f out/debug/matter-silabs-device_tests.s37 -o out.log
    ```
