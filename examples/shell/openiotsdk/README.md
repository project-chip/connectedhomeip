# Matter Open IoT SDK Shell Example Application

The example exposes configuration and management APIs via a command line
interface. It parses a command line and calls the corresponding service
execution. There is a set of common shell commands which perform basic device
operations.

For more details see
[Common shell commands](../README.md#matter-shell-command-details).

## Build and run

For information on how to build and run this example and further information
about the platform it is run on see
[Open IoT SDK examples](../../../docs/examples/openiotsdk_examples.md).

The example name to use in the scripts is `shell`.

## Using the example

Communication with the application goes through the active telnet session. When
the application runs these lines should be visible:

```
[INF] [SH] Open IoT SDK shell example application start
[INF] [SH] Open IoT SDK shell example application run
>
```

The shell application launched correctly.

Pass commands to the terminal and wait for the response. The application
supports common Matter shell commands. They are used to control the basic
functionalities of the device.

For more details read:
[Common shell commands](../README.md#matter-shell-command-details)

Example:

```
> echo Hello
Hello
Done
```
