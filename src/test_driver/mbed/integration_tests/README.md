![ARM Mbed-OS logo](https://raw.githubusercontent.com/ARMmbed/mbed-os/master/logo.png)

<h1> Matter Arm Mbed OS Integration Tests </h1>

This page describes the Matter project's functional testing approach for Mbed
applications. It allows you to better understand the use of tools and frameworks
for target validation.

<hr>

-   [Overview](#overview)
-   [Setup](#setup)
    -   [Environment setup](#environment-setup)
    -   [Tools installation](#tools-installation)
    -   [Setup WiFi access point](#setup-wifi-access-point)
    -   [Mount the device](#mount-the-device)
-   [Prepare device under test](#prepare-device-under-test)
-   [Tests configuration](#tests-configuration)
-   [Test run](#test-run)
-   [Tests results](#tests-results)

<hr>

# Overview

According to the Matter project design idea:

_"The ability to run tests on actual and emulated hardware is paramount in
embedded projects. CHIP is no exception. We want on-device testing to be a first
class goal of CHIP architecture. On-device testing requirements apply both to
Continuous Integration testing for main CHIP software stack development and to
eventual CHIP product certification."_

However, functional testing requires a host machine to run and control the
testing program. Connection and data exchange between the host machine and the
device under test is necessary. Various communication interfaces and protocols
are used for this purpose (i.e serial port, network communication via Ethernet,
or WiFi).

The common functional test scenario involves the following steps:

-   Get binaries image of testing application (build directly or download CI
    artifacts)
-   Flash binary image to the device under test
-   Configuration functional tests
-   Run testing
-   Collecting and processing the results

Matter Arm Mbed OS Integration Tests happens at system level on real hardware.
The host machine running the tests should be connected to the board under test
via serial port. The tests send a succession of commands to the connected boards
or trigger external tools action that changes the device state and asserts the
results reported by the hardware.

For example, here is a test validating that WiFi connection works:

**Preparation:** The device is connected to the host via serial port and
executes the shell example application.

**The test:**

1. Checking if the device is connected
1. Sending the command connecting with network credentials
1. Check device response
1. Check the device IP address

The Mbed integration tests are coded in Python and use the pyTest framework.
This also allows for easy integration with external CHIP tools such as
device-controller.

Python CHIP Controller and RPC console are required for the correct run
integration tests.

# Setup

## Environment setup

The first step check out the Matter repository and sync submodules using the
following command:

    $ git submodule update --init

Building the example application requires the use of **ARM Mbed-OS** sources and
the **arm-none-gnu-eabi** toolchain.

The Cypress OpenOCD package is required for flashing purpose. Install the
Cypress OpenOCD and set env var `OPENOCD_PATH` before calling the flashing
script.

```
cd ~
wget https://github.com/Infineon/openocd/releases/download/release-v4.3.0/openocd-4.3.0.1746-linux.tar.gz
tar xzvf openocd-4.3.0.1746-linux.tar.gz
export OPENOCD_PATH=$HOME/openocd
```

Some additional packages may be needed, depending on selected build target and
its requirements.

> **The VSCode devcontainer has these components pre-installed. Using the VSCode
> devcontainer is the recommended way to interact with Arm Mbed-OS port of the
> Matter Project.**
>
> **Please read this [README.md](../../../../docs/VSCODE_DEVELOPMENT.md) for
> more information about using VSCode in container.**

To initialize the development environment, download all registered sub-modules
and activate the environment:

```
$ source ./scripts/bootstrap.sh
$ source ./scripts/activate.sh
```

If packages are already installed then you just need to activate the development
environment:

```
$ source ./scripts/activate.sh
```

## Tools installation

Python CHIP Controller and RPC console are required for the correct run
integration tests. Build and install them inside the development environment.

For building and installing Python CHIP Controller please visit
[Python CHIP Controller](../../../../docs/guides/python_chip_controller_building.md)

For building and installing RPC console please visit
[CHIP RPC CONSOLE](../../../../examples/common/pigweed/rpc_console/README.md)

## Setup WiFi access point

The way of running the WiFi access point depends on the platform on which the
tests are run. Choose the best one. Just remember the network credentials ssid
and password.

## Mount the device

There is a special script for the easy Mbed board mounting. It adds a static
entry in fstab with a new connected device.

Then you can just need to call `sudo mount -a` to mount the device.

To add the new Mbed device you should:

1. Run script `bash src/scripts/tests/mbed/mount_new_mbed_device.sh`
2. Plug the device
3. Run `sudo mount -a`

That's all. The new device is ready for testing.

# Prepare device under test

Preparation of the device for testing should consist of two steps:

-   prepare binary image - building an right application from sources
-   flash binary image to device

For more information how to build and flash example application please visit
their documentation.

# Tests configuration

Mbed integration tests can be configured by PyTest command line arguments. Every
test call may contain a specific test configuration. The list of supported
parameters:

**[Common]**

-   _platforms_ - list of platforms that can be used to run the tests. Platforms
    are separated by a comma
-   _binaries_ - platform and associated binary in the form platform:binary.
    Multiple values are separated by a comma
-   _serial_inter_byte_delay_ - time in second between two bytes sent on the
    serial line (accepts floats), default=None
-   _serial_baudrate_ - baudrate of the serial port used, default=115200

**[Test specific]**

-   _network_ - WiFi network credentials to which we want to connect device.
    Format network_ssid:network_password

# Test run

To run Mbed integration tests execute the pytest command with the arguments
mentioned above. For example:

`pytest --platforms=CY8CPROTO_062_4343W --network=$AP_SSID:$AP_PASSWORD ... src/test_driver/mbed/integration_tests/{APP_NAME}/test_app.py`

The Mbed integration testes cases are divided into separate directory depends on
the testing Matter application:

-   shell - testing shell commands and check base device functionalities
-   lock-app - testing WiFi provisioning and execute ZCL command to control
    lock, use RPC client to run base device functionalities, control lock and
    trigger some button actions
-   lighting-app - testing WiFi provisioning and execute ZCL command to control
    light, use RPC client to run base device functionalities, control light and
    trigger some button actions
-   pigweed-app - use RPC client to send echo message and receive the response
-   unit-tests - check unit-tests result

For more details on how to run tests using PyTest see:
[PyTest doc](https://docs.pytest.org/en/6.2.x/usage.html)

Pytest markers have been added to run a specific set of tests:

-   smoketest - check base communication and correct launch of the application

# Tests results

Adding `-rAv` arguments to Pytest cause that short tests summary is on the
output.

For example:

```
pytest -rAv simple_test.py
```

Output:

```
=================================================================================================================== short test summary info ====================================================================================================================
PASSED CHIP/src/test_driver/mbed/integration_tests/shell/test_app.py::test_smoke_test
PASSED CHIP/src/test_driver/mbed/integration_tests/shell/test_app.py::test_help_check
PASSED CHIP/src/test_driver/mbed/integration_tests/shell/test_app.py::test_log_check
PASSED CHIP/src/test_driver/mbed/integration_tests/shell/test_app.py::test_rand_check
PASSED CHIP/src/test_driver/mbed/integration_tests/shell/test_app.py::test_base64_encode_decode
FAILED CHIP/src/test_driver/mbed/integration_tests/shell/test_app.py::test_wifi_mode - AssertionError: assert 'true' == 'false'
===================================================================================================== 1 failed, 4 passed, 7 warnings in 20.71s (0:03:29) =====================================================================================================
```

There is also an option to save test results to HTML file. Adding
`--html=<file_name.html>` arguments to Pytest cause that all test results are
saved to HTML file and can be open in your browser.

For example:

```
pytest --html=result.html simple_test.py
```
