# How to Flash a SiWx917 SoC Device

Once you have an image built, you can flash it onto your SiWx917 SoC device over USB connected to your development machine. This can be done using Simplicity Studio, Simplicity Commander or Ozone Debugger.


## Simplicity Commander
Links to download Simplicity Commander's standalone versions are included below. Full documentation on Simplicity Commander is included in the [Simplicity Commander Reference Guide](https://www.silabs.com/documents/public/user-guides/ug162-simplicity-commander-reference-guide.pdf)
 - Linux: https://www.silabs.com/documents/public/software/SimplicityCommander-Linux.zip
 - Mac: https://www.silabs.com/documents/public/software/SimplicityCommander-Mac.zip
 - Windows: https://www.silabs.com/documents/public/software/SimplicityCommander-Windows.zip
To Flash the bootloader and Binaries, refer [Flashing using Commander](./FLASHING_USING_COMMANDER.md)

## Simplicity Studio:
Simplicity Studio is a complete development environment and tool suite. It has the ability to discover USB connected development boards and flash them.
- [Download Simplicity Studio](https://www.silabs.com/developers/simplicity-studio)
- [Simplicity Studio Reference Guide](https://docs.silabs.com/simplicity-studio-5-users-guide/latest/ss-5-users-guide-building-and-flashing/flashing)

## Ozone J-Link Debugger:
- Multi-platform debugger and performance analyzer for J-Link and J-Trace
  - Download for: [Windows](https://www.segger.com/downloads/jlink/Ozone_Setup_Windows_x64.exe) | [Mac](https://www.segger.com/downloads/jlink/Ozone_macOS_Universal.pkg) | [Linux](https://www.segger.com/downloads/jlink/Ozone_Linux_x86_64.deb)
- [Flashing and debug procedure for Ozone](../wifi/RUN_DEMO_SiWx917_SoC.md)

## (Optional) J-Link RTT for Logging:
Multi-platform Logging software for device logs. Links to download J-Link RTT for logging are included below
 - Linux: https://www.segger.com/downloads/jlink/JLink_Linux_x86_64.deb
 - Mac: https://www.segger.com/downloads/jlink/JLink_MacOSX_universal.pkg
 - Window: https://www.segger.com/downloads/jlink/JLink_Windows_x86_64.exe