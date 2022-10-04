# How to Build and Flash the Matter Accessory Device (MAD)

The Matter Accessory Device, such as the lighting-app, is the actual Matter
device that you will commission onto the Matter network and control using the
chip-tool.

## Step 1: Get the Image File to Flash the MAD

We have provided two ways to get the required image to flash the MAD. You can
use one of the following options:

1. Use the pre-built image file
2. Build the image file from the '`matter`' repository

<br>

-   ### **Using the Pre-Built Image File**

    All of the Matter Accessory Device image files are accessible through the
    [Matter Artifacts Page](../general/ARTIFACTS.md). If you are using a
    pre-built image file, you can skip forward to Step #2: Flashing the MAD.

    If you are coming from Simplicity Studio, you may have already installed the
    demo image in Simplicity Studio in which case you can skip forward to the
    next step.

<br>

-   ### **Building the Matter Image File from the Repository**

    > **IMPORTANT:** A complete list of software requirements for Silicon Labs
    > Matter 15.4 development is included on the
    > [Matter Software Requirements](../general/SOFTWARE_REQUIREMENTS.md) page.
    > Be sure that you have satisfied these requirements before proceeding.

    The Matter Accessory Device (lighting-app) can be built out of this repo.

    Documentation on how to build and use the lighting-app Matter Accessory
    Device is provided in this
    [README.md](../../../examples/lighting-app/efr32/README.md)

    Please note that you only need to build a single device for the demo such as
    the lighting-app. If you wish to build other examples such as the sleepy end
    device you are welcome to, but it is not necessary for the demo.

    The build process puts all image files in the following location:

    `<git location>/matter/out/<app name>/<board name>`

<br>

## Step 2: Flash the Matter Accessory Device

For more information on how to flash your Silabs development platform see the
following instructions:
[How to Flash a Silicon Labs Device](../general/FLASH_SILABS_DEVICE.md)

Once your Matter Accessory Device has been flashed it should show a QR code on
the LCD. If no QR Code is present it may be that you need to add a bootloader to
your device. Bootloader images are provided on the
[Matter Artifacts page](../general/ARTIFACTS.md).

---

[Table of Contents](../README.md) | [Thread Demo](./DEMO_OVERVIEW.md) |
[Wi-Fi Demo](../wifi/DEMO_OVERVIEW.md)
