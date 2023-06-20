# Google Ecosystem Setup and Demo

## 1. Hardware Requirements

- EFR32 MG24 (BRD4186C/BRD4187C)
- SiWx917 NCP
- Google nesthub (eg: Nest Hub 2nd Gen)
- Android phone (eg: Google pixel 5)
## 2. Set Up Google Home and Android Smartphone
### 2a. Google Matter EAP:

The Google Matter **Early Access Program** is a partnership between Google and silicon providers who support Matter development.
This partnership allows for faster onboarding of new devices into Matter and Thread by lowering the bar for starting with development of a new Matter-based product.
* The Google Matter Early Access Program is run through the Google Developer Center. You can access the Google developer center here:
<https://developers.home.google.com/>
* The Matter focused portion of the Google Developer Center is located here:
<https://developers.home.google.com/matter>
* The Google Matter Early Access Program is located here:
<https://developers.home.google.com/matter/eap>
```
Note: Until the public preview, access to this page is reserved to those allowed in by the Google Partner engineering team.
```

### 2b. Prerequisites for Google Setup

- In order to run the Google Ecosystem demo you will need both Google and matter device . you will need Google Nest Hub 2nd Generation and
an Android phone (at least a Pixel 5 is recommended) that can run at least Android 8 (8.1, API Level 27) or newer and has Bluetooth LE capability
### 2c. Instructions for Setting Up EAP:

- Once you have access, you will need to set up the Nest Hub 2nd Gen and Android phone with the Google Home app using the same Google Account that is used to access the EAP website.
- If you have set up the Nest Hub 2nd Gen with the correct Google account, you will receive the OTA update to the Beta version within 24 hours.
You can verify this by going to Device information > Technical information > Update Channel and the channel should read "matter-dev-current-beta-channel".
### 2d. Set Up the Android Phone

Follow these instructions to set up the Android phone with the necessary applications:
- Set up the Google Home app: <https://developers.home.google.com/matter/eap/setup/home-app>
- Set up Google Play Services :- <https://developers.home.google.com/matter/eap/setup/play-services>
### 2e. Create a Matter integration in the Google Developer Console.

Follow these instructions to create a Matter integration in the Google Developer Console.
http://developers.home.google.com/matter/eap/project/create

- After completing these steps, you should be ready to build your Matter accessory
## 3. Matter Integration Setup in the Developer Console

- Now that you have created a home in your android smartphone, add your Nest Hub to that home. 
- After this, on a browser on your PC go to the following webpage to create a project:
  https://developers.home.google.com/matter
- Click “Console” at the top of the page
- On the next page, click “Create a Project"
- Give your project a name and click “Create a new project”
- On the next page click “Add Matter integration”
- On the next page click “Next: Develop”
- Click “Next: Setup”
- Set Up the fields on this page as shown below:
  - Product name: Light
  - Device type: light
  - Vendor ID (VID) Test VID
  - Test VID: 0xFFF1
  - Product ID (PID) 0x8005

- Product ID options for Matter devices are as follows:
  - Light-Switch: 0x8004 
  - Light: 0x8005
  - Lock: 0x8006 
  - Thermostat: 0x800E 
  - Window Covering: 0x8010
- Click “Save & Continue.”
- On the next page Click “Save”.
- You will now see a Matter integration for device type light in your console 
- You have now completed setting up the following:
   - Your home in the Google home app in your Android smartphone
   -	A project in your Google developer console
- A matter integration for the light device type
Having finished the above, the only step left to have your setup ready is to open a QR code webpage for the light device type in your PC. This QR code webpage will be used to commission your light device.
To open it, open the following URL: <https://project-chip.github.io/connectedhomeip/qrcode.html?data=MT%3A6FCJ142C00KA0648G00>
 
## 4. Matter Demo using Google Home
### 4a. Commission MG24 + SiWx917 NCP matter device through Google Home app
- Follow the instructions in this link to build the Matter application: [Wifi Software Setup](./SW_SETUP.md)
- Power on your SiWx917 + MG24 setup and open the Google home app on your phone.
- Flash the bootloader binary for your device along with the application (e.g., lighting , lock, thermostat, window covering, light-switch).
   Follow the instructions in this link to flash binaries: [Run Wi-Fi Demo](./RUN_DEMO.md)
- Open the Google Home app on your phone.
- In it, tap the “+” sign in the top right corner.
- In the next screen, tap to select the “Set up device” option.
- Select the “New device” option.
- In the next screen make sure your home is selected and tap “Next”.
- The smartphone app will now search for the Matter device. If the device is not found tap "matter enabled device".
- Once the smartphone app has found the device, it will ask you to scan its QR code.  
     For example, the following QR Link has a Setup Discriminator value as - Decimal : **3840** / Hex : 0xF00 which is by default.
   <https://project-chip.github.io/connectedhomeip/qrcode.html?data=MT%3A6FCJ142C00KA0648G00>
- The Google Home app will now ask you if you want to connect this device to your Google account. Tap “I agree”. 
- The Google Home app will now commission the device with Bluetooth LE.
- Once you see the message stating that the device has been connected, tap “Done”.
- You will now see your light shown as being connected to your Google account.
### 4b. Control the Light via Google Home App
- In the Google Home app, you will now be able to tap your light to turn it ON and OFF.
- You can control the light by giving a voice command (for example, 'Ok Google! Turn ON Light') and through the app user interface.
- You will see the LED labeled LED1 on your MG24 dev board turn on or off depending on the command you enter.
