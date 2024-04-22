# How to Flash a Silicon Labs Device

Once you have an image built, you can flash it onto your EFR or SiWx917 device (either a development board or the Thunderboard Sense 2) over USB connected to your development machine. This can be done using Simplicity Commander.

## Simplicity Commander

Links to download Simplicity Commander's standalone versions are included below. Full documentation on Simplicity Commander is included in the [Simplicity Commander Reference Guide](https://www.silabs.com/documents/public/user-guides/ug162-simplicity-commander-reference-guide.pdf)

- Linux: https://www.silabs.com/documents/public/software/SimplicityCommander-Linux.zip
- Mac: https://www.silabs.com/documents/public/software/SimplicityCommander-Mac.zip
- Windows: https://www.silabs.com/documents/public/software/SimplicityCommander-Windows.zip

To Flash the bootloader and Binaries, refer [Flashing using Commander](./FLASHING_USING_COMMANDER.md)

>**Note**: Flashing the bootloader binaries is only required for NCP devices (MG12 and MG24). It is not applicable to the SiWx917 SoC.

## (Optional) J-Link RTT for Logging:

Multi-platform Logging software for device logs. Links to download J-Link RTT for logging are included below
 - Linux: https://www.segger.com/downloads/jlink/JLink_Linux_x86_64.deb
 - Mac: https://www.segger.com/downloads/jlink/JLink_MacOSX_universal.pkg
 - Window: https://www.segger.com/downloads/jlink/JLink_Windows_x86_64.exe