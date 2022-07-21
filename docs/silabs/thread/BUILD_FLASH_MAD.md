[<< Back to TOC](../README.md)

# How to Build and Flash the Matter Accessory Device (MAD)

The Matter Accessory Device, such as the lighting-app, is the actual Matter
device that you will commission onto the Matter network and control using the
Chip-Tool.

## Step 1: Get the Image File to Flash the MAD

We have provided two ways to get the required image to flash the MAD. You can
use one of the following options:

1. Using the pre-built image file
2. Building the image file from the '`connectedhomeip`' repository

<br>

### **Using the Pre-Built Image File**

All of the Matter Accessory Device image files are accessible through the
[Matter Artifacts Page](../general/ARTIFACTS.md). If you are using the pre-built
image file, you can skip forward to Step #2: Flashing the MAD.

<br>

### **Building the Matter Image File from the Repository**

**1. Clone the Silicon Labs Matter repository**

    Since you are reading this documentation it is assumed that you have already cloned the Silicon Labs Matter GitHub repository and have the right branch. If you have not you can follow the directions below.

The Silicon Labs Matter repo is located in Github here:
https://github.com/SiliconLabs/matter.

In order to clone the Matter repo you'll need to have Git installed on your
local machine. Once you have Git installed you can use the following command:

> `$ git clone https://github.com/SiliconLabs/matter.git`

Once you have cloned the repo, enter the repo and sync all the submodules with
the following command:

> `$ cd matter` <br> `$ git submodule update --init --recursive`

<br>

**2. Build the Matter Accessory Device**

The Matter Accessory Device (lighting-app) can be built out of this repo. Documentation on how to build and use the lighting-app Matter Accessory Device is provided in this [README.md](../../../examples/lighting-app/efr32/README.md)

Please note that you only need to build a single device for
the demo such as the lighting-app. If you wish to build other examples such as
the sleepy end device you are welcome to it is just not necessary for the demo.

The build process puts all image files in the following location:

> \<git location>/matter/out/\<app name>/\<board name>

<br>

## Step 2: Flash the Matter Accessory Device

For more information on how to flash your Silabs development platform consider
the following instructions:
[How to Flash a Silicon Labs Device](../general/FLASH_SILABS_DEVICE.md)

Once your Matter Accessory Device has been flashed it should show a QR code on
the LCD. If no QR Code is present it may be that you need to add a bootloader to
your device.
