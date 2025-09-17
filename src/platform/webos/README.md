# Overview of CHIP webOS Adaption

The following is a quick overview of the webOS adaptation of CHIP. Most of this
code will have parallels in any new adaptation.

(All file names are relative to `connectedhomeip/src/...`).

`include/platform/webos/PlatformManagerImpl.h`<br>`webos/PlatformManagerImpl.cpp`

-   Concrete implementation of PlatformManager interface
-   Provides initialization of the CHIP stack and core event loop for the chip
    task
-   Relies on GenericPlatformManagerImplPOSIX<> class to provide most of the
    implementation

`include/platform/webos/ConfigurationManagerImpl.h`<br>`webos/ConfigurationManagerImpl.cpp`

-   Concrete implementation of ConfigurationManager interface
-   Manages storage and retrieval of persistent configuration data
-   Relies on GenericConfigurationManagerImpl<> classes to implement most API
    functionality
-   Delegates low-level reading and writing of persistent values to PosixConfig
    class

`include/platform/webos/ConnectivityManagerImpl.h`<br>`webos/ConnectivityManagerImpl.cpp`

-   Concrete implementation of ConnectivityManager interface
-   Provides high-level APIs for managing device connectivity
-   Relies on various generic implementation classes to provide API
    functionality
-   Very much a work-in-progress in the Linux branch

`include/platform/webos/ThreadStackManagerImpl.h`<br>`webos/ThreadStackManagerImpl.cpp`

-   Concrete implementation of ThreadStackManager interface
-   Supports Thread stack initialization and core event loop processing
-   Relies on GenericThreadStackManagerImpl_OpenThread/POSIX<> classes to
    implement most API functionality

`include/platform/webos/BLEManagerImpl.h`<br>`webos/BLEManagerImpl.cpp`

-   Concrete implementation of the BLEManager interface
-   Maps CHIP's BLE interface abstractions (BleLayer, BlePlatformDelegate,
    BleApplicationDelegate) onto the platform's native BLE services
-   Implements chip-compatible BLE advertising.

`platform/webos/Entropy.cpp`

-   Implements interface to platform entropy source

`platform/webos/Logging.cpp`

-   Adaption of chip debug logging to platform logging facility.

`platform/webos/PosixConfig.cpp`

-   Implements low-level read/write of persistent configuration values
-   Class API specifically designed to work in conjunction with the
    GenericConfigurationManagerImpl<> class.
