[<< Back to TOC](../README.md)

# Software Setup and Preliminaries

## Software Setup

Run below commands on a Linux terminal running on either Linux machine, WSL or
Virtual Machine.

1. To download the
   [Matter codebase](https://github.com/project-chip/connectedhomeip.git) run
   the following commands. Create a working directory - we will name it `matter`
   as part of this example flow:

    > `$ mkdir matter`

    > `$ cd matter`

    > `$ MATTER_WORKDIR=pwd` &emsp;&emsp; --> We will use $MATTER_WORKDIR later

    > `$ git clone https://github.com/project-chip/connectedhomeip.git`

2. Bootstrapping:

    > `$ cd connectedhomeip`

    > `$ git submodule update --init --recursive`

    > `$ . scripts/bootstrap.sh`

    Create a directory where binaries will be updated/compiled. We will call it
    `out` in this example:

    > `$ mkdir out`

<br>

## Building Software

The following commands are for building the example. Depending on which device
you are using, select the appropriate build command to run.

Build command for RS911x:
`$ ./scripts/examples/gn_efr32_example.sh examples/lighting-app/efr32/ out/rs911x_lighting BRD4161A --wifi rs911x |& tee out/rs911x_lighting.out`

Build command for WF200:
`$ ./scripts/examples/gn_efr32_example.sh examples/lighting-app/efr32/ out/wf200_lighting BRD4161A is_debug=false --wifi wf200 |& tee out/rs911x_lighting.out`

Run the following:

> `$ cd connectedhomeip`

> `$ <appropriate_build_command_from_above>`

The generated software can be found in `out/rs911x_xxx/BRD4161A/*.out` for the
RS9116 and in `out/wf200_xxx/BRD4161A/*.out` for the WF200.

This is what you will burn onto the EFR32.

## Compiling the ChipTool

-   Build the chiptool on a laptop or Raspberry-pi ( If you are not using Linux
    Laptop ) which has Wifi and BLE
-   Run the following commands on a terminal where you will run chip-tool:

    > `$ mkdir matter`

    > `$ cd matter`

    > `$ MATTER_WORKDIR=pwd` &emsp;&emsp; --> We will use $MATTER_WORKDIR later

    > `$ git clone https://github.com/project-chip/connectedhomeip.git`

2. Bootstrapping:

    > `$ cd connectedhomeip`

    > `$ git submodule update --init --recursive`

    > `$ . scripts/bootstrap.sh`

    Create a directory where chip-tool binary will be updated/compiled:

    > `$ mkdir out`

    > `$ ./scripts/examples/gn_build_example.sh examples/chip-tool out/standalone`

    This will build chiptool in `out/standalone`.

    After this, follow the steps on the page
    '[Runing the Matter Demo over Wifi](RUN_DEMO.md)' to flash the binaries and
    execute the demo.
