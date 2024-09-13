# CHIP Test Suites

-   [CHIP Test Suites](#chip-test-suites)
-   [Controllers](#controllers)
-   [Automatic conversion process](#automatic-conversion-process)
-   [Lists](#lists)
    -   [List: _chip_tests_](#list-chip_tests)
        -   [Property: _test_](#property-test)
        -   [Property: _config_](#property-config)
            -   [Property: _{variable_name}_](#property-variable_name)
    -   [List: _chip_tests_items_](#list-chip_tests_items)
        -   [Property: _tests_](#property-tests)
            -   [Property: _arguments_](#property-arguments)
            -   [Property: _values_](#property-values)
            -   [Property: _response_](#property-response)
            -   [Property: _constraints_](#property-constraints)
    -   [List: _chip_tests_item_parameters_](#list-chip_tests_item_parameters)
    -   [List: _chip_tests_item_response_parameters_](#list-chip_tests_item_response_parameters)
    -   [List: _chip_tests_pics_](#list-chip_tests_pics)
    -   [List: _chip_tests_config_](#list-chip_tests_config)
-   [YAML Test Definition](#yaml-test-definition)
    -   [Location of Test Definitions](#location-of-test-definitions)
-   [PICS Usage](#pics-usage) - [Required Files](#required-files) -
    [Example PICS Command](#example-pics-command)
-   [Examples](#examples)
    -   [YAML Examples](#yaml-examples)
    -   [ZAP Example](#zap-example)
        -   [Required Files](#required-files-1)
        -   [Generate Example Script](#generate-example-script)
-   [Index](#index)

This directory contains a set of tests describing interactions between nodes,
and more specifically between a controller and a controllee.

This test set is written in a high level language before being automatically
converted into a language understandable by the different controllers.

# Controllers

There are currently 4 implementations of the CHIP device controller. These
different implementations share a common configuration file describing the
supported functionalities in terms of clusters, commands and attributes:
[controllers-clusters.zap](../../../controller/data_model/controller-clusters.zap)

| Controllers | Testing |
| ----------- | ------- |
| POSIX CLI   | y       |
| Darwin CLI  | y       |
| iOS         | y       |
| Python      | n       |
| Android     | n       |

For more information on the different implementations of the CHIP device
controller, see [README.md](../../../controller/README.md)

# Automatic conversion process

The process of automatic conversion of test files depends on the
[ZAP](https://github.com/project-chip/zap) tool.

Each of the CHIP device controller implementations uses a dedicated template
translating the tests into an appropriate format.

| Controllers | Template                                                                                                                                               |
| ----------- | ------------------------------------------------------------------------------------------------------------------------------------------------------ |
| POSIX CLI   | [examples/chip-tool/templates/partials/test_cluster.zapt](../../../../examples/chip-tool/templates/partials/test_cluster.zapt)                         |
| Darwin CLI  | [examples/darwin-framework-tool/templates/partials/test_cluster.zapt](../../../../examples/darwin-framework-tool/templates/partials/test_cluster.zapt) |
| iOS         | [src/darwin/Framework/CHIP/templates/partials/test_cluster.zapt](../../../../src/darwin/Framework/CHIP/templates/partials/test_cluster.zapt)           |
| Python      |                                                                                                                                                        |
| Android     |                                                                                                                                                        |

These dedicated templates share a common
[script](../../zap-templates/common/ClusterTestGeneration.js) which augments the
test file with the content of the ZAP database built from the
[definition files](../../zap-templates/zcl) of the specification.

# Lists

This common script exposes the result of the analysis in the form of multiple
lists accessible from the dedicated template files.

These lists contain elements of which certain properties are directly
configurable through the source test file, but also additional properties added
during the analysis.

## List: _chip_tests_

**{{chip\_tests}}** takes as parameter a list of test files and returns an
iterable list of objects.

Each element of the list is an object with the following properties:

#### Property: [_test_](./examples/Test_Example.yaml)

| Name   | Description                                                                         | Required |
| ------ | ----------------------------------------------------------------------------------- | -------- |
| name   | Name of the test set in human readable format                                       | Yes      |
| config | Default configuration that is inherited by each of the tests in this set            | Yes      |
| tests  | The set of tests. It is possible to iterate over it using **{{chip\_test\_items}}** | Yes      |

#### Property: [_config_](#basic-yaml-config-example)

| Name            | Description                                                                                      | Required |
| --------------- | ------------------------------------------------------------------------------------------------ | -------- |
| cluster         | Name of the cluster listed in [Cluster Names](#index)                                            | No       |
| endpoint        | Endpoint Identifier that will be targeted by default by each of the tests in this set            | No       |
| identity        | Name of the controller to perform tests with. alpha, beta, and gamma are the only ones supported | No       |
| {variable_name} | Name of test variable to use within test.                                                        | No       |

##### Property: [_{variable_name}_](./examples/Config_Variables_Example.yaml)

| Name            | Description                                      |
| --------------- | ------------------------------------------------ |
| {variable_name} | Name of test variable to use within test.        |
| type            | Macro type from [ZAP atomics](#zap-atomic-types) |

Additionally, the object exhibits the following autogenerated properties:

| Name       | Description                                               |
| ---------- | --------------------------------------------------------- |
| filename   | Name of the source file from which the test set was built |
| totalTests | Total number of tests contained in this set               |

## List: _chip_tests_items_

**{{chip\_tests\_items}}** can be used inside a **{{chip\_tests}}** block and
iterates over the test set.

Each element of the list is an object with the following configurable
properties:

#### Property: [_tests_](./examples/Test_Example.yaml)

| Name                      | Description                                                                                                                                                                                                 | Required |
| ------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------- |
| label                     | Test name                                                                                                                                                                                                   | Yes      |
| disabled                  | Allows to deactivate a test                                                                                                                                                                                 | No       |
| command                   | Name of the command to execute. The command can be any of the commands available for the cluster, or a special command.                                                                                     | No       |
| attribute                 | If the command is a special attribute command, then this property contains the name of the attribute targeted by the command.                                                                               | No       |
| optional                  | Allows you to specify that a test is optional. An optional test passes if the controllee does not support the requested command or if the command is supported, and the specified conditions are validated. | No       |
| cluster                   | Name from [Cluster names](#index) . _If not specified, the default configuration is used._                                                                                                                  | No       |
| endpoint                  | The identifier of the target endpoint. _If not specified, the default configuration is used._                                                                                                               | No       |
| arguments                 | A list of arguments to pass to the command. It is possible to iterate over it using **{{chip\_test\_item\_parameters}}**                                                                                    | No       |
| response                  | A list of expected results or result constraints. It is possible to iterate over it using **{{chip\_test\_item\_response\_parameters}}**                                                                    | No       |
| PICS                      | Protocol Implementation Conformance Statement. A conditional flag that this determines whether a test step should run.                                                                                      | No       |
| timedInteractionTimeoutMs | Sets a timeout for Writing to attribute and sending a command.                                                                                                                                              | No       |

##### Property: [_arguments_](./examples/Test_Example.yaml)

| Name   | Description         | Required |
| ------ | ------------------- | -------- |
| values | A list of arguments | Yes      |

##### Property: [_values_](./examples/Test_Example.yaml)

| Name  | Description                                           | Required |
| ----- | ----------------------------------------------------- | -------- |
| name  | The name of the argument to be used                   | No       |
| value | The value of the argument to be used within test step | Yes      |

##### Property: [_response_](./examples/Test_Example.yaml)

| Name        | Description                                                                                                  | Required               |
| ----------- | ------------------------------------------------------------------------------------------------------------ | ---------------------- |
| values      | A list of values to expect on response. See [Property: value](#property-values)                              | No (If other provided) |
| value       | The values expected on the the response from the command                                                     | No (If other provided) |
| error       | Error code that is expected from response                                                                    | No (If other provided) |
| saveAs      | Save a value to a variable name for later use. Value above required. [Example](./examples/Test_Example.yaml) | No                     |
| constraints | Constraints to check for on response from the command                                                        | No                     |

##### Property: [_constraints_](../../../../src/app/tests/suites/TestConstraints.yaml)

| Name        | Description                                                 | Required               |
| ----------- | ----------------------------------------------------------- | ---------------------- |
| hasValue    | If true, must have value. If false, must not have value.    | No (If other provided) |
| minValue    | Minimum value to expect from the command response.          | No (If other provided) |
| maxValue    | Maximum value to expect from the command response.          | No (If other provided) |
| notValue    | Validate the the value is not what is provided.             | No (If other provided) |
| minLength   | Minimum length of the response parameter.                   | No (If other provided) |
| maxLength   | Maximum length of the string parameter.                     | No (If other provided) |
| startsWith  | Condition is which will validate what a string starts with. | No (If other provided) |
| endsWith    | Condition is which will validate what a string ends with.   | No (If other provided) |
| isLowerCase | Validates if the char_string is lower case.                 | No (If other provided) |
| isUpperCase | Validates if the char_string is upper case.                 | No (If other provided) |
| isHexString | Checks whether the char_string is a hex string.             | No (If other provided) |

Note: The hasValue constraint is only applied to optional fields. The other
constraints are ignored for optional fields that do not have a value.

Additionally, the object exhibits the following autogenerated properties:

| Name  | Description        |
| ----- | ------------------ |
| index | Overall test index |

## List: _chip_tests_item_parameters_

**{{chip\_tests\_item\_parameters}}** can be used inside a
**{{chip\_test\_items}}** block and iterates over the test arguments for the
target command.

This object contains all the properties contained in the ZAP database related to
the command or the attribute in the case of a special attribute command.

It is increased by the value to be used contained in the test file.

## List: _chip_tests_item_response_parameters_

**{{chip\_tests\_item\_response\_parameters}}** can be used inside a
**{{chip\_test\_items}}** block and iterates over the test response arguments
for the target command.

This object contains all the properties contained in the ZAP database related to
the response or the attribute response in the case of a special attribute
command.

It is increased by the expected value or the constraints to be used contained in
the test file.

## List: _chip_tests_pics_

**{{chip\_tests\_pics}}** can be used to get the collection of all PICS for a
given test.

## List: _chip_tests_config_

**{{chip\_tests\_config}}** can be used to get the collection of user defined
variables in the config. See [ZAP Tests Config](#examples) below

This object contains {{name}} {{definedValue}} and {{type}}.
(chip_tests_config_has definedValue) can be used to determine if there is a
value set for a particular user defined variable.

| Name         | Description                           |
| ------------ | ------------------------------------- |
| name         | The name of the user defined variable |
| type         | [ZAP Atomic Types](#index) Macro      |
| definedValue | The value of the user define variable |

# YAML Test Definition

YAML is used to generate tests into a programming language understood by
commissioners, controllers, or simulated devices. There is a define set of
keyword/values that are used to define test behavior.

YAML is the high level language chosen to generate test against a controller or
a controllee. [Test Definition Example](#yaml-examples)

Top level key to define a YAML test: [test](#property-test)

## Location of Test Definitions

-   [Simulated README](../../../../docs/guides/simulated_device_linux.md)
-   Simulated tests:
    [tests.js](../../../../examples/placeholder/linux/apps/app1/tests.js)

-   chip-tool(darwin) tests:
    [tests.js](../../../../examples/darwin-framework-tool/templates/tests/tests.js)

-   chip-tool tests:
    [tests.js](../../../../examples/chip-tool/templates/tests/tests.js)

    ```javascript
    // Manual test definitions. Does not run in CI.
    function  getManualTests();

    // Test definition that run in CI.
    function getTests();
    ```

# PICS Usage

#### Required Files

-   [PICS Test YAML](./examples/PICS_Example.yaml)
-   [TestPICS](./examples/TestPICS)

#### Example PICS Command

```
chip-tool tests Test 1 --PICS TestPICS.txt
```

# Examples

## YAML Examples

-   [Basic YAML Config Example](./examples/Config_Example.yaml)
-   [YAML Response Example](./examples/Response_Example.yaml)
-   [YAML Tests Example](./examples/Test_Example.yaml)
-   [YAML Config Variables Example](./examples/Config_Variables_Example.yaml)

## ZAP Example

### Required Files

-   [ZAP Example](./examples/TestGenExample.zapt)
-   [Test1](./examples/Test_Example_1.yaml)
-   [Test2](./examples/Test_Example_2.yaml)
-   [Test3](./examples/Test_Example_3.yaml)

### Generate Example Script

The following script will generate an output file TestGenExample.out in
src/app/tests/suites/examples/out. The directory and the file will be created.
This is a basic demonstration on how tests are generated.

```
src/app/tests/suites/examples/gen_readme_example.sh
```

# Index

<details><summary>Atomic Type Info</summary><p>

| Name              | Size | Macro             | ID   | Description              | C type     |
| ----------------- | ---- | ----------------- | ---- | ------------------------ | ---------- |
| no_data           | 0    | NO_DATA           | 0x00 | No data                  | uint8_t \* |
| boolean           | 1    | BOOLEAN           | 0x10 | Boolean                  | uint8_t    |
| bitmap8           | 1    | BITMAP8           | 0x18 | 8-bit bitmap             | uint8_t    |
| bitmap16          | 2    | BITMAP16          | 0x19 | 16-bit bitmap            | uint16_t   |
| bitmap32          | 4    | BITMAP32          | 0x1B | 32-bit bitmap            | uint32_t   |
| bitmap64          | 8    | BITMAP64          | 0x1F | 64-bit bitmap            | uint8_t \* |
| int8u             | 1    | INT8U             | 0x20 | Unsigned 8-bit integer   | uint8_t    |
| int16u            | 2    | INT16U            | 0x21 | Unsigned 16-bit integer  | uint16_t   |
| int24u            | 3    | INT24U            | 0x22 | Unsigned 24-bit integer  | uint32_t   |
| int32u            | 4    | INT32U            | 0x23 | Unsigned 32-bit integer  | uint32_t   |
| int40u            | 5    | INT40U            | 0x24 | Unsigned 40-bit integer  | uint8_t \* |
| int48u            | 6    | INT48U            | 0x25 | Unsigned 48-bit integer  | uint8_t \* |
| int56u            | 7    | INT56U            | 0x26 | Unsigned 56-bit integer  | uint8_t \* |
| int64u            | 8    | INT64U            | 0x27 | Unsigned 64-bit integer  | uint8_t \* |
| int8s             | 1    | INT8S             | 0x28 | Signed 8-bit integer     | int8_t     |
| int16s            | 2    | INT16S            | 0x29 | Signed 16-bit integer    | int16_t    |
| int24s            | 3    | INT24S            | 0x2A | Signed 24-bit integer    | int32_t    |
| int32s            | 4    | INT32S            | 0x2B | Signed 32-bit integer    | int32_t    |
| int40s            | 5    | INT40S            | 0x2C | Signed 40-bit integer    | int8_t \*  |
| int48s            | 6    | INT48S            | 0x2D | Signed 48-bit integer    | int8_t \*  |
| int56s            | 7    | INT56S            | 0x2E | Signed 56-bit integer    | int8_t \*  |
| int64s            | 8    | INT64S            | 0x2F | Signed 64-bit integer    | int8_t \*  |
| enum8             | 1    | ENUM8             | 0x30 | 8-bit enumeration        | uint8_t    |
| enum16            | 2    | ENUM16            | 0x31 | 16-bit enumeration       | uint16_t   |
| single            | 4    | SINGLE            | 0x39 | Single precision         | uint8_t \* |
| double            | 8    | DOUBLE            | 0x3A | Double precision         | uint8_t \* |
| octet_string      |      | OCTET_STRING      | 0x41 | Octet String             | uint8_t \* |
| char_string       |      | CHAR_STRING       | 0x42 | Character String         | uint8_t \* |
| long_octet_string |      | LONG_OCTET_STRING | 0x43 | Long Octet String        | uint8_t \* |
| long_char_string  |      | LONG_CHAR_STRING  | 0x44 | Long Character String    | uint8_t \* |
| array             |      | ARRAY             | 0x48 | List                     | uint8_t \* |
| struct            |      | STRUCT            | 0x4C | Structure                | uint8_t \* |
| tod               | 4    | TOD               | 0xE0 | Time of day              | uint8_t \* |
| date              | 4    | DATE              | 0xE1 | Date                     | uint32_t   |
| utc               | 4    | UTC               | 0xE2 | UTC Time                 | uint8_t \* |
| epoch_us          | 8    | EPOCH_US          | 0xE3 | Epoch Microseconds       | uint8_t \* |
| epoch_s           | 4    | EPOCH_S           | 0xE4 | Epoch Seconds            | uint8_t \* |
| systime_us        | 8    | SYSTIME_US        | 0xE5 | System Time Microseconds | uint8_t \* |
| percent           | 1    | PERCENT           | 0xE6 | Percentage units 1%      | uint8_t \* |
| percent100ths     | 2    | PERCENT100THS     | 0xE7 | Percentage units 0.01%   | uint8_t \* |
| cluster_id        | 4    | CLUSTER_ID        | 0xE8 | Cluster ID               | uint16_t   |
| attrib_id         | 4    | ATTRIB_ID         | 0xE9 | Attribute ID             | uint8_t \* |
| field_id          | 4    | FIELD_ID          | 0xEA | Field ID                 | uint8_t \* |
| event_id          | 4    | EVENT_ID          | 0xEB | Event ID                 | uint8_t \* |
| command_id        | 4    | COMMAND_ID        | 0xEC | Command ID               | uint8_t \* |
| action_id         | 1    | ACTION_ID         | 0xED | Action ID                | uint8_t \* |
| trans_id          | 4    | TRANS_ID          | 0xEF | Transaction ID           | uint8_t \* |
| node_id           | 8    | NODE_ID           | 0xF0 | Node ID                  | uint8_t \* |
| vendor_id         | 2    | VENDOR_ID         | 0xF1 | Vendor ID                | uint8_t \* |
| devtype_id        | 4    | DEVTYPE_ID        | 0xF2 | Device Type ID           | uint8_t \* |
| fabric_id         | 8    | FABRIC_ID         | 0xF3 | Fabric ID                | uint8_t \* |
| group_id          | 2    | GROUP_ID          | 0xF4 | Group ID                 | uint8_t \* |
| status            | 2    | STATUS            | 0xF5 | Status Code              | uint8_t \* |
| data_ver          | 4    | DATA_VER          | 0xF6 | Data Version             | uint_ver_t |
| event_no          | 8    | EVENT_NO          | 0xF7 | Event Number             | uint8_t \* |
| endpoint_no       | 2    | ENDPOINT_NO       | 0xF8 | Endpoint Number          | uint8_t \* |
| fabric_idx        | 1    | FABRIC_IDX        | 0xF9 | Fabric Index             | uint8_t \* |
| ipadr             |      | IPADR             | 0xFA | IP Address               | uint8_t \* |
| ipv4adr           | 4    | IPV4ADR           | 0xFB | IPv4 Address             | uint8_t \* |
| ipv6adr           | 16   | IPV6ADR           | 0xFC | IPv6 Address             | uint8_t \* |
| ipv6pre           |      | IPV6PRE           | 0xFD | IPv6 Prefix              | uint8_t \* |
| hwadr             |      | HWADR             | 0xFE | Hardware Address         | uint8_t \* |
| unknown           | 0    | UNKNOWN           | 0xFF | Unknown                  | uint8_t \* |

</details>
