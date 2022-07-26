# Matter over Thread Demo Overview

This section reviews the steps for running an example lighting-app for Matter
over Thread. See [this file](THREAD.md) for an introduction to the
Matter over Thread setup.

At a high level, we will walk through starting a Thread network, commissioning a
new device to the Thread network using Bluetooth LE, and finally sending a basic
OnOff command to the end device.

<br>

## Prerequisites

Before beginning your Matter project, you should have the software and systems described in
[Matter Hardware and Software Prerequisites](./THREAD_PREREQS.md).

<br>

## Step 1: Setting up the Matter Hub (Raspberry Pi)

The Matter Hub consists of the OTBR and the ChipTool running on a Raspberry Pi.
Silicon Labs has developed a Raspberry Pi image that can be downloaded and
flashed onto an SD Card, which is then inserted into the Raspberry Pi.

The Matter Controller sends IPv6 packets to the OTBR, which converts the IPv6
packets into Thread packets. The Thread packets are then routed to the Silicon
Labs end device.

See [How to use Matter Hub \(Raspberry Pi\) Image](./RASPI_IMG.md) for setup
instructions. 

<br>

## Step 2: Flash the RCP

The Radio Co-Processor (RCP) is a Thread device that connects to the Raspberry
Pi via USB. To flash the RCP, connect it to your laptop via
USB. Thereafter, it should be connected to the Raspberry Pi via USB as well. Prebuilt RCP images are available for the demo

Information on flashing and optionally building the RCP is located here:
[How To Build and Flash the RCP](RCP.md)

<br>

## Step 3: Build and Flash the MAD

The Matter Accessory Device (MAD) is the actual Matter device that will be
commissioned onto the Matter network and controlled using the ChipTool. Prebuilt MAD images are available for the demo.

Information on flashing and optionally building the Matter Accessory device is located
here: [How To Build and Flash the Matter Accessory Device](./BUILD_FLASH_MAD.md)

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

----
[Table of Contents](../README.md) | [Thread Demo](./DEMO_OVERVIEW.md) | [Wi-Fi Demo](../wifi/DEMO_OVERVIEW.md)