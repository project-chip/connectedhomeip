# Ozone Environment Setup for a SiWx917 SoC Device

- Auto detection of SiWx917 SoC device in Ozone is not enabled.
- Follow the steps to manually configure the SiWx917 SoC with Latest Ozone Debugger.

## Steps to Configure the SiWx917 SoC on the Ozone Debugger

1.  Update the **JlinkDevices.xml** and **ELF** files found in the [Matter Artifacts page](../general/ARTIFACTS.md).

     - Download the **JLinkDevices.xml** file and copy it in your Ozone installation path shown in this  [JLinkDevices folder](https://wiki.segger.com/J-Link_Device_Support_Kit#JLinkDevices_folder). If there is no JLinkDevices Folder, create a JLinkDevices folder and copy the **JlinkDevices.xml** file.

     - In the `JLinkDevices` folder, create a `Devices` folder and then create a sub-folder named `SiliconLabs`.

     - Download the **ELF** file (Flash driver) and copy it in the created `SiliconLabs` folder.

2.  Launch Ozone. The SiWx917 Dual Flash SoC device should be visible in the **Device** field's selection list.

![Silicon Labs - SiWx917 SoC](./images/SiWx917_Target_Ozone.png)

3.  If the above steps are successful, you can download and flash the Matter application 
    
    - [Flashing the Application with Ozone](./RUN_DEMO_SiWx917_SoC.md)

---

[Table of Contents](../README.md) | [Thread Demo](../thread/DEMO_OVERVIEW.md) |
[Wi-Fi Demo](./DEMO_OVERVIEW.md)
