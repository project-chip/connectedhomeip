# Overview of Zephyr Adaption

The following is a quick overview of the Zephyr adaptation of CHIP. Most of this
code will have parallels in any new adaptation.

(All file names are relative to `connectedhomeip/src/...`).

`include/platform/Zephyr/PlatformManagerImpl.h`<br>`Zephyr/PlatformManagerImpl.cpp`

-   Concrete implementation of PlatformManager interface
-   Provides initialization of the CHIP stack and core event loop for the chip
    task
-   Relies on GenericPlatformManagerImpl_POSIX<> class to provide most of the
    implementation

`include/platform/Zephyr/ConfigurationManagerImpl.h`<br>`Zephyr/ConfigurationManagerImpl.cpp`

-   Concrete implementation of ConfigurationManager interface
-   Manages storage and retrieval of persistent configuration data
-   Relies on GenericConfigurationManagerImpl<> classes to implement most API
    functionality
-   Delegates low-level reading and writing of persistent values to ZephyrConfig
    class

`include/platform/Zephyr/ThreadStackManagerImpl.h`<br>`Zephyr/ThreadStackManagerImpl.cpp`

-   Concrete implementation of ThreadStackManager interface
-   Supports Thread stack initialization and core event loop processing
-   Relies on GenericThreadStackManagerImpl_OpenThread/POSIX<> classes to
    implement most API functionality

`include/platform/Zephyr/BLEManagerImpl.h`<br>`Zephyr/BLEManagerImpl.cpp`

-   Concrete implementation of the BLEManager interface
-   Maps CHIP's BLE interface abstractions (BleLayer, BlePlatformDelegate,
    BleApplicationDelegate) onto the platform's native BLE services
-   Implements chip-compatible BLE advertising.

`platform/Zephyr/Logging.cpp`

-   Adaption of chip debug logging to platform logging facility.

`platform/Zephyr/ZephyrConfig.cpp`

-   Implements low-level read/write of persistent configuration values
-   Class API specifically designed to work in conjunction with the
    GenericConfigurationManagerImpl<> class.

`platform/Zephyr/CHIPDevicePlatformEvent.h`

-   Defines platform-specific event types and data for the chip Device Layer.
