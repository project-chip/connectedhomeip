# Configuring OpenThread libraries on TI CC13x4_CC26x4 Platforms

## Overview

TI Matter example applications provide configuration options for how Thread code
is included in the build. Thread code can be included in as:

1. Thread certified library optimized for Matter (recommended for development
   and production)
2. Full Source (can be used for development)
3. Custom option where customers can update the Thread config file to change the
   included OpenThread feature set

The library builds have been optimized for Matter to disable features such as
Thread Joiner capability in order to save on Flash/RAM usage. Refer to the
`ti_matter_production_certification.md` to get the cert ID for your Matter
certification application when using the certified Thread libraries from TI.

Build arguments have been added to enable Matter applications to link against
certified OpenThread FTD/MTD libraries or custom user libraries.

## Platform Code Changes

To configure a TI example to utilize either an OpenThread source or library
build, open up the `args.gni` file of the example:

To configure the device as an FTD or MTD, set following parameter to either true
or false respectively.

```
chip_openthread_ftd = true
```

-   **TI Certified OpenThread Library**:

    -   Typically this is used for development.

    1. `ot_ti_lib_dir` Is set to an empty string

        ```
        ot_ti_lib_dir=""
        ```

    2. `chip_openthread_target` Points to the TI certified library dependency

        ```
        chip_openthread_target = "${chip_root}/third_party/openthread/platforms/ti:ot-ti-cert"
        ```

    3. `openthread_external_platform` Points to the OpenThread build dependency
        ```
        `openthread_external_platform="${chip_root}/third_party/openthread/platforms/ti:libopenthread-ti"`
        ```

-   **OpenThread Library From Source**:

    1. `ot_ti_lib_dir` Is set to an empty string

        ```
        ot_ti_lib_dir=""
        ```

    2. `chip_openthread_target` Is set to an empty string

        ```
        chip_openthread_target = ""
        ```

    3. `openthread_external_platform` Points to the OpenThread build dependency
        ```
        openthread_external_platform="${chip_root}/third_party/openthread/platforms/ti:libopenthread-ti"
        ```

-   **Custom OpenThread Library**:

    -   The custom OpenThread library is used to implement extra features, or
        when modifying the stack in anyway.

    1.  `ot_ti_lib_dir` Points to a library directory containing a custom
        `libopenthread-ftd/mtd` variant

        ```
        ot_ti_lib_dir="${chip_root}/CUSTOM_LIB_DIR_PATH/"
        ```

    2.  `chip_openthread_target` Is set to an empty string

        ```
        chip_openthread_target = ""
        ```

    3.  `openthread_external_platform` Points to the OpenThread build dependency

        ```
        openthread_external_platform="${chip_root}/third_party/openthread/platforms/ti:libopenthread-ti"
        ```

The TI OpenThread certified library for the MTD/FTD configurations is based on
the following header file:

-   `third_party/openthread/ot-ti/src/openthread-core-cc13xx_cc26xx-config-matter.h`

In order to update the OpenThread configuration when building from source or a
custom library, users may adjust features via the following configuration header
file:

-   `${chip_root}/examples/platform/cc13x4_26x4/project_include/OpenThreadConfig.h`

Please refer to TI's standalone OpenThread Application build process for
instructions on acquiring FTD/MTD libraries as they are automatically built when
generating the standalone applications.

-   `third_party/openthread/ot-ti/README.md`
