# Performing Device Firmware Upgrade in the nRF Connect examples

Some examples for the development kits from Nordic Semiconductor support
over-the-air Device Firmware Upgrade.

Currently, the Bluetooth LE is the only available transport for performing the
DFU operation and it uses nRF Connect SDK's
[Simple Management Protocol](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/guides/device_mgmt/index.html#device-mgmt).
The upgrade can be done either using a smartphone application or a PC command
line tool.

## Device Firmware Upgrade using smartphone

To upgrade your device firmware over Bluetooth LE using smartphone, complete the
following steps:

1. Install one of the following applications on your smartphone:

    - [nRF Connect for Mobile](https://www.nordicsemi.com/Software-and-Tools/Development-Tools/nRF-Connect-for-mobile)
    - [nRF Toolbox](https://www.nordicsemi.com/Software-and-Tools/Development-Tools/nRF-Toolbox)

2. Push the appropriate button on the device to enable the software update
   functionality (if it is not enabled by default) and start the Bluetooth LE
   advertising of SMP service. See the user interface section in the example
   documentation to check the button number.
3. Push the appropriate button on the device to start the Bluetooth LE
   advertising. See the user interface section in the example documentation to
   check the button number.
4. Follow the instructions about downloading the new image to a device on the
   [FOTA upgrades](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/ug_nrf52.html#fota-upgrades)
   page in the nRF Connect SDK documentation.

## Device Firmware Upgrade using PC command line tool

To upgrade your device firmware over Bluetooth LE, you can use the PC command
line tool provided by the [mcumgr](https://github.com/zephyrproject-rtos/mcumgr)
project.

> **_WARNING:_**
>
> -   The mcumgr tool using Bluetooth LE is available only for Linux and macOS
>     systems. On Windows, there is no support for Device Firmware Upgrade over
>     Bluetooth LE yet.
> -   It might not be possible to connect to the nRF device when using the
>     mcumgr on Linux with the built-in Bluetooth LE adapter. In such cases, you
>     can use Zephyr's Bluetooth HCI USB sample and program it to a Nordic
>     Semiconductor's development kit to form an external Bluetooth LE adapter.
>     For example, to build the sample for the nRF52840 DK, use the following
>     command:
>
>         cd zephyr/samples/bluetooth/hci_usb && west build -b nrf52840dk_nrf52840 -- -DCONFIG_BT_LL_SW_SPLIT=y

Complete the following steps to perform DFU using mcumgr:

> **_NOTE:_** In all of the commands listed in the following steps, replace
> `ble-hci-number` with the Bluetooth hci integer value (for example, `0`) and
> `ble-device-name` with the CHIP device name advertised over Bluetooth LE (for
> example, `MatterLock`).

1.  Install the tool by following the
    [mcumgr command line tool installation instructions](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/guides/device_mgmt/index.html#command-line-tool).
2.  Push the appropriate button on the device to enable the software update
    functionality (if it is not enabled by default) and start the Bluetooth LE
    advertising of SMP service. See the user interface section in the example
    documentation to check the button number.
3.  Observe that the LED on the device is flashing (short flash on), which means
    that the Bluetooth LE advertising has started. See the user interface
    section in the example documentation to check the LED number.
4.  Upload the firmware image to the device by running the following command in
    your example directory:

        $ sudo mcumgr --conntype ble --hci ble-hci-number --connstring peer_name='ble-device-name' image upload build/zephyr/app_update.bin

    The operation can take few minutes. Wait until the progress bar reaches
    100%.

5.  Obtain the list of images present in the device memory by running following
    command:

        $ sudo mcumgr --conntype ble --hci ble-hci-number --connstring peer_name='ble-device-name' image list

    The displayed output contains the old image in slot 0 that is currently
    active and the new image in slot 1, which is not active yet (flags field
    empty):

        Images:
        image=0 slot=0
            version: 0.0.0
            bootable: true
            flags: active confirmed
            hash: 7bb0e909a846e833465cbb44c581cf045413a5446c6953a30a3dcc2c3ad51764
        image=0 slot=1
            version: 0.0.0
            bootable: true
            flags:
            hash: cbd58fc3821e749d3abfb00b3069f98c078824735f1b2a333e8a1579971e7de1
        Split status: N/A (0)

6.  Swap the firmware images by calling the following method with `image-hash`
    replaced by the image present in the slot 1 hash (for example,
    `cbd58fc3821e749d3abfb00b3069f98c078824735f1b2a333e8a1579971e7de1`):

        $ sudo mcumgr --conntype ble --hci ble-hci-number --connstring peer_name='ble-device-name' image test image-hash

    You can observe that the `flags:` field in the image for slot 1 changes
    value to `pending`:

        Images:
        image=0 slot=0
            version: 0.0.0
            bootable: true
            flags: active confirmed
            hash: 7bb0e909a846e833465cbb44c581cf045413a5446c6953a30a3dcc2c3ad51764
        image=0 slot=1
            version: 0.0.0
            bootable: true
            flags: pending
            hash: cbd58fc3821e749d3abfb00b3069f98c078824735f1b2a333e8a1579971e7de1
        Split status: N/A (0)

7.  Reset the device with the following command to let the bootloader swap
    images:

        $ sudo mcumgr --conntype ble --hci ble-hci-number --connstring peer_name='ble-device-name' reset

    The device is reset and the following notifications appear in its console:

        *** Booting Zephyr OS build zephyr-v2.5.0-1101-ga9d3aef65424  ***
        I: Starting bootloader
        I: Primary image: magic=good, swap_type=0x2, copy_done=0x1, image_ok=0x1
        I: Secondary image: magic=good, swap_type=0x2, copy_done=0x3, image_ok=0x3
        I: Boot source: none
        I: Swap type: test

    Swapping operation can take some time, and after it completes, the new
    firmware is booted.

Visit the
[mcumgr image management](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/guides/device_mgmt/indexhtml#image-management)
section to get familiar with all image management commands supported by the
tool.
