# Configuring Openthread libraries on TI CC13x4 Platforms

## Overview

TI Matter example applications provide configuration option for how Thread code
is included in the build. Thread code can be included in as (1) full source or
(2) as optimized for matter thread certified library. The library builds have
been optimized for matter to disable features such as Thread Joiner capability
in order to save on Flash/RAM usage.

Build arguments have been added to enable Matter applications to link against
certified Openthread FTD/MTD libraries or custom user libraries.

## Platform Code Changes

To configure a TI example to utilize either an Openthread source or library
build, open up the `args.gni` file of the example:

To configure the device as an FTD or MTD, set following parameter to either true
or false respectively.

```
chip_openthread_ftd = true
```

-   **Openthread Library From Source**:

    1. `ot_ti_lib_dir` Is set to an empty string

        ```
        ot_ti_lib_dir=""
        ```

    2. `chip_openthread_target` Is set to an empty string

        ```
        chip_openthread_target = ""
        ```

    3. `openthread_external_platform` Points to the Openthread build dependency
        ```
        openthread_external_platform="${chip_root}/third_party/openthread/platforms/ti:libopenthread-ti"
        ```

-   **TI Certified Openthread Library**:

    1. `ot_ti_lib_dir` Is set to an empty string

        ```
        ot_ti_lib_dir=""
        ```

    2. `chip_openthread_target` Points to the TI certified library dependency

        ```
        chip_openthread_target = "${chip_root}/third_party/openthread/platforms/ti:ot-ti-cert"
        ```

    3. `openthread_external_platform` Points to the Openthread build dependency
        ```
        `openthread_external_platform="${chip_root}/third_party/openthread/platforms/ti:libopenthread-ti"`
        ```

-   **Custom Openthread Library**:

    1.  `ot_ti_lib_dir` Points to a library directory containing a custom
        libopenthread-ftd/mtd variant

        ```
        ot_ti_lib_dir="${chip_root}/CUSTOM_LIB_DIR_PATH/"
        ```

    2.  `chip_openthread_target` Is set to an empty string

        ```
        chip_openthread_target = ""
        ```

    3.  `openthread_external_platform` Points to the Openthread build dependency

        ```
        openthread_external_platform="${chip_root}/third_party/openthread/platforms/ti:libopenthread-ti"
        ```

The TI Openthread certified library for the MTD/FTD configurations is based on
the following header file:

-   `third_party/openthread/ot-ti/src/openthread-core-cc13xx_cc26xx-config-matter.h`

In order to update the Openthread configuration when building from source or a
custom library, users may adjust features via the following configuration header
file:

-   `${chip_root}/examples/platform/cc13x4_26x4/project_include/OpenThreadConfig.h`

Please refer to TI's standalone Openthread Application build process for
instructions on acquiring FTD/MTD libraries as they are automatically built when
generating the standalone applications.

-   `third_party/openthread/ot-ti/README.md`
