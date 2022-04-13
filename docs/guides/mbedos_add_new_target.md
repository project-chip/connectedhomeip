![ARM Mbed-OS logo](https://raw.githubusercontent.com/ARMmbed/mbed-os/master/logo.png)

<h1> Mbed-OS add new hardware target </h1>

# Overview

This document shows how to add the new Mbed OS hardware target to Matter
project.

Please check the list of supported
[development boards](https://os.mbed.com/platforms/) which are compatible with
Mbed OS.

In order to adapt the new hardware target to the Matter project, you need to
remember about the following requirements:

-   Bluetooth Low Energy support.
-   Wireless communication module with WiFi or Thread network support (IPv6
    protocol is required).
-   Serial port support
-   On-chip debug with OpenOCD support

Additional target component requirements are different for each of example
application. Check the **Device UI** paragraph in example description.

# Example Application

The first step to add the new target to each of example application is to modify
the `examples/example_name/mbed/mbed_app.json` file. It contains the common
project settings and override the default values for supported boards. You
should add the necessary components and parameters for the target there.

If the new target uses the external libraries, it will be required to link it in
the CMakeLists.txt file.

# Building

To add the new hardware target to the build system the
`scripts/examples/mbed_example.sh` script should be modify. Extend
**SUPPORTED_TARGET_BOARD** variable with a new target name.

Example:

    SUPPORTED_TARGET_BOARD=(CY8CPROTO_062_4343W NEW_TARGET_NAME)

The next step is add the target name to build task in `.vscode/task.json` file.
Extend the **options** variable in **mbedTarget** input setting.

Example:

    {
        "type": "pickString",
        "id": "mbedTarget",
        "description": "What mbed target do you want to use?",
        "options": ["CY8CPROTO_062_4343W", "NEW_TARGET_NAME"],
        "default": "CY8CPROTO_062_4343W"
    }

# Flashing

Mbed OS example application flashing process uses the
[Open On-Chip Debugger](http://openocd.org/). The first step is to create the
target configuration file inside `config/mbed/scripts` directory. The file name
should be the same as target and the extension should be **.tcl**. The target
CPU and programming interface definitions are essential parts of the
configuration file.

The next steps are the same as for building process. Adding the name of the
target to `scripts/examples/mbed_example.sh` and `.vscode/task.json` files
allows the use of available flashing processes.

Additional flashing option is based on VSCode launch task. Adding the new target
to it required `.vscode/launch.json` modification. Extend the **options**
variable in **mbedTarget** input setting.

Example:

    {
        "type": "pickString",
        "id": "mbedTarget",
        "description": "What mbed target do you want to use?",
        "options": ["CY8CPROTO_062_4343W", "NEW_TARGET_NAME"],
        "default": "CY8CPROTO_062_4343W"
    }

# Debugging

Debugging process of Mbed OS applications is also based on VSCode launch task.
Adding the new target to it required `.vscode/launch.json` modification. Extend
the **options** variable in **mbedTarget** input setting.

Example:

    {
        "type": "pickString",
        "id": "mbedTarget",
        "description": "What mbed target do you want to use?",
        "options": ["CY8CPROTO_062_4343W", "NEW_TARGET_NAME"],
        "default": "CY8CPROTO_062_4343W"
    }

# CI

The Matter project continue integration process is based on Github Actions tool.
It uses workflow configuration files to execute actions on CI server.

To add the new target to the validation process of building Mbed OS applications
you need to modify the `.github/workflows/examples-mbed.yaml` file. Extend the
job's environment variable **APP_TARGET** with the target name.

Example:

    APP_TARGET: [CY8CPROTO_062_4343W, NEW_TARGET_NAME]
