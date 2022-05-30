# BL602

This example functions as a wifi light bulb device type, with on/off capabilities. The steps were verified on BL602-IoT-DVK-3S board.

BL602-IoT-DVK-3S
<img src="../../../platform/bouffalolab/bl602/doc/images/bl602_iot_3S_v2.jpg" style="zoom:25%;" />

## Initial setup

-   Setting up the environment on ubuntu 20.04 or 18.04

```
$ sudo apt-get update
$ sudo apt-get upgrade
$ sudo apt-get install git gcc g++ python pkg-config libssl-dev libdbus-1-dev libglib2.0-dev libavahi-client-dev ninja-build python3-venv python3-dev python3-pip unzip libgirepository1.0-dev libcairo2-dev bluez avahi-daemon
$ sudo apt-get install pi-bluetooth (if not raspberry pi, sudo apt-get install bluetooth)
$ reboot

```
- Clone and initialize the connectedhomeip repo

```
git clone https://github.com/project-chip/connectedhomeip.git
cd connectedhomeip
git submodule update --init --recursive
```

-   Install packets

```
$ cd {path-to-connectedhomeip}
connectedhomeip$ source ./scripts/bootstrap.sh
connectedhomeip$ source ./scripts/activate.sh

```

## Check project configuration

-   Update the `VENDOR_ID` in your example project config (**`CHIPProjectConfig.h`**). This should be your VID as per the CSA Matter specification, or [a VID allocated for testing purposes](https://developers.home.google.com/home/matter/eap#vendor_id). The value you provide for `VENDOR_ID` **must match** the one you use when [creating the Matter integration](https://developers.home.google.com/matter/eap/vendors/nxp#next_steps) in the Google Home Developer Center Console.

    ```
    grep VENDOR_ID ./examples/lighting-app/boullalolab/bl602/include/CHIPProjectConfig.h
    ```

    ```
     * CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID
    #define CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID your-hex-VID
    ```
    
-   Check the `PRODUCT_ID` in your example project config (`CHIPProjectConfig.h`). The `PRODUCT_ID` value can remain the same as the example's default, or changed as desired, but it must adhere to the current [Product ID restrictions described in the Prerequisites](https://developers.home.google.com/home/matter/eap#product_id). Also note that the value you provide for `PRODUCT_ID` **must match** the one you use when [creating the Matter integration](https://developers.home.google.com/matter/eap/vendors/nxp#next_steps) in the Google Home Developer Center Console.

    ```
    grep PRODUCT_ID ./examples/lighting-app/bouffalolab/bl602/include/CHIPProjectConfig.h
    ```

    ```
     * CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID
    #define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID 0x4B4C
     
    ```
    
 ## Build the image 
-   Build the example application:

    `connectedhomeip$ ./scripts/build/build_examples.py --target bl602-light build`

    Generated files

    `connectedhomeip/out/bl602-light/chip-bl602-lighting-example.bin`

    -   To delete generated executable, libraries and object files use:

    ```
    $ cd ~/connectedhomeip/
    $ rm -rf out/
    ```

## Flash the board

- Download  [Bouffalo Lab Dev Cube](https://dev.bouffalolab.com/download/).

  Log in to the site as a guest.

  <img src="../../../platform/bouffalolab/bl602/doc/images/image-web-login.png" style="zoom:25%;" />

  <img src="../../../platform/bouffalolab/bl602/doc/images/dev-cube.png" style="zoom:30%;" />

  

  run the softaware :
  
  <img src="../../../platform/bouffalolab/bl602/doc/images/dev-cube-home.png" style="zoom:50%;" />
  
  
  
  <img src="../../../platform/bouffalolab/bl602/doc/images/devcub-iot.png" style="zoom:70%;" />
  
  The following picture shows the burning interface. To download the firmware, you need to enter the download mode: press and hold the Boot (IO8) pin of the development board, press the RST button, and then release the Boot button to enter the burning and downloading mode.

<img src="../../../platform/bouffalolab/bl602/doc/images/image-dev-cube.png" style="zoom:60%;" />

Notice: Latest version Bouffalolab devcub is recommeded.

Factory Params: BouffaloLabDevCube-1.7.2-linux-x86/chips/bl602/device_tree/bl_factory_params_IoTKitA_40M.dts

Partition Table: BouffaloLabDevCube-1.7.2-linux-x86/chips/bl602/partition/partition_cfg_2M.toml 

Boot2 Bin: BouffaloLabDevCube-1.7.2-linux-x86/chips/bl602/builtin_imgs/boot2_iap_v5.5/boot2_iap_release.bin

Firmware Bin: connectedhomeip/out/bl602-light/chip-bl602-lighting-example.bin

Partition Table： 

1. FW: The size of FW size0 must be larger than the bin size, we can do it by reducing the size of FW size1 and media partition size0.

COM Port:

```
   ls -la /dev/ttyUSB*
```

   select the big one.

## Validate the example

   1.The device should present itself as a USB serial device on your computer. You may look it up in `/dev/`:

   ```
   ls -la /dev/tty*
   ```

You can open the serial console with minicom or picocom. For example, if the device is at `/dev/USB1`:

```
picocom -b 2000000 /dev/ttyUSB1
```

To  reset the board, press the **RST** button. And you will see the log from the demo board.

## Commissioning prerequisites

Before commissioning the bouffaloalab bl602 development board, be aware of the following:

1. BLE advertising must be enabled before the device can be discovered and commissioned within the Google Home ecosystem. To temporarily enable advertising (for 1 minutes, per the Matter specification), reset the board by pressing RST button, the ble will start advertising.

## Next steps

When your Matter example has been successfully built, [create a Matter Integration](https://developers.home.google.com/matter/eap/project/create).
