# TI Matter v1.2-1.0 Release Notes

This Texas Instruments Matter GitHub repository contains the software
development tools that enable engineers to develop Matter Device and is the
starting point for Matter development on all SimpleLink™ Thread and Wi-Fi®
devices.

## Supported Matter Devices

| Device                                            | Matter Thread | Matter Wi-Fi | Production Ready | Evaluation Only |
| ------------------------------------------------- | :-----------: | :----------: | :--------------: | :-------------: |
| [CC2674R10](https://www.ti.com/product/CC2674R10) |       x       |              |        x         |                 |
| [CC2674P10](https://www.ti.com/product/CC2674P10) |       x       |              |        x         |                 |
| [CC3235SF](https://www.ti.com/product/CC3235SF)   |               |      x       |                  |        x        |

## What's New

-   MATTER-276: Add support for matter v1.2 specification
-   MATTER-289: Add support to build matter application with pre-built certified
    thread libraries for simplifying matter certification process.
-   MATTER-264: Add Intermittent connected devices feature support
-   MATTER-225: Add support for ability to create programmable image build with
    unique factory data elements per device.
-   MATTER-226: Add support for CC1354P10-6 platform
-   MATTER-308: Deprecate CC2652x7 platform support
-   MATTER-286: Add support for CC2674 platform via migration guide
-   MATTER-324: Update Matter opensource component to commit id b4650b9db0f
-   MATTER-311: Update OpenThread to v1.3 commit id e7fbbcc60c2
-   MATTER-325: Add support for pulling in FreeRTOS from TI SimpleLink F2 SDK
-   MATTER-268: Add support to build TI SDK drivers at build time
-   MATTER-326: Update versions for TI SimpleLink F2 SDK to v7.40.00.77 and
    Sysconfig to v 1.18.1.

## Fixed Issues

-   MATTER-313: Fixed issue where factory reset will not occur after long left
    (BTN-1) press on the LaunchPad.

## Known Issues

-   None

## Versioning

This Product follows a version format, v<Matter_Spec_Version>-\<MM\>.\<mm\>

-   Matter_Spec_Version is the matter spec version
-   MM - major release version
-   mm - minor release version

# Operating System Support

-   Ubuntu v22.04

# Dependencies

-   TI OpenThread v1.3-1.0 commit id
    [e7fbbcc](https://github.com/TexasInstruments/ot-ti/tree/e7fbbcc60c25d1dec3ed4d02cff9acd866091ce7)
    -   Based on Thread v1.3.0.1 with commit id
        [8bc2504](https://github.com/openthread/openthread/tree/8bc25042ba5cde20605eec2a329c0dff575303f4)
-   Bluetooth LE Stack v5.x with SimpleLink SDK version: 7.40.00.77 or
    SimpleLink SDK version/commit id
    [374a26a45a5](https://github.com/TexasInstruments/simplelink-lowpower-f2-sdk/tree/374a26a45a5b05cd87c62d9a5da04d9e6d0ed319)
-   FreeRTOS: 10.5.1
-   Sysconfig: 1.18.1
-   GCC: Arm GNU Toolchain 12.2.MPACBTI-Rel1 12.2.1

## Technical Support and Product Updates

-   [TI SimpleLink Solutions](https://www.ti.com/wireless-connectivity/overview.html)
-   [TI E2E Community](https://e2e.ti.com/)
-   [TI Matter Overview](https://www.ti.com/matter)
