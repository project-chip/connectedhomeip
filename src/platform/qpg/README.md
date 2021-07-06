# Overview of CHIP Qorvo QPG Adaption

The following is an overview of the Qorvo adaptation of CHIP. Most of this code
will have parallels in any new adaptation.

(All file names are relative to `connectedhomeip/src/platform/qpg...`).

`PlatformManagerImpl.h`<br>`PlatformManagerImpl.cpp`

-   Concrete implementation of PlatformManager interface
-   Provides initialization of the CHIP stack and core event loop for the chip
    task
-   Relies on GenericPlatformManagerImpl_FreeRTOS<> class to provide most of the
    implementation

`ConfigurationManagerImpl.h`<br>`ConfigurationManagerImpl.cpp`

-   Concrete implementation of ConfigurationManager interface
-   Manages storage and retrieval of persistent configuration data
-   Relies on GenericConfigurationManagerImpl<> classes to implement most API
    functionality
-   Delegates low-level reading and writing of persistent values to NRF5Config
    class

`ConnectivityManagerImpl.h`<br>`ConnectivityManagerImpl.cpp`

-   Concrete implementation of ConnectivityManager interface
-   Provides high-level APIs for managing device connectivity
-   Relies on GenericConnectivityManagerImpl_Thread<> class to provide most of
    the implementation

`ThreadStackManagerImpl.h`<br>`ThreadStackManagerImpl.cpp`

-   Concrete implementation of ThreadStackManager interface
-   Supports Thread stack initialization and core event loop processing
-   Relies on GenericThreadStackManagerImpl_OpenThread/FreeRTOS/LwIP<> classes
    to implement most API functionaltiy

`BLEManagerImpl.h`<br>`BLEManagerImpl.cpp`

-   Concrete implementation of the BLEManager interface
-   Maps CHIP's BLE interface abstractions (BleLayer, BlePlatformDelegate,
    BleApplicationDelegate) onto the platform's native BLE services
-   Implements CHIP-compatible BLE advertising and GATT service using the
    Softdevice BLE stack

`Entropy.cpp`

-   Implements interface to platform entropy source

`Logging.cpp`

-   Adaption of chip debug logging to platform logging facility
