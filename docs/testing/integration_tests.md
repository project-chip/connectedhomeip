# Integration and Certification Tests

Integration tests use a server and a controller or controllers to test the
behavior of a device. Certification tests are all integration tests. For
certified products, the device under test (DUT) is tested against one of the SDK
controller implementations (either chip-tool or the python-based controller,
depending on the test type). For software component certification, the software
component is tested against a sample device built from the SDK.

Certification tests require an accompanying certification test plan in order to
be used in the certification testing process. More information about test plans
can be found in the
[test plans repository](https://github.com/CHIP-Specifications/chip-test-plans/tree/master/docs).
Integration testing can also be used outside of the certification testing
program to test device behavior in the SDK. Certification tests are all run in
the [CI](./ci_testing).

There are two main integration test types:

-   [YAML](./yaml.md)
-   [Python framework](./python.md)

YAML is a human-readable serialization language that uses structured tags to
define test steps. Tests are defined in YAML, and parsed and run through a
runner that is backed by the chip-tool controller.

The Python framework tests are written in python and use the
[Mobly](https://github.com/google/mobly) test framework to execute tests.

## Which test framework to use

Both types of tests can be run through the Test Harness for certification
testing, locally for the purposes of development and in the CI for the SDK. The
appropriate test framework to use is whatever lets you automate your tests in a
way that is understandable, readable, and has the features you need

-   YAML
    -   pros: more readable, simpler to write, easy for ATLs to parse and
        understand
    -   cons: conditionals are harder (not all supported), no branch control,
        schema not well documented
-   python
    -   pros: full programming language, full control API with support for core
        (certs, commissioning, etc), less plumbing if you need to add features,
        can use python libraries
    -   cons: more complex, can be harder to read

## Running integration tests locally

When integration tests are run locally, the test runner (YAML or python) needs
to mock network connectivity between the controller and the device under test,
so that all tests can be run without actual hardware. In case of a simple test
case when a single device is tested with on-network commissioning, nothing
special is needed - the controller can connect to the device directly over the
local (loopback) network interface. However, for more complex test cases that
involve multiple devices or other than on-network commissioning (e.g. ble-wifi
or ble-thread), some additional setup is needed.

### Running tests in Linux network namespaces

The simplest way to mock more complex network topologies is to use Linux network
namespaces. Each device (controller or DUT) is run in its own network namespace,
which allows them to have their own network interfaces and corresponding IP
addresses.

For convenience, there is a script that can set up network namespaces and run a
test case in them:

```shell
# Build the chip-tool and chip-all-clusters-app if not done already
scripts/build/build_examples.py --target linux-x64-chip-tool --target linux-x64-all-clusters build
# Run the TestOperationalState test case in the Linux network namespaces
scripts/tests/run_test_suite.py --runner chip_tool_python \
    --chip-tool out/linux-x64-chip-tool/chip-tool \
    --target TestOperationalState \
    --log-level=debug \
    run
```

### Running tests with mocked BLE and Wi-Fi connectivity

For more complex commissioning flows that involve BLE and Wi-Fi, the test runner
needs to mock BLE and Wi-Fi connectivity as well. On Linux, BLE and Wi-Fi are
provided by the BlueZ stack and WPA supplicant, respectively. The SDK uses D-Bus
to interact with these services. This allows the test runner to mock BLE and
Wi-Fi connectivity by simply mocking used D-Bus APIs of these services.

Additionally, the Matter specification forbids device to advertise on more than
one network type at a time, so in case of ble-wifi commissioning, the DUT shall
not be accessible over Wi-Fi until it is commissioned. This can be achieved by
using Linux network namespaces as described above, but instead of setting up
network interfaces before commissioning, the test runner assigns IP address to
the DUT's network interface only after mock Wi-Fi connection is associated.

See the diagram below for an overview of the setup:

```mermaid
flowchart TD

    subgraph Mocked BlueZ
        BA1[adapter1<br>00:00:00:11:11:11]
        BA2[adapter2<br>00:00:00:22:22:22]
    end

    subgraph Mocked WPA supplicant
        WL0[wlan0]
    end

    subgraph Test D-Bus system bus
        direction TB
        org.bluez.hci0[org.bluez.Adapter<br>hci0]
        org.bluez.hci1[org.bluez.Adapter<br>hci1]
        fi.w1[fi.w1.wpa_supplicant1.Interface<br>wlan0]
    end

    BA1 --- org.bluez.hci0
    BA2 --- org.bluez.hci1
    WL0 --- fi.w1

    subgraph ETH["TOOL network namespace"]
        CONTROLLER[chip-tool]
        ETH0[eth0<br>fd00:0:1:1::2]
    end

    subgraph WLAN["DUT network namespace"]
        DUT[chip-all-clusters-app]
        WLAN0[wlan0<br>not assigned]
    end

    fi.w1 --- DUT
    org.bluez.hci0 --- DUT
    org.bluez.hci1 --- CONTROLLER

```

In order to run tests with mocked BLE and Wi-Fi connectivity and Linux network
namespaces use the `--ble-wifi` option to the `run` command of the
`scripts/tests/run_test_suite.py` script:

```shell
# Run the TestOperationalState test case with ble-wifi commissioning
scripts/tests/run_test_suite.py --runner chip_tool_python \
    --chip-tool out/linux-x64-chip-tool/chip-tool \
    --target TestOperationalState \
    --log-level=debug \
    run --ble-wifi
```
