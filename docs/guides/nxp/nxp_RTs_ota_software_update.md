# Matter Over-The-Air Software Update with NXP RTs example applications

## Overview

This document describes OTA feature on NXP devices:

-   RW61x
-   RT1060_EVK-C
-   RT1170_EVK-B

The OTA Requestor feature enables the device to be informed of, download and
apply a software update from an OTA Provider.

This section explains how to perform an OTA Software Update with NXP RTs example
applications. Throughout this guide, the all-clusters application is used as an
example.

In general, the Over-The-Air Software Update process consists of the following
steps :

-   The OTA Requestor queries an update image from the OTA Provider which
    responds according to its availability.
-   The update image is received in blocks and stored in the external flash of
    the device.
-   Once the update image is fully downloaded, the bootloader is notified and
    the device resets applying the update in test-mode.
-   If the test is successful, the update is applied permanently. Otherwise, the
    bootloader reverts back to the primary application, preventing any
    downgrade.

### Flash Memory Layout

The RTs Flash is divided into different regions as follow :

-   Bootloader : MCUBoot resides at the base of the flash.
-   Primary application partition : The example application which would be run
    by the bootloader (active application). The size reserved for this partition
    is 4.4 MB.
-   Secondary application partition : Update image received with the OTA
    (candidate application). The size reserved for the partition is 4.4 MB.

Notes :

-   For RW61x: The CPU1/CPU2 firmware are embedded in the CPU3 example
    application.
-   The sizes of the primary and secondary applications are provided as an
    example (currently 4.4 MB is reserved for each partition). The size can be
    changed by modifying the `m_app_max_sectors` value in the linker script of
    the application .

### MCUBoot Bootloader

