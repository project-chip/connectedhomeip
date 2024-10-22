# Matter Open IoT SDK unit tests

The unit testing approach is to create a separate application with Matter test
library dependence. Each Matter project component implements the set of unit
tests that are located in the `test` directory, e.g. `src/inet/tests`. Those
sources are built as a static library that can be linked to the unit test
application separately or as a monolithic test library. The common Matter test
library collects all test cases and provides the engine based on
[Pigweed Unit Test](https://pigweed.dev/pw_unit_test) to run them in the
application.

The Open IoT SDK unit tests implementation are located in the
`src/test_driver/openiotsdk/unit-tests` directory. This project builds a
separate application for each Matter component that is tested. It's built using
[Open IoT SDK](https://gitlab.arm.com/iot/open-iot-sdk) and run inside an
emulated target through the
[Arm FVP model for the Corstone-300 MPS3](https://developer.arm.com/downloads/-/arm-ecosystem-fvps).

The list of currently supported Matter's component tests:

```
accesstest
AppDataModelTests
AppTests
ASN1Tests
BDXTests
ChipCryptoTests
ControllerDataModelTests
CoreTests
CredentialsTest
ICDServerTests
InetLayerTests
MdnsTests
MessagingLayerTests
MinimalMdnsCoreTests
MinimalMdnsRecordsTests
MinimalMdnsRespondersTests
PlatformTests
RawTransportTests
RetransmitTests
SecureChannelTests
SetupPayloadTests
SupportTests
SystemLayerTests
TestShell
UserDirectedCommissioningTests
```

Each application links the specific Matter test library, executes registered
tests and prints the result which is the number of tests that failed.

## Environment setup

The required environment is the same as for the Matter examples. For information
on how to set it up see
[Open IoT SDK examples environment](./openiotsdk_examples.md#environment-setup).

## Configuration

The configuration options are the same as for the Matter examples. For
information on how to configure unit-tests applications see
[Open IoT SDK examples configuration](./openiotsdk_examples.md#configuration).

## Building

The build process means creating a separate executable file for each Matter
tested component. It assumes the use of all supported test libraries and
creating independent applications from them.

You can build unit tests by using a VSCode task or by calling the build script
directly from the command line.

### Building using the VSCode task

-   Open the Command Palette: <kbd>F1</kbd>
-   Select `Tasks: Run Task`
-   Select `Build Open IoT SDK unit-tests`
-   Decide on debug mode support
-   Decide on LwIP debug logs support
-   Choose crypto algorithm
-   Choose socket API

This will call the script with the selected parameters.

### Building using CLI

You can call the script directly yourself.

```
${MATTER_ROOT}/scripts/examples/openiotsdk_example.sh unit-tests
```

Use `--help` to get more information about the script options.

## Running

Unit-tests applications are run independently. It runs in the background and
opens a telnet session. The telnet client connects to the port used by the
`FVP`. When the telnet process is terminated it will also terminate the `FVP`
instance.

To exit the telnet session, type <kbd>CTRL + ]</kbd>. This changes the command
prompt to show as:

```
telnet>
```

Back in the terminal, type in the word 'close' to terminate the session.

```
telnet> close
```

You can run specific unit test by using a VSCode task or by calling the run
script directly from the command line.

### Running using the VSCode task

-   Open the Command Palette: <kbd>F1</kbd>
-   Select `Tasks: Run Task`
-   Select `Run Open IoT SDK unit-tests`
-   Choose unit test name

This will call the script with the selected example name.

### Running using CLI

You can call the script directly yourself.

```
${MATTER_ROOT}/scripts/examples/openiotsdk_example.sh -C run unit-tests <unit test name>
```

## Testing

Run the Pytest integration test for the specific unit test application.

The test result can be found in the
`src/test_driver/openiotsdk/integration-tests/unit-tests/test_report_<unit test name>.json`
file.

You can execute the integration test for specific unit test by using a VSCode
task or by calling the run script directly from the command line.

### Testing using the VSCode task

-   Open the Command Palette: <kbd>F1</kbd>
-   Select `Tasks: Run Task`
-   Select `Test Open IoT SDK unit-tests`
-   Choose unit test name

This will call the scripts with the selected example name.

### Testing using CLI

You can call the script directly yourself.

```
${MATTER_ROOT}/scripts/examples/openiotsdk_example.sh -C test unit-tests <unit test name>
```

> 💡 **Notes**:
>
> Use `test` command without a specific test name, runs all supported unit
> tests:
>
> `${MATTER_ROOT}/scripts/examples/openiotsdk_example.sh -C test unit-tests`

## Debugging

Before debugging ensure the following:

1. The debug environment is correctly setup:
   [debugging setup](./openiotsdk_examples.md#debugging-setup).

2. The unit tests are compiled with debug symbols enabled:

    For CLI:

    ```
    ${MATTER_ROOT}/scripts/examples/openiotsdk_example.sh -d true unit-tests
    ```

    For the VSCode task:

    ```
    => Use debug mode (true)
    ```

You can debug the specific unit test by using a VSCode launch task:

-   Click `Run and Debug` from the primary side menu or press
    <kbd>Ctrl+Shift+D</kbd>
-   Select `Debug Open IoT SDK unit-tests application` from the drop down list
-   Click `Start Debugging`(green triangle) or press <kbd>F5</kbd>
-   Choose unit test name twice

As soon as a debugging session starts, the `DEBUG CONSOLE` panel is displayed
and shows the debugging output. Use debug controls to debug the current
application.

The application with GDB Remote Connection Plugin runs in the background and
opens a telnet session in terminal. The telnet client connects to the port used
by the `FVP`. When the telnet process is terminated it will also terminate the
`FVP` instance.

To exit the telnet session, type <kbd>CTRL + ]</kbd>. This changes the command
prompt to show as:

```
telnet>
```

Back in the terminal, type in the word 'close' to terminate the session.

```
telnet> close
```

> 💡 **Notes**:
>
> As you can see above, you will need to select the name of the unit test twice.
> This is because the debug task needs to launch the run task and currently VS
> code has no way of passing parameters between tasks.

## Add existing Matter's component test

To to add an existing Matter's component test to unit tests project, extend the
list in the `src/test_driver/openiotsdk/unit-tests/test_components.txt` file
with a test name (`test_name`). After that, the new test is built and available
in all necessary tools such as helper script
`scripts/examples/openiotsdk_example.sh` or VSCode tasks.

Example:

```
...
test_name
...
```

> 💡 **Notes**:
>
> The existing Matter's component tests are built as a separate libraries. The
> `src/BUILD.gn` GN project collects them in the target group. Make sure that
> the test you want to add is not skipped for the Open IoT SDK platform.
>
> Remember to update the list of supported Matter's component tests at the top
> of this document.
