[<< Back to TOC](../README.md)

# Commissioning Wifi Device using Android

Commissioning can be done using an Android Phone through the following steps.

Download pre-build application from the
[Matter Artifacts page](../general/ARTIFACTS.md).

1. Open the .apk that is installed in the Android mobile
2. Connect the Android phone to the Wi-Fi Access Point that is going to be used
3. Run the installed CHIP app
4. Click 'Provision Chip device with Wi-Fi'
5. The app will bring up the camera:
    - Hold the camera to the LCD on the WSTK board
    - Scan the QR Code displayed on LCD screen of EFR32 MG12 Platform
6. Input the SSID/Passphrase of the Wi-Fi Access Point on the next screen
7. Hold the phone very close (a few centimeters) from the EFR32 platform (as the
   Bluetooth Antenna is slightly weak)
8. You will see messages (Toasts) pop up - saying that the App is 'Scanning',
   then 'Pairing', followed by 'Commissioning Done'
9. Once commissioning is completed, the app will go back to the original/home
   screen
10. Click on 'Light ON/OFF & Level Cluster'
11. You can then bring up the On/Off Cluster and send On/Off Commands (Toggle
    does not work as required currently) - this will cause LED 1 on the WSTK to
    change states

> If the Commissioning is not successful, try to re-boot your mobile and try
> again.

> Once commissioning is completed, if you want to repeat the test, follow these
> steps:
>
> -   Remove power to the system (EFR32MG12 + RS9116)
> -   Power up the system again - this should cause the LCD to turn on and the
>     QR code to show up
> -   Press the BTN0 button and keep it pressed for about 1 min - this should
>     cause LED0 and LED1 to turns ON and OFF for 3 times. You can then leave
>     the button
