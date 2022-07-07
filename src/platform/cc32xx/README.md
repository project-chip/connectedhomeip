# Overview of CHIP CC32XX Adaption

The following is an overview of the CC32XX adaptation of CHIP. Most of this code
will have parallels in any new adaptation.

(All file names are relative to `connectedhomeip/src/platform/CC32XX...`).

`BlePlatformConfig.h`

-   Configuration header for BLE specific configurations
-   Required by AutoConf build

**NOTE** empty for now

`CC32XXConfig.h` <br> `CC32XXConfig.cpp`

-   Concrete implementation of the Non-Volatile storage of information for chip

**NOTE** empty for now

`CHIPDevicePlatformConfig.h`

-   Configuration for the chip stack for the CC32XX platform

`CHIPDevicePlatformEvent.h`

-   Definition of platform events to be handled by the chip processing loop
-   Currently there are no events that need special handling

`CHIPPlatformConfig.h` <br> `InetPlatformConfig.h` <br> `SystemPlatformConfig.h`

-   Definitions for the chip stack to work with the configurations of the CC32XX
    platform

`ConfigurationManagerImpl.h` <br> `ConfigurationManagerImpl.cpp`

-   Concrete implementation of ConfigurationManager interface
-   Manages storage and retrieval of persistent configuration data
-   Relies on GenericConfigurationManagerImpl<> classes to implement most API
    functionality
-   Delegates low-level reading and writing of persistent values to
    `CC32XXConfig` class

`ConnectivityManagerImpl.h`<br>`ConnectivityManagerImpl.cpp`

-   Concrete implementation of `ConnectivityManager` interface
-   Provides high-level APIs for managing device connectivity
-   Relies on `GenericConnectivityManagerImpl_Thread<>` class to provide most of
    the implementation

`Entropy.cpp`

-   Concrete implementation of an entropy source based on the TRNG

**NOTE** empty for now

`Logging.cpp`

-   Concrete implementation of the logging functions
-   Currently logs out the User UART interface on the XDS110

`ThreadStackManagerImpl.h` <br> `ThreadStackManagerImpl.cpp`

-   Concrete implementation of ThreadStackManager interface
-   Supports Thread stack initialization and core event loop processing
-   Relies on `GenericThreadStackManagerImpl_OpenThread/FreeRTOS/LwIP<>` classes
    to implement most API functionality

**NOTE** Currently disabled because OpenThread Libraries are not built or linked
in.

`PlatformManagerImpl.h`<br>`PlatformManagerImpl.cpp`

-   Concrete implementation of `PlatformManager` interface
-   Provides initialization of the CHIP stack and core event loop for the chip
    task
-   Relies on `GenericPlatformManagerImpl_FreeRTOS<>` class to provide most of
    the implementation

`SoftwareUpdateManagerImpl.cpp`

-   Concrete implementation of the Software Update Manager
-   Relies on the `GenericSoftwareUpdateManagerImpl`
