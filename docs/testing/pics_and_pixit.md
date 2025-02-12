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
| ANC.S.C01.Rsp | Device accepts ANC command 0x00 on the ANC server cluster                  |
| ANC.S.C02.Tx  | Device generates ANC command response 0x02 on the ANC server cluster       |
| ANC.S.E00     | Device generates ANC event 0x00 on the ANC server cluster                  |
| ANC.C         | Device supports an ANC client                                              |
| ANC.C.F00     | Device ANC client is capable of understanding the feature with bit 0       |
| ANC.C.F01     | Device ANC client is capable of understanding the feature with bit 0       |
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
