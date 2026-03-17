# Overview of CHIP Realtek Zephyr Adaption

The following is an overview of the Realtek adaptation of CHIP. Most of this
code will have parallels in any new adaptation.

(All file names are relative to
`connectedhomeip/src/platform/realtek/zephyr...`).

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
