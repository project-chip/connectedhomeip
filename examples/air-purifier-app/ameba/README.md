# CHIP Ameba Air Purifier Example

This example demonstrates the Matter air purifier application on Ameba platform.

---

-   [CHIP Ameba Air Purifier Example](#chip-ameba-air-purifier-example)
    -   [Supported Device](#supported-device)
    -   [Building the Example Application](#building-the-example-application)
    -   [Commissioning](#commissioning)
        -   [BLE mode](#ble-mode)
        -   [IP mode](#ip-mode)
    -   [Cluster Control](#cluster-control)

---

## Supported Device

The CHIP demo application is supported on
[Ameba RTL8722DM Board](https://www.amebaiot.com/en/amebad).

## Building the Example Application

-   Check out the Ameba repository in the same folder/directory as the Matter
    SDK repository:

```
git clone https://github.com/ambiot/ambd_matter.git
```

-   Setup build environment:

```
$ cd connectedhomeip
$ source scripts/bootstrap.sh
```

-   To build the demo application:

```
$ cd ambd_matter/project/realtek_amebaD_va0_example/GCC-RELEASE/project_lp/
$ make all

$ cd ambd_matter/project/realtek_amebaD_va0_example/GCC-RELEASE/project_hp/
$ make -C asdk air_purifier

From the same directory:
$ make all
```

-   Combine the three output images for flashing using the **Ameba Image Tool**:

```
$ cd ambd_matter/tools/AmebaD/Image_Tool_Linux

$ sudo ./AmebaD_ImageTool -combine \
  ../../../project/realtek_amebaD_va0_example/GCC-RELEASE/project_lp/asdk/image/km0_boot_all.bin 0x0000 \
  ../../../project/realtek_amebaD_va0_example/GCC-RELEASE/project_hp/asdk/image/km4_boot_all.bin 0x4000 \
  ../../../project/realtek_amebaD_va0_example/GCC-RELEASE/project_hp/asdk/image/km0_km4_image2.bin 0x6000
```

-   This will produce a combined Image_All.bin file alongside the image tool
    that can be flashed using the **Ameba Image Tool**:

1.  Connect your device via USB
2.  Edit the `ambd_matter/tools/AmebaD/mpp.ini` file with the correct port
    setting (the rest of the settings should be correct)
3.  Click **Download** button and the **Reset** button to get the board into
    serial download mode
4.  Flash on the image:

```
$ cd ambd_matter/tools/AmebaD/Image_Tool_Linux
$ ./AmebaD_ImageTool -download
```

## Commissioning

There are two commissioning modes supported by Ameba platform:

### BLE mode

1. Build and Flash
2. The example will run automatically after booting the Ameba board.
3. Test with
   [Chip-Tool](https://github.com/project-chip/connectedhomeip/tree/master/examples/chip-tool)

### IP mode

1. Build and Flash
2. The example will run automatically after booting the Ameba board.
3. Connect to AP using `ATW0, ATW1, ATWC` commands
4. Test with
   [Chip-Tool](https://github.com/project-chip/connectedhomeip/tree/master/examples/chip-tool)

## Cluster Control

After successful commissioning, the air purifier clusters can be read and
controlled using
[Chip-Tool](https://github.com/project-chip/connectedhomeip/tree/master/examples/chip-tool#using-the-client-to-send-matter-commands).

The Air Purifier is a composed device. The example has endpoints configured as
follows:

-   Air purifier on endpoint 1
-   Air quality sensor on endpoint 2
-   Temperature sensor on endpoint 3
-   Relative humidity sensor on endpoint 4
-   Thermostat on endpoint 5

Example commands using the chip tool:

```
$ ./chip-tool fancontrol write speed-setting 10 ${NODE_ID_TO_ASSIGN} 1
$ ./chip-tool formaldehydeconcentrationmeasurement read level-value ${NODE_ID_TO_ASSIGN} 2
$ ./chip-tool temperaturemeasurement read measured-value ${NODE_ID_TO_ASSIGN} 3
$ ./chip-tool relativehumiditymeasurement read measured-value ${NODE_ID_TO_ASSIGN} 4
```
