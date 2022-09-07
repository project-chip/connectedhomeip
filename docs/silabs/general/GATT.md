# Using Simplicity Studio's GATT Configurator with Matter

The Simplicity Studio Bluetooth GATT Configurator is an Advanced Configurator
within the Simplicity Studio Project Configuration suite. For more information on
using the
[Simplicity Studio Project Configurator in Matter refer to this guide](./PINTOOL.md).
Use of the BLE GATT Configurator in Matter assumes that you are already familiar
with creating and using an `Empty C++` Project in Simplicity Studio.

The basic steps for using the Simplicity Studio BLE GATT Configurator are as
follows:

1. Create an `Empty C++` project in Simplicity Studio's Project Creation Wizard

    More information on creating an `Empty C++` project can be found in
    [the Project Configurator guide here.](./PINTOOL.md)

2. Add the BLE component to your project

    Once you have opened your `Empty C++` project, you can add the BLE GATT
    configuration by adding the `Bluetooth > GATT > Configuration` component to
    your project. This will enable the BLE GATT Configurator under
    `Configuration Tools > Bluetooth GATT Configurator`

3. Open the BLE GATT Advanced Configurator in Simplicity Studio

    In the top menu of your project navigate to
    `Configuration Tools > Bluetooth GATT Configurator`

4. Change your GATT settings as you wish for your project.

    More information on using the
    [BLE GATT Configurator is provided here.](https://docs.silabs.com/simplicity-studio-5-users-guide/latest/ss-5-users-guide-developing-with-project-configurator/bluetooth-gatt-configurator)

5. Save your project, which will force the generation of your BLE GATT
   Configuration

6. BLE GATT database is stored in the `autogen` folder in your project directory

7. Move the BLE GATT db into your Matter project.

    Copy the BLE GATT db header files out of your project and into the Matter
    project. BLE GATT db header and `.c` files are located in
    `./src/platform/EFR32` in the Silicon Labs Matter GitHub repo.

---

[Table of Contents](../README.md) | [Thread Demo](../thread/DEMO_OVERVIEW.md) |
[Wi-Fi Demo](../wifi/DEMO_OVERVIEW.md)
