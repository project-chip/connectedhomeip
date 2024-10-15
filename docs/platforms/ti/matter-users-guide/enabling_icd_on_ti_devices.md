# Configuring Intermittently Connected Devices on TI SimpleLink CC13x4_CC26x4 Platforms

## Overview

Intermittently Connected Devices are devices in a network that do not always
need to be active. Matter has defined a cluster that helps capture this
behavior; this configuration is ideal for devices that need to operate with low
power consumption or do not have a need to always be on the network. Matter
examples on the TI CC13x4_CC26x4 platform can be configured to act as ICDs.

## Platform Code Changes

To configure a TI example as an ICD, open up the `args.gni` file of the example
and set the following parameter to true:

```
chip_enable_icd_server = true
```

To enable LIT ICD behavior, Check In Protocol Support and User Active Mode
Trigger Support, set the following parameter to true:

```
chip_enable_icd_lit = true
```

Persistent subscriptions allow devices to attempt resuming existing
subscriptions following a device reset. To enable persistent subscriptions, set
the following parameter to true:

```
chip_persist_subscriptions = true
```

Subscription timeout resumption allows devices to attempt re-establishing
subscriptions that may have expired. This feature is disabled out of box.

In addition, various ICD parameters such as idle/active mode duration, active
mode threshold, and polling intervals can be configured in
`src/platform/cc13xx_26xx/cc13x4_26x4/CHIPPlatformConfig.h`

```
#define CHIP_CONFIG_ICD_ACTIVE_MODE_DURATION_MS 1000
#define CHIP_CONFIG_ICD_ACTIVE_MODE_THRESHOLD_MS 500
#define CHIP_CONFIG_ICD_IDLE_MODE_DURATION_SEC 300
#define CHIP_DEVICE_CONFIG_ICD_SLOW_POLL_INTERVAL chip::System::Clock::Milliseconds32(5000)
#define CHIP_DEVICE_CONFIG_ICD_FAST_POLL_INTERVAL chip::System::Clock::Milliseconds32(100)
```

To enable LIT ICD behavior, set the polling period to be greater than 15
seconds, and the active mode threshold to at least 5000 milliseconds.

## ZAP File Changes

Open up the ZAP file (in `examples/<example-name>/<example-name>-common`) for
the example being configured as an ICD. Add the ICD Management Cluster for
Endpoint 0 as either a Server or Client, depending on your configuration.

To enable LIT ICD behavior, set the FeatureMap to 0x0007 to enable Check-In
Protocol Support, User Active Mode Trigger Support, and Long Idle Time Support.
In addition, enable the UserActiveModeTriggerHint,
UserActiveModeTriggerInstruction, and MaximumCheckInBackOff attributes.

After making the desired changes in the zap file, generate the .matter file by
running the following commands:

```
$ cd /connectedhomeip/scripts/tools/zap
$ ./generate.py examples/<example>/<example>-common/<example>-app.zap

```
