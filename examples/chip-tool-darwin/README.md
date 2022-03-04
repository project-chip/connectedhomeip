# Matter chip-tool-darwin

An example application that uses Matter to send messages to a Matter server.

IMPORTANT: Must have an Apple developer signed certificate. Information can be
found at [code-signing](https://developer.apple.com/support/code-signing/).

---

-   [Building the Example Application](#building-the-example-application)
-   [Using the Client to Request an Echo](#using-the-client-to-request-an-echo)

---

## Building the Example Application

See [the build guide](../../docs/guides/BUILDING.md#prerequisites) for general
background on build prerequisites.

Building the example application is quite straightforward.

```
scripts/examples/gn_build_example.sh examples/chip-tool-darwin SOME-PATH/
```

which puts the binary at `SOME-PATH/chip-tool-darwin`.

## Using the Client to commission a device

In order to send commands to a device, it must be commissioned with the client.
chip-tool-darwin currently only supports commissioning and remembering one
device at a time. The configuration state is stored in
`/tmp/chip_tool_config.ini`; deleting this and other `.ini` files in `/tmp` can
sometimes resolve issues due to stale configuration.

#### Commission a device

To initiate a client commissioning request to a device, run the built executable
and choose the pairing mode.

#### Pair a device over IP

The command below will pair devices with the provided IP, discriminator and
setup code.

    $ chip-tool-darwin pairing ethernet {NODE_ID_TO_ASSIGN} 20202021 3840 {IP_ADDRESS}

In this case, the device will be assigned node id `${NODE_ID_TO_ASSIGN}` (which
must be a decimal number or a 0x-prefixed hex number).

### Forget the currently-commissioned device

    $ chip-tool-darwin pairing unpair

## Using the Client to Send Matter Commands

To use the Client to send Matter commands, run the built executable and pass it
the target cluster name, the target command name as well as an endpoint id.

The endpoint id must be between 1 and 240.

    $ chip-tool-darwin onoff on 1

The client will send a single command packet and then exit.

### How to get the list of supported clusters

To get the list of supported clusters, run the built executable without any
arguments.

    $ chip-tool-darwin

Example output:

```bash
Usage:
  ./chip-tool-darwin cluster_name command_name [param1 param2 ...]

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

    $ chip-tool-darwin onoff

### How to get the list of supported attributes for a specific cluster

To the the list of attributes for a specific cluster, run the built executable
with the target cluster name and the `read` command name.

    $ chip-tool-darwin onoff read

### How to get the list of parameters for a command

To get the list of parameters for a specific command, run the built executable
with the target cluster name and the target command name

    $ chip-tool-darwin onoff on

# Command Reference

## Command Details

[Cluster Commands](../../zzz_generated/chip-tool-darwin/zap-generated/README_CLUSTER_COMMANDS.md)
