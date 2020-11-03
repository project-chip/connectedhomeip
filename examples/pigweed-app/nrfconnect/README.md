# CHIP nRF Connect nRF52840 Pigweed Example Application

An example application showing the use
[CHIP](https://github.com/project-chip/connectedhomeip) on the Nordic nRF52840.

<hr>

-   [CHIP nRF52840 Pigweed Example Application](#chip-nrf52840-pigweed-example-application)
    -   [Introduction](#introduction)
    -   [Building](#building)
        -   [Using Docker container](#using-docker-container)
        -   [Using Native shell](#using-native-shell)
        -   [Supported nRF Connect SDK versions](#supported-nrf-connect-sdk-versions)
    -   [Configuring the example](#configuring-the-example)
    -   [Flashing and debugging](#flashing-and-debugging)
    -   [Currently implemented features](#currently-implemented-features)

<hr>

<a name="intro"></a>

## Introduction

![nrf52840 DK](../../platform/nrf528xx/doc/images/nrf52840-dk.jpg)

The nRF52840 Pigweed example app exercises functionalities in
third_party/pigweed, to see what is needed for integrating Pigweed to CHIP, as
well as a precursor to CHIP functionalities like on-device testing.

The example makes use of the CMake build system to generate the ninja build
script. The build system takes care of invoking the CHIP library build with all
necessary flags exported from the Zephyr environment.

<a name="building"></a>

## Building

### Using Docker container

> **Important**:
>
> Due to
> [certain limitations of Docker for MacOS](https://docs.docker.com/docker-for-mac/faqs/#can-i-pass-through-a-usb-device-to-a-container)
> it is impossible to use the Docker container to communicate with a USB device
> such as nRF 52840 DK. Therefore, MacOS users are advised to follow the
> [Using Native shell](#using-native-shell) instruction.

The easiest way to get started with the example is to use nRF Connect SDK Docker
image for CHIP applications. Run the following commands to start a Docker
container:

        $ mkdir ~/nrfconnect
        $ mkdir ~/connectedhomeip
        $ docker pull nordicsemi/nrfconnect-chip
        $ docker run --rm -it -e RUNAS=$(id -u) -v ~/nrfconnect:/var/ncs -v ~/connectedhomeip:/var/chip \
            -v /dev/bus/usb:/dev/bus/usb --device-cgroup-rule "c 189:* rmw" nordicsemi/nrfconnect-chip

> **Note**:
>
> -   `~/nrfconnect` can be replaced with an absolute path to nRF Connect SDK
>     source directory in case you have it already installed.
> -   Likewise, `~/connectedhomeip` can be replaced with an absolute path to
>     CHIP source directory.
> -   `-v /dev/bus/usb:/dev/bus/usb --device-cgroup-rule 'c 189:* rmw`
>     parameters can be omitted if you're not planning to flash the example onto
>     hardware. The parameters give the container access to USB devices
>     connected to your computer such as the nRF52840 DK.
> -   `--rm` flag can be omitted if you don't want the container to be
>     auto-removed when you exit the container shell session.
> -   `-e RUNAS=$(id -u)` is needed to start the container session as the
>     current user instead of root.

If you use the container for the first time and you don't have nRF Connect SDK
and CHIP sources downloaded yet, run `setup` command in the container to pull
the sources into directories mounted as `/var/ncs` and `/var/chip`,
respectively:

        $ setup --ncs 83764f
        /var/ncs repository is empty. Do you wish to check out nRF Connect SDK sources [83764f]? [Y/N] y
        ...
        /var/chip repository is empty. Do you wish to check out Project CHIP sources [master]? [Y/N] y
        ...

It is important to remember about running activate.sh script, as it is necessary to build Pigweed library
and it can be done, by typing following command:

        $ source scripts/activate.sh

Now you may build the example by running the commands below in the Docker
container:

        $ cd /var/chip/examples/pigweed-app/nrfconnect
        $ west build -b nrf52840dk_nrf52840

If the build succeeds, the binary will be available under
`/var/chip/examples/pigweed-app/nrfconnect/build/zephyr/zephyr.hex`. Note that
other operations described in this document like flashing or debugging can also
be done in the container.

### Using native shell

Before building the example,
[download the nRF Connect SDK and install all requirements](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/gs_installing.html).
Please read the
[Supported nRF Connect SDK versions](#supported-nrf-connect-sdk-versions)
section to learn which version to use to avoid unexpected compatibility issues.

If you don't want to use SEGGER Embedded Studio, you may skip the part about
installing and configuring it.

Download and install the
[nRF Command Line Tools](https://www.nordicsemi.com/Software-and-Tools/Development-Tools/nRF-Command-Line-Tools).

Download and install [GN meta-build system](https://gn.googlesource.com/gn/).

Make sure that you source the following file:

        $ source <ncs-dir>/zephyr/zephyr-env.sh

> **Note:**
>
> Ensure that `$ZEPHYR_BASE`, `$GNUARMEMB_TOOLCHAIN_PATH`, and
> `$ZEPHYR_TOOLCHAIN_VARIANT` environment variables are set in your current
> terminal before building. `$GNUARMEMB_TOOLCHAIN_PATH` and
> `$ZEPHYR_TOOLCHAIN_VARIANT` must be set manually.

Make sure that you run activate.sh script, as it is necessary to build Pigweed library and it can be done, by typing
following command:

        $ source scripts/activate.sh


After your environment is set up, you are ready to build the example. The
recommended tool for building and flashing the device is
[west](https://docs.zephyrproject.org/latest/guides/west/).

The following commands will build the `pigweed-app` example:

        $ cd ~/connectedhomeip/examples/pigweed-app/nrfconnect

        # If this is a first time build or if `build` directory was deleted
        $ west build -b nrf52840dk_nrf52840

        # Any subsequent build
        $ west build

After a successful build, the binary will be available under
`<example-dir>/build/zephyr/zephyr.hex`

### Supported nRF Connect SDK versions

It is recommended to use the nRF Connect version which is being verified as a
part of CHIP Continuous Integration testing, which happens to be `83764f` at the
moment. You may verify that the revision is used in
[chip-build-nrf-platform](https://github.com/project-chip/connectedhomeip/blob/master/integrations/docker/images/chip-build-nrf-platform/Dockerfile)
Docker image in case of doubt.

Please refer to
[this section](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/gs_installing.html#updating-the-repositories)
in the user guide to learn how to update nRF Connect SDK repository. For example
to checkout given `83764f` revision the following commands should be called:

        # Phrase <nrfconnect-dir> should be replaced with an absolute path to nRF Connect SDK source directory.
        $ cd <nrfconnect-dir>/nrf

        $ git fetch origin

        # Number `83764f` can be replaced with the desired revision number.
        $ git checkout 83764f

        $ west update

<a name="configuring"></a>

## Configuring the example

The Zephyr ecosystem is higly configurable and allows the user to modify many
aspects of the application. The configuration system is based on `Kconfig` and
the settings can be modified using the `menuconfig` utility.

To open the configuration menu, do the following:

        $ cd <example-dir>
        # First time build
        $ west build -b nrf52840dk_nrf52840 -t menuconfig

        # Any subsequent build
        $ west build -t menuconfig

        # Running menuconfig with ninja
        $ cd <example-dir>/build
        $ ninja menuconfig

Changes done with `menuconfig` will be lost, if the `build` directory is
deleted. To make them persistent, save the configuration options in `prj.conf`
file.

<a name="flashing"></a>

## Flashing and debugging

The example application is designed to run on the
[Nordic nRF52840 DK](https://www.nordicsemi.com/Software-and-Tools/Development-Kits/nRF52840-DK)
development kit.

To flash the application to the device, use the `west` tool:

        $ cd <example-dir>
        $ west flash

If you have multiple nRF52840 DK boards connected, `west` will prompt you to
pick the correct one.

To debug the application on target:

        $ cd <example-dir>
        $ west debug

<a name="currently-implemented-features"></a>

## Currently implemented features

### Echo RPC:

```
python -m pw_hdlc_lite.rpc_console --device /dev/ttyACM0 -b 115200 $CHIP_ROOT/third_party/pigweed/repo/pw_rpc/pw_rpc_protos/echo.proto -o /tmp/pw_rpc.out
```

will start an interactive python shell where Echo RPC can be invoked as

```
rpcs.pw.rpc.EchoService.Echo(msg="hi")
```
