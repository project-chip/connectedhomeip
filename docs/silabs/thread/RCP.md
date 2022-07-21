[<< Back to TOC](../README.md)

# How to Build and Flash the Radio Co-Processor (RCP)

The Radio Co-Processor is a 15.4 stack image flashed onto a Silicon Labs
development kit or Thunderboard Sense 2. The 15.4 stack on the development kit
communicates with the higher layers of the Thread stack running on the Raspberry
Pi over a USB connection.

First, in order to flash the RCP, it should be connected to your laptop directly
over USB.

<!-- In order to for the OTBR to work, both the RCP and
the OTBR need to be built off a commit that allows them to communicate properly. -->

<br>

## Step 1: Get or Build the Image File to Flash the RCP

We have provided two ways to get the required image to flash the RCP. You can
use one of the following options:

1. Using the pre-built image `ot-rcp` image file
2. Building the image file from the '`ot-efr32`' repository which is listed on
   the [Matter Repositories and Commit Hashes page](../general/COMMIT_HASHES.md)

<br>

### **Using the Pre-built Image File**

All of the RCP image files are accessible through the
[Matter Artifacts Page](../general/ARTIFACTS.md). If you are using the pre-built
image file, you can skip forward to Step #2: Flashing the RCP.

<br>

### **Building the Image File from the Repository**

**1. Clone the ot-efr32 repository**

The ot-efr32 repo is located in Github here:
https://github.com/SiliconLabs/ot-efr32.

In order to clone the ot-efr32 repo, you'll need to have Git installed on your
local machine. Once you have Git installed you can use the following command:

> `$ git clone https://github.com/SiliconLabs/ot-efr32.git`

Once you have cloned the repo, enter the repo and sync all the submodules with
the following command:

> `$ cd ot-efr32` <br> > `$ git submodule update --init --recursive`

After updating the submodules you can checkout the correct branch or commit hash
for the system. Check the current branch and commit hash used here:
[Matter Branches and Commit Hashes](../general/COMMIT_HASHES.md)

> `$ git checkout <commit hash>`

<br>

**2. Build the RCP**

Once you have checked out the correct hash, follow the instructions here:
https://github.com/SiliconLabs/ot-efr32/blob/main/src/README.md to build the RCP
image for your EFR platform.

This process will build several images for your board. The filename of the image
to be flashed onto the board to create an RCP is '`ot-rcp.s37`'.

The output of the build process puts all the image files in the following
location: `<git>/ot-efr32/build/<efr32xgxx>`

<br>

## Step 2: Flash the RCP

Once you get the RCP image '`ot-rcp.s37`' after either downloading it from the
link provided above, or building the image file from the repo as documented
above, you can flash it onto your device which will become the RCP attached to
your Raspberry Pi. Flashing of the device is done directly from your laptop and
not through the Raspberry Pi, so make sure that the device is connected directly
over USB to your laptop. Further information on flashing a Silicon Labs device
is located here:
[How to Flash a Silicon Labs Device](../general/FLASH_SILABS_DEVICE.md)

Once you have flashed your RCP device you can disconnect it from you laptop and
connect it via USB to the Raspberry Pi.

The Raspberry Pi's Open Thread Border Router can then use the RCP to communicate
with the Thread network.
