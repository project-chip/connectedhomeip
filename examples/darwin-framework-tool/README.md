# Matter darwin-framework-tool

An example application that uses Matter to send messages to a Matter server.

IMPORTANT: Must have an Apple developer signed certificate. Information can be
found at [code-signing](https://developer.apple.com/support/code-signing/).

---

-   [Building the Example Application](#building-the-example-application)
-   [Using the Client to Commission a Device](#using-the-client-to-commission-a-device)

---

## Building the Example Application

See [the build guide](../../docs/guides/BUILDING.md#prerequisites) for general
background on build prerequisites.

Building the example application is quite straightforward.

```
scripts/examples/gn_build_example.sh examples/darwin-framework-tool SOME-PATH/
```

which puts the binary at `SOME-PATH/darwin-framework-tool`.

## Using the Client to commission a device

In order to send commands to a device, it must be commissioned with the client.
darwin-framework-tool currently only supports commissioning and remembering one
device at a time. The configuration state is stored in
`/tmp/chip_tool_config.ini`; deleting this and other `.ini` files in `/tmp` can
sometimes resolve issues due to stale configuration.

#### Commission a device

To initiate a client commissioning request to a device, run the built executable
and choose the pairing mode.

#### Pair a device over IP

The command below will pair devices with the provided IP, discriminator and
setup code.

    $ darwin-framework-tool pairing ethernet {NODE_ID_TO_ASSIGN} 20202021 3840 {IP_ADDRESS}

In this case, the device will be assigned node id `${NODE_ID_TO_ASSIGN}` (which
must be a decimal number or a 0x-prefixed hex number).

### Forget the currently-commissioned device

    $ darwin-framework-tool pairing unpair

## Using the Client to Send Matter Commands

To use the Client to send Matter commands, run the built executable and pass it
the target cluster name, the target command name as well as an endpoint id.

The endpoint id must be between 1 and 240.

    $ darwin-framework-tool onoff on 1

The client will send a single command packet and then exit.

### How to get the list of supported clusters

To get the list of supported clusters, run the built executable without any
arguments.

    $ darwin-framework-tool

Example output:

```bash
Usage:
  ./darwin-framework-tool cluster_name command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Clusters:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * basic                                                                             |
  | * colorcontrol                                                                      |
  | * doorlock                                                                          |
  | * groups                                                                            |
  | * iaszone                                                                           |
  | * identify                                                                          |
  | * levelcontrol                                                                      |
  | * onoff                                                                             |
  | * pairing                                                                           |
  | * payload                                                                           |
  | * scenes                                                                            |
  | * temperaturemeasurement                                                            |
  +-------------------------------------------------------------------------------------+
```

### How to get the list of supported commands for a specific cluster

To get the list of commands for a specific cluster, run the built executable
with the target cluster name.

    $ darwin-framework-tool onoff

### How to get the list of supported attributes for a specific cluster

To the the list of attributes for a specific cluster, run the built executable
with the target cluster name and the `read` command name.

    $ darwin-framework-tool onoff read

### How to get the list of parameters for a command

To get the list of parameters for a specific command, run the built executable
with the target cluster name and the target command name

    $ darwin-framework-tool onoff on

## Using Interactive mode

To start the interactive mode run the following command:

    $ darwin-framework-tool interactive start

Once in interactive mode, 'help' will display commands available

## Using the OTA Software Update app

OTA SW app will only work in interactive mode. In interactive mode there will be
an additional command 'otasoftwareupdateapp'. Running the following command in
interactive will display available commands.

    $ otasoftwareupdateapp

The following json is an example of a list of candidates to set in interactive
mode with `otasoftwareupdateapp candidate-file-path`:

```json
{
    "deviceSoftwareVersionModel": [
        {
            "vendorId": 65521,
            "productId": 32769,
            "softwareVersion": 10,
            "softwareVersionString": "1.0.0",
            "cDVersionNumber": 18,
            "softwareVersionValid": true,
            "minApplicableSoftwareVersion": 0,
            "maxApplicableSoftwareVersion": 100,
            "otaURL": "/Users/josh/Desktop/OTACandidates/ota_v10.bin"
        }
    ]
}
```

darwin-framework-tool allows to set the consent status on the Provider side with
the following command:

    $ otasoftwareupdateapp set-consent-status [granted, obtaining, denied]

By default, the consent will be set to unknown and the requestor will have to
consent. If the requestor cannot consent, the update will be denied.
