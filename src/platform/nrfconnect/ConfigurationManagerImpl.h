/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *          Provides an implementation of the ConfigurationManager object
 *          for nRF Connect SDK platforms, by extending the Zephyr platform
 *          implementation.
 */

#pragma once

#include <platform/Zephyr/ConfigurationManagerImpl.h>

#include <system/SystemClock.h>

namespace chip {
namespace DeviceLayer {

struct ChipDeviceEvent;

/**
 * Concrete implementation of the ConfigurationManager singleton object for the nRF Connect SDK
 * platform.
 *
 * It reuses the Zephyr implementation and overrides only the factory reset handling, which on
 * nRF devices additionally routes the reset through the Matter server, suspends MPSL to speed up
 * the flash operations and optionally reports how long the reset took.
 */
class ConfigurationManagerImplNrf : public ConfigurationManagerImpl
{
public:
    static ConfigurationManagerImplNrf & GetDefaultInstance();

protected:
    ConfigurationManagerImplNrf() = default;

private:
    // ===== Members that implement the ConfigurationManager public interface.

    CHIP_ERROR Init() override;
    void InitiateFactoryReset() override;

    // NOTE: All the remaining methods are implemented by ConfigurationManagerImpl<>.

    // ===== Private members reserved for use by this class only.

    static void FactoryResetEventHandler(const ChipDeviceEvent * event, intptr_t arg);
    static void DoFactoryReset(intptr_t arg);

#ifdef CONFIG_CHIP_FACTORY_RESET_TIME_MEASUREMENT
    System::Clock::Milliseconds64 GetFactoryResetDuration() const;

    System::Clock::Timestamp mFactoryResetStartTime;
#endif // CONFIG_CHIP_FACTORY_RESET_TIME_MEASUREMENT

    // Tells whether the Matter server has already deleted all the fabrics and emitted the Leave
    // event for this reset. Until it does, InitiateFactoryReset() delegates the work to the server
    // so that every factory reset source behaves in the same way.
    bool mFactoryResetScheduled = false;
};

} // namespace DeviceLayer
} // namespace chip
