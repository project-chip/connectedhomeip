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

-   All test classes inherit from MatterBaseTest in
    [matter_testing_support.py](https://github.com/project-chip/connectedhomeip/blob/master/src/python_testing/matter_testing_support.py)
    -   support for commissioning using the python controller
    -   default controller (self.default_controller) of type ChipDeviceCtrl
    -   MatterBaseTest inherits from the Mobly BaseTestClass
-   Test function(s) (start with test\_) and are all run automatically
    -   To run in the test harness, the test name must be test_TC_PICSCODE\_#\_#
        -   more information about integration with the test harness can be
            found in [Test Harness helpers](#test-harness-helpers) section
    -   any tests that use async function (read / write / commands) should be
        decorated with the @async_test_body decorator
-   Use ChipDeviceCtrl to interact with the DUT
    -   Controller API is in ChipDeviceCtrl.py (see API doc in file)
    -   some support functions in matter_testing_support.py
-   Use Mobly assertions for failing tests
-   self.step() along with a steps\_ function to mark test plan steps for cert
    tests
-

### A simple test

```
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

In this test, asserts.assert_equal is used to fail the test on condition failure
(throws an exception).

Because the test requires the use of the async function
read_single_attribute_check_success, the test is decorated with the
`@async_test_body` decorator

The default_matter_test_main() function is used to run the test on the command
line. These two lines should appear verbatim at the bottom of every python test
file.

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

-   Enums
-   Bitmaps
-   Structs
-   Attributes
-   Commands
-   Events

### Attributes

Attributes derive from ClusterAttributeDescriptor

Each `Clusters.<ClusterName>.Attributes.<AttributeName>` class has:

-   cluster_id
-   attribute_id
-   attribute_type
-   value

Example:

-   class - Clusters.OnOff.Attributes.OnTime
    -   used for Read commands
-   instance - Clusters.OnOff.Attributes.OnTime(5)
    -   sets the value to 5
    -   pass the instance to write commands to write the value

### Commands

Commands derive from ClusterCommand

Each `Clusters.<ClusterName>.Commands.<CommandName>` class has:

-   cluster_id
-   command_id
-   is_client
-   response_type (None for status response)
-   descriptor
-   data members (if required)

Example:

-   Clusters.OnOff.Commands.OnWithTimedOff(onOffControl=0, onTime=5,
    offWaitTime=8)
-   Clusters.OnOff.Commands.OnWithTimedOff()
    -   command with no fields

### Events

Events derive from ClusterEvent

Each `Clusters.<ClusterName>.Events.<EventName>` class has:

-   cluster_id
-   event_id
-   descriptor
-   data members if required

Example:

-   Clusters.AccessControl.Events.AccessControlEntryChanged.adminNodeID

### Enums

Enums derive from MatterIntEnum

Each `Clusters.<ClusterName>.Enum.<EnumName>` has

-   k<value>
-   kUnknownEnumValue (used for testing, do not transmit)

Example:

-   Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister

### Bitmaps

Bitmaps derive from IntFlag

Each `Clusters.<ClusterName>.Bitmaps.<BitmapName>` has: - k<value>

Special class:

-   class Feature(IntFlag) - contains the feature map bitmaps

Example:

-   Clusters.LaundryWasherControls.Bitmaps.Feature.kSpin

### Structs

Structs derive from ClusterObject

Each `Clusters.<ClusterName>.Structs.<StructName>` has:

-   descriptor
-   data members

Example

-   Clusters.BasicInformation.Structs.ProductAppearanceStruct(
-   finish=Clusters.BasicInformation.Enums.ProductFinishEnum.kFabric,
-   primaryColor=Clusters.BasicInformation.Enums.ColorEnum.kBlack)

## Accessing Clusters and Cluster Elements by ID

[ClusterObjects.py](https://github.com/project-chip/connectedhomeip/blob/master/src/controller/python/chip/clusters/ClusterObjects.py)
has a set of objects that map ID to the code generated object.

`chip.clusters.ClusterObjects.ALL_CLUSTERS`

-   dict[int, Cluster] - maps cluster ID to Cluster class
-   cluster = chip.clusters.ClusterObjects.ALL_CLUSTERS[cluster_id]

`chip.clusters.ClusterObjects.ALL_ATTRIBUTES`

-   dict[int, dict[int, ClusterAttributeDescriptor]] - maps cluster ID to a dict
    of attribute ID to attribute class
-   attr = chip.clusters.ClusterObjects.ALL_ATTRIBUTES[cluster_id][attribute_id]

`chip.clusters.ClusterObjects.ALL_ACCEPTED_COMMANDS/ALL_GENERATED_COMMANDS`

-   dict[int, dict[int, ClusterCommand]]
-   cmd = chip.clusters.ClusterObjects.ALL_ACCEPTED_COMMANDS[cluster_id][cmd_id]

## ChipDeviceCtrl API

The ChipDeviceCtrl API is implemented in
[ChipDeviceCtrl.py](https://github.com/project-chip/connectedhomeip/blob/master/src/controller/python/chip/ChipDeviceCtrl.py).

The ChipDeviceCtrl implements a python-based controller that can be used to
commission and control devices. The API is documented here in the
[ChipDeviceCtrl API documentation](./ChipDeviceCtrlAPI.md)

The API doc gives full descriptions of the APIs being used. The most commonly
used functions are linked below

### [Read](./ChipDeviceCtrlAPI.md#read)

-   Read both attributes and events
-   Can handle wildcard or concrete path

### [ReadAttribute](./ChipDeviceCtrlAPI.md#readattribute)

-   convenience wrapper for Read for attributes

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

-   convenience wrapper for Read
-   Similar to ReadAttribute, but the tuple includes urgency as the last number

Example:

```
urgent = 1

await dev_ctrl ReadEvent(node_id, [(1,
Clusters.TimeSynchronization.Events.MissingTrustedTimeSource, urgent)])
```

### Subscriptions

Subscriptions are handled in the Read / ReadAttribute / ReadEvent APIs. To
initiate a subscription, set the `reportInterval` tuple to set the floor and
ceiling. The `keepSubscriptions` and `autoResubscribe` parameters also apply to
subscriptions.

Subscription return `ClusterAttribute.SubscriptionTransaction`. This can be used
to set callbacks. The object is returned after the priming data read is
complete, and the values there are used to populate the cache. The attribute
callbacks are called on update.

-   SetAttributeUpdateCallback
    -   Callable[[TypedAttributePath, SubscriptionTransaction], None]
-   SetEventUpdateCallback
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

-   Instantiate the command with the values you need to populate
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
    -   read_single_attribute_check_success
    -   read_single_attribute_expect_error
    -   send_single_cmd
-   step() function to mark step progress for the test harness
-   skip / skip_step / skip_remaining_steps functions for test harness
    integration
-   check_pics / pics_guard to handle pics

## Mobly helpers

The test system is based on Mobly, and the
[matter_testing_support.py](https://github.com/project-chip/connectedhomeip/blob/master/src/python_testing/matter_testing_support.py)
class provides some helpers for Mobly integration

-   default_matter_test_main
    -   Sets up commissioning and finds all tests, parses command arguments

use as:

```
if __name__ == "__main__":
default_matter_test_main()
```

-   Mobly will run all functions starting with test\_ by default
    -   use --tests command line argument to specify
-   Setup / teardown functions
    -   setup_class / teardown_class
    -   setup_test / teardown_test
    -   Don’t forget to call the super() if you override these

## Test harness helpers

The python testing system also includes several functions for integrations with
the test harness. To integrate with the test harness, you can define the
following functions on your class to allow the test harness UI to properly work
through your tests.

All of these functions are demonstrated in the
[hello_example.py](https://github.com/project-chip/connectedhomeip/blob/master/src/python_testing/hello_test.py)
reference.

-   step enumeration
    -   define a function called `steps_YourFunctionName` to allow the test
        harness to display the steps
    -   use the self.step(`<stepnum>`) function to walk through the steps
-   test description
    -   define a function called `desc_YourFunctionName` to send back a string
        with the test description
-   top level PICS
    -   To guard your test on a top level PICS, define a function called
        `pics_YourFunctionName` to send back a list of pics. If this function is
        omitted, the test will be run for every endpoint on every device.
-   overriding the default timeout
    -   if the test is exceptionally long running, define a property function
        `default_timeout` to adjust the timeout. The default is 90 seconds

Deferred failures: For some tests, it makes sense to perform the entire test
before failing and collect all the errors so the developers can address all the
failures without needing to re-run the test multiple times. For example, tests
that look at every attribute on the cluster and perform independent operations
on them etc.

For such tests, use the ProblemNotice format and the convenience functions:

-   self.record_error
-   self.record_warning

These functions keep track of the problems, and will print them at the end of
the test. The test will not be failed until the assert is called.

A good example of this type of test can be found in the device basic composition
tests, where all the test steps are independent and performed on a single read.
See
[Device Basic Composition tests](https://github.com/project-chip/connectedhomeip/blob/master/src/python_testing/TC_DeviceBasicComposition.py)

## Command line arguments

-   Use help to get a full list
-   --commissioning-method
    -   need to re-commission to python controller as chip-tool and python
        commissioner do not share a credentials
-   --discriminator, --passcode, --qr-code, --manual-code
-   --tests to select tests
-   --PICS
-   --int-arg, --bool-arg, --float-arg, --string-arg, --json-arg, --hex-arg
    -   specify as key:value ex --bool-arg pixit_name:False
    -   used for custom arguments to scripts (PIXITs)

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

## Support functions

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
the fabric admin

Fabric admin for default controller:

```
fa=self.certificate_authority_manager.activeCaList[0].adminList[0]
second_ctrl = fa.new_fabric_admin.NewController(nodeId=node_id)
```

## other support functions

-   basic_composition_support
    -   wildcard read, whole device analysis
-   CommissioningFlowBlocks
    -   various commissioning support for core tests
-   spec_parsing_support
    -   parsing data model XML into python readable format

# Running tests locally

You can run the python script as-is for local testing against an already-running
DUT

`./scripts/tests/run_python_test.py` is a convenient script to fire up an
example DUT on the host, with factory reset support

`./scripts/tests/run_python_test.py --factoryreset --app <your_app> --app-args "whatever" --script <your_script> --script-args "whatever"`

Note that devices must be commissioned by the python test harness to run tests.
chip-tool and the python test harness DO NOT share a fabric.

# Running tests in CI

-   add to .github/workflows/tests.yaml repl_tests_linux
-   don’t forget to set the PICS file to the ci-pics-values
-   if there are things in your test that will fail on CI (ex. test vendor
    checks), gate them on the PICS_SDK_CI_ONLY
    -   is_ci = self.check_pics('PICS_SDK_CI_ONLY')
