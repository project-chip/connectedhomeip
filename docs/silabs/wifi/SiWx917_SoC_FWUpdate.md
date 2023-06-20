# Updating SiWx917 SoC Firmware using Tera Term

> Please note that the method of updating the SiWx917 SoC firmware listed below is considered a legacy mode of operation. You may also update the SiWx917 SoC firmware using Simplicity Commander. For instructions on how to do this please consider [Running Demo on the SiWx917SoC](./RUN_DEMO_SiWx917_SoC.md)

## Steps to update the firmware on SiWx917 SoC radio board

1. Connect power cable to radio board.
    
2. Jumper should be connected.

3. Switches: Left switch should be **ON/high**, and the right switch should be on the **ISP** side.

![Silicon Labs - SiWx917 SoC RadioBoard](./images/SiWx917_SoC_RadioBoard.png)

4. Open Tera term in windows PC and check for connected USB devices.

5. Select JLink CDC UART Port and change baud rate to 921600 to increase speed.

6. From terminal type "CTRL + |" for boot message "U", then type "U" for boot menu, type "B", "0" to burn wireless / "4", "1" to burn MCU firmware, and give (Ctrl + \ + c) to exit from kermit.

![Silicon Labs - SiWx917 FW Flash Options](./images/SiWx917_SoC_FWFlashOption.png)

7. From terminal, go to Transfer->Kermit→Send and select *.rps file to load.

![Silicon Labs - SiWx917 FW send](./images/SiWx917_SoC_Kermit.png)

8. Once the image transfer is complete, type "c" to continue to check the upgrade sequence, once the upgrade is successful , Enter "1" for the Enter next command , once text "Loading..." appears type (Ctrl + \ + c) to exit from Kermit and "q" to quit and return to the terminal.

9. Reset both the buttons to Off/Low state.

10. Flash and run the application - [Running the Matter Demo on SiWx917 SoC](RUN_DEMO_SiWx917_SoC.md).
