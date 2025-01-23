# CHIP TV Casting App Example

This is a CHIP TV Casting app that can be used to cast content to a TV. This app
discovers TVs on the local network that act as commissioners, lets the user
select one, sends the TV a User Directed Commissioning request, enters
commissioning mode, advertises itself as a Commissionable Node and gets
commissioned. Then it allows the user to send CHIP commands to the TV.

Refer to the
[Matter Casting APIs documentation](https://project-chip.github.io/connectedhomeip-doc/examples/tv-casting-app/APIs.html)
to build the Matter “Casting Client” into your consumer-facing mobile app.

<hr>

-   [CHIP TV Casting App Example](#chip-tv-casting-app-example)
    -   [Building](#building)
    -   [Running the Complete Example on Linux](#running-the-complete-example-on-linux)
        -   [Commissioning the tv-casting-app](#commissioning-the-tv-casting-app)
        -   [Re-Running the Example on Linux with Cached Fabrics](#re-running-the-example-on-linux-with-cached-fabrics)
        -   [Sending Arbitrary Cluster commands](#sending-arbitrary-cluster-commands)

<hr>

## Building

-   Install tool chain

          $ sudo apt-get install git gcc g++ python pkg-config libssl-dev libdbus-1-dev libglib2.0-dev ninja-build python3-venv python3-dev unzip

-   Build the example application:

          $ cd ~/connectedhomeip/examples/tv-casting-app/linux
          $ git submodule update --init
          $ source third_party/connectedhomeip/scripts/activate.sh
          $ gn gen out/debug
          $ ninja -C out/debug

-   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip/examples/tv-casting-app/linux
          $ rm -rf out/

## Running the Complete Example on Linux

-   Pre-requisite: Build and run the tv-app

        $ cd ~/connectedhomeip/examples/tv-app/linux
        $ out/debug/chip-tv-app --secured-device-port 5640 --secured-commissioner-port 5552

-   Run the tv-casting-app (clean start)

        $ cd ~/connectedhomeip/examples/tv-casting-app/linux
        (delete any stored fabrics from previous runs)
        $ rm -rf /tmp/rm -rf /tmp/chip*
        $ out/debug/chip-tv-casting-app

    Follow the on-screen prompts on the tv-casting-app console

-   Re-run the tv-casting-app (cached fabrics)

        $ cd ~/connectedhomeip/examples/tv-casting-app/linux
        $ out/debug/chip-tv-casting-app

    Follow the on-screen prompts on the tv-casting-app console. NOTE: When there
    are cached fabrics, re-commissioning onto the same fabric will fail. See
    below for details.

### Commissioning the tv-casting-app

This example Matter TV Casting iOS application can be tested with the following
video players:

1. With the
   [example Matter tv-app](https://github.com/project-chip/connectedhomeip/tree/master/examples/tv-app)
   running on a Raspberry Pi - works out of the box. The tv-casting-app will
   automatically discover video players and print these out upon startup. The
   user-directed-commissioning (UDC) process can be initiated using the shell by
   specifying the index of the discovered video player in the printed list.

-   Initiate UDC for the discovered video player with index 0

        tv-casting-app> cast request 0

-   After sending UDC command to tv-app, you should be able to enter into tv-app
    shell:

        tv-app> controller ux ok

-   Re-run commissioner discovery

        tv-casting-app> cast discover

2. With a FireTV device - requires your Amazon Customer ID to be allow-listed
   first.

### Re-Running the Example on Linux with Cached Fabrics

After successfully commissioning the tv-casting-app onto a fabric the app can be
re-run using a cached fabric so that the commissioning step does not need to be
repeated. This allows the app to be commissioned once and then re-run upon
demand. Shell commands allow viewing, selecting and deleting a cached fabric. If
no fabric is specified for a command that required a fabric, then the first
valid cached fabric will be used.

-   Print all shell commands

        tv-casting-app> help
        tv-casting-app> cast help

-   Print all fabrics

        tv-casting-app> cast print-fabrics

-   Delete fabric with index 1

        tv-casting-app> cast delete-fabric 1

-   Switch to fabric index

        tv-casting-app> cast set-fabric 1

### Sending Arbitrary Cluster commands

The linux tv-casting-app can invoke any cluster command using the same syntax as
chip-tool. This can be done from the command line and from the shell. The
default (or first valid) cached fabric will be used unless a different fabric
has been set already using shell commands.

Similar to chip-tool arguments, the target nodeId for the command is specified
along with the endpoint. For the casting app, the nodeId would be the nodeId for
the video player (TV). For convenience, when nodeId '0' is used, the
tv-casting-app will use the cached nodeId for the video player corresponding to
the current fabric.

The tv-casting-app is able to determine the nodeId for the given fabric by
checking its binding table since the video player sets bindings on the
tv-casting-app for each endpoint to which the tv-casting-app is granted access
during commissioning. Cluster commands can be invoked via command line arguments
passed to the chip-tv-casting-app executable or via the built-in interactive
shell by prefixing the words "cast cluster" before the command.

-   Run the tv-casting-app and invoke a cluster command using default fabric,
    target video player nodeId 18446744004990074879

        $ out/debug/chip-tv-casting-app onoff read on-off 18446744004990074879 2

-   Run the tv-casting-app and invoke a cluster command using default fabric,
    target video player nodeId 0xFFFFFFEFFFFFFFFF (hex)

        $ out/debug/chip-tv-casting-app onoff read on-off 0xFFFFFFEFFFFFFFFF 2

-   Run the tv-casting-app and invoke a cluster command using default fabric,
    video player nodeId for that fabric

        $ out/debug/chip-tv-casting-app onoff read on-off 0 2

-   Run the tv-casting-app and invoke a different cluster command using default
    fabric, video player nodeId for that fabric

        $ out/debug/chip-tv-casting-app keypadinput send-key 1 0 1

-   Run the tv-casting-app and invoke a cluster command from the shell using
    default fabric, video player nodeId for that fabric

        tv-casting-app> cast cluster keypadinput send-key 1 0 1

-   Run the tv-casting-app and invoke a cluster command from the shell using
    default fabric, target video player nodeId 0xFFFFFFEFFFFFFFFF

        tv-casting-app> cast cluster keypadinput send-key 1 0xFFFFFFEFFFFFFFFF 1
