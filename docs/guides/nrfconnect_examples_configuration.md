# Configuring nRF Connect SDK examples

The nRF Connect SDK example applications all come with a default configuration for building.
Check the information on this page if you want to modify the application configuration or
add new functionalities to build your own application based on the provided example.
This page also contains information about the configuration structure, which can be useful
to better understand the building process.

This guide can be used with the following examples:

- [CHIP nRF Connect Lock Example Application](../../examples/lock-app/nrfconnect/README.md)
- [CHIP nRF Connect Lighting Example Application](../../examples/lighting-app/nrfconnect/README.md)
- [CHIP nRF Connect Pigweed Example Application](../../examples/pigweed-app/nrfconnect/README.md)

<hr>

## Configuring application

Changing the default application configuration can be done either temporarily or permanently.
Changing configuration temporarily is useful for testing the impact of changes
on the application behavior. Making permanent changes is better if you want
to develop your own application, as it helps avoid repeating the configuration process.

<hr>

### Temporary changes to configuration

You can change the configuration temporarily by editing the `zephyr/.config` file, which stores all
configuration options for the application generated as a result of the build process.
As long as you do not remove the current build directory or delete this file, your changes will be kept.
However, if you do a clean build, your changes are gone, so it is not possible to save changes permanently this way.

Complete the following steps:
1. Build the application by typing the following command in the example directory:

        # <board_name> should be replaced with the target board name (e.g. nrf52840dk_nrf52840)
        $ west build -b <board_name>

   See [Building nRF Connect examples](TODO:) for more details about building.
2. Run the terminal-based interface called menuconfig by typing the following command:

        $ west build -t menuconfig

   The menuconfig terminal window appears, in which you can navigate using arrow keys and other keys,
   based on the description at the bottom of the window.
3. Make the desired changes by following the menuconfig terminal instructions.
4. Press `Q` to save and quit.

At this point, the configuration changes are applied to the output file and it can be
flashed to the device.

<hr>

### Permanent changes to configuration

The permanent solution is based on modifying the Kconfig files, which are used as
components of the building process. This makes the changes persistent across builds.

The best practice to make permanent changes is to edit the main application configuration file
`prj.conf`, which is located in the example directory.
This will result in overriding the existing configuration values.

This method is valid for the majority of cases. If you are interested in understanding the big picture
of the configuration process, read the [Configuration structure overview](#configuration-structure-overview) section below.

#### Assigning values to Kconfig options

Assigning value to a configuration option is done by typing its full name preceded by the
`CONIG_` prefix, and adding the `=` mark and the value.

Configuration options have different types and it is only possible to assign them values of proper type.
Few examples:

    # assigning logical boolean true value to the option
    CONFIG_SAMPLE_BOOLEAN_OPTION=y
    # assigning numeric integer 1234 value to the option
    CONFIG_SAMPLE_INTEGER_OPTION=1234
    # assigning text string "some_text" value to the option
    CONFIG_SAMPLE_STRING_OPTION="some_text"

For more detailed information, read about [setting Kconfig values](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/guides/kconfig/setting.html#setting-configuration-values)
in the nRF Connect SDK documentation.

Because Kconfig files are used in the building process, make sure that you rebuild your application
after editing them by typing the following command in the example directory:

        # board_name should be replaced with the actual target board name
        $ west build -b board_name

See Building nRF Connect examples for details.

<hr>

<a name="configuration-structure-overview"></a>

## Configuration structure overview

The application configuration is specified using Kconfig files.
Typically, there are many files having impact on the final configuration shape,
because most of the components have their own configuration files.

There is no need to modify all these files separately.
See the following list for types of files you can find in the project
and which of them are important from your perspective:

- **Software libraries' configuration files.**
  Many libraries, components and modules have their own configuration files,
  but in the majority of cases you can override their configuration values
  in the main application configuration file, meaning that you do not need to modify
  their files unless you are interested in developing these libraries.

- **Application configuration file.**
  Every example application has its main configuration file called `prj.conf` that is
  located in the example directory. This file contains application-specific configuration
  or the most frequently changed options. Almost every configuration can be overridden
  in this file and probably this file is the most important one.

- **Overlays.**
  Overlay files are usually used to extract configuration for some specific
  case or feature from the general application configuration. The main
  difference between them and the application `prj.conf` file is that they are
  not included automatically, so you can decide whether to build sample with
  or without them.

- **Board configuration files.**
  These are hardware-platform-dependent configuration files, which are
  automatically included based on the compilation target board name.
  They contain configuration for board and its peripherals.

Read the [Kconfig](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/guides/kconfig/index.html#kconfig)
guide in the nRF Connect SDK's Zephyr documentation if you are interested in getting more advanced and detailed information about
the configuration structure.
