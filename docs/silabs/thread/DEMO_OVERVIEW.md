[<< Back to TOC](../README.md)

# Matter Demo over Thread Overview

This guide goes through the steps for running an example lighting-app for Matter
over Thread. Please look at [this](THREAD.md) file for an introduction to the
Matter over Thread setup.

At a high level, we will walk through starting a thread network, commissioning a
new device to the thread network using BLE, and finally sending a basic OnOff
command to the end device.

<br>

## Step 0: Prerequisites

Before beginning your Matter project, consider the
[Matter Hardware and Software Prerequisites](./THREAD_PREREQS.md). Here you will find all the information you need on Silicon Labs hardware supported for Matter development.

<br>

## Step 1: Setting up the Matter Hub (Raspberry Pi)

The Matter Hub consists of the OTBR and the ChipTool running on a Raspberry Pi.
Silicon Labs has developed a Raspberry Pi image which can be downloaded and
flashed onto an SD Card that is then to be inserted into the Raspberry Pi.

The Matter Controller sends IPv6 packets to the OTBR, which converts the IPv6
packets into Thread packets. The Thread packets are then routed to the Silicon
Labs end device.

Please refer to this guide for the setup:
[How to use Matter Hub \(Raspberry Pi\) Image](./RASPI_IMG.md)

<br>

## Step 2: Build and Flash the RCP

The Radio Co-Processor (RCP) is a thread device that connects to the Raspberry
Pi via USB. First, to flash the RCP, it should be connected to your laptop via
USB. Thereafter, it should be connected to the Raspberry Pi via USB as well.

Information on building and flashing the RCP is located here:
[How To Build and Flash the RCP](RCP.md)

<br>

## Step 3: Build and Flash the MAD

The Matter Accessory Device (MAD) is the actual Matter device that will be
commissioned onto the Matter network and control using the ChipTool.

Information on how to build and flash the Matter Accessory device is located
here: [How To Build and Flash the Matter Accessory Device](./MATTER_DEVICE.md)

<br>

## Step 4: Commission and Control the MAD

Once the Matter Accessory device has been flashed onto your hardware you can
commission it from the Matter Hub using the commands provided in the Raspberry
Pi image:

| Command                | Usage                                              |
| ---------------------- | -------------------------------------------------- |
| mattertool startThread | Starts the thread network on the OTBR              |
| mattertool bleThread   | Starts commissioning of a MAD using ChipTool       |
| mattertool on          | Sends an **on** command to the MAD using ChipTool  |
| mattertool off         | Sends an **off** command to the MAD using ChipTool |

<br>
