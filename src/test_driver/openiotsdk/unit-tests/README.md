# Matter Open IoT Unit Tests Application

The Open IoT SDK Unit Tests Application executes all supported unit tests on the
target.

The Matter unit tests are included in a set of libraries and allow to validate
most of the components used by Matter examples applications. The main goal of
this application is to run registered tests on Open IoT SDK target and check the
results. The final result is the number of tests that failed.

## Environment setup

The required environment is the same as for the Matter examples. For information
on how to setup it see
[Open IoT SDK examples](../../../../docs/examples/openiotsdk_examples.md#Environment-setup).

## Building

The build process means creating separate executable file for each Matter tested
component. It assumes the use of all supported test libraries and creating
independent applications from them.

You build using a vscode task or call the script directly from the command line.

### Building using vscode task

```
Command Palette (F1) => Run Task... => Build Open IoT SDK unit-tests => (debug on/off)
```

This will call the scripts with the selected parameters.

### Building using CLI

You can call the script directly yourself.

```
${MATTER_ROOT}/scripts/examples/openiotsdk_example.sh unit-tests
```

Use `--help` to get more information about the script options.

## Running

Unit-tests applications can be run independently or as an entire set. It runs in
the background and opens a telnet session. The script will open telnet for you
and connect to the port used by the `FVP`. When the telnet process is terminated
it will also terminate the `FVP` instance.

You can run the application script from a vscode task or call the script
directly.

Expected output of each executed test:

```
 [ATM] Open IoT SDK unit-tests start
 [ATM] Open IoT SDK unit-tests run...
 ...
 [ATM] Test status: 0
```

### Running using vscode task

```
Command Palette (F1) => Run Task... => Run Open IoT SDK unit-tests => <test name> or all (to run all tests)
```

### Running using CLI

You can call the script directly yourself.

```
${MATTER_ROOT}/scripts/examples/openiotsdk_example.sh -C run unit-tests <test name> (optional to run specific test)
```

## Debugging

Debugging can be started using a VS code launch task:

```
Run and Debug (Ctrl+Shift+D) => Debug Open IoT SDK unit-tests application => Start Debugging (F5) => <test name> => <test name>
```

As you can see above, you will need to select the name of the test twice. This
is because the debug task needs to launch the run task and currently VS code has
no way of passing parameters between tasks.
