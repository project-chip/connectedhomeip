# Matter over Wi-Fi Demo Overview

This document walks through the steps to build the Matter application i.e lighting, thermostat, door-lock for<br>
EFR32MG12 boards:<br>
`BRD4161A, BRD4163A, BRD4164A`

EFR32MG24 boards:<br>
`BRD4186C, BRD4187C`

Silicon Labs has two families of Wi-Fi adapters (1.RS911X 2.WF200). Both are supported in this Wi-Fi port of Matter.

## Step 1: Matter Wi-Fi Prerequisites

Before you being running the Matter Wi-Fi demo or developing for Wi-Fi you will
want to make sure that you have all the required hardware and software for each
use case.

1. [Matter Wi-Fi Prerequisites](WIFI_PREREQS.md)

## Step 2: Building the Chip-Tool for Wi-Fi

In order to run the Matter Wi-Fi demo you will need to run the Chip-Tool on one
of two platforms, either Linux/Mac or Raspberry Pi.

1. [Build Linux Environment](BUILD_CHIP_ENV.md)

2. [Build Raspberry-Pi Environment](BUILD_PI_ENV.md)

## Step 3: Building the Matter Accessory Device (MAD) for Wi-Fi

Follow the steps in [Software Setup](SW_SETUP.md)

Build commands are provided for both the RS911x and WF200 adapters.

## Step 4: Running the Demo

[Running Matter Demo over Wi-Fi using Linux](RUN_DEMO.md)

This contains instructions to run the Matter Wi-Fi demo using ChipTool
running on a Linux Machine (either Laptop or Raspberry Pi) - follow this
after successfully executing the above steps.

---

[Table of Contents](../README.md) | [Thread Demo](../thread/DEMO_OVERVIEW.md) |
[Wi-Fi Demo](./DEMO_OVERVIEW.md)
