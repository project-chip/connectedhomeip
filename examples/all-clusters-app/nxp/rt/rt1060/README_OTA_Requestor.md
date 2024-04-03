# OTA requestor feature - WARNING NOT SUPPORTED

### Overview
The OTA Requestor functionality enables the device to communicate with an OTA Provider in order to query, download and apply a software update.

This section explains how to perform an OTA Software Update on the RT1060 with the All-Clusters application.

### Build
To build the application with OTA Requestor functionality enabled, argument "chip_enable_ota_requestor=true" must be added to the *gn gen* command.

### General OTA Process for the RT1060

In general, the Over-The-Air Software Update process consists of the following steps :
- The OTA Requestor (RT1060) queries an update image to the OTA Provider which responds according to its availability.
- The update image is received in blocks and stored in the external flash of the RT1060.
- Once the update image is fully downloaded, the bootloader is notified and the device resets in test-mode.
- If the test is successful, the update is applied permanently. Otherwise, the bootloader enables the device to revert back to the primary application, preventing any downgrade. 

### Flash Memory Layout

The RT1060 Flash is divided into different regions as follow :
- Bootloader : MCUBoot resides at the base of the flash and occupies 0x11000 (68 kBytes)
- Primary application partition : All-clusters application which would be run by the bootloader (active application)
- Secondary application partition : Update image received with the OTA (candidate application)

### MCUBoot Bootloader

