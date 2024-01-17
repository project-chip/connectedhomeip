# Matter over Wi-Fi Demo Overview

This document walks through the steps to build the Matter 1.1 and 1.2 applications, such as Lighting, Light Switch, Window Covering, Thermostat, Door Lock and On/Off Plug for both the NCP and SoC examples on the 
EFR32MG24 and SiWx917 SoC boards. A complete list of hardware supported for Wi-Fi is included on the [Hardware Requirements page](../general/HARDWARE_REQUIREMENTS.md).

## Step 1: Matter Wi-Fi Prerequisites

Before running the Matter Wi-Fi demo or developing for Wi-Fi make sure that you have all the required hardware and software for each
use case.

1. [Matter Hardware Requirements](../general/HARDWARE_REQUIREMENTS.md)
1. [Matter Software Requirements](../general/SOFTWARE_REQUIREMENTS.md)

## Step 2: Building the chip-tool for Wi-Fi

In order to run the Matter over Wi-Fi demo you will need to run the chip-tool on one
of two platforms, either Linux/Mac or Raspberry Pi.

1. [Build Linux Environment](SW_SETUP.md)

2. [Build Raspberry-Pi Environment](BUILD_PI_ENV.md)

## Step 3: Building the Matter Accessory Device (MAD) for Wi-Fi

To build a Matter Wi-Fi Accessory Device, follow the steps in the [Software Setup](SW_SETUP.md)

Build commands are provided for the EFR32MG24 host processor, with the RS9116, SiWx917 and WF200 network co-processors.

Build commands are also provided for the SiWx917 SoC processor.

## Step 4: Running the Demo

[Running the Matter Demo on EFR32 hosts](RUN_DEMO.md)

[Running the Matter Demo on SiWx917 SoC](RUN_DEMO_SiWx917_SoC.md)

This contains instructions to run the Matter Wi-Fi demo using the chip-tool
running on a Linux Machine (either Laptop or Raspberry Pi) - follow this
after successfully executing the above steps.
