# Matter Open IoT SDK All-Clusters-App Example Application

The Open IoT SDK All Clusters Example demonstrates various ZCL clusters control.

The example behaves as a Matter accessory, device that can be paired into an
existing Matter network and can be controlled by it.

You can use this example as a reference for creating your own application.

## Build-run-test-debug

For information on how to build, run, test and debug this example and further
information about the platform it is run on see
[Open IoT SDK examples](../../../docs/guides/openiotsdk_examples.md).

The example name to use in the scripts is `all-clusters-app`.

## Example output

When the example runs, these lines should be visible:

```
[INF] [-] Open IoT SDK all-clusters-app example application start
...
[INF] [-] Open IoT SDK all-clusters-app example application run
```

This means the all-clusters-app application launched correctly and you can
follow traces in the terminal.

### Commissioning

Read the
[Open IoT SDK commissioning guide](../../../docs/guides/openiotsdk_commissioning.md)
to see how to use the Matter controller to commission and control the
application.

### AccessControl cluster usage

The application fully supports the AccessControl cluster. For more details about
access control please visit
[Access Control Guide](../../../docs/guides/access-control-guide.md). Use
cluster commands to trigger actions on the device. You can issue commands
through the same Matter controller you used to perform the commissioning step
above.

Example command:

```
chip-tool accesscontrol read acl 123 0
```

The numeric arguments are: device node ID and device endpoint ID, respectively.

The device sent a response and you should see this line in the controller
output:

```
CHIP:TOO: Endpoint: 0 Cluster: 0x0000_001F Attribute 0x0000_0000 DataVersion: 3442030892
CHIP:TOO:   ACL: 1 entries
CHIP:TOO:     [1]: {
CHIP:TOO:       Privilege: 5
CHIP:TOO:       AuthMode: 2
CHIP:TOO:       Subjects: 1 entries
CHIP:TOO:         [1]: 112233
CHIP:TOO:       Targets: null
CHIP:TOO:       FabricIndex: 1
CHIP:TOO:      }
```

These are automatically installed ACL entries after commissioning.

### BasicInformation cluster usage

One of the fully supported clusters by this example is BasicInformation cluster.
Use cluster commands to trigger actions on the device. You can issue commands
through the same Matter controller you used to perform the commissioning step
above.

Example command:

```
chip-tool basicinformation read vendor-id 123 0
```

The numeric arguments are: device node ID and device endpoint ID, respectively.

The device send a response with its vendor ID number and you should see this
line in the controller output:

```
CHIP:TOO:   VendorID: 65521
```

The `65521` value is the default `vendor ID` for Matter examples.

**NOTE**

More details about the `chip-tool` controller can be found
[here](../../chip-tool/README.md).
