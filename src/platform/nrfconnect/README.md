# Overview of CHIP nRF Connect Adaption

The following is a quick overview of the nRF Connect adaptation of CHIP. Most of
this code will have parallels in any new adaptation.

(All file names are relative to `connectedhomeip/src/...`).

`include/platform/nrfconnect/PlatformManagerImpl.h`

-   Uses Zephyr platform implementation of PlatformManager interface.

`include/platform/nrfconnect/ConfigurationManagerImpl.h`

-   Uses Zephyr platform implementation of ConfigurationManager interface.

`include/platform/nrfconnect/ConnectivityManagerImpl.h`<br>`nrfconnect/ConnectivityManagerImpl.cpp`

-   Concrete implementation of ConnectivityManager interface
-   Provides high-level APIs for managing device connectivity
-   Relies on various generic implementation classes to provide API
    functionality
-   Very much a work-in-progress in the nrfconnect branch

`include/platform/nrfconnect/DeviceNetworkProvisioningDelegateImpl.h`<br>`nrfconnect/DeviceNetworkProvisioningDelegateImpl.cpp`

-   Concrete implementation of DeviceNetworkProvisioningDelegate interface
-   Provides high-level APIs for managing network provisioning

`include/platform/nrfconnect/ThreadStackManagerImpl.h`

-   Uses Zephyr platform implementation of ThreadStackManager interface.

`include/platform/nrfconnect/BLEManagerImpl.h`

-   Uses Zephyr platform implementation of BLEManager interface.

`include/platform/nrfconnect/CHIPDevicePlatformEvent.h`

-   Uses Zephyr platform definitions of platform-specific event types and data
    for the chip Device Layer.
