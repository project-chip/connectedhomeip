[<< Back to TOC](../README.md)

# Matter over Wifi Demo Overview

This document walks through the steps to build Matter Lighting-app project using
EFR32 BRD4161A + RS911X and EFR32 BRD4161A + WF200. Silicon Labs has two
families of WiFi adatpers (1. RS911X 2. WF200). Both are supported in this WiFi
port of Matter.

<br>

## Setup Overview

![Overview](./images/wifi_setup.png)

## EFR32MG12 + WF200 Connection

![EFR32MG12 + WF200 connection ](./images/MG12_WF200.jpg)

<br>

<!-- ## Overview -->

This document walks through the steps to build the Matter Lighting-app project
using EFR32 BRD4161A + RS911X and EFR32 BRD4161A + WF200.

Follow this sequence if you are setting up for the first time:

1. [Matter Wifi Prerequisites](WIFI_PREREQS.md)

    This page goes through the hardware and software (host side) required for
    the demo - make sure you have all the necessary hardware available before
    continuing.

2. [Build Linux Environment](BUILD_CHIP_ENV.md)

    This part gives you steps to setup the build environment on a Linux machine.
    The instructions gives you build steps for the application and the ChipTool.

3. [Build Raspberry-Pi Environment](BUILD_PI_ENV.md)

    This section documents steps to setup the build environment on the Raspberry
    Pi.

4. [Software Setup](SW_SETUP.md)

    This part gives you steps to build the Lighting-app over Wifi. Two separate
    build commands are mentioned for RS911x and WF200 adapters to use
    accordingly.

5. [Running Matter Demo over Wifi using Linux](RUN_DEMO.md)

    This part give you steps to run the Matter Wifi demo using ChipTool running
    on a Linux Machine (either Laptop or Raspberry Pi) - follow this after
    successfully executing the above steps.

6. [optional] [Running Matter Demo over Wifi using Android](WIFI_ANDROID.md)

    This part give you steps to run the Matter Wifi demo using ChipTool running
    on an Android phone (either Laptop or Raspberry Pi).
