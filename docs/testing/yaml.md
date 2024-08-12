# YAML tests

YAML is a structured, human-readable data-serialization language. Much like json
or proto, YAML refers to the structure and parser, and the schema used for any
particular application is defined by the application.

In Matter, we use YAML for describing tests and test steps. A YAML parser and
runner is then used to translate the YAML instructions into actions used to
interact with the device under test (DUT).

The main runner we use for testing in Matter parses the YAML instructions into
chip-tool commands.

The schema description for the Matter test YAML is available here:
[YAML Schema](./yaml_schema.md)

## Writing YAML tests

Most YAML tests are written for certification. These follow a standard format
that is used to display the test easily in the test harness.

### Placeholder for anatomy of a yaml test - need diagram

### Placeholder for anatomy of a test step - need diagram

### Common actions

#### Sending a cluster command

The following shows a test step sending a simple command with no arguments.

```
    - label: "This label gets printed"
      cluster: "On/Off"
      command: "On"
```

-   label - label to print before performing the test step
-   cluster - name of the cluster to send the command to
-   command - name of the command to send

This send the On command to the On/Off cluster on the DUT. For most tests, the
nodeID of the DUT and endpoint for the cluster are defined in the top-level
config section of the file and applied to every test step. However, these can
also be overwritten in the individual test steps.

The following shows how to send a command with arguments:

```
    - label: "This label gets printed before the test step"
      command: "MoveToColor"
      arguments:
          values:
              - name: "ColorX"
                value: 32768
              - name: "ColorY"
                value: 19660
              - name: "TransitionTime"
                value: 0
              - name: "OptionsMask"
                value: 0
              - name: "OptionsOverride"
                value: 0
```

-   label - label to print before performing the test step
-   command - name of the command to send
-   argument - this is a list parameter that takes either a "value" or "values"
    tag. Commands with arguments all use structured fields, which require the
    "values" tag with a list. Each of the fields is represented by a "name" and
    "value" pair

In this command, the cluster: tag is elided. The cluster for the entire test can
be set in the config section at the top of the test. This can be overwritten for
individual test steps (as above).

#### Reading and writing attributes

Reading and writing attributes is represented in the Matter test YAML schemas as
a special command that requires an additional "attribute" tag.

The following YAML would appear as a test step, and shows how to read an
attribute.

```
- label: "TH reads the ClusterRevision from DUT"
  command: "readAttribute"
  attribute: "ClusterRevision"
```

The following YAML would appear as a test step and shows how to write an
attribute. Commands to write attributes always require an argument: tag.

```
- label: "Write example attribute"
  command: "writeAttribute"
  attribute: "ExampleAttribute"
  arguments:
    value: 1
```

#### Parsing Responses

After sending a command or read or write attribute request, you may want to
verify the response. This is done using the "response" tag with various
sub-tags.

The following shows a simple response parsing with two (somewhat redundant)
checks.

```
- label: "TH reads the ClusterRevision from DUT"
  command: "readAttribute"
  attribute: "ClusterRevision"
  response:
    value: 1
    constraints:
      minValue: 1
```

The following tags can be used to parse the response

| Example                                                                                                     | Description                                                                              |
| :---------------------------------------------------------------------------------------------------------- | :--------------------------------------------------------------------------------------- |
| response:<br />&emsp;value: [1, 2, 3, 4]                                                                    | must match exactly. Variables and saveAs values allowed                                  |
| response:<br />&emsp;values:<br />&emsp;&emsp;- name: response_field<br />&emsp;&emsp;&nbsp;&nbsp; value: 1 | Must match exactly<br />Use for commands that return command responses with named fields |
| response:<br /> &emsp; error: CONSTRAINT_ERROR                                                              | expect an error back (omit for success)<br />Variables and saveAs values will not work.  |
| response:<br />&emsp;constraints:                                                                           | more complex checks - see [Schema](./yaml_schema.md) for a complete description          |

#### Lists and structs

Lists and structs can be represented as follows:

Lists: `[1,2,3,4,5]`

structs: `{field1:value, field2:value}`

lists of structs:

```
[

{field1:value, field2:value, optionalfield:value},

{field1:value, field2:value},

]
```

Note that structs are different than command and command response fields, which
are represented using name:, value: tags.

#### Pseudo clusters

Tests often require functionality that is not strictly cluster-based. Some of
this functionality is supported in YAML using pseudo-clusters. These clusters
accept command: tags like the DUT clusters to control the pseudo-cluster
functionality.

Some of the more common functionality is shown below:

Establishing a connection to the DUT. This is the first step in nearly every
test.

```
    - label: "Establish a connection to the DUT"
      cluster: "DelayCommands"
      command: "WaitForCommissionee"
      arguments:
          values:
              - name: "nodeId"
                value: nodeId
```

Wait for a user action:

```
    - label: "Do a simple user prompt message. Expect 'y' to pass."
      cluster: "LogCommands"
      command: "UserPrompt"
      arguments:
          values:
              - name: "message"
                value: "Please enter 'y' for success"
              - name: "expectedValue"
                value: "y"
```

Wait for a time:

```
    - label: "Wait for 5S"
      cluster: "DelayCommands"
      command: "WaitForMs"
      arguments:
          values:
              - name: "ms"
                value: 5000
```

A full description of the available pseudo-clusters and their commands is
available at [Pseudo-cluster description](./yaml_pseudocluster.md).

#### Config variables and saveAs:

