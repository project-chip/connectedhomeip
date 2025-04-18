# CHIP NXP All-clusters Application

## Overview

The all-clusters example implements a server which can be accessed by a CHIP
controller and can accept basic cluster commands.

The example is based on
[Project CHIP](https://github.com/project-chip/connectedhomeip) and NXP SDK, and
provides a prototype application that demonstrates device commissioning and
different cluster control.

### Supported platforms

The All-Clusters example is supported on the following platforms :

| NXP platform        | Dedicated readme                                                             |
| ------------------- | ---------------------------------------------------------------------------- |
| RW61x (Zephyr OS)   | [NXP Zephyr Guide](../../../docs/platforms/nxp/nxp_zephyr_guide.md)          |
| RW61x (FreeRTOS OS) | [NXP RW61x (FreeRTOS) Guide](../../../docs/platforms/nxp/nxp_rw61x_guide.md) |
| RT1170              | [NXP RT1170 Guide](../../../docs/platforms/nxp/nxp_rt1170_guide.md)          |
| RT1060              | [NXP RT1060 Guide](../../../docs/platforms/nxp/nxp_rw61x_guide.md)           |

For details on platform-specific requirements and configurations, please refer
to the respective platform's readme.

## Environment Setup, Building, and Testing

All the information required to set up the environment, build the application,
and test it can be found in the common readme for NXP platforms :

-   NXP FreeRTOS Platforms : Refer to the
    [CHIP NXP Examples Guide for FreeRTOS platforms](../../../docs/platforms/nxp/nxp_examples_freertos_platforms.md)
-   NXP Zephyr Platform : Refer to the
    [NXP Zephyr Application](../../../docs/platforms/nxp/nxp_zephyr_guide.md)

> Note : Matter-over-WiFi + Thread Border Router configuration is not supported
> in this application.
