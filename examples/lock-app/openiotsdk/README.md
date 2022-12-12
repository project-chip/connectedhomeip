# Matter Open IoT SDK Lock-App Example Application

The Open IoT SDK Lock Example demonstrates how to remotely control a door lock a
device with one basic bolt.

The example behaves as a Matter accessory, device that can be paired into an
existing Matter network and can be controlled by it.

## Build and run

For information on how to build and run this example and further information
about the platform it is run on see
[Open IoT SDK examples](../../../docs/examples/openiotsdk_examples.md).

The example name to use in the scripts is `lock-app`.

## Using the example

Communication with the application goes through the active telnet session. When
the application runs these lines should be visible:

```
[INF] [-] Open IoT SDK lock-app example application start
...
[INF] [-] Open IoT SDK lock-app example application run
```

The lock-app application launched correctly and you can follow traces in the
terminal.

### Commissioning

Read the
[Open IoT SDK commissioning guide](../../../docs/guides/openiotsdk_commissioning.md)
to see how to use the Matter controller to commission and control the
application.

### DoorLock cluster usage

The application fully supports the DoorLock cluster. Use its commands to trigger
actions on the device. You can issue commands through the same Matter controller
you used to perform the commissioning step above.

Example command:

```
zcl DoorLock LockDoor 1234 1 pinCode=str:12345
```

In response the device will output this line to the terminal:

```
[INF] [ZC] Lock App: specified PIN code was found in the database, setting door lock state to "Locked" [endpointId=1]
```
