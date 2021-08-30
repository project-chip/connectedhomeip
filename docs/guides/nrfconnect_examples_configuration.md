# Configuring nRF Connect examples

The nRF Connect example applications all come with a default configuration for
building. Check the information on this page if you want to modify the
application configuration or add new functionalities to build your own
application based on the provided example. This page also contains information
about the configuration structure, which can be useful to better understand the
building process.

<hr>

## Configuring application

Changing the default application configuration can be done either temporarily or
permanently. Changing configuration temporarily is useful for testing the impact
of changes on the application behavior. Making permanent changes is better if
you want to develop your own application, as it helps avoid repeating the
configuration process.

<hr>

### Temporary changes to configuration

You can change the configuration temporarily by editing the `.config` file in
the `build/zephyr/` directory, which stores all configuration options for the
application generated as a result of the build process. As long as you do not
remove the current build directory or delete this file, your changes will be
kept. However, if you do a clean build, your changes are gone, so it is not
possible to save changes permanently this way.

Complete the following steps:

1.  Build the application by typing the following command in the example
    directory, with _build-target_ replaced with the build target name of the
    kit, for example _nrf52840dk_nrf52840_:

         $ west build -b build-target

2.  Run the terminal-based interface called menuconfig by typing the following
    command:

         $ west build -t menuconfig

    The menuconfig terminal window appears, in which you can navigate using
    arrow keys and other keys, based on the description at the bottom of the
    window.

3.  Make the desired changes by following the menuconfig terminal instructions.
4.  Press `Q` to save and quit.
5.  Rebuild the application.

At this point, the configuration changes are applied to the output file and it
can be flashed to the device.

<hr>

### Permanent changes to configuration

The permanent solution is based on modifying the Kconfig configuration files,
which are used as components of the building process. This makes the changes
persistent across builds.

The best practice to make permanent changes is to edit the main application
configuration file `prj.conf`, which is located in the example directory. This
will result in overriding the existing configuration values.

This method is valid for the majority of cases. If you are interested in
understanding the big picture of the configuration process, read the
[Configuration structure overview](#configuration-structure-overview) section
below.

#### Assigning values to Kconfig options

Assigning value to a configuration option is done by typing its full name
preceded by the `CONFIG_` prefix, and adding the `=` mark and the value.

Configuration options have different types and it is only possible to assign
them values of proper type. Few examples:

-   Assigning logical boolean true value to the option:
    `CONFIG_SAMPLE_BOOLEAN_OPTION=y`
-   Assigning numeric integer 1234 value to the option:
    `CONFIG_SAMPLE_INTEGER_OPTION=1234`
-   Assigning text string "some_text" value to the option:
    `CONFIG_SAMPLE_STRING_OPTION="some_text"`

For more detailed information, read about
[setting Kconfig values](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/guides/kconfig/setting.html#setting-configuration-values)
in the nRF Connect SDK documentation.

Because Kconfig configuration files are used in the building process, make sure
that you rebuild your application after editing them by typing the following
command in the example directory, with _build-target_ replaced with the build
target name of the kit, for example _nrf52840dk_nrf52840_:

        $ west build -b build-target

<hr>

<a name="configuration-structure-overview"></a>

## Configuration structure overview

Zephyr RTOS and related software components, like drivers and libraries, provide
a set of Kconfig files which define available configuration options and assign
them default values for any application.

The application configuration is specified using Kconfig configuration files
(`*.conf`), where available Kconfig options can be used and their default values
overrided. Typically, there are many files having impact on the final
configuration shape.

There is no need to modify all these files separately. See the following list
for types of files you can find in the project and which of them are important
from your perspective:

-   **Build target configuration files.** These are hardware-platform-dependent
    configuration files, which are automatically included based on the
    compilation build target name. They contain configuration for the kit and
    its peripherals.

-   **Project configuration file.** Every example application has its main
    configuration file called `prj.conf` that is located in the example
    directory. This file contains application-specific configuration or the most
    frequently changed options. Almost every configuration can be overridden in
    this file and probably this file is the most important one.

-   **Overlays.** Overlays are optional files usually used to extract
    configuration for some specific case or feature from the general application
    configuration. The main difference between them and the application
    `prj.conf` file is that they are not included automatically, so you can
    decide whether to build sample with or without them. Overlay files can be
    added to build by typing the following command in the example directory,
    with _build-target_ replaced with the build target name (e.g.
    _nrf52840dk_nrf52840_) and _overlay_file_name_ replaced with the overlay
    configuration file name (e.g. _overlay-usb_support.conf_):

          $ west build -b build-target -- -DOVERLAY_CONFIG=overlay_file_name

Read the
[Kconfig](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/guides/kconfig/index.html#kconfig)
guide in the nRF Connect SDK's Zephyr documentation if you are interested in
getting more advanced and detailed information about the configuration
structure.
