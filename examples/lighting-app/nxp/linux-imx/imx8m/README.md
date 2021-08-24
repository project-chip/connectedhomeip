# Building and Running CHIP Linux Lighting Example for i.MX 8M Mini EVK

This document describes how to build the
[CHIP Linux lighting Example](../../../linux) with the NXP embedded Linux Yocto
SDK and then run the output executable file on the NXP i.MX 8M Mini EVK
development board. This document has been tested on:

-   x64 host machine to build (cross-compile) the example
    1.  running Ubuntu for 64bit PC(AMD64) desktop 20.04 LTS.
-   NXP i.MX 8M Mini EVK board to run the example
    1.  running Yocto image generated from the NXP released Yocto source code.

The Yocto Project is an open source collaboration project focused on embedded
Linux OS development. For more information about this project, see the
[Yocto Project page](https://www.yoctoproject.org/).

<hr>

-   [Building and Running CHIP Linux Lighting Example for i.MX 8M Mini EVK](#building-and-running-chip-linux-lighting-example-for-imx-8m-mini-evk)
    -   [Building](#building)
    -   [Commandline Arguments](#command-line-args)
    -   [Running the Complete Example on i.MX 8M Mini EVK](#running-complete-example-on-imx8mmevk)

<hr>

<a name="building"></a>

## Building

Before building the CHIP Linux Lighting Example, the Yocto source code released
by NXP needs to be downloaded, then the Yocto SDK and the EVK Linux SD card
image need to be generated.

-   Download the Yocto source code and generate the Yocto SDK and the SD card
    image

    The Yocto source code is maintained with with a repo manifest, the tool
    `repo` is used to download the source code.

    This document is tested with the i.MX Yocto 5.10.35_2.0.0 release. Run the
    commands below to download this release:

          $ mkdir ~/bin
          $ curl http://commondatastorage.googleapis.com/git-repo-downloads/repo  > ~/bin/repo
          $ chmod a+x ~/bin/repo
          $ export PATH=${PATH}:~/bin

          $ mkdir yocto            # this directory will be the top directory of the Yocto source code
          $ cd yocto
          $ repo init -u https://source.codeaurora.org/external/imx/imx-manifest  -b imx-linux-hardknott -m imx-5.10.35-2.0.0.xml
          $ repo sync

    To build the Yocto Project, some packages need to be installed, the list of
    host packages required can be found at
    [Yocto Project Quick Start](https://www.yoctoproject.org/docs/current/ref-manual/ref-manual.html)

    Essential Yocto Project host packages are:

          $ sudo apt-get install gawk wget git-core diffstat unzip texinfo gcc-multilib \
            build-essential chrpath socat cpio python3 python3-pip python3-pexpect \
            xz-utils debianutils iputils-ping python3-git python3-jinja2 libegl1-mesa libsdl1.2-dev \
            pylint3 xterm

    More information about the downloaded Yocto release can be found in the
    corresponding i.MX Yocto Project User’s Guide which can be found at NXP
    official website.

    Change the current directory to the top directory of the Yocto source code
    and execute the commands below to generate the Yocto SDK:

          $ MACHINE=imx8mmevk DISTRO=fsl-imx-xwayland source ./imx-setup-release.sh -b bld-xwayland
          $ bitbake imx-image-core -c populate_sdk

    After the execution of the previous two commands, the SDK installation file
    can be found at tmp/deploy/sdk as a shell script. With the test environment
    of this document, the installation file name is:

    > fsl-imx-xwayland-glibc-x86_64-imx-image-core-cortexa53-crypto-imx8mmevk-toolchain-5.10-hardknott.sh

    Change the current directory to the top directory of the Yocto source code
    and execute the commands below to generate the Yocto SD card image:

          $ MACHINE=imx8mmevk DISTRO=fsl-imx-xwayland source ./imx-setup-release.sh -b bld-xwayland
          $ echo "IMAGE_INSTALL_append += \"libavahi-client\"" >> conf/local.conf
          $ bitbake imx-image-core

    The Yocto image can be found at
    tmp/deploy/images/imx8mmevk/imx-image-core-imx8mmevk.wic.bz2. The `bzip2`
    command should be used to unzip this file then the `dd` command should be
    used to program the output file to a microSD card by running the commands
    below. Then the microSD card can be used with the i.MX 8M Mini EVK.

    **Be cautious when executing the `dd` command below, make sure the `of`
    represents the microSD card device!**, `/dev/sdc` in the command below
    represents a microSD card connected to the host machine with a USB adapter,
    however the output device name may vary.

          $ bzip2 -d imx-image-core-imx8mmevk-20210812084502.rootfs.wic.bz2
          $ sudo dd if=imx-image-core-imx8mmevk-20210812084502.rootfs.wic of=/dev/sdc bs=4M conv=fsync

-   Install the NXP Yocto SDK and source the environment setup script

    Execute the SDK installation file with root permission.

          $ sudo tmp/deploy/sdk/fsl-imx-xwayland-glibc-x86_64-imx-image-full-cortexa53-crypto-imx8mmevk-toolchain-5.10-hardknott.sh

    After the Yocto SDK is installed on the host machine, an environment setup
    script is also generated, and there are prompt lines telling the user to
    source the script each time when using the SDK in a new shell, for example:

          $ . /opt/fsl-imx-xwayland/5.10-hardknott/environment-setup-cortexa53-crypto-poky-linux

-   Build the example application:

          $ cd ~/connectedhomeip/examples/lighting-app/linux
          $ git submodule update --init
          $ source third_party/connectedhomeip/scripts/activate.sh
          $ PLATFORM_CFLAGS="-DCHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME=\\\"mlan0\\\""
          $ PKG_CONFIG_SYSROOT_DIR=${PKG_CONFIG_SYSROOT_DIR} \
            PKG_CONFIG_LIBDIR=${PKG_CONFIG_PATH} \
            gn gen out/aarch64 --args='target_os="linux" target_cpu="arm64" arm_arch="armv8-a"
                import("//build_overrides/build.gni")
                target_cflags=[ "--sysroot='${SDKTARGETSYSROOT}'", "'${PLATFORM_CFLAGS}'" ]
                target_ldflags = [ "--sysroot='${SDKTARGETSYSROOT}'" ]
                custom_toolchain="${build_root}/toolchain/custom"
                target_cc="'${OECORE_NATIVE_SYSROOT}'/usr/bin/aarch64-poky-linux/aarch64-poky-linux-gcc"
                target_cxx="'${OECORE_NATIVE_SYSROOT}'/usr/bin/aarch64-poky-linux/aarch64-poky-linux-g++"
                target_ar="'${OECORE_NATIVE_SYSROOT}'/usr/bin/aarch64-poky-linux/aarch64-poky-linux-ar"'
          $ ninja -C out/aarch64

    The executable file named chip-lighting-app is built under out/aarch64, it
    can be executed on the i.MX 8M Mini EVK running the Yocto image previously
    generated as described in the sections below.

<a name="command-line-args"></a>

## Commandline arguments

The generated executable file supports to work with below commandline argument:

-   `--ble-device <interface id>`

    Use the specific Bluetooth interface for BLE advertisement and connections.

    `interface id`: the number after `hci` when listing BLE interfaces using the
    `hciconfig` command, for example, `--ble-device 1` means using `hci1`
    interface. Default: `0`.

    The BLE device on i.MX 8M Mini EVK is the NXP 88W8987 WiFi/BT module.

<a name="running-complete-example-on-imx8mmevk"></a>

## Running the Complete Example on i.MX 8M Mini EVK

-   Prerequisites

    By following the [Building](#building) section of this document, the Yocto
    image is cross-compiled and programed to a microSD card.

    Follow the steps below to setup the environment needed to run the example on
    the i.MX 8M Mini EVK:

    -   Plug the microSD card with Yocto image into the card slot of the i.MX 8M
        Mini EVK.
    -   Change the boot switch on the i.MX 8M Mini EVK board to boot from
        MicroSD/SDHC2 based on the silkscreen print on the board.
    -   Use a Type-A to Micro-B cable to connect the DEBUG port of the i.MX 8M
        Mini EVK to a host machine, and use a serial communication program like
        minicom or Putty to execute commands and inspect the logs.
    -   Power on the board to boot up the Yocto image, logging in with user name
        `root` via the serial communication program.
    -   Copy the executable file chip-lighting-app to the i.MX 8M Mini EVK,
        using either of the two methods below:
        -   Connect the i.MX 8M Mini EVK to ethernet via the onboard ethernet
            port, then use the `scp` command on the build machine to copy the
            executable file to the i.MX 8M Mini EVK.
        -   Use a U-disk to copy the executable file between the build machine
            and the i.MX 8M Mini EVK.

    In order to test the CHIP protocol functions, another device on the same
    network is needed to run the
    [ChipDeviceController](../../../../../src/controller/python) tool to
    communicate with the i.MX 8M Mini EVK.

    This controller device can be a laptop / workstation. Bluetooth
    functionality is needed on this device.

    For the test environment used with this document, a Raspberry Pi is used to
    run the ChipDeviceController tool.

    Follow the steps below to setup the controller environment on a Raspberry
    Pi:

    -   Install Ubuntu Server 20.04.2 LTS on the Raspberry Pi with reference to
        this page:
        [Install Ubuntu on a Raspberry Pi](https://ubuntu.com/download/raspberry-pi).
    -   Boot up Ubuntu on the Raspberry Pi
    -   Clone this connectedhomeip project
    -   Follow Python ChipDeviceController
        [README.md](../../../../../src/controller/python/README.md) document.
        Refer to the "Building and installing" part to build the tool.

-   Running

    -   Find the Bluetooth device id for i.MX 8M Mini EVK by executing the
        command below. The number following string `hci` is the Bluetooth device
        id, `0` in this example.

              $ hciconfig
              hci0:   Type: Primary  Bus: USB
                      BD Address: 00:1A:7D:DA:71:13  ACL MTU: 310:10  SCO MTU: 64:8
                      UP RUNNING
                      RX bytes:73311 acl:1527 sco:0 events:3023 errors:0
                      TX bytes:48805 acl:1459 sco:0 commands:704 errors:0

    -   Run the Linux Lighting Example App

              $ modprobe moal mod_para=nxp/wifi_mod_para.conf       # Load the Wi-Fi/BT firmware
              $ hciattach /dev/ttymxc0 any 115200 flow              # Initialize the BT device
              $ /home/root/chip-lighting-app --ble-device 0         # The bluetooth device used is hci0

    -   Run [ChipDeviceController](../../../../../src/controller/python) on the
        controller device to communicate with i.MX 8M Mini EVK running the
        example.

              $ sudo out/python_env/bin/chip-device-ctrl                 # execute the tool
                chip-device-ctrl > connect -ble 3840 20202021 8889       # connect to i.MX 8M Mini EVK
                chip-device-ctrl > zcl OnOff Toggle 8889 1 0             # send command to i.MX 8M Mini EVK

        (Note that the last two commands `connect -ble 3840 20202021 8889` and
        `zcl OnOff Toggle 8889 1 0` are Python CHIP Device Controller commands,
        not shell commands.)

        After the previous commands are executed, inspect the logs of both the
        i.MX 8M Mini EVK and the controller device to observe connection and
        control events.
