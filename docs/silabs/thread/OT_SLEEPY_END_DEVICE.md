# Matter Intermittently Connected Devices over OpenThread

This page explains how Matter OpenThread Intermittently Connected Devices (ICDs) work and how to configure an ICD example.

## Overview

Matter provides an Intermittently Connected Device (ICD) operating mode to extend the battery life of power-limited devices.
The Matter ICD manager leverages subscription report synchronization and OpenThread functionalities to allow devices to sleep for set periods
without disrupting their Matter sessions.

In Matter v1.2, only ICD's with Short Idle Time (SIT) are supported. Matter 1.3 alpha provides provisional support for Long Idle Time (LIT) ICD's. A SIT ICD  are devices that SHOULD be configured with a Slow Polling Interval shorter than or equal to 15 seconds. For example, in a typical scenario for door locks and window coverings, commands need to be sent to the ICD with a use-case imposed latency requirement. Typically, devices that are Short Idle Time ICDs are not initiators in the communication flow.

## Operating Modes

ICDs have two operating modes, Idle and Active. An ICD alternates normally between the Idle mode and Active mode  based on the  `IdleModeInterval` and `ActiveModeInterval` respectively

When the device is in _Active Mode_, the ICD is set into a fast-polling interval for maximum responsiveness.
The `CHIP_DEVICE_CONFIG_ICD_FAST_POLL_INTERVAL` parameter communicates the maximum sleep interval of a node in active mode.

Any of the following device states will start or keep the ICD in _Active Mode_:
  - A commissioning window is open 
  - An exchange context is awaiting a response or ack
  - The fail-safe is armed.
  
Any of the following events can trigger the start of the _Active Mode_ interval or extend it by one `ActiveModeThreshold`:
  - A Message needs to be Sent.
  - A Message was received.
  - An implemented User action occurred.

Once the active mode is triggered, the ICD stays in this mode for a minimum duration of `ActiveModeInterval`
When the active interval has elapsed and none of the aforementioned states are active, the device will switch its operating mode to the Idle Mode.

In _Idle mode_, the ICD will poll its associated router at its slow-polling interval to see if another device has tried to communicate with it while it was sleeping.
If no event occurs, the ICD stays in its idle mode for the entirety of the `IdleModeInterval`.
The `CHIP_DEVICE_CONFIG_ICD_SLOW_POLL_INTERVAL` parameter communicates the slow-polling interval and therefore the maximum sleep interval of the node in idle mode. This parameter affects both the minimum power consumption and maximum latency.

## Thread Communication

In order to receive messages that were sent while the ICD was sleeping, the ICD relies on its associated Thread router which buffers any incoming messages.
The Thread router will send all buffered messages to the ICD when it polls the router at the end of its slow-polling interval.

## Configuration

Matter exposes some defines to configure the polling intervals of the openthread stack in both Idle and Active modes. 

| Parameter Name | Define | Description | Default Value | Maximum allowed Value |
| - | - | - | - | - |
| SlowPollInterval | CHIP_DEVICE_CONFIG_ICD_SLOW_POLL_INTERVAL | Interval, in milliseconds, at which the thread radio will poll its network in idle mode. | 15000 ms | <= IdleModeInterval |
| FastPollInterval | CHIP_DEVICE_CONFIG_ICD_FAST_POLL_INTERVAL | Interval, in milliseconds, at which the thread radio will poll its network in active mode. | 200 ms | < ActiveModeInterval |

For Matter configuration, see the [Matter ICD](../general/MATTER_ICD.md) documentation.

### Usage

The default values for these defines for the Silabs platform is split in two files:

and `src/platform/silabs/CHIPDevicePlatformConfig.h` is where the openthread polling intervals default are set.

```c++
#ifndef CHIP_DEVICE_CONFIG_ICD_SLOW_POLL_INTERVAL
#define CHIP_DEVICE_CONFIG_ICD_SLOW_POLL_INTERVAL chip::System::Clock::Milliseconds32(SL_OT_IDLE_INTERVAL)
#endif // CHIP_DEVICE_CONFIG_SED_IDLE_INTERVAL

#ifndef CHIP_DEVICE_CONFIG_ICD_FAST_POLL_INTERVAL
#define CHIP_DEVICE_CONFIG_ICD_FAST_POLL_INTERVAL chip::System::Clock::Milliseconds32(SL_OT_ACTIVE_INTERVAL)
#endif // CHIP_DEVICE_CONFIG_SED_ACTIVE_INTERVAL
```


The default values for the defines previously shown are located `third_party/silabs/efr32_sdk.gni`.
```bash
  # ICD Openthread Configuration flags
  sl_ot_idle_interval_ms = 15000  # 15s Idle Intervals
  sl_ot_active_interval_ms = 200  # 200ms Active Intervals
```

In some cases, as with the light-switch-app, these default values are overridden as in: `examples/light-switch-app/silabs/openthread.gni`
```bash
  # Openthread Configuration flags
  sl_ot_idle_interval_ms = 30000  # 30s Idle Intervals
  sl_ot_active_interval_ms = 500  # 500ms Active Intervals
```

There are two methods with which you can change these defaults values.

The first method is by adding these defines to `CHIPProjectConfig.h` file of your project.
For the lighting-app, the file is `examples/lighting-app/silabs/include/CHIPProjectConfig.h`.

```c++
#define CHIP_DEVICE_CONFIG_ICD_FAST_POLL_INTERVAL chip::System::Clock::Milliseconds32(<value_ms>)
#define CHIP_DEVICE_CONFIG_ICD_SLOW_POLL_INTERVAL chip::System::Clock::Milliseconds32(<value_ms>)
```

The second method is by adding arguments to the build command. Here is an example building the EFR32MG24 BRD4186C with different values.
```bash
./scripts/examples/gn_silabs_example.sh examples/lighting-app/silabs out/lighting-app BRD41686C sl_ot_idle_interval_ms=10000 sl_ot_active_interval_ms=200 sl_idle_mode_interval_ms = 300000 sl_active_mode_interval_ms=5000 sl_active_mode_threshold_ms=1000 --icd
```

If both methods of changing the default values are used, the defines added to the `CHIPProjectConfig.h` take precedence on the build arguments.

## Building 

### Enabling Sleepy Functionalities

To build an OpenThread ICD example, three conditions must be met: 
    1) The Intermittently Connected Device cluster must be enabled as a server on the end device application. 
    2) The following macro must be defined : `CHIP_CONFIG_ENABLE_ICD_SERVER`.
       *This can be set with gn build argument `chip_enable_icd_server=true`* 
    3) The example must use the MTD OpenThread libraries to be able to leverage OpenThread Sleepy functionalities.

Adding the preset `--icd` to your build command will ensure that conditions 2 and 3 are met to enable icd functionalities. This of course only works on sample apps that respect the first condition. The Lock and Light-switch sample applications are configured with the ICD cluster enabled.

Here is an example to build the light-switch-app as an ICD for the EFR32MG24 BRD4186C.

```bash
./scripts/examples/gn_silabs_example.sh ./examples/light-switch-app/silabs ./out/light-switch-app_SED BRD4186C --icd
```

### Minimal Power Consumption

Simply enabling ICD functionalities does not give the application the best power consumption.
By default, several features that increase power consumption are enabled in the example applications.
The following set of features increase power consumption.

- Matter Shell
- OpenThread CLI
- LCD and Qr Code

To achieve the most power-efficient build, add the following build arguments to the build command to disable all power-consuming features.

```bash
./scripts/examples/gn_silabs_example.sh ./examples/light-switch-app/silabs ./out/light-switch-app_ICD BRD4186C --icd --low-power
```