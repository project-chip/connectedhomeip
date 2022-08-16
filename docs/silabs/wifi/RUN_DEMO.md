# Runing the Matter Demo over Wi-fi

## Flashing images/binaries on MG12 platform using Ozone, Simplicity Studio or Simplicity Commander.

1. Plug the WSTK and EFR into the laptop.

2. Launch Ozone, Simplicity Studio or Simplicity Commander Standalone - this will display a GUI. If you are not using Ozone, you can follow the instructions to [Flash a Silicon Labs Device](../general/FLASH_SILABS_DEVICE.md)


3. In 'New Project Wizard':

    1. Click the three-dots on the 'Device' tab and select 'Manufacturer' as
       '`Silicon Labs`'
    2. Select 'Device' as '`EFR32MG12PXXF1024`'
    3. Click 'OK'
    4. The 'Register set' tab will get filled automatically
    5. 'Peripherals' tab need not be changed as it is optional

4. Click 'Next' - the window that is displayed will contain:

    - Serial Number: Read from device
    - Target Interface: JTAG
    - Speed: 4MHz
    - Host Interface: USB
    - There will be one product in the 'Emulators connected via USB' tab -
      select this and click 'Next'

5. Click on the 'Silicon Labs device' detected, and then click 'Next'

6. You will be asked to select the image/binary to be loaded: click on the
   three-dots on that tab and navigate to '`out/rs911x_lighting/BRD4161A`'
   through 'Browse', select the file named `chip-efr32-lighting-example.out` and
   click 'Next'. This is the image built in previous step.

7. Make sure the next screen has 'Initial PC' selected as 'ELF Entry Point' -
   click 'Finish'

    > Ignore Diagnostics warning about 'FreeRTOS' detected - click 'Continue'

8. Select 'Download and Reset Program' in the dropdown next to the Power button
   on the top left of the page

    > The EFR32MG12 will be erased and programmed

9. Run the image by clicking the 'Play' button on the top left (or press the F5
   key)

    > The output of the EFR32 can be viewed on the console of the Ozone GUI

10. **[Optional]** This step is for when the device has already been flashed and
    is being used for testing/debug purposes:

    > If you are restarting the device and do not need to re-flash the EFR
    > firmware then you can use these commands (JLinkExe & JLinRTTClient). The
    > following commands will provide you with a serial console of the EFR32
    > <br>

    > `$ konsole -e JLinkExe -device EFR32MG12PXXXF1024 -if JTAG -speed 4000 -autoconnect 1 &`
    > (Put it in the background)

    > `$ sleep 3`

    > `$ konsole -e JLinkRTTClient &`

    > You may need to press <ENTER> in the JLinkExe console first

<br>

## Demo Execution - Commissioning a Wi-Fi Device using ChipTool for Linux

> Commissioning can be done using ChipTool running either on
> Linux or Raspberry Pi

1. Get the SSID and PSK of the Wi-Fi network (WPA2 - Security) you are connected
   to.
2. Run the following:

    > `$ cd $MATTER_WORKDIR/matter`

    ### Commissioning Command:

    > `$ out/standalone/chip-tool pairing ble-wifi 1122 $SSID $PSK 20202021 3840`

    > The node ID used here is 1122. This will be used in future commands.
    > '\$SSID' is a placeholder for your Wi-Fi SSID and '\$PSK' is a placeholder
    > for the password of your Wi-Fi network.

4. To turn **on** the LED on the EFR32MG12: <br> &emsp;
   `$ out/standalone/chip-tool onoff on 1122 1`
5. To turn **off** the LED on the EFR32MG12: <br> &emsp;
   `$ out/standalone/chip-tool onoff off 1122 1`

> If there are any failures, run the following command and then re-run the
> ChipTool command: `$ rm -rf /tmp/chip_*`

> If you are having difficulty getting the Chip-Tool to commission the device successfully, it may be because you have more than one network interface available to the Chip-Tool. The device on which you are running the Chip-Tool must be on the same Wi-Fi network as your RS9116 or WF200 and there cannot be another network interface on the device that is running the Chip-Tool. For instance, if you have an Ethernet connection as well as a Wi-Fi connection, you need to unplug the Ethernet connection and try running the Chip-Tool as in step #3 above.

> As the device remembers the Access Point credentials given for commissioning,
> if you want to run the demo multiple times, do a factory reset by pressing the
> BTN0 on EFR32 MG12 for about 6-7 seconds. The LED0 and LED1
> will flash 3 times and the QR code will appear again on the LCD screen.

<br>

The commissioning command mentioned above does the following:

-   ChipTool scans BLE and locates the Silicon Labs device that uses the specified
    discriminator
-   Sends the Wi-Fi SSID and Passkey
-   The Silicon Labs device will join the Wi-Fi network and get an IPv4 address. It
    then starts providing mDNS records on IPv4 and IPv6
-   ChipTool then locates the device over Wi-Fi and establishes
    operational certificates
-   Future communications (tests) will then happen over Wi-Fi


----
[Table of Contents](../README.md) | [Thread Demo](../thread/DEMO_OVERVIEW.md) | [Wi-Fi Demo](./DEMO_OVERVIEW.md)