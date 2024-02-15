# Simulated Device How-To (Linux)

This document contains instructions on how to build, run, and interact with a
simulated device. All virtual accessories live in
[examples/placeholder/linux/apps](https://github.com/project-chip/connectedhomeip/tree/master/examples/placeholder/linux/apps).

Each accessory needs to be hosted into a subfolder. It will be the name of the
application. For example `app1` will create a binary named `chip-app1`.

If some parameters need to be overridden, a `CHIPProjectConfig.h` file can be
placed under an ‘include’ folder into the app folder. For example
`examples/placeholder/linux/apps/app1/include/CHIPProjectConfig.h`

Simulated Device: simulation of an application in which tests can be added. It
is defined by a ZAP config file and tests can be added with a
[YAML file](../../src/app/tests/suites/certification/Test_TC_DM_1_3_Simulated.yaml).

### Prerequisite

-   [Building Prerequisites](./BUILDING.md#prerequisites)
-   [Prepare For Building](./BUILDING.md#prepare-for-building)
-   [Code Generate](../code_generation.md)
-   [ZAP Installed](../code_generation.md#installing-zap-and-environment-variables)

## Building the default Simulated App with Script

In order to utilize the app against a commissioner or controller, the app will
need to be specifically built.

1. To build the `chip-app1` binary completing the following steps:

    ```
    ./scripts/examples/gn_build_example.sh examples/placeholder/linux out/debug/simulated/ chip_tests_zap_config=\"app1\"
    ```

## Build the App with gn and ninja (alternative)

In order to utilize the app against a commissioner or controller, the app will
need to be specifically built.

1. To only build the `chip-app1` binary completing the following steps:

    ```
    source scripts/activate.sh
    gn gen --check --root=examples/placeholder/linux out/simulated --args="chip_tests_zap_config=\"app1\""
    ninja -C out/simulated
    ```

## Running the app

Now that the building is completed there is a `chip-app1` binary created. This
binary can be executed on a linux os.

```
./out/debug/simulated/chip-app1
```

## Running the app with test parameter

Now that the building is completed there is a `chip-app1` binary created. This
binary can be executed on a linux os with test commands.

```
./scripts/tests/yaml/runner.py [TEST NAME] app1
```

## Interacting with the simulated app

Now that the building the app and starting it is complete, you will be able to
interact with it using chip-tool

1. Follow the instruction to build chip-tool in the
   [chip-tool readme](../../examples/chip-tool/README.md).

2. Run this command to commission with whatever is listed on the "SetupQRCode:"
   line in the log output:

    ```
      ./out/debug/standalone/chip-tool pairing code 0x654321 MT:-24J0AFN00KA0648G00
    ```

3. Most tests will start at this point and now an send cluster commands with
   chip-tool as follow.

    ```
    ./out/debug/standalone/chip-tool onoff on 0x654321 1
    ./out/debug/standalone/chip-tool onoff read on-off 0x654321 1
    ./out/debug/standalone/chip-tool onoff write on-time 1 0x654321 1
    ```

    See [chip-tool readme](../../examples/chip-tool/README.md) for additional
    commands.

## Adding simulated Tests via YAML

In order to validate commissioner/controller behavior, tests need to be added to
the simulated device test framework. To achieve this, YAML files are created.

1. YAML test file are located in
   [YAML folder](../../src/app/tests/suites/certification/)

2. Test names must follow a strict format dues to CI of test recognition. The
   format is as follows:

    - Test_TC\_[`CATEGORY ABBREVIATION`]\_[`SECTION NUMBER`]\_[`SUBSECTION
      NUMBER`]\_Simulated.yaml
    - <strong>`IMPORTANT`: The test name must end in Simulated with the
      capital.</strong>

3. Available properties can be found in
   [YAML Test Name](../../src/app/tests/suites/README.md)

4. An Additional property is as follows:

    | Name | Description                                                                 |
    | ---- | --------------------------------------------------------------------------- |
    | wait | The command that is expected to be received on the app from the controller. |

5. [Test_TC_DM_1_3_Simulated](../../src/app/tests/suites/certification/Test_TC_DM_1_3_Simulated.yaml)
   is an example of a written test that runs on the simulated device.
