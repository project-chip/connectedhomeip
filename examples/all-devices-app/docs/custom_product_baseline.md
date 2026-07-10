# Custom Product Baseline Guide

A production Matter device requires a fixed application architecture rather than
the CLI-configured simulator approach.

This guide explains adapting `all-devices-app` patterns to build a standalone
product application.

---

## 1. Architectural Differences

When transitioning to a product application, replace dynamic configurations with
static definitions:

| Component / Layer        | `all-devices-app`                                                      | Custom Application                                                              |
| :----------------------- | :--------------------------------------------------------------------- | :------------------------------------------------------------------------------ |
| **Data Model Structure** | Dynamic, parsed entirely from CLI `--device` arguments at boot.        | **Static / Fixed**, defined explicitly in your application initialization code. |
| **Device Registry**      | Uses `DeviceFactory` singleton to map string names to device creators. | **Removed**. Devices are instantiated directly as members.                      |
| **Build Source List**    | `enabled_devices.cmake`/`.gni` compiling all device types.             | Only compiles the C++ device classes required by the product.                   |

---

## 2. Recommended Project Structure

For a standalone firmware product (e.g., `examples/my-custom-bulb-app/`), mirror
the decoupled platform structure:

```text
my-custom-bulb-app/
├── BUILD.gn                     # Root build configuration for your app
├── CMakeLists.txt               # ESP32 / ESP-IDF CMake build setup
├── include/
│   └── CHIPProjectAppConfig.h   # Feature configurations and descriptor Overrides
├── common/
│   ├── MyBulbDeviceManager.h    # Encapsulated ownership of your fixed device objects
│   └── MyBulbDeviceManager.cpp
└── posix/                       # OS or Hardware-specific entrypoints
    └── main.cpp
```

---

## 3. Extracting the Product Baseline Code

Rather than using `DeviceFactory`, implement a `DeviceManager` to own Matter
endpoints and Interaction Model clusters.

### The Device Manager Header (`MyBulbDeviceManager.h`)

Instantiate device classes (such as `LoggingDimmableLight`) directly.

```cpp
#pragma once

#include <app/persistence/DefaultAttributePersistenceProvider.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/types/dimmable-light/LoggingDimmableLight.h>
#include <lib/core/CHIPError.h>
#include <platform/DefaultTimerDelegate.h>

namespace chip::app {

class MyBulbDeviceManager
{
public:
    MyBulbDeviceManager() = default;
    ~MyBulbDeviceManager() = default;

    static MyBulbDeviceManager & Instance()
    {
        static MyBulbDeviceManager sInstance;
        return sInstance;
    }

    // Explicit runtime initialization hook called during application boot
    CHIP_ERROR Init(PersistentStorageDelegate & storageDelegate, Credentials::GroupDataProvider & groupDataProvider, FabricTable & fabricTable);

    // Exact tear down contract
    void Shutdown();

    CodeDrivenDataModelProvider & DataModelProvider() { return *mDataModelProvider; }

private:
    DefaultAttributePersistenceProvider mAttributePersistence;
    std::unique_ptr<CodeDrivenDataModelProvider> mDataModelProvider;
    std::unique_ptr<DeviceInterface> mMainLightEndpoint;
    DeviceLayer::DefaultTimerDelegate mTimerDelegate;
};

} // namespace chip::app
```

### The Device Manager Source (`MyBulbDeviceManager.cpp`)

Instantiate endpoints and bind them to hardcoded `EndpointId` values.

```cpp
#include "MyBulbDeviceManager.h"
#include <lib/support/CodeUtils.h>

namespace chip::app {

CHIP_ERROR MyBulbDeviceManager::Init(PersistentStorageDelegate & storageDelegate, Credentials::GroupDataProvider & groupDataProvider, FabricTable & fabricTable)
{
    // 1. Initialize attribute persistence and instantiate our CodeDriven Data Model Provider
    ReturnErrorOnFailure(mAttributePersistence.Init(&storageDelegate));
    mDataModelProvider = std::make_unique<CodeDrivenDataModelProvider>(storageDelegate, mAttributePersistence);

    // 2. Instantiate the precise C++ functional device object for our Smart Bulb
    mMainLightEndpoint = std::make_unique<LoggingDimmableLight>(LoggingDimmableLight::Context{
        .groupDataProvider = groupDataProvider,
        .fabricTable       = fabricTable,
        .timerDelegate     = mTimerDelegate,
    });

    // 3. Register the endpoint and its encapsulated server clusters into our Data Model Provider
    ReturnErrorOnFailure(mMainLightEndpoint->Register(EndpointId(1), *mDataModelProvider));

    return CHIP_NO_ERROR;
}

void MyBulbDeviceManager::Shutdown()
{
    if (mMainLightEndpoint)
    {
        VerifyOrDie(mDataModelProvider != nullptr);
        mMainLightEndpoint->Unregister(*mDataModelProvider);
        mMainLightEndpoint.reset();
    }
    mDataModelProvider.reset();
}

} // namespace chip::app
```

---

## 4. Hooking into Application Entry Points

In the platform entrypoint (`main.cpp` or `AppTask::Init()`), initialize the
static configuration after the Matter stack initializes:

```cpp
#include "MyBulbDeviceManager.h"
#include <platform/PlatformManager.h>

void ApplicationResumedHook(PersistentStorageDelegate & storageDelegate)
{
    // Fetch SDK core dependencies
    auto & groupDataProvider = *Credentials::GetGroupDataProvider();
    auto & fabricTable       = Server::GetInstance().GetFabricTable();

    // Boot our fixed production device topology
    VerifyOrDie(chip::app::MyBulbDeviceManager::Instance().Init(storageDelegate, groupDataProvider, fabricTable) == CHIP_NO_ERROR);
}

void ApplicationShutdownHook()
{
    // Safely unregister endpoints before the underlying stack halts
    chip::app::MyBulbDeviceManager::Instance().Shutdown();
}
```

---

## 5. Commercial Firmware Guidelines

1. **Link Specific Devices**: In `BUILD.gn` or `CMakeLists.txt`, link directly
   against required device modules (e.g.,
   `all-devices-common/device/types/dimmable-light`) rather than
   `device-factory` to minimize RAM and Flash usage.
2. **Statically Define Topologies**: Configure fixed `EndpointId` parameters
   instead of using auto-incrementing IDs to ensure a fixed data model.
3. **Persist Hardware State**: Replace simulated cluster implementations with
   hardware drivers (e.g., binding a PWM driver to WriteAttribute callbacks) and
   persist calibration data via storage delegates.
