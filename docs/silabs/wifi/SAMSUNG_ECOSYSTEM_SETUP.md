# Samsung Ecosystem Setup and Demo Execution

## Hardware Requirements
Hardware Required for Samsung Smart Thing EcoSystem [Refer Ecosystem Setup Prequisites Section](./ECOSYSTEM_SETUP.md#prerequisites)
## Software Requirements
- Samsung Account
- Samsung Smart things App on smartphone.

## Setup of Samsung Smart Home Hub
[Refer Official Set up Smart Home Hub Page](https://aeotec.freshdesk.com/support/solutions/articles/6000240326-how-to-setup-smart-home-hub)

## Matter Demo Execution using Samsung Smart Aeotec

1. Build Matter Application by referring page
  [Build MATTER Application](./SW_SETUP.md)

2. To Connect Board to a Computer follow below steps
    - For Wi-Fi NCP Mode Boards
        1. Mount the EFx32 radio board on the EFx32 WSTK board.

           ![Silicon Labs - design](./images/mount-efr32.png)

        2. Connect the NCP expansion board to the EXP header on the EFx32 WSTK board.

           ![Silicon Labs - design](./images/mount-expansion.png)

        3. Toggle the upper switch on the NCP expansion board to EXP-UART.
    
    - For Wi-Fi SOC Mode Boards
        1. Mount the SiWx917 radio board on the SiWx917 WSTK board.

            ![Silicon Labs - design](./images/mount-soc.png)

        2. Connect your SiWx917 Wireless Starter Kit (WSTK) board to your computer using a USB cable.

3. Flash the bootloader binary for your device along with the application (e.g., lighting , lock, thermostat, window covering, light-switch).
   - For Wi-Fi NCP Mode Boards [Follow the instructions in this link to flash binaries](../general/FLASH_SILABS_DEVICE.md)
   - For Wi-Fi SOC Mode Boards [Follow the instructions in this link to flash binaries](../general/FLASH_SILABS_SiWx917_SOC_DEVICE.md)

4. Open Smart things app , tap on “+" ,Select **Add device**

![Silicon Labs - design](./images/samsung-app-add-device.png)

5. Select Partner Devices.

![Silicon Labs - design](./images/samsung-app-select-partner.png)

6. Through the smart things app scan the Application QR code to trigger commissioning.
 
7. After scanning QR code ,verify Commissioning is triggered by checking on the DUT logs.

![Silicon Labs - design](./images/samsung-app-commissioning.png)

8. While commissioning at last step it will Register your device with Access Point.

![Silicon Labs - design](./images/samsung-register-device.png)

9. Once commissioning is success, verify Matter Application is added in one  room of Samsung smart App and is in online mode.

![Silicon Labs - design](./images/samsung-light-added.png)

10. Send UI command from Smart thing app to control Matter Application.
   
   **For Example:** Tap on Light UI to turn ON/OFF.

## Control the Light via Samsung Smart Thing App
- In the Samsung Smart Thing app, you will now be able to tap your light to turn it ON and OFF.
- You will see the LED1 on your WSTK board turned on or off depending on the command you enter.
   - **Note**: Samsung Smart Thing won't support voice control commands.

## Deleting the Matter Application From Samsung Smart App

- In order to remove the Matter Application from Samsung Smart things app go to **Devices Section**.
- Long Press and Hold Matter Application and click on **Remove**.
- If you want to add Matter Application again first factory reset the device and follow step 4 again.