MCUBoot is an open-source secure bootloader used by RW61x to apply the
self-upgrade. For more details, please refer to the
[MCUBoot documentation](https://github.com/mcu-tools/mcuboot/blob/main/docs/design.md).

For RTs platform, the bootloader is configured to use the flash remapping
mechanism by default, in order to perform the image upgrade. This is achieved by
using the `MCUBoot DIRECT-XIP` upgrade mode.

## OTA Software Update process for RTs example application

### Flashing the bootloader

In order for the device to perform the software update, the MCUBoot bootloader
must be flashed first at the base of the flash. A step-by-step guide is given
below.

-   It is recommended to start with erasing the external flash of the device,
    for this JLink from Segger can be used. It can be downloaded and installed
    from https://www.segger.com/products/debug-probes/j-link. Once installed,
    JLink can be run using the command line :

```
$ JLink
```

Run the following commands :

Connect J-Link debugger to device:

```sh
J-Link > connect
Device> ? # you will be presented with a dialog -> select `RW612` for RW61x, `MIMXRT1062XXX6B` for RT1060, `MIMXRT1176xxxA_M7` for RT1170
Please specify target interface:
J) JTAG (Default)
S) SWD
T) cJTAG
TIF> S
Specify target interface speed [kHz]. <Default>: 4000 kHz
Speed> # <enter>
```

Erase flash:

```
J-Link > exec EnableEraseAllFlashBanks
```

For RW61x

```
J-Link > erase 0x8000000, 0x88a0000
```

For RT1060-EVK-C

```
J-Link > erase 0x60000000, 0x61000000
```

For RT1170-EVK-B

```
J-Link > erase 0x30000000, 0x34000000
```

-   MCUBoot application can be built with SDK installed, using instructions
    below.
-   Retrieve the mcuboot directory located at
    _'<matter_repo_root>/third_party/nxp/nxp_matter_support/github_sdk/common_sdk/repo/examples/<a href="#1" id="1-ref">`RTboard`<sup>1</sup></a>/ota_examples/`mcuboot_opensource/armgcc`'_

_<a id="1" href="#1-ref"><sup>1</sup></a> `rdrw612bga` or `frdmrw612` for RW61x,
`evkcmimxrt1060` for RT1060-EVK-C, `evkbmimxrt1170` for RT1170-EVK-B_

```
user@ubuntu: cd ~/Desktop/connectedhomeip/third_party/nxp/nxp_matter_support/github_sdk/common_sdk/repo/examples/<RT_board>/ota_examples/mcuboot_opensource/armgcc
```

-   Build the mcuboot application with running
    <a href="#2" id="2-ref">`build_script`<sup>2</sup></a>

```
user@ubuntu: chmod +x <build_script>
user@ubuntu: export ARMGCC_DIR=/opt/gcc-arm-none-eabi-10.3-2021.10   # with ARMGCC_DIR referencing the compiler path
user@ubuntu: ./<build_script>
```

_<a id="2" href="#2ref"><sup>2</sup></a> `build_flash_release.sh` for RW61x,
`build_flexspi_nor_release.sh` for RT1060 and RT1170_

-   Program the generated binary to the target board.

```
J-Link > loadbin <path_to_mcuboot>/mcuboot_opensource.elf
```

-   If it runs successfully, the following logs will be displayed on the
    terminal :

```
hello sbl.
Disabling flash remapping function
Bootloader Version 2.0.0
Image 0 Primary slot: Image not found
Image 0 Secondary slot: Image not found
No slot to load for image 0
Unable to find bootable image
```

Note : By default, mcuboot application considers the primary and secondary
partitions to be the size of 4.4 MB. If the size is to be changed, the partition
addresses should be modified in the `flash_partitioning.h` accordingly. For more
information about the flash partitioning with mcuboot, please refer to the
dedicated `readme.txt` located in

> _<matter_repo_root>/third_party/nxp/nxp_matter_support/github_sdk/common_sdk/repo/examples/<a href="#1" id="1ref">`RTboard`<sup>1</sup></a>/ota_examples/`mcuboot_opensource`/._

### Generating and flashing the signed application image

After flashing the bootloader, the application can be programmed to the board.
The image must have the following format :

-   Header : contains general information about the image (version, size,
    magic...)
-   Code of the application : generated binary
-   Trailer : contains metadata needed by the bootloader such as the image
    signature, the upgrade type, the swap status...

The all-clusters application can be generated using the instructions from the

README.md<a href="#3" id="3-ref"><sup>3</sup></a>'Building' section. The
application is automatically linked to be executed from the primary image
partition, taking into consideration the offset imposed by mcuboot.

_<a id="3" href="#3-ref"><sup>3</sup></a>
[RW61x README.md 'Building'](../../../examples/all-clusters-app/nxp/rt/rw61x/README.md#building),
[RT1060 README.md 'Building'](../../../examples/all-clusters-app/nxp/rt/rt1060/README.md#building),
[RT1170 README.md 'Building'](../../../examples/all-clusters-app/nxp/rt/rt1170/README.md#building)_

The resulting executable file found in
out/release/chip-<a href="#4" id="4-ref">`board`<sup>4</sup></a>-all-cluster-example
needs to be converted into raw binary format as shown below.

_<a id="4" href="#4-ref"><sup>4</sup></a> `rw61x` for RW61x, `rt1060` for
RT1060-EVK-C, `rt1170` for RT1170-EVK-B_

```sh
arm-none-eabi-objcopy -R .flash_config -R .NVM -O binary chip-<"board">-all-cluster-example chip-<"board">-all-cluster-example.bin
```

To sign the image and wrap the raw binary of the application with the header and
trailer, "`imgtool`" is provided in the SDK and can be found in
"`<matter_repo_root>/third_party/nxp/nxp_matter_support/github_sdk/common_sdk/repo/middleware/mcuboot_opensource/scripts/`".

The following commands can be run (make sure to replace the /path/to/file/binary
with the adequate files):

```sh
user@ubuntu: cd ~/Desktop/<matter_repo_root>/third_party/nxp/nxp_matter_support/github_sdk/common_sdk/repo/middleware/mcuboot_opensource/scripts/

user@ubuntu: python3 imgtool.py sign --key ~/Desktop/<matter_repo_root>/third_party/nxp/nxp_matter_support/github_sdk/common_sdk/repo/examples/<RT board>/ota_examples/mcuboot_opensource/keys/sign-rsa2048-priv.pem --align 4 --header-size 0x1000 --pad-header --pad --confirm --slot-size 0x440000 --max-sectors 1088 --version "1.0" ~/Desktop/connectedhomeip/examples/all-clusters-app/nxp/rt/<"rt_board">/out/debug/chip-<"rt_board">-all-cluster-example.bin ~/Desktop/connectedhomeip/examples/all-clusters-app/nxp/rt/<"rt_board">/out/debug/chip-<"rt_board">-all-cluster-example_SIGNED.bin
```

Notes :

-   The arguments `slot-size` and `max-sectors` are aligned with the size of the
    partitions reserved for the primary and the secondary applications. (By
    default the size considered is 4.4 MB for each application). If the size of
    these partitions are modified, the `slot-size` and `max-sectors` should be
    adjusted accordingly.
-   In this example, the image is signed with the private key provided by the
    SDK as an example
    (`<matter_repo_root>/third_party/nxp/nxp_matter_support/github_sdk/common_sdk/repo/examples/<RT_board>/ota_examples/mcuboot_opensource/keys/sign-rsa2048-priv.pem`),
    MCUBoot is built with its corresponding public key which would be used to
    verify the integrity of the image. It is possible to generate a new pair of
    keys using the following commands. This procedure should be done prior to
    building the mcuboot application.

-   To generate the private key :

```
user@ubuntu: python3 imgtool.py keygen -k priv_key.pem -t rsa-2048
```

-   To extract the public key :

```
user@ubuntu: python3 imgtool.py getpub -k priv_key.pem
```

-   The extracted public key can then be copied to the
    `<matter_repo_root>/third_party/nxp/nxp_matter_support/github_sdk/common_sdk/repo/examples/<RT_board>/ota_examples/mcuboot_opensource/keys/sign-rsa2048-pub.c`,
    given as a value to the rsa_pub_key[] array.

The resulting output is the signed binary of the application version "1.0".

JLink can be used to flash the application using the command :

For RW61x

```
J-Link > loadbin chip-rw61x-all-cluster-example_SIGNED.bin 0x8020000
```

For RT1060-EVK-C

```
J-Link > loadbin chip-rt1060-all-cluster-example_SIGNED.bin 0x60040000
```

For RT1170-EVK-B

```
J-Link > loadbin chip-rt1170-all-cluster-example_SIGNED.bin 0x30040000
```

The bootloader should then be able to jump directly to the start of the
application and run it.

### Generating the OTA Update Image

To generate the OTA update image the same procedure can be followed from the
[Generating and flashing the signed application image](#generating-and-flashing-the-signed-application-image)
sub-section, replacing the "--version "1.0"" argument with "--version "2.0""
(recent version of the update), without arguments "--pad" "--confirm" when
running `imgtool` script during OTA Update Image generation.

Note : When building the update image, the build arguments
nxp_software_version=2 nxp_software_version_string=\"2.0\" can be added to the
gn gen command in order to specify the upgraded version.

When the signed binary of the update is generated, the file should be converted
into OTA format. To do so, the ota_image_tool is provided in the repo and can be
used to convert a binary file into an .ota file.

```sh
user@ubuntu:~/connectedhomeip$ : ./src/app/ota_image_tool.py create -v 0xDEAD -p 0xBEEF -vn 2 -vs "2.0" -da sha256 chip-<"rt_board">-all-cluster-example_SIGNED.bin chip-rw61x-all-cluster-example.ota
```

The generated OTA file can be used to perform the OTA Software Update. The
instructions below describe the procedure step-by-step.

### Performing the OTA Software Update

Setup example :

-   [Chip-tool](../../../examples/chip-tool/README.md) application running on
    the RPi.
-   OTA Provider application built on the same RPi (as explained below).
-   RT board programmed with the example application (with the instructions
    above).

Before starting the OTA process, the Linux OTA Provider application can be built
on the RPi (if not already present in the pre-installed apps) :

```
user@ubuntu:~/connectedhomeip$ : ./scripts/examples/gn_build_example.sh examples/ota-provider-app/linux out/ota-provider-app chip_config_network_layer_ble=false
user@ubuntu:~/connectedhomeip$ : rm -rf /tmp/chip_*
```

```sh
user@ubuntu:~/connectedhomeip$ : ./out/ota-provider-app/chip-ota-provider-app -f chip-<"rt_board">-all-cluster-example.ota
```

The OTA Provider should first be provisioned with chip-tool by assigning it the
node id 1, and then granted the ACL entries :

```
user@ubuntu:~/connectedhomeip$ : ./out/chip-tool-app/chip-tool pairing onnetwork 1 20202021
user@ubuntu:~/connectedhomeip$ : ./out/chip-tool-app/chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}, {"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": null, "targets": null}]' 1 0
```

The second step is to provision the device with the node id 2 using ble-wifi or
ble-thread commissioning. For example :

```
user@ubuntu:~/connectedhomeip$ : ./out/chip-tool-app/chip-tool pairing ble-wifi 2 WIFI_SSID WIFI_PASSWORD 20202021 3840
```

Once commissioned, the OTA process can be initiated with the
"announce-ota-provider" command using chip-tool (the given numbers refer
respectively to [ProviderNodeId][vendorid] [AnnouncementReason][endpoint]
[node-id][endpoint-id]) :

```
user@ubuntu:~/connectedhomeip$ : ./out/chip-tool-app/chip-tool otasoftwareupdaterequestor announce-otaprovider 1 0 0 0 2 0
```

When the full update image is downloaded and stored, the bootloader will be
notified and the device will reboot with the update image.
