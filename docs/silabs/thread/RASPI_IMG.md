# Setting up the Matter Hub (Raspberry Pi)

The Matter Hub consists of the Open Thread Border Router (OTBR) and the ChipTool
running on a Raspberry Pi. Silicon Labs has developed a Raspberry Pi image that
can be downloaded and flashed onto an SD Card for the Raspberry Pi.

In short, the Matter Controller sends IPv6 packets to the OTBR, which converts
the IPv6 packets into Thread packets. The Thread packets are then routed to the
Silicon Labs end device.

<br>

## How to use the Silicon Labs Matter Raspberry Pi Image (Matter Hub)

<br>

### Step 1. Raspberry Pi Image Download

The provided Raspberry Pi image is used as a Matter Controller with the OTBR.

The image can be downloaded from the
[Matter Artifacts page](../general/ARTIFACTS.md)

> Please note that this image, even when zipped up, is quite large ~10GB so this download will take a while if you are on a slow connection. This image includes both the Ubuntu operating system as well as the OTBR and Chip-Tool, so there is a lot of sofware pre-packaged in this image, hence the size.

<br>

### Step 2. Flashing your Raspberry Pi

[Raspberry Pi Disk Imager](https://www.raspberrypi.com/software/) can be used to
flash the SD Card that contains the operating system for the Raspberry Pi. Under Operating System select 'Use Custom' and then select the .img file. 

Alternatively, a tool like [balenaEtcher](https://www.balena.io/etcher/) can be
used to flash the image to a micro SD card.

> After flashing the SD card, insert it into the Raspberry Pi and reset the
> Raspberry Pi by unplugging it from the power source and plugging it back in. Then, wait at least 10 seconds for it to come up and start the
> SSH server.

<br>

### Step 3. Finding your Raspberry Pi on the Network

The Raspberry Pi should be connected to a network - this could be Ethernet or a
Wi-Fi network.

The preference here is to use Ethernet, however if you are using Wi-Fi for your connection to the Raspberry Pi, please see [Connecting Raspberry Pi to Wi-Fi](https://www.raspberrypi.com/documentation/computers/configuration.html#configuring-networking) for instructions on how to connect your Raspberry Pi to a Wi-Fi network.

Once you have connected your Raspberry Pi to the network, you will want to connect to your Raspberry Pi over SSH. In order to do this you will need the IP address of your Raspberry Pi. Please see the [Finding Your Raspberry Pi page](../general/FIND_RASPI.md) for
more information on finding the Raspberry Pi on the local network.

#### Raspberry Pi Login Credentials

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

----
[Table of Contents](../README.md) | [Thread Demo](./DEMO_OVERVIEW.md) | [Wi-Fi Demo](../wifi/DEMO_OVERVIEW.md)