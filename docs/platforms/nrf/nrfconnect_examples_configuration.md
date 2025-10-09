# Configuring nRF Connect examples

The nRF Connect example applications all come with a default configuration for
building.

Check the information on this page if you want to modify the application
configuration or add new functionalities to build your own application based on
the provided example. This page also contains information about the
configuration structure, which can be useful to better understand the building
process.

<hr>

## Configuring application

Changing the default application configuration can be done either temporarily or
permanently.

-   Changing configuration temporarily is useful for testing the impact of
    changes on the application behavior.
-   Making permanent changes is better if you want to develop your own
    application, as it helps avoid repeating the configuration process.

Regardless of the option, you will need to rebuild your application. This will
require you to provide the build target name of the kit you are using. You can
find the build target names in the Requirements section of the example you are
building or on the
[Board support](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/app_dev/board_support/index.html)
page in the nRF Connect SDK documentation.

### Temporary changes to configuration

You can change the configuration temporarily by editing the `.config` file in
the `build/zephyr/` directory, which stores all configuration options for the
application generated as a result of the build process.

As long as you do not remove the current build directory or delete this file,
your changes will be kept. However, if you do a clean build, your changes are
gone, so it is not possible to save changes permanently this way.

Complete the following steps:

1.  Build the application by typing the following command in the example
    directory, with _build-target_ replaced with the build target name of the
    kit, for example _nrf52840dk_nrf52840_:

    ```
    west build -b build-target
    ```

2.  Run the terminal-based interface called menuconfig by typing the following
    command:

    ```
    west build -t menuconfig
    ```

    The menuconfig terminal window appears, in which you can navigate using
    arrow keys and other keys, based on the description at the bottom of the
    window.

3.  Make the desired changes by following the menuconfig terminal instructions.
4.  Press `Q` to save and quit.
5.  Rebuild the application.

At this point, the configuration changes are applied to the output file and it
can be flashed to the device.

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

You can assigning a value to a configuration option by typing its full name
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
[setting Kconfig values](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/build/kconfig/setting.html)
in the nRF Connect SDK documentation.

Because Kconfig configuration files are used in the building process, make sure
that you rebuild your application after editing them by typing the following
command in the example directory, with _build-target_ replaced with the build
target name of the kit, for example _nrf52840dk_nrf52840_:

```
west build -b build-target
```

<hr>

## Configuration structure overview

Zephyr RTOS and related software components, like drivers and libraries, provide
a set of Kconfig files which define available configuration options and assign
them default values for any application.

The application configuration is specified using Kconfig configuration files
(`*.conf`), where available Kconfig options can be used and their default values
overridden. Typically, there are many files having impact on the final
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
[Kconfig](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/build/kconfig/index.html)
guide in the nRF Connect SDK's Zephyr documentation if you are interested in
getting more advanced and detailed information about the configuration
structure.

<hr>

## Configuring Matter in nRF Connect platform

When configuring Matter support using the nRF Connect platform, some
configuration options are required, while other are optional and depend on what
application behavior you want to achieve.

### Mandatory configuration

To use the Matter protocol, complete the following steps:

1. Set the `CONFIG_CHIP` Kconfig option. Setting this option enables the Matter
   protocol stack and other associated Kconfig options, including
   `CONFIG_CHIP_ENABLE_DNSSD_SRP` that is required for the Matter device to be
   discoverable using DNS-SD.
2. Set the `CONFIG_CHIP_PROJECT_CONFIG` Kconfig option and define the path to
   the configuration file that specifies Vendor ID, Product ID, and other
   project-specific Matter settings.

### Optional configuration

After enabling the Matter protocol and defining the path to the Matter
configuration file, you can enable additional options in Kconfig.

#### Sleepy End Device support

You can enable the support for Thread Sleepy End Device in Matter by setting the
following Kconfig options:

-   `CONFIG_OPENTHREAD_MTD`
-   `CONFIG_CHIP_ENABLE_ICD_SUPPORT`

The following Kconfig options can be used to tune Thread Sleepy End Device wake
intervals:

-   `CONFIG_CHIP_ICD_SLOW_POLL_INTERVAL`
-   `CONFIG_CHIP_ICD_FAST_POLLING_INTERVAL`

#### Onboarding payload delivered over NFC

You can configure the Matter protocol to use an NFC tag for delivering the
onboarding payload, instead of using a QR code, which is the default
configuration.

To enable sharing the onboarding payload in an NFC tag, set the
`CONFIG_CHIP_NFC_ONBOARDING_PAYLOAD` option.

#### Factory reset behavior

By default, the factory reset procedure implemented in the Matter stack removes
Matter-related settings only. If your application does not depend on any
device-lifelong data stored in the non-volatile storage, set the
`CONFIG_CHIP_FACTORY_RESET_ERASE_SETTINGS` option to fully erase the
non-volatile settings partition at the factory reset. This approach is more
robust and regains the original non-volatile performance in case it has been
polluted with unwanted entries.

#### Logging

You can enable logging for both the stack and Zephyr’s
[Logging](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/services/logging/index.html)
API by setting the `CONFIG_LOG` option.

Zephyr allows you to configure log levels of different software modules
independently. To change the log level configuration for the Matter module, set
one of the available options:

-   `CONFIG_MATTER_LOG_LEVEL_ERR`
-   `CONFIG_MATTER_LOG_LEVEL_INFO`
-   `CONFIG_MATTER_LOG_LEVEL_DBG`

#### Shell

You can enable the Matter shell library using the `CONFIG_CHIP_LIB_SHELL`
Kconfig option. This option lets you use the Matter specific shell commands. See
[Using CLI in nRF Connect examples](nrfconnect_examples_cli.md) for the list of
available Matter shell commands.

#### Matter device identification

Matter has many mandatory and optional ways to identify a specific device. These
can be used for various purposes, such as dividing devices into groups (by
function, by vendor, or by location), device commissioning or vendor-specific
cases before the device was commissioned (for example, identifying factory
software version or related features).

Only some part of these features can be configured using Kconfig options:

-   `CONFIG_CHIP_DEVICE_TYPE` - This option specifies the type of device that
    uses the Matter Device Type Identifier, for example Door Lock (0x000A) or
    Dimmable Light Bulb (0x0101).
-   `CONFIG_CHIP_COMMISSIONABLE_DEVICE_TYPE` - This option enables including
    optional device type subtype in the commissionable node discovery record,
    which allows filtering of the discovery results to find the nodes that match
    the device type.
-   `CONFIG_CHIP_ROTATING_DEVICE_ID` - This option enables the rotating device
    identifier, an optional feature that provides an additional unique
    identifier for each device. This identifier is similar to the serial number,
    but it additionally changes at predefined times to protect against long-term
    tracking of the device.
