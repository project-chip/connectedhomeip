# Matter over Wi-Fi Demo Overview

This document walks through the steps to build the Matter Lighting-app project using
EFR32 BRD4161A, BRD4163A, or BRD4164A + RS911X and EFR32 BRD4161A, BRD4163A, or BRD4164A + WF200. 
Silicon Labs has two families of Wi-Fi adapters (1. RS911X 2. WF200). Both are supported in this Wi-Fi
port of Matter.

## Step 1: Matter Wi-Fi Prerequisites

Before you being running the Matter Wi-Fi demo or developing for Wi-Fi you will want to make sure that you have all the required hardware and software for each use case.

1. [Matter Wi-Fi Prerequisites](WIFI_PREREQS.md) 

   This page goes through the hardware and software (host side) required for the demo. 

## Step 2: Building the Chip-Tool for Wi-Fi

In order to run the Matter Wi-Fi demo you will need to run the Chip-Tool on one of two platforms, either Linux/Mac or Raspberry Pi.

1. [Build Linux Environment](BUILD_CHIP_ENV.md)

    This page contains instructions to set up the build environment on a Linux machine. 
    The instructions provide build steps for the application and the ChipTool.

1. [Build Raspberry-Pi Environment](BUILD_PI_ENV.md)

    This page contains instructions to set up the build environment on the Raspberry
    Pi. This is an alternative if you do not have a Linux machine.

## Step 3: Building the Matter Accessory Device (MAD) for Wi-Fi

If you are running the demo, you can simply download the proper image for your Matter Accessory Device from the [Matter Software Artifacts Page](../general/ARTIFACTS.md). Otherwise if you wish to build your Matter Accessory Device from scratch, you can begin with the [Software Setup](SW_SETUP.md):

1. [Software Setup](SW_SETUP.md)

    This page contains instructions to build the Lighting-app over Wi-Fi. Build commands are provided for both 
    the RS911x and WF200 adapters.

## Step 4: Running the Demo

1. [Running Matter Demo over Wi-Fi using Linux](RUN_DEMO.md)

    This contains instructions to run the Matter Wi-Fi demo using ChipTool running
    on a Linux Machine (either Laptop or Raspberry Pi) - follow this after
    successfully executing the above steps.

1. [optional] [Running Matter Demo over Wi-Fi using Android](WIFI_ANDROID.md)

    This part give you steps to run the Matter Wi-Fi demo using ChipTool running
    on an Android phone (either Laptop or Raspberry Pi).

----
[Table of Contents](../README.md) | [Thread Demo](../thread/DEMO_OVERVIEW.md) | [Wi-Fi Demo](./DEMO_OVERVIEW.md)