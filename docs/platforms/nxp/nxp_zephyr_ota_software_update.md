# Matter Over-The-Air Software Update with NXP using Zephyr example applications

## Overview

The OTA Requestor feature enables the device to be informed of, download and
apply a software update from an OTA Provider.

This section explains how to perform an OTA Software Update with NXP platform
using NXP/Zephyr SDK. Throughout this guide, the all-clusters application is
used as an example.

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

The Flash is divided into different regions as follow :

-   Bootloader : MCUBoot resides at the base of the flash.
-   Primary application partition : The example application which would be run
    by the bootloader (active application).
-   Secondary application partition : Update image received with the OTA
    (candidate application).

The size reserved for each partition can be found in
`<example folder>/boards/<board>.overlay`.

Notes :

-   When applicable, BLE/15.4/Wi-Fi firmware are embedded in the application
    binary, ensuring compatibility between the application and the controllers.
-   The sizes of the primary and secondary applications are provided as an
    example. The size can be changed by overriding the partitions located at
    `<board>.overlay`.

### MCUBoot Bootloader

Current OTA solution is using MCUBoot Bootloader. MCUBoot is an open-source
secure bootloader used to apply the self-upgrade. For more details, please refer
to the
[MCUBoot documentation](https://github.com/mcu-tools/mcuboot/blob/main/docs/design.md).

In our use case, the bootloader runs the application residing in the primary
partition. In order to run the OTA update image, the bootloader will swap the
content of the primary and the secondary partitions. This type of upgrade is
called swap-move and is the default upgrade configured by MCUBoot.

## OTA Software Update process

### Generating and flashing application image

The all cluster app is able to demonstrate the usage of OTA. To have this OTA
support, prj_ota.conf configurations needs to be used. This can be done by
adding `-DEXTRA_CONF_FILE=prj_ota.conf` to the west build command.

Current OTA implementation automates the following procedures:

-   Generation of MCUBOOT image (File generated:
    `modules/connectedhomeip/build_mcuboot/zephyr/zephyr.bin`)\*
-   Generation of Matter application image (File generated:
    `zephyr/zephyr.bin`)\*
-   Signature of the application image (File generated:
    `zephyr/zephyr.signed.bin`)\*
-   Generation of a single binary merging the signed application with the
    MCUBoot Image (File generated: `zephyr/zephyr_full.bin`)\*

> **Note**: \*All paths are relative to the output folder.

The final binary to be used is `zephyr_full.bin`.

The application image have the following format :

-   Header : contains general information about the image (version, size,
    magic...)
-   Code of the application : generated binary
-   Trailer : contains metadata needed by the bootloader such as the image
    signature, the upgrade type, the swap status...

In the all-cluster-app example, the image is signed with the default private key
provided by MCUBoot(`/zephyrproject/bootloader/mcuboot/root-rsa-2048.pem`).
MCUBoot is built with its corresponding public key which would be used to verify
the integrity of the image. It is possible to generate a new pair of keys using
the following commands. This procedure should be done prior to building the
mcuboot application.

-   To generate the private key :

```
user@ubuntu: python3 imgtool.py keygen -k priv_key.pem -t rsa-2048
```

-   To extract the public key :

```
user@ubuntu: python3 imgtool.py getpub -k priv_key.pem
```

To use a different key than the default one, `CONFIG_BOOT_SIGNATURE_KEY_FILE`
and `CONFIG_MCUBOOT_SIGNATURE_KEY_FILE` needs to point to that same key.

-   `CONFIG_BOOT_SIGNATURE_KEY_FILE`: This is used for the MCUboot bootloader
    image. The path to the key can be either absolute or relative. Relative
    paths starts from the MCUBoot repository root. This option can be changed
    in: `config/nxp/app/bootloader.conf`

-   `CONFIG_MCUBOOT_SIGNATURE_KEY_FILE`: This is used for the application to be
    loaded by the bootloader. The path can be either absolute or relative.
    Relative paths starts from the west workspace location. This option can be
    changed in the application .conf files.

Refer to those two files for more information:

-   [MCUBoot Config used for the MCUBoot Image](https://github.com/zephyrproject-rtos/mcuboot/blob/main/boot/zephyr/Kconfig)
-   [MCUBoot Config used for the application](https://github.com/zephyrproject-rtos/zephyr/blob/main/modules/Kconfig.mcuboot)

When an OTA image is received it can either be marked as permanent or as a test,
The Kconfig `CONFIG_CHIP_OTA_REQUEST_UPGRADE_TYPE` can choose one of those
configurations (Defined in `/config/nxp/chip-module/Kconfig`):

-   `CONFIG_CHIP_OTA_REQUEST_UPGRADE_PERMANENT`: From the next reboot, this
    image will be run permanently.
-   `CONFIG_CHIP_OTA_REQUEST_UPGRADE_TEST`: The image will be run on the next
    reboot, but it will be reverted if it doesn't get confirmed. The image needs
    to confirm itself to become permanent.

By default, the upgrade type used is `CONFIG_CHIP_OTA_REQUEST_UPGRADE_TEST`, and
OTA image confirms itself during the initialization stage after the fundamental
parts of the application are initialized properly to make sure this new image
boots correctly. This confirmation is done by
`chip::NXP::App::OTARequestorInitiator::HandleSelfTest()`.

JLink can be used to flash the mixed binary at the base address 0x8000000, using
the command :

```
J-Link > loadbin zephyr_full.bin 0x8000000
```

The bootloader should then be able to jump directly to the start of the
application and run it.

### Generating the OTA Update Image

The same procedure can be followed from the
[Generating and flashing application image](#generating-and-flashing-application-image)
sub-section, replacing `CONFIG_CHIP_DEVICE_SOFTWARE_VERSION` with a number
greater than the initial one used on the active application (Candidate
application version number should be greater than the one used on the active
application). By default the value is set to 1, try resetting this option to 2
to generate the OTA update Image. You can do this by adding
`-DCONFIG_CHIP_DEVICE_SOFTWARE_VERSION=2` to the west build command.

The current implementation automates the following procedures:

-   Generation of the Image to be used for update (File generated:
    `zephyr/zephyr.bin`)\*
-   Signature of the Image (File generated: `zephyr/zephyr.signed.bin`)\*
-   Conversion of the signed Image into the OTA format (.ota file) (File
    generated: `zephyr/matter.ota`)\*

> **Note**: \*All paths are relative to the output folder.

The generated OTA file `matter.ota` can be used to perform the OTA Software
Update. The instructions below describes the procedure step-by-step.

### Performing the OTA Software Update

Setup example :

-   [Chip-tool](../../../examples/chip-tool/README.md) application running on
    the RPi.
-   OTA Provider application built on the same RPi (as explained below).
-   Board programmed with the example application (with the instructions above).

Before starting the OTA process, the Linux OTA Provider application can be built
on the RPi (if not already present in the pre-installed apps) :

```
user@ubuntu:~/connectedhomeip$ : ./scripts/examples/gn_build_example.sh examples/ota-provider-app/linux out/ota-provider-app chip_config_network_layer_ble=false

user@ubuntu:~/connectedhomeip$ : rm -rf /tmp/chip_*
user@ubuntu:~/connectedhomeip$ : ./out/ota-provider-app/chip-ota-provider-app -f matter.ota
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
