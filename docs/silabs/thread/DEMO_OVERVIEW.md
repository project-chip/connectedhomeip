# Matter over Thread Demo Overview

This section reviews the steps for running an example lighting app for Matter
over Thread. See [this file](THREAD.md) for an introduction to the Matter over
Thread setup.

At a high level, this section walks through starting a Thread network, commissioning a
new device to the Thread network using Bluetooth LE, and finally sending a basic
OnOff command to the end device.

<br>

## Step 0: Prerequisites

Before beginning your Silicon Labs Matter over Thread project be sure you have satisfied all
of the [Matter Hardware](../general/HARDWARE_REQUIREMENTS.md) and
[Matter Software](../general/SOFTWARE_REQUIREMENTS.md) Requirements.

<br>

## Step 1: Setting up the Matter Hub (Raspberry Pi)

The Matter Hub consists of the Open Thread Border Router (OTBR) and the chip-tool running on a Raspberry Pi.
Silicon Labs has developed a Raspberry Pi image combining the OTBR and chip-tool that can be downloaded and
flashed onto an SD Card, which is then inserted into the Raspberry Pi.

The Matter Controller sends IPv6 packets to the OTBR, which converts the IPv6
packets into Thread packets. The Thread packets are then routed to the Silicon
Labs end device.

See [How to use Matter Hub \(Raspberry Pi\) Image](./RASPI_IMG.md) for setup
instructions.

<br>

## Step 2: Flash the RCP

The Radio Co-Processor (RCP) is a Thread device that connects to the Raspberry
Pi via USB. To flash the RCP, connect it to your laptop via USB. Thereafter, it
should be connected to the Raspberry Pi via USB as well. Prebuilt RCP images are
available for the demo

Information on flashing and optionally building the RCP is located here:
[How To Build and Flash the RCP](RCP.md)

<br>

## Step 3: Build and Flash the MAD

The Matter Accessory Device (MAD) is the actual Matter device that will be
commissioned onto the Matter network and controlled using the chip-tool. Prebuilt
MAD images are available for the demo.

Information on flashing and optionally building the Matter Accessory device is
located here:
[How To Build and Flash the Matter Accessory Device](./BUILD_FLASH_MAD.md)

<br>

## Step 4: Commission and Control the MAD

Once the Matter Accessory device has been flashed onto your hardware you can
commission it from the Matter Hub using the commands provided in the Raspberry
Pi image:

| Command                | Usage                                              |
| ---------------------- | -------------------------------------------------- |
| mattertool startThread | Starts the thread network on the OTBR              |
| mattertool bleThread   | Starts commissioning of a MAD using chip-tool       |
| mattertool on          | Sends an **on** command to the MAD using chip-tool  |
| mattertool off         | Sends an **off** command to the MAD using chip-tool |

<br>
