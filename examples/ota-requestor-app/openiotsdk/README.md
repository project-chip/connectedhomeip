# Matter Open IoT SDK OTA-Requestor-App Example Application

The Open IoT SDK OTA-Requestor Example demonstrates how to remotely trigger
update image downloading and apply it if needed. It provides the service for
Matter's `OTA` clusters. This application plays both roles: the server for the
`OTA Requestor` cluster, and the client of the `OTA Provider` cluster. It can
initiate a software update with a given `OTA Provider` node, download a binary
file and apply it.

The application is configured to support:

-   [TF-M](../../../docs/guides/openiotsdk_examples.md#trusted-firmware-m)
-   [Device Firmware Update](../../../docs/guides/openiotsdk_examples.md#device-firmware-update)

The example behaves as a Matter accessory, device that can be paired into an
existing Matter network and can be controlled by it.

## Build-run-test-debug

For information on how to build, run, test and debug this example and further
information about the platform it is run on see
[Open IoT SDK examples](../../../docs/guides/openiotsdk_examples.md).

The example name to use in the scripts is `ota-requestor-app`.

## Example output

When the example runs, these lines should be visible:

```
[INF] [-] Open IoT SDK ota-requestor-app example application start
...
[INF] [-] Open IoT SDK ota-requestor-app example application run
```

This means the ota-requestor-app application launched correctly and you can
follow traces in the terminal.

### Commissioning

Read the
[Open IoT SDK commissioning guide](../../../docs/guides/openiotsdk_commissioning.md)
to see how to use the Matter controller to commission and control the
application.

### Device Firmware Upgrade

Read the
[Matter Open IoT SDK Example Device Firmware Upgrade](../../../docs/guides/openiotsdk_examples_software_update.md)
to see how to use Matter OTA for firmware update.

### OtaSoftwareUpdateRequestor cluster usage

The application fully supports the `OTA Requestor` cluster. Use its commands to
trigger actions on the device. You can issue commands through the same Matter
controller you used to perform the commissioning step above.

Example command:

```
chip-tool otasoftwareupdaterequestor announce-ota-provider 1234 0 2 0 4321 0
```

The `OTA Requestor` application with node ID 1234 will process this command and
send a `QueryImage` command to the `OTA Provider` with node ID 4321. This starts
the `OTA` process. On receiving the `QueryImageResponse` from the `OTA Provider`
application, the `OTA Requestor` application will verify that the software
version specified in the `SoftwareVersion` field of the response contains a
value newer than the current running version. The available version will be
printed to the terminal, for example:

```
[INF] [-] New version of the software is available: 2
```

If the validation does not pass the update will not proceed. The next step is
downloading the update image. If this step is completed, a new image will be
installed and the application will be reboot.

More details about device firmware update over Matter can be found
[here](../../../docs/guides/openiotsdk_examples_software_update.md).
