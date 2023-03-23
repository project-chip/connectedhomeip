# Matter over Wi-Fi Demo Overview

This document walks through the steps to build the Matter 1.1 applications, such as lighting, light Switch, window covering, thermostat, door lock and On/Off Plug for
EFR32MG24 and SiWx917 SoC boards. A complete list of hardware supported for Wi-Fi is included on the [Hardware Requirements page](../general/HARDWARE_REQUIREMENTS.md).

## Step 1: Matter Wi-Fi Prerequisites

Before running the Matter Wi-Fi demo or developing for Wi-Fi make sure that you have all the required hardware and software for each
use case.

1. [Matter Hardware Requirements](../general/HARDWARE_REQUIREMENTS.md)
1. [Matter Software Requirements](../general/SOFTWARE_REQUIREMENTS.md)

## Step 2: Building the chip-tool for Wi-Fi

In order to run the Matter Wi-Fi demo you will need to run the chip-tool on one
of two platforms, either Linux/Mac or Raspberry Pi.

1. [Build Linux Environment](BUILD_CHIP_ENV.md)

2. [Build Raspberry-Pi Environment](BUILD_PI_ENV.md)

## Step 3: Building the Matter Accessory Device (MAD) for Wi-Fi

Follow the steps in [Software Setup](SW_SETUP.md)

Build commands are provided for the EFR32MG24 host processor, with the RS9116, SiWx917 and WF200 network co-processors.

Build command is provided for the SiWx917 SoC processor.

## Step 4: Running the Demo

[Running the Matter Demo on EFR32 hosts](RUN_DEMO.md)

[Running the Matter Demo on SiWx917 SoC](RUN_DEMO_SiWx917_SoC.md)

This contains instructions to run the Matter Wi-Fi demo using chip-tool
running on a Linux Machine (either Laptop or Raspberry Pi) - follow this
after successfully executing the above steps.

---

[Table of Contents](../README.md) | [Thread Demo](../thread/DEMO_OVERVIEW.md) |
[Wi-Fi Demo](./DEMO_OVERVIEW.md)
