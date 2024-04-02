# Configuring Intermittently Connected Devices on TI CC13x4 Platforms

## Overview

Intermittently Connected Devices are devices in a network that do not always need to be active. Matter has defined a cluster that helps capture this behavior; this configuration is ideal for devices that need to operate with low power consumption or do not have a need to always be on the network. Matter examples on the TI CC13x4 platform can be configured to act as ICDs.

## Platform Code Changes
To configure a TI example as an ICD, open up the `args.gni` file of the example and set the following parameter to true:

```
chip_enable_icd_server = true
```

TI examples have only been tested with the ICD Server configuration. To enable the client configuration, set `chip_enable_icd_client` to true.

Persistent subscriptions allow devices to attempt resuming existing subscriptions following a device reset. To enable persistent subscriptions, set the following parameter to true:

```
chip_persist_subscriptions = true
```

Subscription timeout resumption allows devices to attempt re-establishing subscriptions that may have expired. This feature is disabled out of box. 

In addition, various ICD parameters such as idle/active mode duration, active mode threshold, and polling intervals can be configured in `src/platform/cc13xx_26xx/cc13x4_26x4/CHIPPlatformConfig.h`

```
#define CHIP_CONFIG_ICD_ACTIVE_MODE_DURATION_MS 1000
#define CHIP_CONFIG_ICD_ACTIVE_MODE_THRESHOLD_MS 500
#define CHIP_CONFIG_ICD_IDLE_MODE_DURATION_SEC 300
#define CHIP_DEVICE_CONFIG_ICD_SLOW_POLL_INTERVAL chip::System::Clock::Milliseconds32(5000)
#define CHIP_DEVICE_CONFIG_ICD_FAST_POLL_INTERVAL chip::System::Clock::Milliseconds32(100)
```

## ZAP File Changes

Open up the ZAP file (in `examples/<example-name>/<example-name>-common`) for the example being configured as an ICD. Add the ICD Management Cluster for Endpoint 0. 

Open up the .matter file (in `examples/<example-name>/<example-name>-common`) corresponding to the example and add in the ICDManagement cluster.

In addition, each endpoint has a list of clusters that it supports. Add the ICDManagement cluster to this list. 

The lock-app example's .matter file can be used as a reference. These additions allow the ICDManagement cluster's callbacks to be accessed. 

