# Running the Matter Demo over Wi-Fi on an EFR32 device

## Flashing Images/Binaries on EFR32MG24 Platform using Ozone, Simplicity Studio, or Simplicity Commander

> Note: If you are coming from Simplicity Studio, you may have already installed
> the demo image using Simplicity Studio, in which case you can skip to the
> next step.

1.  Plug the WSTK and EFR into the laptop.

2.  Based on the Application being built, make sure to flash proper [bootloader](../general/ARTIFACTS.md) internal or external binaries. 

3.  Launch Ozone, Simplicity Studio or Simplicity Commander Standalone - this
    will display a GUI. 
    
    If you are using Simplicity Studio or Simplicity Commander, you can follow the general instructions for flashing a Silicon Labs device.
    [Flash a Silicon Labs Device](../general/FLASH_SILABS_DEVICE.md). If you are using Ozone, you can follow the instructions below.

## Directions for Flashing using Ozone

1.  In 'New Project Wizard':

    1. Click the three-dots on the 'Device' tab and select 'Manufacturer' as
       '`Silicon Labs`'
    2. Select 'Device' as `EFR32MG24Bxxx1536`
       depending on the device you are using.
    3. Click 'OK'
    4. The 'Register set' tab will get filled automatically
    5. 'Peripherals' tab need not be changed as it is optional

2.  Click 'Next' - the window that is displayed will contain:

    - Serial Number: Read from device
    - Target Interface: JTAG
    - Speed: 4MHz
    - Host Interface: USB
    - There will be one product in the 'Emulators connected via USB' tab -
        select this and click 'Next'

3.  Click on the 'Silicon Labs device' detected, and then click 'Next'

4.  You will be asked to select the image/binary to be loaded: click the
    three-dots on that tab and navigate to '`out/rs911x_lighting/BRD41xxx`'
    through 'Browse', select the file named `chip-efr32-lighting-example.out`
    and click 'Next'. This is the image built in previous step.

    > Note that you may be using a different board like the
    > BRD4186C or BRD4187C, in which case your image will be in a directory
    > corresponding to that board identifier.

5.  Make sure the next screen has 'Initial PC' selected as 'ELF Entry Point' -
    click 'Finish'

    > Ignore Diagnostics warning about 'FreeRTOS' detected - click 'Continue'

6.  Select 'Download and Reset Program' in the dropdown next to the Power button
    on the top left of the page

    > The Silabs chip, EFR32MG24, will be erased and programmed

7.  Run the image by clicking the 'Play' button on the top left (or press the F5
    key)

    > The output of the EFR32 can be viewed on the console of the Ozone GUI

8. **[Optional]** This step is for when the device has already been flashed and
    is being used for testing/debug purposes:

    > If you are restarting the device and do not need to re-flash the EFR
    > firmware, then you can use these commands (JLinkExe & JLinRTTClient). The
    > following commands will provide you with a serial console of the EFR32

    ```shell
    $ konsole -e JLinkExe -device EFR32MG24Bxxx1536 -if JTAG -speed 4000 -autoconnect 1 &
    ```

    > (Put it in the background)

    ```shell
    $ sleep 3
    ```

    ```shell
    $ konsole -e JLinkRTTClient &
    ```

    > You may need to press <ENTER> in the JLinkExe console first

## Demo Execution - Commissioning a Wi-Fi Device using chip-tool for Linux

> Commissioning can be done using chip-tool running either on Linux or Raspberry
> Pi

1. Get the SSID and PSK of the Wi-Fi network (WPA2 - Security) you are connected
   to.
2. Run the following:

    ```shell
    $ cd $MATTER_WORKDIR/matter
    ```

    ### Commissioning Command:

    ```shell
    $ out/standalone/chip-tool pairing ble-wifi 1122 $SSID $PSK 20202021 3840
    ```

    > The node ID used here is 1122. This will be used in future commands.
    > '\$SSID' is a placeholder for your Wi-Fi SSID and '\$PSK' is a placeholder
    > for the password of your Wi-Fi network.

3. To turn **on** the LED on the EFR32MG24:

    ```shell
    $ out/standalone/chip-tool onoff on 1122 1
    ```
4. To turn **off** the LED on the EFR32MG24:

    ```shell
    $ out/standalone/chip-tool onoff off 1122 1
    ```

If there are any failures, run the following command and then re-run the
chip-tool command:

```shell
$ rm -rf /tmp/chip_*
```

If you are having difficulty getting the chip-tool to commission the device
successfully, it may be because you have more than one network interface
available to the chip-tool. The device on which you are running the chip-tool
must be on the same Wi-Fi network as your RS9116 or WF200 and there cannot be
another network interface on the device that is running the chip-tool. For
instance, if you have an Ethernet connection as well as a Wi-Fi connection, you
need to unplug the Ethernet connection and try running the chip-tool as in step
#2 above.

As the device remembers the Access Point credentials given for commissioning, if
you want to run the demo multiple times, do a factory reset by pressing the BTN0
on EFR32MG24 for about 6-7 seconds. The LED0 will flash 3 times.

The commissioning command mentioned above does the following:

- chip-tool scans BLE and locates the Silicon Labs device that uses the
specified discriminator
- Sends the Wi-Fi SSID and Passkey
- The Silicon Labs device will join the Wi-Fi network and get an IP address.
It then starts providing mDNS records on IPv4 and IPv6
- chip-tool then locates the device over Wi-Fi and establishes operational
certificates
- Future communications (tests) will then happen over Wi-Fi
