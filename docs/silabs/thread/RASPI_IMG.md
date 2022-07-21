[<< Back to TOC](../README.md)

# Setting up the Matter Hub (Raspberry Pi)

The Matter Hub consists of the Open Thread Border Router and the ChipTool
running on a Raspberry Pi. Silicon Labs has developed a Raspberry Pi image which
can be downloaded and flashed onto an SD Card for the Raspberry Pi.

In short, the Matter Controller sends IPv6 packets to the OTBR, which converts
the IPv6 packets into Thread packets. The Thread packets are then routed to the
Silicon Labs end device.

<br>

## How to use the Silicon Labs Matter Raspberry Pi Image (Matter Hub)

<br>

The Raspberry Pi should be connected to a network - this could be ethernet or a
Wifi network.

Please refer to this page on how to connect your Raspberry Pi to a wifi network:
[Connecting Raspberry Pi to Wifi](https://www.raspberrypi.com/documentation/computers/configuration.html#configuring-networking)

<br>

### Step 1. Raspberry Pi Image Download

The provided Raspberry Pi image is used as a Matter Controller with the OTBR.

The image can be downloaded from the
[Matter Artifacts page](../general/ARTIFACTS.md)

<br>

### Step 2. Flashing your Raspberry Pi

[Raspberry Pi Disk Imager](https://www.raspberrypi.com/software/) can be used to
flash the SD Card which contains the operating system for the Raspberry Pi.

Alternatively, a tool like [balenaEtcher](https://www.balena.io/etcher/) can be
used to flash the image to a micro SD card.

> After flashing the SD card, insert it into the Raspberry Pi and reset the
> Raspberry Pi. Then, wait at least 10 seconds for it to come up and start the
> SSH server.

<br>

### Step 3. Finding your Raspberry Pi on the Network

Please see the [Finding Your Raspberry Pi page](../general/FIND_RASPI.md) for
more information on finding the Raspberry Pi on the local network.

##### Raspberry Pi Login Credentials

-   user: **ubuntu**
-   password: **raspberrypi**

> When you log into the Raspberry Pi for the first time over SSH you may receive
> a warning regarding a 'key fingerprint' - this is normal and expected. You can
> get past this by just typing '_yes_' at the prompt.

<br>

### Step 4: Using the Matter Hub

The ChipTool, also referred to as the `mattertool`, is provided as a pre-built
application inside the Raspberry Pi image.

Please refer to the ChipTool page for information on using the Matter Hub with
`mattertool` commands: [ChipTool page](./CHIP_TOOL.md)
