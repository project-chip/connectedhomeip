# Overview of CHIP Linux Adaption

The following is a quick overview of the Linux adaptation of CHIP. Most of this
code will have parallels in any new adaptation.

(All file names are relative to `connectedhomeip/src/...`).

`include/platform/Linux/PlatformManagerImpl.h`<br>`Linux/PlatformManagerImpl.cpp`

-   Concrete implementation of PlatformManager interface
-   Provides initialization of the CHIP stack and core event loop for the chip
    task
-   Relies on GenericPlatformManagerImpl_POSIX<> class to provide most of the
    implementation

`include/platform/Linux/ConfigurationManagerImpl.h`<br>`Linux/ConfigurationManagerImpl.cpp`

-   Concrete implementation of ConfigurationManager interface
-   Manages storage and retrieval of persistent configuration data
-   Relies on GenericConfigurationManagerImpl<> classes to implement most API
    functionality
-   Delegates low-level reading and writing of persistent values to PosixConfig
    class

`include/platform/Linux/ConnectivityManagerImpl.h`<br>`Linux/ConnectivityManagerImpl.cpp`

-   Concrete implementation of ConnectivityManager interface
-   Provides high-level APIs for managing device connectivity
-   Relies on various generic implementation classes to provide API
    functionality
-   Very much a work-in-progress in the Linux branch

`include/platform/Linux/ThreadStackManagerImpl.h`<br>`Linux/ThreadStackManagerImpl.cpp`

-   Concrete implementation of ThreadStackManager interface
-   Supports Thread stack initialization and core event loop processing
-   Relies on GenericThreadStackManagerImpl_OpenThread/POSIX<> classes to
    implement most API functionality

`include/platform/Linux/BLEManagerImpl.h`<br>`Linux/BLEManagerImpl.cpp`

-   Concrete implementation of the BLEManager interface
-   Maps CHIP's BLE interface abstractions (BleLayer, BlePlatformDelegate,
    BleApplicationDelegate) onto the platform's native BLE services
-   Implements chip-compatible BLE advertising.

`platform/Linux/Entropy.cpp`

-   Implements interface to platform entropy source

`platform/Linux/Logging.cpp`

-   Adaption of chip debug logging to platform logging facility.

`platform/Linux/PosixConfig.cpp`

-   Implements low-level read/write of persistent configuration values
-   Class API specifically designed to work in conjunction with the
    GenericConfigurationManagerImpl<> class.

`platform/Linux/StorageBackend.cpp`

-   Provides services to PosixConfig
-   Controls datastore used to store KV pairs (filesystem, database, etc.)
-   Controls locations within a particular data store (for example file system
    paths)
-   Controls the data format (.ini, .json, etc.)
