# PICS and PIXITs

## What are PICS

In many Standards Defining Organizations including the CSA, the concept of a
“Protocol Implementation Conformance Statement” or “PICS” code is introduced to
simplify description of protocol elements.

Each PICS code is a binary value that describes the presence or absence of a
particular element or capability on a device. Each cluster has a defined PICS
prefix string and defines a set of PICS codes to describe whether the cluster is
present, and the presence of each feature, attribute, command, and event for
both the server and client side.

For example if there was a cluster ANeatCluster with the PICS code ANC, two
features (bit 0 and 1), two attributes (0x0000 and 0x0001), two accepted
commands (0x00 and 0x01), a single command response (0x02) and one event (0x00),
the following PICS codes would be defined

| CODE          | Desc                                                                       |
| ------------- | -------------------------------------------------------------------------- |
| ANC.S         | Device implements the ANC cluster as a server                              |
| ANC.S.F00     | Device implements ANC feature at bit 0 on the ANC server cluster           |
| ANC.S.F01     | Device implements ANC feature at bit 1 on the ANC server cluster           |
| ANC.S.A0000   | Device implements ANC attribute 0x0000 on the ANC server cluster           |
| ANC.S.A0001   | Device implements ANC attribute 0x0001 on the ANC server cluster           |
| ANC.S.C00.Rsp | Device accepts ANC command 0x00 on the ANC server cluster                  |
| ANC.S.C01.Rsp | Device accepts ANC command 0x01 on the ANC server cluster                  |
| ANC.S.C02.Tx  | Device generates ANC command response 0x02 on the ANC server cluster       |
| ANC.S.E00     | Device generates ANC event 0x00 on the ANC server cluster                  |
| ANC.C         | Device supports an ANC client                                              |
| ANC.C.F00     | Device ANC client is capable of understanding the feature with bit 0       |
| ANC.C.F01     | Device ANC client is capable of understanding the feature with bit 1       |
| ANC.C.A0000   | Device ANC client is capable of reading or subscribing to attribute 0x0000 |
| ANC.C.A0001   | Device ANC client is capable of reading or subscribing to attribute 0x0001 |
| ANC.C.C00.Rsp | Device ANC client is capable of sending the command with id 0x00           |
| ANC.C.C01.Rsp | Device ANC client is capable of sending the command with id 0x01           |
| ANC.C.C02.Tx  | Device ANC client understands the command response with id 0x02            |
| ANC.C.E00     | Device ANC client understands the event with id 0x00                       |

