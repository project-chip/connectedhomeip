# Performing Device Firmware Upgrade in the nRF Connect examples

Some examples for the development kits from Nordic Semiconductor support
over-the-air (OTA) Device Firmware Upgrade (DFU) using one of the following
protocols:

-   Matter-compliant OTA update protocol that uses the Matter operational
    network for querying and downloading a new firmware image.
-   [Simple Management Protocol](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/guides/device_mgmt/index.html#device-mgmt)
    over Bluetooth LE. In this case, the DFU can be done either using a
    smartphone application or a PC command line tool. Note that this protocol is
    not part of the Matter specification.

## Device Firmware Upgrade over Matter

> **_NOTE:_** The procedure presented below requires that you have OpenThread
> Border Router (OTBR) set up either in Docker or on a Raspberry Pi. Read
> [Setup OpenThread Border Router on Raspberry Pi](openthread_border_router_pi.md)
> to learn how to install the OTBR on a Raspberry Pi.

The DFU over Matter involves two kinds of nodes: OTA Provider and OTA Requestor.
An OTA Provider is a node that can respond to the OTA Requestors' queries about
available software updates and share the update packages with them. An OTA
Requestor is any node that needs to be updated and can communicate with the OTA
Provider to fetch applicable software updates. In the procedure described below,
the OTA Provider will be a Linux application and the example running on the
Nordic Semiconductor's board will work as the OTA Requestor.

To test the DFU over Matter, you need to complete the following steps:

1.  Navigate to the CHIP root directory.
2.  Build the OTA Provider application for Linux:

        ```
        scripts/examples/gn_build_example.sh examples/ota-provider-app/linux out/provider chip_config_network_layer_ble=false
        ```

3.  Build chip-tool for Linux:

        ```
        scripts/examples/gn_build_example.sh examples/chip-tool out/chiptool 'chip_mdns="platform"'
        ```

4.  Run OTA Provider application with _matter.ota_ replaced with the path to the
    Matter OTA image which you wish to provide to the Matter device. Note that
    the Matter OTA image is, by default, generated at _zephyr/matter.ota_ in the
    example's build directory:

         $ out/provider/chip-ota-provider-app -f matter.ota

    Keep the application running and use another terminal for the remaining
    steps.

5.  Commission the OTA Provider into the Matter network using Node Id 1:

        ```
        ./out/chiptool/chip-tool pairing onnetwork 1 20202021
        ```

6.  Use the OTBR web interface to form a new Thread network using the default
    network settings.
7.  Commission the Matter device into the same Matter network using Node Id 2.
    The parameter starting with the _hex:_ prefix is the Thread network's Active
    Operational Dataset. It can be retrieved from the OTBR in case you have
    changed the default network settings when forming the network.

        ```
        ./out/chiptool/chip-tool pairing ble-thread 2 hex:000300000f02081111111122222222051000112233445566778899aabbccddeeff01021234 20202021 3840
        ```

8.  Configure the Matter device with the default OTA Provider by running the
    following command. The last two arguments are Requestor Node Id and
    Requestor Endpoint Id, respectively:

        ```
        ./out/chiptool/chip-tool otasoftwareupdaterequestor write default-ota-providers '[{"fabricIndex": 1, "providerNodeID": 1, "endpoint": 0}]' 2 0
        ```

9.  Configure the OTA Provider with the access control list (ACL) that grants
    _Operate_ privileges to all nodes in the fabric. This is necessary to allow
    the nodes to send cluster commands to the OTA Provider:

        ```
        ./out/chiptool/chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}, {"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": null, "targets": null}]' 1 0
        ```

10. Initiate the DFU procedure in one of the following ways:


    -   If you have built the device firmware with `-DCONFIG_CHIP_LIB_SHELL=y`
        option, which enables Matter shell commands, run the following command
        on the device shell:

               ```
               matter ota query
               ```

    -   Otherwise, use chip-tool to send the Announce OTA Provider command to
        the device. The numeric arguments are Provider Node Id, Provider Vendor
        Id, Announcement Reason, Provider Endpoint Id, Requestor Node Id and
        Requestor Endpoint Id, respectively.

               ```
               ./out/chiptool/chip-tool otasoftwareupdaterequestor announce-ota-provider 1 0 0 0 2 0
               ```

        Once the device is made aware of the OTA Provider node, it automatically
        queries the OTA Provider for a new firmware image.

11. When the firmware image download is complete, the device is automatically
    rebooted to apply the update.

## Device Firmware Upgrade over Bluetooth LE using smartphone

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

## Device Firmware Upgrade over Bluetooth LE using PC command line tool

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
> `ble-device-name` with the Matter device name advertised over Bluetooth LE
> (for example, `MatterLock`).

1.  Install the tool by following the
    [mcumgr command line tool installation instructions](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/guides/device_mgmt/index.html#command-line-tool).
2.  Push the appropriate button on the device to enable the software update
    functionality (if it is not enabled by default) and start the Bluetooth LE
    advertising of SMP service. See the user interface section in the example
    documentation to check the button number.
3.  Observe that the LED on the device is flashing (short flash on), which means
    that the Bluetooth LE advertising has started. See the user interface
    section in the example documentation to check the LED number.
4.  Upload the application firmware image to the device by running the following
    command in your example directory:

        ```
        sudo mcumgr --conntype ble --hci ble-hci-number --connstring peer_name='ble-device-name' image upload build/zephyr/app_update.bin -n 0 -w 1
        ```

    The operation can take a few minutes. Wait until the progress bar reaches
    100%.

5.  Obtain the list of images present in the device memory by running following
    command:

        ```
        sudo mcumgr --conntype ble --hci ble-hci-number --connstring peer_name='ble-device-name' image list
        ```

    The displayed output contains the old image in slot 0 that is currently
    active and the new image in slot 1, which is not active yet (flags field
    empty):

        ```
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
        ```

6.  Swap the firmware images by calling the following method with `image-hash`
    replaced by the image present in the slot 1 hash (for example,
    `cbd58fc3821e749d3abfb00b3069f98c078824735f1b2a333e8a1579971e7de1`):

        ```
        sudo mcumgr --conntype ble --hci ble-hci-number --connstring peer_name='ble-device-name' image test image-hash
        ```

    You can observe that the `flags:` field in the image for slot 1 changes
    value to `pending`:

        ```
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
        ```

7.  If you are using the nRF5340DK board, which supports multi-image device
    firmware upgrade, complete the following substeps. If you are not using one,
    go straight to the step 8.

    a. Upload the network core firmware image to the device by running the
    following command in your example directory:

         ```
         sudo mcumgr --conntype ble --hci ble-hci-number --connstring peer_name='ble-device-name' image upload build/zephyr/net_core_app_update.bin -n 1 -w 1
         ```

    The operation can take a few minutes. Wait until the progress bar reaches
    100%.

    b. Obtain the list of images present in the device memory by running
    following command:

         ```
         sudo mcumgr --conntype ble --hci ble-hci-number --connstring peer_name='ble-device-name' image list
         ```

    The displayed output contains the old application image in slot 0 that is
    currently active, the new application image in slot 1 in pending state, and
    the new network image which is in slot 1 and not active yet (flags field
    empty):

         ```
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
         image=1 slot=1
             version: 0.0.0
             bootable: true
             flags:
             hash: d9e31e73cb7a959c26411250c2b3028f3510ae88a4549ae3f2f097c3e7530f48
         Split status: N/A (0)
         ```

    c. Swap the firmware images by calling the following method with
    `image-hash` replaced by the image present in the slot 1 hash (for example,
    `d9e31e73cb7a959c26411250c2b3028f3510ae88a4549ae3f2f097c3e7530f48`):

         ```
         sudo mcumgr --conntype ble --hci ble-hci-number --connstring peer_name='ble-device-name' image test image-hash
         ```

    You can observe that the `flags:` field in the image for slot 1 changes
    value to `pending`:

         ```
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
         image=1 slot=1
             version: 0.0.0
             bootable: true
             flags: pending
             hash: d9e31e73cb7a959c26411250c2b3028f3510ae88a4549ae3f2f097c3e7530f48
         Split status: N/A (0)
         ```

8.  Reset the device with the following command to let the bootloader swap
    images:

        ```
        sudo mcumgr --conntype ble --hci ble-hci-number --connstring peer_name='ble-device-name' reset
        ```

The device is reset and the following notifications appear in its console:

       ```
       *** Booting Zephyr OS build zephyr-v2.5.0-1101-ga9d3aef65424  ***
       I: Starting bootloader
       I: Primary image: magic=good, swap_type=0x2, copy_done=0x1, image_ok=0x1
       I: Secondary image: magic=good, swap_type=0x2, copy_done=0x3, image_ok=0x3
       I: Boot source: none
       I: Swap type: test
       ```

Swapping operation can take some time, and after it completes, the new firmware
is booted.

Visit the
[mcumgr image management](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/guides/device_mgmt/indexhtml#image-management)
section to get familiar with all image management commands supported by the
tool.
