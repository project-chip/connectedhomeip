# NXP Matter Application Architecture for FreeRTOS platforms

## Overview

This guide explains the architecture of NXP Matter applications with CMake build
system for FreeRTOS platforms, using an in-tree example to illustrate core
concepts. It also provides insights into customization options for developers
working on out-of-tree applications.

## Application build system architecture

NXP Matter applications rely on two main build systems : GN and CMake. The
architecture is designed to streamline the integration and the customization of
Matter into different applications.

-   **GN build system** : GN is used to compile the Matter stack and platform
    libraries, and generate the CHIP libraries which will be linked to the
    application.
-   **CMake build system** : CMake is used to configure and compile the
    application and the NXP SDK, and link the libraries to generate the final
    executable. CMake build system is using Kconfig to configure system options.
    All related information can be found in
    [NXP MCUX Build And Configuration System](https://mcuxpresso.nxp.com/mcuxsdk/latest/html/develop/build_system/index.html).

### Matter stack and NXP platform port libraries

Below is a detailed description of CMake/Kconfig files used to generate the
Matter stack & NXP port libraries.

-   `config/nxp/chip-cmake-freertos/CMakeLists.txt` : CMake wrapper which will
    map CMake configuration on GN configuration and generate the `CHIP`
    libraries for NXP FreeRTOS platforms. With this CMake file, Matter can be
    integrated as an
    [external module of NXP MCUX SDK](https://mcuxpresso.nxp.com/mcuxsdk/latest/html/develop/sdk/integration.html#integrate-other-cmake-build-system).
-   `config/nxp/cmake/common.cmake` : CMake file common to Zephyr and FreeRTOS
    platforms, used to configure GN arguments based on CMake configuration.

-   `config/nxp/chip-cmake-freertos/Kconfig` : Kconfig file which defines Matter
    Kconfig symbols common to NXP FreeRTOS platforms.
-   `config/nxp/chip-cmake-freertos/Kconfig.defaults` : The role of this Kconfig
    is to override default values of Kconfig symbols. These values are tuned for
    Matter requirements.
-   `config/nxp/cmake/Kconfig.matter.common` : Kconfig file used to define
    Matter Kconfig symbols common across Matter platforms.
-   `config/nxp/cmake/Kconfig.matter.nxp` : Kconfig file used to define Matter
    Kconfig symbols common to NXP platforms.

### Application

To describe the CMake architecture for the NXP applications, the
`all-clusters-app` is provided as an example of application. For more
information about supported applications, how to setup and build the
application, you can refer to
[CHIP NXP Examples Guide for FreeRTOS platforms](./nxp_examples_freertos_platforms.md).

-   `examples/all-clusters-app/nxp/CMakeLists.txt` : Application CMake list file
    common to NXP FreeRTOS platforms. This CMakeLists.txt is used as the
    entry-point to build the application.
-   `examples/all-clusters-app/nxp/Kconfig` : Application Kconfig file used as
    the entry-point Kconfig. This file can be used to define application
    specific Kconfig symbols.
-   `examples/all-clusters-app/nxp/prj.conf` : Application `prj.conf` used to
    set or unset Kconfig symbols specific to the application. This file is
    automatically loaded by the application's CMakeLists.txt.
-   `examples/platform/nxp/common/app_common.cmake` : Application CMake file for
    building files that are common across applications. This file can be
    included from the application CMakeLists.txt, this will add common
    application files to the build based on the Kconfig enabled.
-   `examples/platform/nxp/common/Kconfig` : Application Kconfig file used to
    define Kconfig symbols common across applications.
-   `examples/platform/nxp/config/prj_<flavour>.conf` : A custom `prj.conf` file
    used to set or unset Kconfig symbols for the application, for a specific
    configuration. The name of the `prj_<flavour>.conf` can be provided in the
    build command line with the `-DCONF_FILE_NAME` variable, to add the
    configuration file to the build. To view the full list of supported
    `prj_<flavour>.conf` and how to use them, you can refer to the
    [Available project configuration files and platform compatibility](./nxp_examples_freertos_platforms.md#available-project-configuration-files-and-platform-compatibility)
    section.

#### Overview on application CMakeLists.txt

Below are the key points on how the NXP application CMakeLists.txt is designed.
This architecture can be common to in-tree applications and out-of-tree
applications.

-   `CHIP` libraries are loaded as an external module of the NXP MCUX SDK. This
    is done by appending to the `EXTRA_MCUX_MODULES` list the path to the
    `config/nxp/chip-cmake-freertos/CMakeLists.txt`. This will enable the build
    system to automatically load and link the module library to the application.
-   NXP MCUX SDK is loaded by the application using the `find_package` CMake
    function, which enables CMake to automatically find the location where the
    SDK was installed. More information can be found in
    [McuxSDK CMake Package](https://mcuxpresso.nxp.com/mcuxsdk/latest/html/develop/sdk/integration.html#mcuxsdk-cmake-package).
-   The application includes `examples/platform/nxp/common/app_common.cmake` to
    add common application files.
-   The application includes
    `third_party/nxp/nxp_matter_support/examples/platform/<platform_family>/nxp_sdk_reconfig.cmake`.
    This CMake file is used to reconfigure the SDK to adapt to Matter
    requirements. In order for it to be correctly processed by CMake, this file
    must be included before creating the CMake `project()`.
-   When creating the CMake `project()`, the build system will automatically
    create the application target `app` and link Matter libraries and the SDK
    libraries to it (`McuxSDK`).
-   Application source files and include directories can be directly added to
    the target `app`.

### Customization options

The NXP application architecture provides ways to customize the build system and
adapt it to the application needs. This could be particularly useful when
integrating Matter to an out-of-tree application.

> Note : For an out-of-tree application,
> `-DCHIP_ROOT=</path/to/connectedhomeip>` could be provided in the build
> command-line to specify the path to the `connectedhomeip` repository.

#### Customize `nxp_sdk_reconfig.cmake`

`third_party/nxp/nxp_matter_support/examples/platform/<platform_family>/nxp_sdk_reconfig.cmake`
is provided as an example of how to reconfigure the NXP MCUX SDK to suit the
Matter application requirements, such as compiler options, linker settings,
board-specific files.

The structure of this file can be extended by users and adapted to suit the
specific needs of their applications.

To achieve this for a custom application, the former file can be copied under
the application environment, modified to suit its requirements, and included in
the application CMakeLists.txt. `NXP_SDK_RECONFIG_CMAKE_DIR` CMake variable
could be used in the build command-line (with CMake `-D` option) to provide the
new path of the `nxp_sdk_reconfig.cmake` to the application build.

#### Customize the application `prj.conf`

`examples/platform/nxp/config/prj_<flavour>.conf` are provided as an example of
configuration of the application.

Users can provide their custom prj.conf that could be adapted to their
application.

The custom file could either be included in the application CMakeLists.txt by
appending its path to the `CONF_FILE` CMake variable, or by providing the
absolute path directly in the `west build` command line using `-DCONF_FILE`.

Example :

```
west build -d build_matter -b rdrw612bga path/to/out-of-tree/application -DCONF_FILE=/absolute_path/to/prj_custom.conf
```

#### Customize SDK middleware configuration

-   `MBEDTLS_USER_CONFIG_FILE` macro may be used by the application to add or
    override MbedTLS configurations defined by default.
    -   This can be done by defining `MBEDTLS_USER_CONFIG_FILE` macro as the
        path of the application's MbedTLS config file, in the application's
        version of `nxp_sdk_reconfig.cmake` file.
    -   To ensure proper functionality, the application's MbedTLS user
        configuration file needs to include the necessary mbedtls macros for
        Matter. This can be achieved by adding
        `#include "nxp_matter_mbedtls_config.h"` in the user config file and
        ensuring that the include path for the
        `third_party/nxp/nxp_matter_support/gn_build/mbedtls/config/nxp_matter_mbedtls_config.h`
        is correctly set.
-   `LWIP_USER_CONFIG_FILE` macro may be used by the application to add or
    override LWIP configurations used by Matter by default.
    -   This can be done by defining `LWIP_USER_CONFIG_FILE` macro as the path
        of the application's LWIP config file, in the application's version of
        `nxp_sdk_reconfig.cmake` file.
-   `WIFI_USER_CONFIG_FILE` macro may be used by the application to add or
    override Wi-Fi configurations used by Matter by default.
    -   This can be done by defining `WIFI_USER_CONFIG_FILE` macro as the path
        of the application's WiFi config file, in the application's version of
        `nxp_sdk_reconfig.cmake` file.
-   FreeRTOS configuration is generated with Kconfig and can be found under
    `<build_directory>/FreeRTOSConfig_Gen.h`. The default values of FreeRTOS
    Kconfig symbols, which are required by Matter, are defined in
    `third_party/nxp/nxp_matter_support/cmake/<platform>/Kconfig.defconfig`.
    These values can be overridden by the application from the `prj.conf`, to be
    adapted to its specific needs.

### How to create an out-of-tree application from Matter in-tree application

NXP Matter in-tree applications can easily be converted to out-of-tree
applications, by following a few simple steps. In this demonstration, we will
use the Thermostat application as an example.

_Step 1_ : Copy the application source files and CMake/Kconfig files to the
out-of-tree application folder. The list of files to copy is the following :

-   Application source files can be found under
    `examples/thermostat/nxp/common/main`. This folder can be copied as is, and
    is containing the following files :
    -   `AppTask.cpp`
    -   `DeviceCallbacks.cpp`
    -   `main.cpp`
    -   `ZclCallbacks.cpp`
    -   `include` folder
-   `ZAP` files can be found under `examples/thermostat/nxp/zap`.
-   Application CMake/Kconfig files :
    -   `examples/thermostat/nxp/CMakeLists.txt`
    -   `examples/thermostat/nxp/Kconfig`
    -   `examples/thermostat/nxp/prj.conf`
-   `nxp_sdk_reconfig.cmake` can be found under
    `third_party/nxp/nxp_matter_support/examples/platform/<platform_family>`
    folder.
-   Additionally the linker file could also be copied for further customization,
    and can be found under
    `third_party/nxp/nxp_matter_support/examples/platform/<nxp_platform>/app/ldscripts`.

_Step 2_ : If the linker file is copied under the out-of-tree application
folder, then its new path should be provided with Kconfig
`CONFIG_MATTER_DEFAULT_LINKER_FILE_PATH` either in the application prj.conf, or
in the command-line with
`-DCONFIG_MATTER_DEFAULT_LINKER_FILE_PATH=<path/to/linkerfile>`.

_Step 3_ : Specify in the build command line the path to the `connectedhomeip`
repository, and the path of `nxp_sdk_reconfig.cmake`.

The build command has the following structure for an out-of-tree application :

```
user@ubuntu:~/Desktop/git/connectedhomeip$ west build -d <build_dir> -b <board> /path/to/out-of-tree_example -DCONF_FILE=/absolute/path/to/custom/prj.conf -DCHIP_ROOT=/path/to/connectedhomeip -DNXP_SDK_RECONFIG_CMAKE_DIR=/path/to/out-of-tree_example/nxp_sdk_reconfig.cmake
```

For more information about environment setup and build instructions, you can
follow the dedicated guide
[Matter NXP Examples Guide for FreeRTOS platforms](./nxp_examples_freertos_platforms.md).
