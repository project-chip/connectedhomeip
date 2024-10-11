# Building and Running CHIP Linux Examples for i.MX 8M Mini EVK

This document describes how to build below Linux examples with the NXP embedded
Linux Yocto SDK and then run the output executable files on the **NXP i.MX 8M**
**Mini EVK** development board.

-   [CHIP Linux All-clusters Example](../../../examples/all-clusters-app/linux/README.md)
-   [CHIP Linux Lighting Example](../../../examples/lighting-app/linux/README.md)
-   [CHIP Linux Thermostat Example](https://github.com/project-chip/connectedhomeip/tree/master/examples/thermostat/linux)
-   [CHIP Linux CHIP-tool Example](../../../examples/chip-tool/README.md)
-   [CHIP Linux OTA-provider Example](../../../examples/ota-provider-app/linux/README.md)

This document has been tested on:

-   x64 host machine to build (cross-compile) the example
    1.  running Ubuntu for 64bit PC(AMD64) desktop 20.04 LTS.
-   **NXP i.MX 8M Mini EVK** board to run the example
    1.  running Yocto image generated from the NXP released Yocto source code.

The Yocto Project is an open source collaboration project focused on embedded
Linux OS development. For more information about this project, see the
[Yocto Project page](https://www.yoctoproject.org/).

<hr>

-   [Building and Running CHIP Linux Examples for i.MX 8M Mini EVK](#building-and-running-chip-linux-examples-for-imx-8m-mini-evk)
    -   [Building](#building)
    -   [Commandline arguments](#commandline-arguments)
    -   [Running the Examples on i.MX 8M Mini EVK](#running-the-examples-on-imx-8m-mini-evk)

<hr>

## Building

Before building the CHIP Linux Examples, the Yocto source code released by NXP
needs to be downloaded, then the Yocto SDK and the EVK Linux SD card image need
to be generated.

-   Download the Yocto source code and generate the Yocto SDK and the SD card
    image

    The Yocto source code is maintained with a repo manifest, the tool `repo` is
    used to download the source code.

    This document is tested with the i.MX Yocto 5.10.35_2.0.0 release. Run the
    commands below to download this release:

          ```
          mkdir ~/bin
          curl http://commondatastorage.googleapis.com/git-repo-downloads/repo  > ~/bin/repo
          chmod a+x ~/bin/repo
          export PATH=${PATH}:~/bin
          ```

          ```
          mkdir yocto            # this directory will be the top directory of the Yocto source code
          cd yocto
          repo init -u https://source.codeaurora.org/external/imx/imx-manifest  -b imx-linux-hardknott -m imx-5.10.35-2.0.0.xml
          repo sync
          ```

    To build the Yocto Project, some packages need to be installed. The list of
    packages required are:

          ```
          sudo apt-get install gawk wget git-core diffstat unzip texinfo gcc-multilib \
          build-essential chrpath socat cpio python3 python3-pip python3-pexpect \
          xz-utils debianutils iputils-ping python3-git python3-jinja2 libegl1-mesa libsdl1.2-dev \
          pylint3 xterm
          ```

    More information about the downloaded Yocto release can be found in the
    corresponding i.MX Yocto Project User’s Guide which can be found at
    [NXP official website](https://www.nxp.com/imxlinux).

    Change the current directory to the top directory of the Yocto source code
    and execute the commands below to generate the Yocto SDK:

          ```
          MACHINE=imx8mmevk DISTRO=fsl-imx-xwayland source ./imx-setup-release.sh -b bld-xwayland
          bitbake imx-image-core -c populate_sdk
          ```

    After the execution of the previous two commands, the SDK installation file
    can be found at tmp/deploy/sdk as a shell script. With the test environment
    of this document, the installation file name is:

    > fsl-imx-xwayland-glibc-x86_64-imx-image-core-cortexa53-crypto-imx8mmevk-toolchain-5.10-hardknott.sh

    Change the current directory to the top directory of the Yocto source code
    and execute the commands below to generate the Yocto SD card image:

          ```
          MACHINE=imx8mmevk DISTRO=fsl-imx-xwayland source ./imx-setup-release.sh -b bld-xwayland
          echo "IMAGE_INSTALL_append += \"libavahi-client\"" >> conf/local.conf
          bitbake imx-image-core
          ```

    The Yocto image can be found at
    tmp/deploy/images/imx8mmevk/imx-image-core-imx8mmevk.wic.bz2. The `bzip2`
    command should be used to unzip this file then the `dd` command should be
    used to program the output file to a microSD card by running the commands
    below. Then the microSD card can be used with the **i.MX 8M Mini EVK**.

    **Be cautious when executing the `dd` command below, make sure the `of`
    represents the microSD card device!**, `/dev/sdc` in the command below
    represents a microSD card connected to the host machine with a USB adapter,
    however the output device name may vary.

          ```
          bzip2 -d imx-image-core-imx8mmevk-20210812084502.rootfs.wic.bz2
          sudo dd if=imx-image-core-imx8mmevk-20210812084502.rootfs.wic of=/dev/sdc bs=4M conv=fsync
          ```

-   Install the NXP Yocto SDK and set toolchain environment variables.

    Execute the SDK installation file with root permission.

          ```
          sudo tmp/deploy/sdk/fsl-imx-xwayland-glibc-x86_64-imx-image-full-cortexa53-crypto-imx8mmevk-toolchain-5.10-hardknott.sh
          ```

    After the Yocto SDK is installed on the host machine, to use the SDK when
    building the CHIP Linux Examples, export a shell environment variable named
    `IMX_SDK_ROOT` to specify the path of the SDK, for example:

          ```
          export IMX_SDK_ROOT=/opt/fsl-imx-xwayland/5.10-hardknott
          ```

-   Build the example application:

    Building those examples with the Yocto SDK specified by the `IMX_SDK_ROOT`
    has been integrated into the tool `build_examples.py` and the tool
    `imxlinux_example.sh`. Choose one of them to build the examples.

    Assuming that the working directory is changed to the top level directory of
    this project.

          ```
          git submodule update --init
          source scripts/activate.sh

          # If the all-clusters example is to be built
          ./scripts/build/build_examples.py --target imx-all-clusters-app build
          # or
          ./scripts/examples/imxlinux_example.sh examples/all-clusters-app/linux examples/all-clusters-app/linux/out/aarch64

          # If the lighting example is to be built
          ./scripts/build/build_examples.py --target imx-lighting-app  build
          # or
          ./scripts/examples/imxlinux_example.sh examples/lighting-app/linux examples/lighting-app/linux/out/aarch64

          # If the thermostat example is to be built
          ./scripts/build/build_examples.py  --target imx-thermostat build
          # or
          ./scripts/examples/imxlinux_example.sh examples/thermostat/linux examples/thermostat/linux/out/aarch64

          # If the chip-tool example is to be built
          ./scripts/build/build_examples.py  --target imx-chip-tool build
          # or
          ./scripts/examples/imxlinux_example.sh examples/chip-tool examples/chip-tool/out/aarch64

          # If the ota-provider example is to be built
          ./scripts/build/build_examples.py  --target imx-ota-provider-app build
          # or
          ./scripts/examples/imxlinux_example.sh examples/ota-provider-app/linux examples/ota-provider-app/linux/out/aarch64

          ```

    If the `build_examples.py` is used, the executable files are built in the
    subdirectories under out/, the subdirectory name is the same as the argument
    specified after the option `--target` when build the examples.

    If the `imxlinux_example.sh` is used, the executable files are built in the
    directory specified by the second parameter when build the examples.

    The executable files can be executed on the **i.MX 8M Mini EVK** which
    running the Yocto image previously generated as described in the sections
    above.

## Commandline arguments

The generated executable files supports to work with below commandline argument:

-   `--wifi`

    Enables Wi-Fi management feature. Required for Wi-Fi provisioning.

    The Wi-Fi device on **i.MX 8M Mini EVK** is a module based on the NXP
    88W8987 Wi-Fi/Bluetooth SoC.

-   `--ble-device <interface id>`

    Use the specific Bluetooth interface for BLE advertisement and connections.

    `interface id`: the number after `hci` when listing BLE interfaces using the
    `hciconfig` command, for example, `--ble-device 1` means using `hci1`
    interface. Default: `0`.

    The BLE device on **i.MX 8M Mini EVK** is a module based on the NXP 88W8987
    Wi-Fi/Bluetooth SoC.

## Running the Examples on i.MX 8M Mini EVK

The steps and commands to run any of the examples are quite similar.
Thermostat-app is used as an example below.

-   Prerequisites

    By following the [Building](#building) section of this document, the Yocto
    image is cross-compiled and programmed to a microSD card.

    Follow the steps below to setup the environment needed to run the example on
    the **i.MX 8M Mini EVK**:

    -   Plug the microSD card with Yocto image into the SD-card slot of the
        **i.MX 8M** **Mini EVK**.
    -   Change the boot switch on the **i.MX 8M Mini EVK** board to boot from
        MicroSD/SDHC2 based on the silkscreen print on the board.
    -   Use a Type-A to Micro-B cable to connect the DEBUG port of the **i.MX
        8M** **Mini EVK** to a host machine, and use a serial communication
        program like minicom or Putty to connect to the debug interface.
    -   Power on the board to boot up the Yocto image, logging in with user name
        `root` via the serial communication program. There is password for the
        root user in the default Yocto image configuration.
    -   Copy the executable file chip-lighting-app to the **i.MX 8M Mini EVK**,
        using either of the two methods below:
        -   Connect the **i.MX 8M Mini EVK** to ethernet via the onboard
            ethernet port, then use the `scp` command on the host machine to
            copy the executable file to the **i.MX 8M Mini EVK**.
        -   Use a U-disk to copy the executable file between the build machine
            and the **i.MX 8M Mini EVK**.

    In order to test the CHIP protocol functions, another device on the same
    network is needed to run the
    [ChipDeviceController](../../../src/controller/python) tool to communicate
    with the **i.MX 8M Mini EVK**.

    The ChipDeviceController can be a laptop / workstation. Bluetooth
    functionality is mandatory on this device.

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
        [README.md](../../../src/controller/python/README.md) document. Refer to
        the "Building and installing" part to build the tool.

-   Running

    -   Initialize the BT device on the **i.MX 8M Mini EVK** board

              ```
              modprobe moal mod_para=nxp/wifi_mod_para.conf       # Load the Wi-Fi/BT firmware
              hciattach /dev/ttymxc0 any 115200 flow              # Initialize the BT device
              ```

    -   Find the Bluetooth device id for **i.MX 8M Mini EVK** by executing the
        command below. The number following string `hci` is the Bluetooth device
        id, `0` in this example.

              ```
              $ hciconfig
              hci0:   Type: Primary  Bus: USB
                      BD Address: 00:1A:7D:DA:71:13  ACL MTU: 310:10  SCO MTU: 64:8
                      UP RUNNING
                      RX bytes:73311 acl:1527 sco:0 events:3023 errors:0
                      TX bytes:48805 acl:1459 sco:0 commands:704 errors:0
              ```

    -   Run the Linux Example App

              ```
              /home/root/thermostat-app --ble-device 0 --wifi  # The bluetooth device used is hci0 and support wifi network
              ```

    -   Run [ChipDeviceController](../../../src/controller/python) on the
        controller device to communicate with **i.MX 8M Mini EVK** running the
        example.

              ```
              $ sudo out/python_env/bin/chip-device-ctrl                                          # execute the tool
                chip-device-ctrl > connect -ble 3840 20202021 8889                                # connect to i.MX 8M Mini EVK
                chip-device-ctrl > zcl Thermostat SetpointRaiseLower 8889 1 0 mode=1 amount=10    # send command to i.MX 8M Mini EVK via BLE
              ```

        (Note that the last two commands `connect -ble 3840 20202021 8889` and
        `zcl Thermostat SetpointRaiseLower 8889 1 0 mode=1 amount=10` are Python
        CHIP Device Controller commands, not shell commands. The 3840 is the
        target device's `discriminator`. The 20202021 is the `setup pin code`.
        8889 is the `node id` and if not input 8889 a random node id will be
        assigned.)

        After the previous commands are executed, inspect the logs of both the
        **i.MX 8M Mini EVK** and the controller device to observe connection and
        control events.

    -   Provision the **i.MX 8M Mini EVK** to a Wi-Fi AP with the following
        commands by `NetworkCommissioning` Cluster.

        Command `AddOrUpdateWiFiNetwork` sends the target Wi-Fi AP's SSID and
        password. The `${SSID}` and `${PASSWORD}` should be in plaintext format.
        At this moment, Wi-Fi is still idle on the **i.MX8 Mini EVK**.

        Command `ConnectNetwork` triggers the Wi-Fi AP connecting operation on
        **i.MX8 Mini EVK**.

                chip-device-ctrl > zcl NetworkCommissioning AddOrUpdateWiFiNetwork 8889 0 0 ssid=str:${SSID} credentials=str:${PASSWORD} breadcrumb=0 timeoutMs=5000
                chip-device-ctrl > zcl NetworkCommissioning ConnectNetwork 8889 0 0 networkID=str:${SSID} breadcrumb=0 timeoutMs=15000

    -   Make sure the controller device is connected to the same network of this
        Wi-Fi AP because the Wi-Fi connection is established between the Wi-Fi
        AP and the **i.MX8 Mini EVK** and mDNS only works on local network.

        Resolve the target device with DNS-SD and update the address of the
        node.

                chip-device-ctrl > close-ble  # Shutdown the BLE connection
                chip-device-ctrl > resolve 8889                                                    # The 8889 is the node ID.
                chip-device-ctrl > zcl Thermostat SetpointRaiseLower 8889 1 0 mode=1 amount=10     # Now the ZCL command will be send via IP network.