MCUBoot is an open-source secure bootloader used by the RT1060 to apply the self-upgrade. For more details, please refer to the [MCUBoot documentation](https://github.com/mcu-tools/mcuboot/blob/main/docs/design.md).

In our use case, the bootloader can only run the application residing in the primary partition. After the OTA process, in order to run the secondary application (the update image), the bootloader will swap the content of the primary and the secondary partitions. This type of upgrade is called swap-move and is the default upgrade configured by MCUBoot.

In order for the device to perform the software update, it is recommended to flash the MCUBoot bootloader first at the base of the flash. A step-by-step guide is given below.

- It is highly recommended to start with erasing the external flash of the RT1060
 using the GUI Flash Tool from MCUXPresso.
- Using MCUXPresso, import the `mcuboot_opensource` demo example from the SDK previously downloaded.
![mcuboot_demo](../../../platform/nxp/common/doc/images/mcuboot_demo.PNG)
- Copy the content of the flash_partitioning.h file from "/connectedhomeip/examples/platfom/nxp/common/mcuboot_app_support/" and paste it in the imported SDK example file : /board/flash_partitioning.h
- Build the demo example project and program it to the target board.
- To run the flashed demo, either press the reset button of the device or use the debugger IDE of MCUXpresso. If it runs successfully, the following logs will be displayed on the terminal :

```
hello sbl.

Bootloader Version 1.7.2

Primary image: magic=unset, swap_type=0x1, copy_done=0x3, image_ok=0x3

Secondary image: magic=unset, swap_type=0x1, copy_done=0x3, image_ok=0x3

Boot source: none

Swap type: none

erasing trailer; fa_id=2

Unable to find bootable image
```

### Generating and flashing the signed application image

Once the bootloader is in place, the application can be flashed on the RT1060. In order for MCUBoot to run the application, the image must have a certain format :
- Header : contains general information about the image (version, size, magic...)
- Code of the application : generated binary 
- Trailer : contains metadata required by the bootloader such as the image signature, the upgrade type, the swap status...

The application can be generated using the instructions from the [Building](#building) section. When building with the "chip_enable_ota_requestor=true", the application is automatically linked to be executed from the primary application partition, taking into consideration the offset imposed by mcuboot.

The resulting executable file found in out/debug/chip-rt1060-all-cluster-example needs to be converted into raw binary format as shown below.
```
arm-none-eabi-objcopy -R .flash_config -R .ivt -R .interrupts_ram -R .ncache.init -R .ncache -R .bss -R .heap -R .stack -R .NVM -O binary chip-rt1060-all-cluster-example chip-rt1060-all-cluster-example.bin
```
MCUBoot requires the generated application binary to be signed in the specific format explained above. The dedicated tool "imgtool" is provided in the SDK and can be found in "/middleware/mcuboot_opensource/scripts/". "Imgtool" will wrap the raw binary of the application with the corresponding header and trailer.

To sign the image, open a terminal and run the following commands (make sure to replace the /path/to/file/binary with the adequate files): 

```
user@ubuntu: cd ~/Desktop/SDK_2_11_1_MIMXRT1060-EVKB/middleware/mcuboot_opensource/scripts

user@ubuntu: python3 imgtool.py sign --key ~/Desktop/SDK_2_11_1_MIMXRT1060-EVKB/boards/evkbmimxrt1060/mcuboot_opensource/keys/sign-rsa2048-priv.pem --align 4 --header-size 0x1000 --pad-header --slot-size 0x300000 --max-sectors 768 --version "1.0" ~/Desktop/connectedhomeip/examples/all-clusters-app/nxp/rt1060/out/debug/chip-rt1060-all-cluster-example.bin ~/Desktop/connectedhomeip/examples/all-clusters-app/nxp/rt1060/out/debug/chip-rt1060-all-cluster-example_SIGNED.bin
```
Note : The image is signed with the private key provided by the SDK (/path_to_sdk/boards/evkbmimxrt1060/mcuboot_opensource/keys/sign-rsa2048-priv.pem), MCUBoot is built with its corresponding public key which would be used to verify the integrity of the image. It is possible to generate a new pair of keys using the following commands. However, this procedure should be done prior to building the mcuboot application.

- To generate the private key :

```
user@ubuntu: python3 imgtool.py keygen -k priv_key.pem -t rsa-2048
```
- To extract the public key :

```
user@ubuntu: python3 imgtool.py getpub -k priv_key.pem
```
- Once, the public key is extracted, it can then be copied to the /path_to_sdk/boards/evkbmimxrt1060/mcuboot_opensource/secure/sign-rsa2048-pub.c given as a value to the rsa_pub_key[] array.


The resulting output is the signed binary of the application version "1.0". 

To flash the signed binary of the application, the GUI Flash Tool from MCUXPresso can be used :

```
Right click on the project -> Debug As -> Debug configurations... -> C/C++ (NXP Semiconductors) MCU Application -> Right Click -> New configuration
```
- In the Debug Configurations interface, in the C/C++ Application section, select the signed binary file /connectedhomeip/examples/all-clusters-app/nxp/rt1060/out/debug/chip-rt1060-all-cluster-example_SIGNED.bin .
- Open the GUI Flash Tool from the interface. In the "Program" section, select "bin" as a format to use for programming. 
![GUI_flash_tool](../../../platform/nxp/common/doc/images/GUI_flash_tool.PNG)
- Define the base address as "0x60011000" to program the application at that address, as the bootloader resides above it. 
- Click on "Run". 
- Once the application is programmed in flash, close the debug interface. The application should now be programmed in flash at the address 0x60011000.
- Press the reset button of the RT1060. 

The bootloader should now be able to jump directly to the start of the application and run it.

### Generating the OTA Update image

To perform the OTA Software Update, an OTA update file must be provided to the OTA Provider. 

To generate the update image the same procedure can be followed from the [Generating and flashing the signed application image](#generating-and-flashing-the-signed-application-image) sub-section, with the minor modification when signing the image : replace the "--version "1.0"" argument with "--version "1.1"" (desired version of the update).

When the signed binary of the update is signed, the file needs to be converted into OTA format. To do so, the ota_image_tool is provided in the repo and can be used to convert a binary file into a .ota file. 

```
user@ubuntu:~/connectedhomeip$ : ./src/app/ota_image_tool.py create -v 0xDEAD -p 0xBEEF -vn 1 -vs "1.0" -da sha256 chip-rt1060-all-cluster-example_SIGNED.bin chip-rt1060-all-cluster-example.ota
```
The generated OTA file can be used to perform the OTA Software Update. The instructions below show how it can be done step-by-step.

### OTA Procedure instructions

Setup requirements : 
- Chip-Tool application built on the RPi (as described [here](https://github.com/project-chip/connectedhomeip/blob/master/examples/chip-tool/README.md#using-the-client-to-send-matter-commands)).
- OTA Provider application built on the same RPi (as explained below).
- RT1060 programmed with the All-clusters application (with the instructions above).

Before starting the OTA process, we must build the Linux OTA Provider application and then start it on the RPi :
```
user@ubuntu:~/connectedhomeip$ : ./scripts/examples/gn_build_example.sh examples/ota-provider-app/linux out/ota-provider-app chip_config_network_layer_ble=false

user@ubuntu:~/connectedhomeip$ : rm -rf /tmp/chip_*
user@ubuntu:~/connectedhomeip$ : ./out/ota-provider-app/chip-ota-provider-app -f chip-rt1060-all-cluster-example.ota
```
The OTA Provider is now running on the Raspberry Pi, the application should first be provisionned with chip-tool by assigning it the node id 1, and then granted the ACL entries :
```
user@ubuntu:~/connectedhomeip$ : ./out/chip-tool-app/chip-tool pairing onnetwork 1 20202021
user@ubuntu:~/connectedhomeip$ : ./out/chip-tool-app/chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}, {"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": null, "targets": null}]' 1 0
```
The second step is to provision the RT1060 with the node id 2 using ble-thread commissioning. The operational dataset info can be found using the command (and replacing "bf7fb6b739a9" with the respective docker container id):
```
sudo docker exec -it bf7fb6b739a9 sh -c "sudo ot-ctl dataset active -x"
```
```
user@ubuntu:~/connectedhomeip$ : ./out/chip-tool-app/chip-tool pairing ble-thread 2 hex:<operationalDataset> 20202021 3840
```
Once the RT1060 commissioning is completed successfully, the OTA process can be initiated with the "announce-ota-provider" command using chip-tool (the given numbers refer respectively to [ProviderNodeId] [VendorId] [AnnouncementReason] [Endpoint] [node-id] [endpoint-id]) :
```
user@ubuntu:~/connectedhomeip$ : ./out/chip-tool-app/chip-tool otasoftwareupdaterequestor announce-ota-provider 1 0 0 0 2 0
```
The OTA Requestor (RT1060) will now be able to communicate with the OTA Provider to query an update image, and the OTA file transfer will start.

When the full update image is downloaded and stored, the bootloader will be notified and the device will reboot with the update image.