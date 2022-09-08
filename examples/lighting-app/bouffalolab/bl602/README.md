# BL602

This example functions as a wifi light bulb device type, with on/off
capabilities. The steps were verified on BL602-IoT-Matter-V1 board.

BL602-IoT-Matter-V1 board:
<img src="../../../platform/bouffalolab/bl602/doc/images/BL602-IoT-Matter_V1.png" style="zoom:25%;" />

## Initial setup

The steps in this document were validated on Ubuntu 18.04 and 20.04.

-   Install dependencies as specified in the connectedhomeip repository:
    [Building Matter](https://github.com/project-chip/connectedhomeip/blob/interop_testing_te9/docs/guides/BUILDING.md).
-   Install other dependencies:

    ```
    sudo apt-get update
    sudo apt-get upgrade
    sudo apt-get install bluez avahi-daemon bluetooth
    reboot
    ```

-   Clone and initialize the connectedhomeip repo

    ```
    git clone https://github.com/project-chip/connectedhomeip.git
    cd connectedhomeip
    git submodule update --init --recursive
    source ./scripts/activate.sh
    ```

## Build the image and flash the board

-   Build the
    [lighting-app](https://github.com/project-chip/connectedhomeip/tree/master/examples/lighting-app/bouffalolab/bl602)

    ```
    ./scripts/build/build_examples.py --target bl602-light build
    ```

-   Build the lighting-app with Pigweed RPC:

    ```
    ./scripts/examples/gn_bl602_example.sh lighting-app ./out/bl602-light 'import("//with_pw_rpc.gni")'
    ```

-   Connect the board to your flashing station (MacOS, Ubuntu, Windows).

-   Set the board to the download mode:

    -   Press and hold the BOOT button.
    -   Press the RESET button and release it.
    -   Release the BOOT button.

-   The device should present itself as a USB serial device on your computer.
    You may look it up in /dev/ttyACM0

    ```
    ls -la /dev/tty*
    ```

    If the device is at /dev/ttyACM0, flash the board using the following
    commands:

    ```
    cd third_party/bouffalolab/bl602_sdk/repo/tools/flash_tool

    ./bflb_iot_tool-ubuntu18 --chipname=BL602 --baudrate=115200  --port=/dev/ttyACM0 --pt=chips/bl602/partition/partition_cfg_4M.toml --dts=chips/bl602/device_tree/bl_factory_params_IoTKitA_40M.dts --firmware=../../../../../../out/bl602-light/chip-bl602-lighting-example.bin
    ```

    ```
    If you want to erase previous network information in flash, you can add --erase parameters to the bflb_iot_tool-ubuntu18 command. For Windows and MacOS, replace bflb_iot_tool-ubuntu18 with bflb_iot_tool.exe and bflb_iot_tool-macos, respectively.
    ```

## Validate the example

1.You can open the serial console. For example, if the device is at
`/dev/ttyACM0`:

```
picocom -b 115200 /dev/ttyACM0
```

2.To reset the board, press the RESET button, and you will see the log in the
`picocom terminal`.

```
Starting bl602 now....
Booting BL602 Chip...
```

3.To control the development board after successfully debugging the development
board, press the BOOT button and you should see the following output in the
terminal:

```
[    404197][:588238200] Short press
[    404198][:588238200] receiving event type: 0
[    404203][:588238200] sending event type: 0
[    404207][:588238200] receiving event type: 0
[    404211][:588238200] Turning light ON
[    406211][:588238200] sending event type: 1
[    406212][:588238200] receiving event type: 1
[    406217][:588238200] Light ON
[    406220][:588238200] updating on/off = 1
```

4.To restore the board to factory Settings, press and hold the BOOT button for 5
seconds, and you should see the following output in the terminal:

```
[     37268][:588238200] LongLong press
[     37269][:588238200] receiving event type: 0
[     37274][:588238200] FactoryReset! please release button!!!
[     37279][:588238200] Toggling state to 1
[     37283][:588238200] brightness: 255, mHue: 0, mSaturation: 0, red: 255, green: 255, blue: 255
[     37292][:588238200] red level: 10000
[     38296][:588238200] Toggling state to 0
[     38297][:588238200] brightness: 0, mHue: 0, mSaturation: 0, red: 0, green: 0, blue: 0
[     38305][:588238200] red level: 0
[     39308][:588238200] Toggling state to 1
[     39309][:588238200] brightness: 255, mHue: 0, mSaturation: 0, red: 255, green: 255, blue: 255
[     39318][:588238200] red level: 10000
[     42323][:588238200] [DL] Easyflash erase: f/1/n
[     42327][:588238200] [DL] Easyflash erase: f/1/i
[     42332][:588238200] [DL] Easyflash erase: f/1/r
[     42335][:588238200] [DL] Easyflash erase: f/1/m
[     42340][:588238200] [DL] Easyflash erase: f/1/o
[     42349][:588238200] [DIS] Fabric (0x1) deleted. Calling OnFabricDeletedFromStorage
[     42403][:588238200] [DL] Easyflash erase: f/1/k/0
[     42462][:588238200] [DL] Easyflash erase: f/1/g
[     42465][:588238200] [DMG] AccessControl: removing fabric 1
```

## Commission a device using chip-tool

To initiate a client commissioning request to a device, run the built executable
and choose the pairing mode.

#### Commissioning over BLE

Run the built executable and pass it the discriminator and pairing code of the
remote device, as well as the network credentials to use.

The command below uses the default values hard-coded into the debug versions of
the BL602 lighting-app to commission it onto a Wi-Fi network:

    ```
    $ sudo ./chip-tool pairing ble-wifi 1 ${SSID} ${PASSWORD} 20202021 3840

    Parameters:
    1. Discriminator: 3840
    2. Setup-pin-code: 20202021
    3. Node ID: 1
    4. SSID : Wi-Fi SSID
    5. PASSWORD : Wi-Fi Password
    ```

### Cluster control

-   After successful commissioning, use the OnOff cluster commands to control
    the OnOff attribute. This allows you to toggle a parameter implemented by
    the device to be On or Off.

    ```
    $ sudo ./chip-tool onoff on 1 1
    ```

-   Use ColorControl cluster command to control the color attributes:

    ```
    $ sudo ./chip-tool colorcontrol move-to-hue-and-saturation 240 100 0 0 0 1 1
    ```

### Running RPC Console

-   Build chip-console following this
    [guide](../../../common/pigweed/rpc_console/README.md)

-   Start the console

    ```
    $ chip-console --device /dev/ttyUSB0 -b 2000000
    ```

-   Get or Set the light state

    `rpcs.chip.rpc.Lighting.Get()`

    `rpcs.chip.rpc.Lighting.Set(on=True, level=128)`
