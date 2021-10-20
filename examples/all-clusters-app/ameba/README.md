# CHIP Ameba All Clusters Example

A prototype application that demonstrates device commissioning and cluster
control.

---

-   [CHIP Ameba All Clusters Example](#chip-ameba-all-clusters-example)
    -   [Supported Device](#supported-device)
    -   [Building the Example Application](#building-the-example-application)
    -   [Commissioning and cluster control](#commissioning-and-cluster-control)
        -   [Commissioning](#commissioning)
        -   [Cluster control](#cluster-control)

---

## Supported Device

The CHIP demo application is supported on
[Ameba RTL8722DM Board](https://www.amebaiot.com/en/amebad).

## Building the Example Application

-   Pull docker image:

          $ docker pull pankore/chip-build-ameba:latest

-   Run docker container:

          $ docker run -it -v ${CHIP_DIR}:/root/chip pankore/chip-build-ameba:latest

-   Setup build environment:

          $ source ./scripts/bootstrap.sh

-   To build the demo application:

          $ ./scripts/build/build_examples.py --target ameba-amebad-all-clusters build

    The output image files are stored in
    `out/ameba-amebad-all-clusters/asdk/image` folder.

-   After building the application, **Ameba Image Tool** is used to flash it to
    Ameba board.

1.  Connect your device via USB and open Ameba Image Tool.
2.  Select correct serial port and set baudrate as **115200**.
3.  Browse and add the corresponding image files in the Flash Download list to
    the correct locations
4.  Click **Download** button.

## Commissioning and Cluster Control

### Commissioning

The commissioning is carried out via WiFi.

1.  After download all-cluster example to Ameba board, boot up the board by
    pressing the reset button.
2.  Use ATW commands to setup network.
3.  Use ATS\$ command to run all-cluster example.
4.  Use
    [standalone chip-tool](https://github.com/project-chip/connectedhomeip/tree/master/examples/chip-tool)
    to communicate with the device.

          $ ./chip-tool pairing bypass node-id-to-assign 192.168.xx.xxx 5540

### Cluster Control

-   After successful commissioning, use the OnOff cluster command to control the
    OnOff attribute. This allows you to toggle a parameter implemented by the
    device to be On or Off.

          $ ./chip-tool onoff on 1
          $ ./chip-tool onoff off 1
