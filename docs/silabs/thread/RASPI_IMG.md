# Setting up the Matter Hub (Raspberry Pi)

The Matter Hub consists of the Open Thread Border Router (OTBR) and the chip-tool
running on a Raspberry Pi. Silicon Labs has developed a Raspberry Pi image that
can be downloaded and flashed onto an SD Card for the Raspberry Pi.

In short, the Matter Controller sends IPv6 packets to the OTBR, which converts
the IPv6 packets into Thread packets. The Thread packets are then routed to the
Silicon Labs end device.

<br>

## How to use the Silicon Labs Matter Raspberry Pi Image (Matter Hub)

> Note that if you have already downloaded the Raspberry Pi image and installed
> it, you may only need to update the image that you already have on your
> Raspberry Pi in order to use it with the current release. In this case you can
> follow the instructions on the [Matter Tool Page](CHIP_TOOL.md) to update your
> existing installation.

<br>

### Step 1. Raspberry Pi Image Download

The provided Raspberry Pi image is used as a Matter Controller with the OTBR.

The image can be downloaded from the
[Matter Artifacts page](../general/ARTIFACTS.md)

> Please note that this image, even when zipped up, is quite large ~5GB so this
> download will take a while if you are on a slow connection. This image
> includes both the Ubuntu operating system as well as the OTBR and chip-tool,
> hence the size.

<br>

### Step 2. Flashing your Raspberry Pi

[Raspberry Pi Disk Imager](https://www.raspberrypi.com/software/) can be used to
flash the SD Card that contains the operating system for the Raspberry Pi. Under
Operating System select 'Use Custom' and then select the .img file.

Alternatively, a tool like [balenaEtcher](https://www.balena.io/etcher/) can be
used to flash the image to a micro SD card.

> After flashing the SD card, insert it into the Raspberry Pi and reset the
> Raspberry Pi by unplugging it from the power source and plugging it back in.
> Then, wait at least 10 seconds for it to come up and start the SSH server.

<br>

### Step 3. Finding your Raspberry Pi on the Network

The Raspberry Pi should be connected to a network - this could be Ethernet or a
Wi-Fi network.

> NOTE: If you cannot connect your Raspberry Pi to a network over Wi-Fi or
> Ethernet you do have the option to connect a monitor and keyboard the the
> Raspberry Pi and interact with it that way. In this case you do not need to
> connect your Raspberry Pi to a network as you can interface with it directly
> as you would with any computer running Ubuntu Linux.

The preference here is to use Ethernet, however, if you are using Wi-Fi for your
connection to the Raspberry Pi, see
[Connecting Raspberry Pi to Wi-Fi](https://www.raspberrypi.com/documentation/computers/configuration.html#configuring-networking)
for instructions on how to connect your Raspberry Pi to a Wi-Fi network.

Once you have connected your Raspberry Pi to the network, you need to connect to
your Raspberry Pi over SSH. This requires the IP address of your Raspberry Pi.
See [Finding Your Raspberry Pi](../general/FIND_RASPI.md) for more information
on finding the IP address and connecting to the Raspberry Pi by SSH.

> Please note that if you are using Wi-Fi to connect to your Raspberry Pi over 2.4 GHz you may have difficulty with commissioning your device. The workaround for this issue is to use either Ethernet or 5GHz to connect to your device in order to reserve the 2.4GHz channel for the Matter commissioning.

#### Raspberry Pi Login Credentials

-   user: **`ubuntu`**
-   password: **`raspberrypi`** **OR** **`ubuntu`** (0.3.0 and above)

> **Note:** On later images of the Matter Hub the password has changed to the
> default "ubuntu". you will be asked to change your password the first time you
> log in. You may change it to whatever value you like.

> **Note:** When you log into the Raspberry Pi for the first time over SSH you
> may receive a warning regarding a 'key fingerprint' - this is normal and
> expected. You can get past this by typing '_yes_' at the prompt.

<br>

### Step 4: Using the Matter Hub

The chip-tool, also referred to as the `mattertool`, is provided as a pre-built
application inside the Raspberry Pi image.

Refer to the [chip-tool page](./CHIP_TOOL.md) for information on using the Matter
Hub with `mattertool` commands.
