# Using Simplicity Studio's Pintool and Project Configuration with Matter

In the course of developing your product you will likely run into a situation
where you need to move your project over to your custom hardware. In this case,
you will likely need to change the pinout and hardware configuration from what
it is in the example project to reflect your own custom project. You can do this
with Simplicity Studio's Pintool starting from a blank C++ project.

## 1. Locating the board support files in the Matter repo

The pin and peripheral configuration for your example application is stored
within the Silicon Labs Matter support directory. For all the examples used in
the matter repository, the peripheral and pin configurations are stored at

`./third_party/silabs/matter_support/matter/efr32/<chip_family>/<board>/config`

When creating a configuration for a custom board you do the following:

1. Create a Custom C++ project within Simplicity Studio.
2. Include your desired peripherals in the project.
3. Copy the generated output config files into a custom board support directory
   within the Matter repository.

## 2. Create a sample "Empty C++ project" in Simplicity Studio

1. Create a project Open Simplicity Studio and click on `Create a New Project`
   to start the project wizard. Choose your development board type, and the
   latest Gecko SDK you'll be working from. Click `Next`

1. Example C++ Project Choose your example `Empty C++ Project` and click `Next`

1. Create your project Click on `Finish` to create your project.

## 3. Customize your Components and Pin configuration in Simplicity Studio

Once you have your project created you will see your project and project
configuration in Simplicity Studio's Project Configurator. Full documentation on
the use of the Project Configurator and Pin Tool are located here:
[Simplicity Studio 5 User's Guide](https://docs.silabs.com/simplicity-studio-5-users-guide/latest/ss-5-users-guide-developing-with-project-configurator/)

## 4. Generate your Component and Pin configuration in Simplicity Studio

When you save your project configuration, you will notice that Simplicity Studio
saves all the generated header files out into a `config` directory in your
project. These are the files that make up the software component and pin tool
configuration for your device.

## 5. Move your pin configuration over to your Matter project

All of the header files in your `config` project directory constitute the
hardware configuration for your device. You will want to copy these files over
into your Matter project so that they can be used in place of the ones provided
in the example.

---

[Table of Contents](../README.md) | [Thread Demo](../thread/DEMO_OVERVIEW.md) |
[Wi-Fi Demo](../wifi/DEMO_OVERVIEW.md)