More information about the PICS code format can be found at
[PICS Guidelines](https://github.com/CHIP-Specifications/chip-test-plans/blob/master/docs/PICS%20Guidelines.md).

In addition to these standard cluster PICS, other PICS may be defined to
describe capabilities that are not directly expressed via the data model. For
example, whether the device responds to manual operations that affect the Matter
data model.

Additionally, there are node-level PICS, which appear as a part of the MCORE
PICS set. These PICS codes describe node level support such as the radio,
whether the device is commissionable, whether the device comes with a QR code
etc.

PICS are used in testing in two ways. "Top level" PICS appear at the top of the
test plan and indicate whether an entire test case should be run. For example, a
test case for ANeatCluster would have a top-level PICS of ANC.S to indicate that
the test would only be run if the ANC cluster is present on the endpoint. Test
cases also use PICS to gate individual steps of the test which are not
applicable if a certain element or capability is not implemented.

The entire collection of PICS codes for a specification release is provided as a
set of PICS XML files. These files are loaded into the PICS tool, which is used
to manually set all the PICS codes for a device. The PICS XML files and the PICS
tool are distributed as part of the official specification release package and
are available on
[Causeway](https://groups.csa-iot.org/wg/members-all/document/folder/2269).

### PIXITs

PIXIT stands for Protocol Implementation eXtra Information for Testing. A PIXIT
value provides an implementation-defined condition or value that is required by
the test.

PIXIT values are used to convey information that is required for testing, but
not normally available to a client interacting with the device. For example, the
key for a test event trigger on the device, or the network credentials for the
test harness.

More information on PIXITs and the PIXIT format can be found in the
[PICS Guidelines](https://github.com/CHIP-Specifications/chip-test-plans/blob/master/docs/PICS%20Guidelines.md).

## Creating PICS conformance statements for Matter devices

Because Matter devices may contain multiple, differing instances of the same
cluster on multiple endpoints, it it not possible to unambiguously describe a
Matter device with a single set of cluster-based PICS files. Instead, Matter
devices use a full set of PICS XML files to describe each endpoint.

To create a conformance statement for a Matter device, for each endpoint, load
the full set of PICS XML files into the
[PICS tool](https://picstool.csa-iot.org/), and select each of the PICS elements
present for the endpoint being described. Documentation on how to use the PICS
tool can be found in the PICS tool readme in the tool menu.

Some of of the full-node MCORE elements really only apply to the root node,
while others apply across all endpoints. For example, the entire device is
commissionable, but commissioning tests only need to be run against EP0, so
MCORE PICS should be set on the EP0 PICS set. Things like MCORE.IDM apply to
every endpoint.

### Helper scripts

The official tooling for CSA certification is the PICS tool provided as a part
of the release. PICS files need to pass validation on the PICS tool to be valid
for certification.

However, in Matter, many of the PICS codes correspond directly to elements that
are exposed directly on the device. For example, cluster presence is determined
from the server list on an endpoint, feature maps, attribute and command lists
correspond directly to the PICS.S.Fxx, PICS.S.Axxxx and PICS.S.Cxx.Xx PICS
codes. The Matter SDK provides a tool to pre-fill these values in the PICS XML
files so they do not have to be individually, manually filled in the PICS tool.

-   [PICS Generator](https://github.com/project-chip/connectedhomeip/tree/master/src/tools/PICS-generator)

PICS codes filled using this tool still need to be validated by the PICS tool.
Note that due to device limitations, the tool will NOT fill the following
categories of PICS codes, and these will need to be filled manually in the PICS
tool:

-   Event PICS (PICS.S.Exx)
-   Client PICS
-   MCORE (base.xml) PICS
-   Manual or other non-element PICS
-   PICS describing whether optionally-writable attributes are writable
-   any other non-element PICS

It is important to note that this script is NOT the official tool for PICS
generation, just a helper to assist with this manual process. It is very
important to go back and check that the values are as expected and to fill in
the other PICS.

### Verifying PICS using the IDM-10.4 certification test

While not all PICS are verifiable on the device, we do have tests that verify
the declared PICS against the device. This is one of the first tests that should
be run at certification, as the PICS files are what determine which set of tests
are required.

To run these tests locally, follow the instructions at
[Running Python tests locally](./python.md#running-tests-locally). The PICS
checker test is TC-IDM-10.4, implemented in
[TC_pics_checker.py](https://github.com/project-chip/connectedhomeip/blob/master/src/python_testing/TC_pics_checker.py).
These tests run a single set of PICS XML files against an endpoint. The
`--endpoint` and `--PICS` flags are therefore required.

Note that you can run tests locally against the PICS XMLs for an endpoint by
supplying the name of the directory containing the set of PICS XML files for
that endpoint.

## Setting PIXITs for Matter devices

Matter tests do not currently have support to read PIXIT values from the PICS
XML files. Instead, tests implement PIXITs as test-specific flags. When running
locally, these are specified on the command line. When running in the test
harness, these are specified in the test parameters section of the test
configuration.

## Declarative PIXITs and harness parameters

The Python testing framework provides a declarative way for a test to state the
parameters it requires. A test implemented as a `MatterBaseTest` subclass in
`src/python_testing` declares its parameters using decorators; the declarations
are validated in `setup_test()` before the test body runs, and the declared
values are recorded in the failure output. This makes a test's required inputs
explicit and self-documenting, and surfaces a missing or incorrectly typed value up front
rather than partway through the test. Two categories of parameter are supported:

-   PIXITs are test-specific values such as application paths, timeouts, or
    trigger keys. They are declared with `@pixit`, read with `self.pixit(name)`,
    and supplied through `user_params` using the `--*-arg` command-line flags.
    They are defined in `matter.testing.pixit`.
-   Harness parameters are the standard runner and commissioning flags such as
    the discriminator, passcode, and Wi-Fi credentials. They are declared with
    `@harness_params`, read with `self.harness_param(name)`, and resolved from
    `MatterTestConfig`. The set of valid names is fixed by a registry in
    `matter.testing.harness_params`.

### Declaring PIXITs

Each PIXIT a test requires is declared with a `@pixit` decorator. Multiple
decorators may be stacked, and they must be placed above `@async_test_body` and
`@run_if_endpoint_matches` so that validation runs before the test body.

```python
from matter.testing.pixit import pixit
from matter.testing.matter_testing import MatterBaseTest, async_test_body


class TC_Example(MatterBaseTest):

    @pixit("th_server_app_path", str, "Path to the TH server application")
    @pixit("timeout_sec", int, "Timeout in seconds", required=False, default=30)
    @async_test_body
    async def test_TC_Example_1_1(self):
        app_path = self.pixit("th_server_app_path")
        timeout = self.pixit("timeout_sec")
```

The decorator signature is
`pixit(name, value_type, description, required=True, default, sensitive=False)`,
where:

-   `name` is the key in `user_params`, also used on the command line as
    `--<flag> name:value`.
-   `value_type` is the expected Python type, enforced with `isinstance` during
    setup.
-   `description` is the text shown in error messages and failure output.
-   `required`, when `True` (the default), causes setup to fail if the value is
    absent.
-   `default` is the value returned for an optional PIXIT that was not supplied;
    it is ignored when `required` is `True`.
-   `sensitive`, when `True`, causes the value to be redacted in the failure
    output.

A value is read with `self.pixit(name)`, which resolves to the value in
`user_params` when present; otherwise, for an optional PIXIT, to the decorator
default; otherwise to the `default` argument passed to `self.pixit`.

The declared `value_type` determines the command-line flag used to supply the
value:

| Python type     | Command-line flag         |
| --------------- | ------------------------- |
| `str`           | `--string-arg name:value` |
| `int`           | `--int-arg name:value`    |
| `bool`          | `--bool-arg name:value`   |
| `float`         | `--float-arg name:value`  |
| `bytes`         | `--hex-arg name:value`    |
| `list` / `dict` | `--json-arg name:value`   |

Supplying a value whose type does not match the declaration causes setup to fail
with a message that names the expected flag.

### Declaring harness parameters

The standard commissioning and runner flags are populated on `MatterTestConfig`
by `runner.py`. A test declares the ones it depends on so that they are
validated during setup and recorded in the failure output. The names must exist
in the registry; their descriptions and command-line hints are taken from the
registry rather than repeated on each test.

```python
from matter.testing.harness_params import harness_params
from matter.testing.pixit import pixit


class TC_Example(MatterBaseTest):

    @harness_params("discriminator", "passcode", optional=("endpoint",))
    @pixit("th_server_app_path", str, "Path to the TH server application")
    @async_test_body
    async def test_TC_Example_1_1(self):
        disc = self.harness_param("discriminator")
        path = self.pixit("th_server_app_path")
```

The signature is `harness_params(*required, optional=())`. The positional names
are required and cause setup to fail when unsatisfied. Names listed in
`optional` are recorded for documentation and failure output but do not affect
validation. A name that is not present in the registry raises `ValueError` when
the decorator is applied. The registry defines the following names:

| Name                   | Description                     | Supplied via                                         | Sensitive |
| ---------------------- | ------------------------------- | ---------------------------------------------------- | --------- |
| `discriminator`        | Long discriminator              | `--discriminator` (or `--qr-code` / `--manual-code`) |           |
| `passcode`             | Setup passcode (PASE)           | `--passcode` (or `--qr-code` / `--manual-code`)      | Yes       |
| `wifi_ssid`            | Wi-Fi SSID                      | `--wifi-ssid`                                        |           |
| `wifi_passphrase`      | Wi-Fi passphrase                | `--wifi-passphrase`                                  | Yes       |
| `commissioning_method` | Commissioning method            | `--commissioning-method`                             |           |
| `dut_node_id`          | DUT node id on the fabric       | `--dut-node-id`                                      |           |
| `endpoint`             | Endpoint under test             | `--endpoint`                                         |           |
| `thread_dataset`       | Thread operational dataset hex  | `--thread-dataset-hex`                               | Yes       |
| `commissionee_ip`      | Commissionee IP (on-network-ip) | `--ip-addr`                                          |           |

The `discriminator` and `passcode` parameters are satisfied by their direct flag
or by `--qr-code` / `--manual-code`. When supplied through a setup code, the
decoded numeric value is not stored on `MatterTestConfig`, so
`self.harness_param` returns `None` and the failure output shows
`<from --qr-code>` or `<from setup code>` in place of the value. A test that
requires the numeric discriminator or passcode should obtain it through
`--discriminator` or `--passcode` rather than a setup code.

### Validation and failure output

`setup_test()` validates the declared PIXITs against `user_params` and the
declared harness parameters against `MatterTestConfig`. If a required value is
missing, or a PIXIT value has the wrong type, the test fails immediately with a
single message that lists the missing values and the flags used to supply them:

```
Test 'test_TC_Example_1_1' is missing required PIXIT value(s):

Missing required PIXITs:
  - th_server_app_path (str): Path to the TH server application
    Provide via: --string-arg th_server_app_path:<value>

Available optional PIXITs:
  - timeout_sec (int): Timeout in seconds
    Default: 30
    Override via: --int-arg timeout_sec:<value>

Test 'test_TC_Example_1_1' is missing required harness parameter(s):

Missing required harness (pass via command line):
  - discriminator: Long discriminator for commissioning (or use --qr-code / --manual-code).
    Provide via: --discriminator <value>  or  --qr-code <payload>  or  --manual-code <code>
```

When a non-commissioning test fails, all declared parameters are recorded in the
log, including those that were present, to aid debugging. Values declared as
sensitive are shown as `***REDACTED***`; this applies to PIXITs declared with
`sensitive=True` and to the registry parameters `passcode`, `wifi_passphrase`,
and `thread_dataset`. Byte values are shown as their length rather than their
contents.

Tests that do not use these decorators are unaffected; validation and failure
output apply only when the decorators are present.

### Command-line example

A test declaring both PIXITs and harness parameters:

```python
@harness_params("discriminator", "passcode", "commissioning_method")
@pixit("th_server_app_path", str, "Path to the TH server application")
@pixit("timeout_sec", int, "Timeout in seconds", required=False, default=30)
@async_test_body
async def test_TC_Example_1_1(self):
    ...
```

is run with:

```bash
python3 src/python_testing/TC_Example.py \
    --commissioning-method on-network \
    --discriminator 3840 \
    --passcode 20202021 \
    --string-arg th_server_app_path:/path/to/th-server \
    --int-arg timeout_sec:60
```

The `--commissioning-method`, `--discriminator`, and `--passcode` flags satisfy
the harness declarations, and the `--string-arg` and `--int-arg` flags satisfy
the PIXITs.

## PICS for test selection

The official source that the CSA certification team uses to determine if all the
required tests have been run at certification is the submitted set of PICS XML
files and the PICS tool.

To generate this set of tests for each endpoint, load all the filled PICS XML
files for a single endpoint into the PICS tool and validate the PICS files. The
PICS files should validate properly. This will also generate the list of test
cases.

Note that the TH will also guide test selection by pre-selecting the required
tests based on the PICS file, but it is the responsibility of the testers and
the ATL to ensure that all the required tests are run and the results are
submitted.

## Creating PICS / PIXITs for new cluster and use in test plans

Placeholder for more information coming in a subsequent PR. Needs to cover

-   formatting (link out to test plans doc)
-   requirement to have all the elements listed
-   how to do conformance in a way the PICS tool can handle - (note - special
    attention to choice conformance, otherwise conformance)
-   why you shouldn't use PICS values in tests other than at the top level

## Using PICS in test scripts

Placeholder for more information coming in a subsequent PR. Needs to cover
top-level and step-wise pics in yaml and python and also where they should NOT
be used.

## CI PICS format

Placeholder for information about the CI PICS format, CI-only PICS and the
gotchas there.
