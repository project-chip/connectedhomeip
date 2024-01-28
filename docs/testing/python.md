# Python framework tests

This file is a placeholder for python framework test information.

NOTE: be sure to include information about how you need to commission with the
python controller, not chip-tool and how to do that in the scripts

Note - go through the docs in the readme and move them over here, and take out
the deprecated stuff

Note - add notes about record error and fail test vs. asserts

---

The python test framework is built on top of the ChipDeviceCtrl.py python
controller API and the mobly test framework. Python tests are interaction tests,
and can be used for certification testing, and / or integration testing in the
CI.

Python tests located in src/python_testing

## Resources for getting started

-   [src/python_testing/hello_test.py](https://github.com/project-chip/connectedhomeip/blob/master/src/python_testing/hello_test.py) -
    sample test showing test setup and test harness integration
-   [https://github.com/google/mobly/blob/master/docs/tutorial.md](https://github.com/google/mobly/blob/master/docs/tutorial.md)
-   [ChipDeviceCtrl.py](https://github.com/project-chip/connectedhomeip/blob/master/src/controller/python/chip/ChipDeviceCtrl.py) -
    Controller implementation with API documentation
-   [scripts/tests/run_python_test.py](https://github.com/project-chip/connectedhomeip/blob/master/scripts/tests/run_python_test.py)
    to easily set up app and script for testing - used in CI

## Writing Python tests

-   All test classes inherit from MatterBaseTest in
    [matter_testing_support.py](https://github.com/project-chip/connectedhomeip/blob/master/src/python_testing/matter_testing_support.py)
    -   support for commissioning using the python controller
    -   default controller (self.default_controller) of type ChipDeviceCtrl
    -   MatterBaseTest inherits from the Mobly BaseTestClass
-   Test function(s) (start with test\_) and are all run automatically
    -   To run in the test harness, the test name must be test*TC_PICSCODE*#\_#
        -   more information about integration with the test harness can be
            found in [Test Harness](#TestHarness)
    -   any tests that use async function (read / write / commands) should be
        decorated with the @async_test_body decorator
-   Use ChipDeviceCtrl to interact with the DUT
    -   Controller API is in ChipDeviceCtrl.py (see API doc in file)
    -   some support functions in matter_testing_support.py
-   Use mobly assertions for failing tests
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

# Cluster Codegen

-   See Objects.py for codegen, ClusterObjects.py for classes
-   import chip.clusters as Clusters
-   Clusters (Cluster)
-   Clusters.<ClusterName>
    -   id
    -   descriptor
    -   Enums
    -   Bitmaps
    -   Structs
    -   Attributes
    -   Commands
    -   Events

# Cluster Codegen - Attributes

-   Attributes (ClusterAttributeDescriptor)
-   Clusters.<ClusterName>.Attributes.<AttributeName>
    -   cluster_id
    -   attribute_id
    -   attribute_type
    -   value
-   ex:
-   class - Clusters.OnOff.Attributes.OnTime
-   instance - Clusters.OnOff.Attributes.OnTime(5)

---

pass the class to read, the instance to write

# Cluster Codegen - Commands

-   Commands (ClusterCommand)
-   Clusters.<ClusterName>.Commands.<CommandName>
    -   cluster_id
    -   command_id
    -   is_client
    -   response_type (None for status response)
    -   descriptor
    -   data members (if required)
-   Clusters.OnOff.Commands.OnWithTimedOff(
-   onOffControl=0, onTime=5, offWaitTime=8)
-   Clusters.OnOff.Commands.OnWithTimedOff()

# Cluster Codegen - Events

-   Events (ClusterEvent)
-   Clusters.<ClusterName>.Events.<EventName>
    -   cluster_id
    -   event_id
    -   descriptor
    -   data members if required
-   Clusters.AccessControl.Events.AccessControlEntryChanged.adminNodeID

# Cluster Codegen - Enums

-   Enums (MatterIntEnum)
-   Clusters.<ClusterName>.Enum.<EnumName>
    -   k<value>
    -   kUnknownEnumValue (used for testing, do not transmit)
-   Clusters.AccessControl.Events.AccessControlEntryChanged.adminNodeID

# Cluster Codegen - Bitmaps

-   Bitmaps (IntFlag)
-   Clusters.<ClusterName>.Bitmaps.<BitmapName>
    -   k<value>
-   special class:
    -   class Feature(IntFlag) - contains the feature map bitmaps
-   Clusters.LaundryWasherControls.Bitmaps.Feature.kSpin

# Cluster Codegen - Structs

-   Structs(ClusterObject)
-   Clusters.<ClusterName>.Structs.<StructName>
    -   descriptor
    -   data members
-   Clusters.BasicInformation.Structs.ProductAppearanceStruct(
-   finish=Clusters.BasicInformation.Enums.ProductFinishEnum.kFabric,
-   primaryColor=Clusters.BasicInformation.Enums.ColorEnum.kBlack)

# Accessing Classes by ID

chip.clusters.ClusterObjects.ALL_CLUSTERS

dict[int, Cluster] - maps cluster ID to Cluster class

cluster = chip.clusters.ClusterObjects.ALL_CLUSTERS[cluster_id]

chip.clusters.ClusterObjects.ALL_ATTRIBUTES

dict[int, dict[int, ClusterAttributeDescriptor]] - maps cluster ID to a dict of
attribute ID to attribute class

attr = chip.clusters.ClusterObjects.ALL_ATTRIBUTES[cluster_id][attribute_id]

chip.clusters.ClusterObjects.ALL_ACCEPTED_COMMANDS/ALL_GENERATED_COMMANDS

dict[int, dict[int, ClusterCommand]]

cmd = chip.clusters.ClusterObjects.ALL_ACCEPTED_COMMANDS[cluster_id][cmd_id]

# ChipDeviceCtrl API - Read

-   Read
    -   Read or subscribe to either events or attributes
-   ReadAttribute, ReadEvent
    -   wrapper of read, easier to user
    -   handle wildcard or concrete path
    -   handle subscriptions and one-shot reads
-   All raise InteractionModelError on failure

# ChipDeviceCtrl API - ReadAttribute

async def ReadAttribute(self, nodeid: int, attributes: typing.List[typing.Union[

None, # Empty tuple, all wildcard

typing.Tuple[int], # Endpoint

typing.Tuple[typing.Type[ClusterObjects.Cluster]], # Wildcard endpoint, Cluster
id present

typing.Tuple[typing.Type[ClusterObjects.ClusterAttributeDescriptor]], # Wildcard
endpoint, Attribute present

typing.Tuple[int, typing.Type[ClusterObjects.Cluster]], # Wildcard cluster id

typing.Tuple[int, typing.Type[ClusterObjects.ClusterAttributeDescriptor]] #
Concrete path

]], dataVersionFilters: typing.List[typing.Tuple[int,
typing.Type[ClusterObjects.Cluster], int]] = None,

returnClusterObject: bool = False,

reportInterval: typing.Tuple[int, int] = None,

fabricFiltered: bool = True, keepSubscriptions: bool = False, autoResubscribe:
bool = True):

for one-shot reads, set reportInterval to None (default)

to start a subscription, set reportInterval + keepSubscriptions +
autoResubscribe as required

Wildcard read (all clusters, all endpoints)

await dev_ctrl.ReadAttribute(node_id, [()])

Wildcard read (single endpoint 0)

await dev_ctrl.ReadAttribute(node_id, [(0)])

Wildcard read (single cluster from single endpoint 0)

await dev_ctrl.ReadAttribute(node_id, [(1, Clusters.OnOff)])

Single attribute

await dev_ctrl.ReadAttribute(node_id, [(1,

Clusters.OnOff.Attributes.OnTime)])

Multipath

await dev_ctrl.ReadAttribute(node_id, [

(1, Clusters.OnOff.Attributes.OnTime)

(1, Clusters.OnOff.Attributes.OnOff)

])

# ChipDeviceCtrl API - ReadEvent

Basically the same as ReadAttribute, but the tuple includes urgency as the last
member

typing.Tuple[int,

typing.Type[ClusterObjects.ClusterEvent], int]

ex:

urgent = 1

await dev_ctrl ReadEvent(node_id, [(1,
Clusters.TimeSynchronization.Events.MissingTrustedTimeSource, urgent)])

# ChipDeviceCtrl API - ReadAttribute / ReadEvent

-   One shot ReadAttribute return:
    -   AsyncReadTransation.ReadResponse.attributes.
    -   dict[int, List[Cluster]]
    -   ret[endpoint_id][<Cluster class>][<Attribute class>]
    -   Ex. To access the OnTime attribute from the OnOff cluster on EP 1
        ret[1][Clusters.OnOff][Clusters.OnOff.Attributes.OnTime]
-   One shot ReadEvent return:
    -   List[ClusterEvent]

# ChipDeviceCtrl API - Subscriptions

-   Subscription return
    -   ClusterAttribute.SubscriptionTransaction
    -   SetAttributeUpdateCallback
        -   Callable[[TypedAttributePath, SubscriptionTransaction], None]
    -   SetEventUpdateCallback
        -   Callable[[EventReadResult, SubscriptionTransaction], None]
    -   await changes in the main loop using a trigger mechanism from the
        callback.

q = queue.Queue()

cb = SimpleEventCallback("cb", cluster_id, event_id, q)

urgent = 1

subscription = await

dev_ctrl.ReadEvent(nodeid=1,

events=[(1, event, urgent)], reportInterval=[1, 3])

subscription.SetEventUpdateCallback(callback=cb)

try:

q.get(block=True, timeout=timeout)

except queue.Empty:

asserts.assert_fail(“Timeout on event”)

# ChipDeviceCtrl API - Write

async def WriteAttribute(self, nodeid: int,

attributes: typing.List[typing.Tuple[int,
ClusterObjects.ClusterAttributeDescriptor]],

timedRequestTimeoutMs: typing.Union[None, int] = None,

interactionTimeoutMs: typing.Union[None, int] = None,

busyWaitMs: typing.Union[None, int] = None)

Concrete paths only (can be a list)

Returns list of PyChipError

Instantiate the ClusterAttributeDescriptor class with the value you want to
send, tuple is (endpoint, attribute)

use timedRequestTimeoutMs for timed request actions

res = await devCtrl.WriteAttribute(

nodeid=0,

attributes=[

(0,Clusters.BasicInformation.Attributes.NodeLabel(

"Test"))])

asserts.assert_equal(ret[0].status, Status.Success, “write failed”)

# ChipDeviceCtrl API - Commands

async def SendCommand(self, nodeid: int,

endpoint: int,

payload: ClusterObjects.ClusterCommand,

responseType=None,

timedRequestTimeoutMs: typing.Union[None, int] = None,

interactionTimeoutMs: typing.Union[None, int] = None,

busyWaitMs: typing.Union[None, int] = None,

suppressResponse: typing.Union[None, bool] = None)

Raises InteractionModelError on error

returns the command response (type as defined by command)

Instantiate the command with the values you need to populate

If there is a non-status return, it’s returned from the command

If there is a pure status return it will return nothing

Raises InteractionModelError on failure

pai = await dev_ctrl.SendCommand(nodeid, 0,

Clusters.OperationalCredentials.Commands.CertificateChainRequest(2))

# MatterBaseTest helpers

-   Because we tend to do a lot of single read / single commands in tests, we
    added a couple of helpers in MatterBaseTest that use some of the default
    values
    -   read_single_attribute_check_success
    -   read_single_attribute_expect_error
    -   send_single_cmd
-   print_step (updates coming with TH integration)
-   check_pics

# Mobly helpers

-   default_matter_test_main
    -   Sets up commissioning and finds all tests, parses command arguments
-   if **name** == "**main**":
-   default_matter_test_main()
-   Mobly will run all functions starting with test\_ by default
    -   use --tests command line argument to specify
-   Setup / teardown functions
    -   setup_class / teardown_class
    -   setup_test / teardown_test
    -   Don’t forget to call the super() if you override these

# test harness helpers

-   steps
-   desc
-   pics
-   overriding the default timeout

# Command line arguments

-   Use help to get a full list
-   --commissioning-method
    -   need to re-commission to python controller as chip-tool and python
        commissioner do not share a credentials
-   --discriminator, --passcode, --qr-code, --manual-code
-   --tests to select tests
-   --PICS
-   --int-arg, --bool-arg, --float-arg, --string-arg, --json-arg, --hex-arg
    -   specify as key:value ex --bool-arg mybool:False
    -   used for custom arguments to scripts (PIXITs)

# PICS and PIXITS

-   PICS
    -   use --PICS on the command line to specify the PICS file
    -   use check_pics to gate steps in a file
-   have_whatever = check_pics(“PICS.S.WHATEVER”)
-   PIXITs
    -   use --int-arg, --bool-arg etc on the command line to specify PIXITs
    -   Warn users if they don’t set required values, add instructions in the
        comments
-   ret = self.user_params.get("pixit_name", default)

# Support functions

To create a controller on a new fabric:

new_CA = self.certificate_authority_manager.NewCertificateAuthority()

new_fabric_admin = new_certificate_authority.NewFabricAdmin(vendorId=0xFFF1,
fabricId=self.matter_test_config.fabric_id + 1)

TH2 = new_fabric_admin.NewController(nodeId=112233)

Open a commissioning window (ECW)

params = dev_ctrl.OpenCommissioningWindow(

nodeid=self.dut_node_id, timeout=900, iteration=10000,

discriminator=discriminator, option=1)

discriminator - between 0 and 4095

option 1 - ECW (the only Mandatory commissioning window)

To create a new controller on the SAME fabric, allocate a new controller from
the fabric admin

Fabric admin for default controller:

fa=self.certificate_authority_manager.activeCaList[0].adminList[0]

second_ctrl = fa.new_fabric_admin.NewController(nodeId=node_id)

# other support functions

-   basic_composition_support
    -   wildcard read, whole device analysis
-   CommissioningFlowBlocks
    -   various commissining support for core tests
-   spec_parsing_support
    -   parsing data model XML into python readable format

# Running tests locally

You can run the python script as-is for local testing against an already-running
DUT

./scripts/tests/run_python_test.py is a convenient script to fire up an example
DUT on the host, with factory reset support

./scripts/tests/run_python_test.py

--factoryreset

--app <your_app> --app-args "whatever"

--script <your_script>

--script-args "whatever"

# Running tests in CI

-   add to .github/workflows/tests.yaml repl_tests_linux
-   don’t forget to set the PICS file to the ci-pics-values
-   if there are things in your test that will fail on CI (ex. test vendor
    checks), gate them on the PICS_SDK_CI_ONLY
    -   is_ci = self.check_pics('PICS_SDK_CI_ONLY')

```

```
