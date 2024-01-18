# Matter Open IoT SDK TV-App Example Application

The Open IoT SDK TV Example is an example implementation of the Casting Video
Player device (refer to Video Player Architecture in the Matter Device Library
Specification document). It supports basic and content launching features. The
example includes a dummy application platform with a few hardcoded apps which
use the pre-defined settings to work.

The TV example also functions as a commissionable node, device that can be
paired into an existing Matter network and can be controlled by it.

> 💡 **Notes**: The Open IoT SDK platform implementation does not yet support
> the commissioner role, which means that some of its features are unavailable
> for this example. Therefore, it does not work properly with the Casting Video
> Client node like
> [CHIP TV Casting App Example](../../tv-casting-app/linux/README.md).

## Build-run-test-debug

For information on how to build, run, test and debug this example and further
information about the platform it is run on see
[Open IoT SDK examples](../../../docs/guides/openiotsdk_examples.md).

The example name to use in the scripts is `tv-app`.

## Example output

When the example runs, these lines should be visible:

```
[INF] [-] Open IoT SDK tv-app example application start
...
[INF] [-] Open IoT SDK tv-app example application run
```

This means the `tv-app` application launched correctly and you can follow traces
in the terminal.

## Shell commands

The application supports Matter shell commands that allow managing the
application platform (see `examples/tv-app/tv-common/src/AppTvShellCommands.cpp`
implementation). Pass commands to the terminal and wait for the response.

Example:

```
> app add 1
[INF] [DL] GetLoadContentAppByVendorId() - vendorId 1, productId 0
[INF] [DL] ContentAppFactoryImpl: LoadContentAppByAppId catalogVendorId=65521 applicationId=1
[INF] [DL]  Looking next=1
[INF] [DL] Adding ContentApp with appid 1
[INF] [ZCL] TV Linux App: ApplicationBasic::SetDefaultDelegate
[INF] [ZCL] TV Linux App: KeypadInput::SetDefaultDelegate
[INF] [ZCL] TV Linux App: ApplicationLauncher::SetDefaultDelegate
[INF] [ZCL] TV Linux App: AccountLogin::SetDefaultDelegate
[INF] [ZCL] TV Linux App: ContentLauncher::SetDefaultDelegate
[INF] [ZCL] TV Linux App: MediaPlayback::SetDefaultDelegate
[INF] [ZCL] TV Linux App: TargetNavigator::SetDefaultDelegate
[INF] [ZCL] TV Linux App: Channel::SetDefaultDelegate
[INF] [DL] Added ContentApp 1 to dynamic endpoint 4 (index=0)
[INF] [DL] added app
Done
```

Run the `app help` command to get all supported commands and their usage.

## Commissioning

Read the
[Open IoT SDK commissioning guide](../../../docs/guides/openiotsdk_commissioning.md)
to see how to use the Matter controller to commission and control the
application.

### ApplicationBasic cluster usage

One of the fully supported clusters by this example is `ApplicationBasic`
cluster. Use cluster commands to trigger actions on the device. You can issue
commands through the same Matter controller you used to perform the
commissioning step above.

Before using the cluster commands, you need to add the application to the
platform using shell commands:

```
> app add 1
```

Example command:

```
chip-tool applicationbasic read vendor-name 123 4
```

The numeric arguments are: device node ID and device endpoint ID, respectively.

The endpoint ID argument is the dynamically assigned endpoint that is associated
with the added application. You can refer to the output logs after completing
the application adding step to find this value, for example:

```
[INF] [DL] Added ContentApp 1 to dynamic endpoint 4 (index=0)
```

The device send a response with its vendor name and you should see this line in
the controller output:

```
CHIP:TOO:   VendorName: Vendor1
```

The `Vendor1` is the hardcoded value for this example.

### ApplicationLauncher cluster usage

The `Application Launcher` is the next example of the supported cluster. Use its
commands to trigger actions on the device. You can issue commands through the
same Matter controller you used to perform the commissioning step above.

Before using the cluster commands, you need to add the application to the
platform using shell commands:

```
> app add 1
```

Example command:

```
chip-tool applicationlauncher launch-app '{"catalogVendorID": 1, "applicationID": "1"}' 123 4
```

The numeric arguments are: device node ID and device endpoint ID, respectively.

The endpoint ID argument is the dynamically assigned endpoint that is associated
with the added application. You can refer to the output logs after completing
the application adding step to find this value, for example:

```
[INF] [DL] Added ContentApp 1 to dynamic endpoint 4 (index=0)
```

`catalogVendorID` and `applicationID` parameters are also printed after
completing the application adding step, for example:

```
[INF] [DL] ContentAppFactoryImpl: LoadContentAppByAppId catalogVendorId=65521 applicationId=1
```

The device send a response that is printed to the controller output:

```
CHIP:TOO:   LauncherResponse: {
CHIP:TOO:     status: 0
CHIP:TOO:     data: 64617461
CHIP:TOO:    }
```

The example application confirms handling the command with the log:

```
[INF] [ZCL] ApplicationLauncherManager::HandleLaunchApp
```