Certain tags can use variables that are either declared in the config: section
or saved from other steps. Variables that are declared in the config can be
overwritten on the command line when running locally or through the config file
in the test harness.

To declare config variables in the config section, use a label with the desired
name, then provide the type and defaultValue tags as sub-tags.

```
config:
    nodeId: 0x12344321
    cluster: "Unit Testing"
    endpoint: 1
    myArg1:
        type: int8u
        defaultValue: 5
```

Variables can also be saved from responses:

```
    - label: "Send Test Add Arguments Command"
      command: "TestAddArguments"
      arguments:
          values:
              - name: "arg1"
                value: 3
              - name: "arg2"
                value: 17
      response:
          values:
              - name: "returnValue"
                saveAs: TestAddArgumentDefaultValue
                value: 20
```

Variables can then be used in later steps:

```
    - label: "Send Test Add Arguments Command"
      command: "TestAddArguments"
      arguments:
          values:
              - name: "arg1"
                value: 3
              - name: "arg2"
                value: 17
      response:
          values:
              - name: "returnValue"
                value: TestAddArgumentDefaultValue
```

Tags where variables can be used are noted in the
[schema description](./yaml_schema.md).

Config variables can be used to implement PIXIT values in tests.

#### Gating tests and steps: PICS, TestEqualities and runIf

The PICS tag can be used to unconditionally gate a test step on the PICS value
in the file.

The PICS tag can handle standard boolean operations on pics (!, ||, &&, ()).

A PICS tag at the top level of the file can be used to gate the entire test in
the test harness. Note that full-test gating is not currently implemented in the
local runner or in the CI.

Some test steps need to be gated on values from earlier in the test. In these
cases, PICS cannot be used. Instead, the runIf: tag can be used. This tag
requires a boolean value. To convert values to booleans, the TestEqualities
function can be use. See
[TestEqualities](https://github.com/project-chip/connectedhomeip/blob/master/src/app/tests/suites/TestEqualities.yaml)
for an example of how to use this pseudo-cluster.

#### Setting step timeouts

The timeout argument can be used for each individual test step to set the time
the runner will wait for a test step to complete before reporting a failure.

Note that this timeout is different than the subscription report timeout and the
subscription report timeout is not currently adjustable in YAML.

There several other options for configuring test steps as shown in the
[YAML schema](./yaml_schema.md) document.

## Running YAML tests

YAML scripts are parsed and run using a python-based runner program that parses
the file, then translates the tags into chip-tool commands, and sends those
commands over a socket to chip-tool (running in interactive mode).

### Running locally

#### Commissioning the DUT

All YAML tests assume that the DUT has previously been commissioned before
running. DUTs should be commissioned using chip-tool. Use the same KVS file when
running the test.

By default, the tests use node ID 0x12344321. The easiest way to run the tests
is to commission with this node ID. Alternately, you can change the target node
ID on the command line, as shown in the [Running the tests](#running-the-tests)
section.

#### Running the tests

There are several options for running tests locally. Because the YAML runner
uses python, it is necessary to compile and install the chip python package
before using any YAML runner script.

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

Next build the python wheels and create a venv (called `py` here, but any name
may be used)

```
./scripts/build_python.sh -i py
source py/bin/activate
```

Compile chip-tool:

```
./scripts/build/build_examples.py --target linux-x64-chip-tool build

```

NOTE: use the target appropriate to your system

[chiptool.py](https://github.com/project-chip/connectedhomeip/blob/master/scripts/tests/chipyaml/chiptool.py)
can be used to run tests against a commissioned DUT (commissioned by chip-tool).
To commission a DUT using chip-tool use the pairing command. For example:

```
./out/linux-x64-chip-tool/chip-tool pairing code 0x12344321 MT:-24J0AFN00KA0648G00
```

In this example, 0x12344321 is the node ID (0x12344321 is the test default) and
MT:-24J0AFN00KA0648G00 is the QR code.

The chiptool.py tool can then be used to run the tests. For example:

```
./scripts/tests/chipyaml/chiptool.py tests Test_TC_OO_2_1 --server_path ./out/linux-x64-chip-tool/chip-tool

```

NOTE: substitute the appropriate test name and chip-tool path as appropriate.

A list of available tests can be generated using:

```
./scripts/tests/chipyaml/chiptool.py list
```

Config variables can be passed to chiptool.py after the script by separating
with --

```
./scripts/tests/chipyaml/chiptool.py tests Test_TC_OO_2_1 --server_path ./out/linux-x64-chip-tool/chip-tool -- nodeId 0x12344321

```

Each test defines a default endpoint to target. Root node cluster tests run
against endpoint 0 by default. Most other cluster tests run against endpoint 1.
You can set the endpoint for the test using the `endpoint` config variable.

#### Factory resetting the DUT

On the host machine, you can simulate a factory reset by deleting the KVS file.
If you did not specify a location for the KVS file when starting the
application, the KVS file will be in /tmp as chip_kvs

### Running in the CI

-   YAML tests added to the certification directory get run automatically
    -   src/app/tests/suites/certification/
    -   PICS file: src/app/tests/suites/certification/ci-pics-values
-   If you DON’T want to run a test in the CI
    -   (ex under development), add it to \_GetInDevelopmentTests in
        `scripts/tests/chiptest/__init__.py`

Please see [CI testing](./ci_testing.md) for more information about how to set
up examples apps, PICS and PIXIT values for use in the CI.

### Running in the TH

TODO: Do we have a permanent link to the most up to date TH documentation? If
so, add here.
