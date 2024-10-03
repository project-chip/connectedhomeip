# Python framework tests

The python test framework is built on top of the ChipDeviceCtrl.py python
controller API and the Mobly test framework. Python tests are interaction tests,
and can be used for certification testing, and / or integration testing in the
CI.

Python tests located in src/python_testing

## Resources for getting started

-   [src/python_testing/hello_test.py](https://github.com/project-chip/connectedhomeip/blob/master/src/python_testing/hello_test.py) -
    sample test showing test setup and test harness integration
-   [https://github.com/google/mobly/blob/master/docs/tutorial.md](https://github.com/google/mobly/blob/master/docs/tutorial.md)
-   [ChipDeviceCtrl.py](https://github.com/project-chip/connectedhomeip/blob/master/src/controller/python/chip/ChipDeviceCtrl.py) -
    Controller implementation - [API documentation](./ChipDeviceCtrlAPI.md)
-   [scripts/tests/run_python_test.py](https://github.com/project-chip/connectedhomeip/blob/master/scripts/tests/run_python_test.py)
    to easily set up app and script for testing - used in CI

## Writing Python tests

-   Defining arguments in the test script
    -   In order to streamline the configuration and execution of tests, it is
        essential to define arguments at the top of the test script. This
        section should include various parameters and their respective values,
        which will guide the test runner on how to execute the tests.
-   All test classes inherit from `MatterBaseTest` in
    [matter_testing_support.py](https://github.com/project-chip/connectedhomeip/blob/master/src/python_testing/matter_testing_support.py)
    -   Support for commissioning using the python controller
    -   Default controller (`self.default_controller`) of type `ChipDeviceCtrl`
    -   `MatterBaseTest` inherits from the Mobly BaseTestClass
-   Test method(s) (start with test\_) and are all run automatically
    -   To run in the test harness, the test method name must be
        `test_TC_PICSCODE_#_#`
        -   More information about integration with the test harness can be
            found in [Test Harness helpers](#test-harness-helpers) section
    -   Any tests that use async method (read / write / commands) should be
        decorated with the @async_test_body decorator
-   Use `ChipDeviceCtrl` to interact with the DUT
    -   Controller API is in `ChipDeviceCtrl.py` (see API doc in file)
    -   Some support methods in `matter_testing_support.py`
-   Use Mobly assertions for failing tests
-   `self.step()` along with a `steps_*` method to mark test plan steps for cert
    tests

### A simple test

```
# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factoryreset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

class TC_MYTEST_1_1(MatterBaseTest):

    @async_test_body
    async def test_TC_MYTEST_1_1(self):

        vendor_name = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller, <span style="color:#38761D"># defaults to
self.default_controlller</span>
            node_id = self.dut_node_id, <span style="color:#38761D"># defaults to
self.dut_node_id</span>
            cluster=Clusters.BasicInformation,
            attribute=Clusters.BasicInformation.Attributes.VendorName,
            endpoint = 0, <span style="color:#38761D">#defaults to 0</span>
        )
        asserts.assert_equal(vendor_name, “Test vendor name”, “Unexpected vendor name”)

if __name__ == "__main__":
default_matter_test_main()
```

---

In this test, `asserts.assert_equal` is used to fail the test on equality
assertion failure (throws an exception).

Because the test requires the use of the async method
`read_single_attribute_check_success`, the test is decorated with the
`@async_test_body` decorator

The `default_matter_test_main()` function is used to run the test on the command
line. These two lines should appear verbatim at the bottom of every python test
file.

The structured comments above the class definition are used to set up the CI for
the tests. Please see [Running tests in CI](#running-tests-in-ci).

## Cluster Codegen

-   [Objects.py](https://github.com/project-chip/connectedhomeip/blob/master/src/controller/python/chip/clusters/Objects.py)
    for codegen,
-   [ClusterObjects.py](https://github.com/project-chip/connectedhomeip/blob/master/src/controller/python/chip/clusters/ClusterObjects.py)
    for classes

Common import used in test files: `import chip.clusters as Clusters`

Each cluster is defined in the `Clusters.<ClusterName>` namespace and contains
always:

-   id
-   descriptor

Each `Clusters.<ClusterName>` will include the appropriate sub-classes (if
defined for the cluster):

-   `Enums`
-   `Bitmaps`
-   `Structs`
-   `Attributes`
-   `Commands`
-   `Events`

### Attributes

Attributes derive from ClusterAttributeDescriptor

Each `Clusters.<ClusterName>.Attributes.<AttributeName>` class has:

-   cluster_id
-   attribute_id
-   attribute_type
-   value

Example:

-   class - `Clusters.OnOff.Attributes.OnTime`
    -   Used for Read commands
-   instance - `Clusters.OnOff.Attributes.OnTime(5)`
    -   Sets the value to `5`
    -   Pass the instance to Write method to write the value

### Commands

Commands derive from `ClusterCommand`.

Each `Clusters.<ClusterName>.Commands.<CommandName>` class has:

-   `cluster_id`
-   `command_id`
-   `is_client`
-   `response_type` (None for status response)
-   `descriptor`
-   data members (if required)

Example:

-   `Clusters.OnOff.Commands.OnWithTimedOff(onOffControl=0, onTime=5, offWaitTime=8)`
-   `Clusters.OnOff.Commands.OnWithTimedOff()`
    -   Command with no fields

### Events

Events derive from `ClusterEvent`.

Each `Clusters.<ClusterName>.Events.<EventName>` class has:

-   `cluster_id`
-   `event_id`
-   `descriptor`
-   Other data members if required

Example:

-   Clusters.AccessControl.Events.AccessControlEntryChanged.adminNodeID

### Enums

Enums derive from `MatterIntEnum`.

Each `Clusters.<ClusterName>.Enum.<EnumName>` has

-   `k<value>` constants
-   `kUnknownEnumValue` (used for testing, do not transmit)

Example:

-   `Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister`

### Bitmaps

Bitmaps derive from IntFlag

Each `Clusters.<ClusterName>.Bitmaps.<BitmapName>` has: - k<value>

Special class:

-   class `Feature(IntFlag)` - contains the feature map bitmaps

Example:

-   `Clusters.LaundryWasherControls.Bitmaps.Feature.kSpin`

### Structs

Structs derive from `ClusterObject`.

Each `Clusters.<ClusterName>.Structs.<StructName>` has:

-   A "descriptor"
-   Data members

Example:

```
Clusters.BasicInformation.Structs.ProductAppearanceStruct(
   finish=Clusters.BasicInformation.Enums.ProductFinishEnum.kFabric,
   primaryColor=Clusters.BasicInformation.Enums.ColorEnum.kBlack)
```

## Accessing Clusters and Cluster Elements by ID

[ClusterObjects.py](https://github.com/project-chip/connectedhomeip/blob/master/src/controller/python/chip/clusters/ClusterObjects.py)
has a set of objects that map ID to the code generated object.

`chip.clusters.ClusterObjects.ALL_CLUSTERS`

-   `dict[int, Cluster]` - maps cluster ID to Cluster class
    -   `cluster = chip.clusters.ClusterObjects.ALL_CLUSTERS[cluster_id]`

`chip.clusters.ClusterObjects.ALL_ATTRIBUTES`

-   `dict[int, dict[int, ClusterAttributeDescriptor]]` - maps cluster ID to a
    dict of attribute ID to attribute class
    -   `attr = chip.clusters.ClusterObjects.ALL_ATTRIBUTES[cluster_id][attribute_id]`

`chip.clusters.ClusterObjects.ALL_ACCEPTED_COMMANDS/ALL_GENERATED_COMMANDS`

-   dict[int, dict[int, ClusterCommand]]
-   cmd = chip.clusters.ClusterObjects.ALL_ACCEPTED_COMMANDS[cluster_id][cmd_id]

## ChipDeviceCtrl API

The `ChipDeviceCtrl` API is implemented in
[ChipDeviceCtrl.py](https://github.com/project-chip/connectedhomeip/blob/master/src/controller/python/chip/ChipDeviceCtrl.py).

The `ChipDeviceCtrl` implements a python-based controller that can be used to
commission and control devices. The API is documented here in the
[ChipDeviceCtrl API documentation](./ChipDeviceCtrlAPI.md)

The API doc gives full descriptions of the APIs being used. The most commonly
used methods are linked below.

### [Read](./ChipDeviceCtrlAPI.md#read)

-   Read both attributes and events
-   Can handle wildcard or concrete path

### [ReadAttribute](./ChipDeviceCtrlAPI.md#readattribute)

-   Convenience wrapper for Read for attributes

Examples: Wildcard read (all clusters, all endpoints):

`await dev_ctrl.ReadAttribute(node_id, [()])`

Wildcard read (single endpoint 0)

`await dev_ctrl.ReadAttribute(node_id, [(0)])`

Wildcard read (single cluster from single endpoint 0)

`await dev_ctrl.ReadAttribute(node_id, [(1, Clusters.OnOff)])`

Single attribute

`await dev_ctrl.ReadAttribute(node_id, [(1, Clusters.OnOff.Attributes.OnTime)])`

Multi-path

`await dev_ctrl.ReadAttribute(node_id, [(1, Clusters.OnOff.Attributes.OnTime),(1, Clusters.OnOff.Attributes.OnOff)])`

### [ReadEvent](./ChipDeviceCtrlAPI.md#readevent)

-   Convenience wrapper for `Read`
-   Similar to `ReadAttribute`, but the tuple includes urgency as the last
    argument

Example:

```
urgent = 1

await dev_ctrl ReadEvent(node_id, [(1,
Clusters.TimeSynchronization.Events.MissingTrustedTimeSource, urgent)])
```

### Subscriptions

Subscriptions are handled in the `Read` / `ReadAttribute` / `ReadEvent` APIs. To
initiate a subscription, set the `reportInterval` tuple argument to set the
floor and ceiling. The `keepSubscriptions` and `autoResubscribe` arguments also
apply to subscriptions.

Subscription return `ClusterAttribute.SubscriptionTransaction`. This can be used
to set callbacks. The object is returned after the priming data read is
complete, and the values there are used to populate the cache. The attribute
callbacks are called on update.

-   `SetAttributeUpdateCallback`
    -   Callable[[TypedAttributePath, SubscriptionTransaction], None]
-   `SetEventUpdateCallback`
    -   Callable[[EventReadResult, SubscriptionTransaction], None]
-   await changes in the main loop using a trigger mechanism from the callback.

Example for setting callbacks:

```
q = queue.Queue()
cb = SimpleEventCallback("cb", cluster_id, event_id, q)

urgent = 1
subscription = await dev_ctrl.ReadEvent(nodeid=1, events=[(1, event, urgent)], reportInterval=[1, 3])
subscription.SetEventUpdateCallback(callback=cb)

try:
    q.get(block=True, timeout=timeout)
except queue.Empty:
    asserts.assert_fail(“Timeout on event”)
```

### [WriteAttribute](./ChipDeviceCtrlAPI.md#writeattribute)

Handles concrete paths only (per spec), can handle lists. Returns list of
PyChipError

-   Instantiate the `ClusterAttributeDescriptor` class with the value you want
    to send, tuple is (endpoint, attribute)
    -   use timedRequestTimeoutMs for timed request actions

Example:

```
res = await devCtrl.WriteAttribute(nodeid=0, attributes=[(0,Clusters.BasicInformation.Attributes.NodeLabel("Test"))])
asserts.assert_equal(ret[0].status, Status.Success, “write failed”)
```

### [SendCommand](./ChipDeviceCtrlAPI.md#sendcommand)

-   Instantiate the command object with the values you need to populate
-   If there is a non-status return, it’s returned from the command
-   If there is a pure status return it will return nothing
-   Raises InteractionModelError on failure

Example:

```
pai = await dev_ctrl.SendCommand(nodeid, 0, Clusters.OperationalCredentials.Commands.CertificateChainRequest(2))
```

## MatterBaseTest helpers

-   Because we tend to do a lot of single read / single commands in tests, we
    added a couple of helpers in MatterBaseTest that use some of the default
    values
    -   `read_single_attribute_check_success()`
    -   `read_single_attribute_expect_error()`
    -   `send_single_cmd()`
-   `step()` method to mark step progress for the test harness
-   `skip()` / `skip_step()` / `skip_remaining_steps()` methods for test harness
    integration
-   `check_pics()` / `pics_guard()` to handle pics

## Mobly helpers

The test system is based on Mobly, and the
[matter_testing_support.py](https://github.com/project-chip/connectedhomeip/blob/master/src/python_testing/matter_testing_support.py)
class provides some helpers for Mobly integration.

-   `default_matter_test_main`
    -   Sets up commissioning and finds all tests, parses command-line arguments

use as:

```
if __name__ == "__main__":
    default_matter_test_main()
```

-   Mobly will run all methods starting with `test_` prefix by default
    -   use `--tests` command line argument to specify exact name,s
-   Setup and teardown methods
    -   `setup_class` / `teardown_class`
    -   `setup_test` / `teardown_test`
    -   Don’t forget to call the `super()` if you override these

## Test harness helpers

The python testing system also includes several methods for integrations with
the test harness. To integrate with the test harness, you can define the
following methods on your class to allow the test harness UI to properly work
through your tests.

All of these methods are demonstrated in the
[hello_example.py](https://github.com/project-chip/connectedhomeip/blob/master/src/python_testing/hello_test.py)
reference.

-   Steps enumeration:
    -   Define a method called `steps_<YourTestMethodName>` to allow the test
        harness to display the steps
    -   Use the `self.step(<stepnum>)` method to walk through the steps
-   Test description:
    -   Define a method called `desc_<YourTestMethodName>` to send back a string
        with the test description
-   Top-level PICS:
    -   To guard your test on a top level PICS, define a method called
        `pics_<YourTestMethodName>` to send back a list of PICS. If this method
        is omitted, the test will be run for every endpoint on every device.
-   Overriding the default timeout:
    -   If the test is exceptionally long running, define a property getter
        method `default_timeout` to adjust the timeout. The default is 90
        seconds.

Deferred failures: For some tests, it makes sense to perform the entire test
before failing and collect all the errors so the developers can address all the
failures without needing to re-run the test multiple times. For example, tests
that look at every attribute on the cluster and perform independent operations
on them etc.

For such tests, use the ProblemNotice format and the convenience methods:

-   `self.record_error`
-   `self.record_warning`

These methods keep track of the problems, and will print them at the end of the
test. The test will not be failed until an assert is called.

A good example of this type of test can be found in the device basic composition
tests, where all the test steps are independent and performed on a single read.
See
[Device Basic Composition tests](https://github.com/project-chip/connectedhomeip/blob/master/src/python_testing/TC_DeviceBasicComposition.py)

## Command line arguments

-   Use `--help` to get a full list
-   `--storage-path`
    -   Used to set a local storage file path for persisted data to avoid
        clashing files. It is suggested to always provide this argument. Default
        value is `admin_storage.json` in current directory.
-   `--commissioning-method`
    -   Need to re-commission to python controller as chip-tool and python
        commissioner do not share a credentials
-   `--discriminator`, `--passcode`, `--qr-code`, `--manual-code`
-   `--tests` to select tests
-   `--PICS`
-   `--int-arg`, `--bool-arg`, `--float-arg`, `--string-arg`, `--json-arg`,
    `--hex-arg`
    -   Specify as key:value ex --bool-arg pixit_name:False
    -   Used for custom arguments to scripts (PIXITs)

## PICS and PIXITS

-   PICS
    -   use --PICS on the command line to specify the PICS file
    -   use check_pics to gate steps in a file
-   have_whatever = check_pics(“PICS.S.WHATEVER”)
-   PIXITs
    -   use --int-arg, --bool-arg etc on the command line to specify PIXITs
    -   Warn users if they don’t set required values, add instructions in the
        comments
-   pixit_value = self.user_params.get("pixit_name", default)

## Support functionality

To create a controller on a new fabric:

```
new_CA = self.certificate_authority_manager.NewCertificateAuthority()

new_fabric_admin = new_certificate_authority.NewFabricAdmin(vendorId=0xFFF1,
    fabricId=self.matter_test_config.fabric_id + 1)

TH2 = new_fabric_admin.NewController(nodeId=112233)
```

Open a commissioning window (ECW):

```
params = self.OpenCommissioningWindow(dev_ctrl=self.default_controller, node_id=self.dut_node_id)
```

To create a new controller on the SAME fabric, allocate a new controller from
the fabric admin.

Fabric admin for default controller:

```
  fa = self.certificate_authority_manager.activeCaList[0].adminList[0]
  second_ctrl = fa.new_fabric_admin.NewController(nodeId=node_id)
```

## Automating manual steps

Some test plans have manual steps that require the tester to manually change the
state of the DUT. To run these tests in a CI environment, specific example apps
can be built such that these manual steps can be achieved by Matter or
named-pipe commands.

In the case that all the manual steps in a test script can be achieved just
using Matter commands, you can check if the `PICS_SDK_CI_ONLY` PICS is set to
decide if the test script should send the required Matter commands to perform
the manual step.

```python
self.is_ci = self.check_pics("PICS_SDK_CI_ONLY")
```

In the case that a test script requires the use of named-pipe commands to
achieve the manual steps, you can use the `write_to_app_pipe(command)` to send
these commands. This command requires the test class to define a `self.app_pipe`
string value with the name of the pipe. This depends on how the app is set up.

If the name of the pipe is dynamic and based on the app's PID, the following
snippet can be added to the start of tests that use the `write_to_app_pipe`
method.

```python
app_pid = self.matter_test_config.app_pid
if app_pid != 0:
	self.is_ci = true
	self.app_pipe = "/tmp/chip_<app name>_fifo_" + str(app_pid)
```

This requires the test to be executed with the `--app-pid` flag set if the
manual steps should be executed by the script. This flag sets the process ID of
the DUT's matter application.

### Running on a separate machines

If the DUT and test script are running on different machines, the
`write_to_app_pipe` method can send named-pipe commands to the DUT via ssh. This
requires two additional environment variables:

-   `LINUX_DUT_IP` sets the DUT's IP address
-   `LINUX_DUT_UNAME` sets the DUT's ssh username. If not set, this is assumed
    to be `root`.

The `write_to_app_pipe` also requires that ssh-keys are set up to access the DUT
from the machine running the test script without a password. You can follow
these steps to set this up:

1. If you do not have a key, create one using `ssh-keygen`.
2. Authorize this key on the remote host: run `ssh-copy-id user@ip` once, using
   your password.
3. From now on `ssh user@ip` will no longer ask for your password.

## Other support utilities

-   `basic_composition_support`
    -   wildcard read, whole device analysis
-   `CommissioningFlowBlocks`
    -   various commissioning support for core tests
-   `spec_parsing_support`
    -   parsing data model XML into python readable format

# Running tests locally

## Setup

The scripts require the python wheel to be compiled and installed before
running. To compile and install the wheel, do the following:

First activate the matter environment using either

```
. ./scripts/bootstrap.sh
```

or

```
. ./scripts/activate.sh
```

bootstrap.sh should be used for for the first setup, activate.sh may be used for
subsequent setups as it is faster.

Next build the python wheels and create / activate a venv (called `pyenv` here,
but any name may be used)

```
./scripts/build_python.sh -i pyenv
source pyenv/bin/activate
```

## Running tests

-   Note that devices must be commissioned by the python test harness to run
    tests. chip-tool and the python test harness DO NOT share a fabric.

Once the wheel is installed, you can run the python script as a normal python
file for local testing against an already-running DUT. This can be an example
app on the host computer (running in a different terminal), or a separate device
that will be commissioned either over BLE or WiFi.

For example, to run the TC-ACE-1.2 tests against an un-commissioned DUT:

```
python3 src/python_testing/TC_ACE_1_2.py --commissioning-method on-network --qr-code MT:-24J0AFN00KA0648G00
```

Some tests require additional arguments (ex. PIXITs or configuration variables
for the CI). These arguments can be passed as sets of key/value pairs using the
`--<type>-arg:<value>` command line arguments. For example:

```
--int-arg PIXIT.ACE.APPENDPOINT:1 --int-arg PIXIT.ACE.APPDEVTYPEID:0x0100 --string-arg PIXIT.ACE.APPCLUSTER:OnOff --string-arg PIXIT.ACE.APPATTRIBUTE:OnOff
```

## Local host app testing

`./scripts/tests/run_python_test.py` is a convenient script that starts an
example DUT on the host and includes factory reset support

`./scripts/tests/run_python_test.py --factoryreset --app <your_app> --app-args "whatever" --script <your_script> --script-args "whatever"`

# Running tests in CI

-   Add test to the `repl_tests_linux` section of `.github/workflows/tests.yaml`
-   Don’t forget to set the PICS file to the ci-pics-values
-   If there are steps in your test that will fail on CI (e.g. test vendor
    checks), gate them on the PICS_SDK_CI_ONLY
    -   ```python
        if not self.is_pics_sdk_ci_only:
            ...  # Step that will fail on CI
        ```

The CI test runner uses a structured environment setup that can be declared
using structured comments at the top of the test file. To use this structured
format, use the `--load-from-env` flag with the `run_python_tests.py` runner.

Ex:
`scripts/run_in_python_env.sh out/venv './scripts/tests/run_python_test.py --load-from-env /tmp/test_env.yaml --script src/python_testing/TC_ICDM_2_1.py'`

## Running ALL or a subset of tests when changing application code

`scripts/tests/local.py` is a wrapper that is able to build and run tests in a
single command.

Example to compile all prerequisites and then running all python tests:

```shell
./scripts/tests/local.py build         # will compile python in out/pyenv and ALL application prerequisites
./scripts/tests/local.py python-tests  # Runs all python tests that are runnable in CI
```

## Defining the CI test arguments

Arguments required to run a test can be defined in the comment block at the top
of the test script. The section with the arguments should be placed between the
`# === BEGIN CI TEST ARGUMENTS ===` and `# === END CI TEST ARGUMENTS ===`
markers. Arguments should be structured as a valid YAML dictionary with a root
key `test-runner-runs`, followed by the run identifier, and then the parameters
for that run, e.g.:

```python
# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${TYPE_OF_APP}
#     app-args: <app_arguments>
#     script-args: <script_arguments>
#     factoryreset: <true|false>
#     quiet: <true|false>
# === END CI TEST ARGUMENTS ===
```

### Description of Parameters

-   `app`: Indicates the application to be used in the test. Different app types
    as needed could be referenced from section [name: Generate an argument
    environment file ] of the file
    [.github/workflows/tests.yaml](https://github.com/project-chip/connectedhomeip/blob/master/.github/workflows/tests.yaml)

    -   Example: `${TYPE_OF_APP}`

-   `factoryreset`: Determines whether a factory reset should be performed
    before the test.

    -   Example: `true`

-   `quiet`: Sets the verbosity level of the test run. When set to True, the
    test run will be quieter.

    -   Example: `true`

-   `app-args`: Specifies the arguments to be passed to the application during
    the test.

    -   Example:
        `--discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json`

-   `app-ready-pattern`: Regular expression pattern to match against the output
    of the application to determine when the application is ready. If this
    parameter is specified, the test runner will not run the test script until
    the pattern is found.

    -   Example: `"Manual pairing code: \\[\\d+\\]"`

-   `script-args`: Specifies the arguments to be passed to the test script.

    -   Example:
        `--storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto`

This structured format ensures that all necessary configurations are clearly
defined and easily understood, allowing for consistent and reliable test
execution.
