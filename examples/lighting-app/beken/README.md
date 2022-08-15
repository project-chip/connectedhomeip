# Matter BEKEN Lighting Example

This example demonstrates the Matter Lighting application on BEKEN platforms.

---

-   [Matter BEKEN Lighting Example](#matter-BEKEN-lighting-example)
    -   [Supported Devices](#supported-devices)
    -   [Building the Example Application](#building-the-example-application)
    -   [Commissioning over BLE using chip-tool](#commissioning-over-ble-using-chip-tool)

---

## Supported Devices

The demo application is intended to work on following categories of BEKEN
devices:

-   BK7235 EVB

## Building the Example Application

Building the example application requires the use of the BEKEN ARMINO IoT
Development Framework ([BK-ARMINO](https://github.com/zhengyaohan/armino)).

-   Setting up the build environment, recommended with Ubuntu 20.04 LTS

    Armino sdk download

        $ cd ~
        $ git clone git@github.com:zhengyaohan/armino.git
        $ git checkout matter

    Toolchain
    [download](http://dl.bekencorp.com/tools/toolchain/risc-v_20220418.tar.gz)
    and install, the toolchain should be installed to "/opt/risc-v"

        $ sudo tar xvzf {abs-path-to-download}/risc-v_20220418.tar.gz -C /opt
        $ sudo mv /opt/risc-v_20220418 /opt/risc-v
        $ sudo chown -R $USER:$USER /opt/risc-v


    Software dependency install

        $ sudo dpkg --add-architecture i386
        $ sudo apt-get update
        $ sudo apt-get install build-essential cmake python3 python3-pip doxygen ninja-build libc6:i386 libstdc++6:i386 libncurses5-dev lib32z1 -y
        $ sudo pip3 install sphinx_rtd_theme future breathe blockdiag sphinxcontrib-seqdiag sphinxcontrib-actdiag sphinxcontrib-nwdiag sphinxcontrib.blockdiag
        $ sudo ln -s /usr/bin/python3 /usr/bin/python

    To activate connectedhomeip files.

        $ cd {abs-path-to-connectedhomeip}
        $ source ./scripts/bootstrap.sh
        $ source ./scripts/activate.sh

    Link connectedhomeip into ARMINO.

        $ cd {abs-path-to-ARMINO}
        $ ln -sfv {abs-path-to-connectedhomeip} components/matter/

-   build project matter,and flash image file

        $ cd {abs-path-to-ARMINO}
        $ make bk7235 PROJECT=matter

    the output bin file is {abs-path-to-ARMINO}/build/matter/bk7235/all-app.bin.
    you can use
    [WINDOWS BEKEN FLASH WRITE TOOL](http://dl.bekencorp.com:8192/tools/flash/BEKEN_WRITER_EN_V2.6.28_20220326.zip)
    to flash the demo application onto the device.

## Commissioning over BLE using chip-tool

-   Please build the standalone chip-tool as described [here](../../chip-tool)
-   Commissioning the WiFi Lighting devices

        $ ./out/debug/chip-tool pairing ble-wifi 12345 <ssid> <passphrase> 20202021 3840

## Cluster Control

After successful commissioning, use the OnOff cluster command to control the
OnOff attribute. This allows you to toggle a parameter implemented by the device
to be On or Off.

    $ ./out/debug/chip-tool onoff on 12345 1
