# Matter Sleepy End Devices over OpenThread

This page explains how Matter OpenThread Sleepy End devices (SEDs) work and how to configure an SED example.

## Overview

Matter provides a Sleepy End Device operating mode to extend the battery life of a power limited devices. This operating mode leverages OpenThread functionalities to enhance the provided Matter features. Matter Sleepy functionalities currently focus on allowing an application to define the interval of time where a device is asleep.

## Operating Modes

SEDs have two operating modes, Idle and Active.

- _Active Mode_ sets the SED into a fast-polling interval for maximum responsiveness when the Node is engaged in ongoing communication, such as an active exchange. The SLEEPY_ACTIVE_INTERVAL parameter communicates the maximum sleep interval of a node in active mode.

- _Idle mode_, or slow-polling, sets the maximum time an SED will sleep before polling. This parameter affects both the minimum power consumption and maximum latency. The SLEEPY_IDLE_INTERVAL parameter communicates the maximum sleep interval of a node in idle mode.

A device determines if it is in Active or Idle mode based on whether it has at least one open exchange in the message layer. As long as the device has one open exchanges, it will remain in Active mode and poll its associated OpenThread router at the fast-polling interval. Once all exchanges are closed, the device will switch operating modes to Idle Mode.

When a device is in _Idle mode_, it will poll its associated router at its slow-polling interval to see if another device has tried to communicate with it while it was sleeping. If the OpenThread router has an outstanding message for the SED, the SED will enter its Active polling mode to process the message.

## Thread Communication

To receive message that were sent while the SED was sleeping, SED relies on its associated Thread router to buffer any incoming message. The Thread router will send all buffered message to the SED when the SED polls the router at the end of its slow-polling interval.

## Configuration

Matter exposes two defines that can be set to configure the SLEEPY_ACTIVE_INTERVAL and SLEEPY_IDLE_INTERVAL parameters. 

| Parameter Name | Define | Description | Default Value | Maximum allowed Value |
| - | - | - | - | - |
| SLEEPY_IDLE_INTERVAL | CHIP_DEVICE_CONFIG_SED_IDLE_INTERVAL | Maximum node sleep interval when in idle mode. | 5000 ms | <= 1 hour|
| SLEEPY_ACTIVE_INTERVAL | CHIP_DEVICE_CONFIG_SED_ACTIVE_INTERVAL | Maximum node sleep interval of when in active mode. | 200 ms | <= 1 hour|

### Usage

The default values for the these defines are located in `src/include/platform/CHIPDeviceConfig.h`

```c++
/**
 * CHIP_DEVICE_CONFIG_SED_IDLE_INTERVAL
 *
 * The default amount of time in milliseconds that the sleepy end device will use as an idle interval.
 * This interval is used by the device to periodically wake up and poll the data in the idle mode.
 */
#ifndef CHIP_DEVICE_CONFIG_SED_IDLE_INTERVAL
#define CHIP_DEVICE_CONFIG_SED_IDLE_INTERVAL 5000_ms32
#endif

/**
 * CHIP_DEVICE_CONFIG_SED_ACTIVE_INTERVAL
 *
 * The default amount of time in milliseconds that the sleepy end device will use as an active interval.
 * This interval is used by the device to periodically wake up and poll the data in the active mode.
 */
#ifndef CHIP_DEVICE_CONFIG_SED_ACTIVE_INTERVAL
#define CHIP_DEVICE_CONFIG_SED_ACTIVE_INTERVAL 200_ms32
#endif
```
To change these default values, add `#define CHIP_DEVICE_CONFIG_SED_ACTIVE_INTERVAL <value>_ms32` to `src/platform/EFR32/CHIPDevicePlatformConfig.h`.

## Building 

### Enabling Sleepy Functionalities

To build an OpenThread SED example, two conditions must be met: 1) The following macro must be defined : `CHIP_DEVICE_CONFIG_ENABLE_SED` and 2) the example must to use the MTD OpenThread libraries to be able to leverage OpenThread Sleepy functionalities.

The `--sed` macro can be added to the build command to enable sleepy functionalities. Here is an example to build the light-switch-app as a SED for the EFR32MG24 BRD4186C.

```bash
./scripts/examples/gn_efr32_example.sh ./examples/light-switch-app/efr32/ ./out/light-switch-app_SED BRD4186C --sed
```

### Minimal Power Consumption

Simply enabling Sleepy functionalities does not give the application the best power consumption. Be default several features, like the LCD, are enabled in example applications that increase the power consumption. The following set of features increase power consumption.

- Matter Shell
- OpenThread CLI
- LCD and Qr Code

To achieve the most power-efficient build, add these build arguments to the build command to disable all power-consuming features.

```bash
./scripts/examples/gn_efr32_example.sh ./examples/light-switch-app/efr32/ ./out/light-switch-app_SED BRD4186C --sed chip_build_libshell=false enable_openthread_cli=false show_qr_code=false disable_lcd=true
```