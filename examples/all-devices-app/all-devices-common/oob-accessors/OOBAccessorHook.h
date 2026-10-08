/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#pragma once

#include <algorithm>
#include <type_traits>
#include <utility>
#include <vector>

#include <device/api/Interface.h>
#include <device/types/ambient-context-sensor/OOBAccessors.h>
#include <device/types/boolean-state-sensor/OOBAccessors.h>
#include <device/types/dimmable-light/OOBAccessors.h>
#include <device/types/dimmable-plug-in-unit/OOBAccessors.h>
#include <device/types/electrical-sensor/OOBAccessors.h>
#include <device/types/mode-select/OOBAccessors.h>
#include <device/types/mounted-dimmable-load-control/OOBAccessors.h>
#include <device/types/mounted-on-off-control/OOBAccessors.h>
#include <device/types/occupancy-sensor/OOBAccessors.h>
#include <device/types/on-off-light/OOBAccessors.h>
#include <device/types/on-off-plug-in-unit/OOBAccessors.h>
#include <device/types/robotic-vacuum-cleaner/OOBAccessors.h>
#include <device/types/root-node/OOBAccessors.h>
#include <lib/support/CodeUtils.h>
#include <oob-accessors/OOBAccessorRegistry.h>

namespace chip::app {

namespace detail {

template <typename T, typename = void>
struct HasOOBAccessors : std::false_type
{
};

template <typename T>
struct HasOOBAccessors<T, std::void_t<decltype(RegisterOOBAccessors(std::declval<T &>(), std::declval<OOBAccessorRegistry &>()))>>
    : std::true_type
{
};

} // namespace detail

/// Static hook for DeviceFactory that registers Out-of-Band (OOB) accessors for devices supporting OOB actions.
class OOBAccessorHook
{
public:
    template <typename TDevice>
    static void OnDeviceRegistered(TDevice & device)
    {
        if constexpr (detail::HasOOBAccessors<TDevice>::value)
        {
            // RAII listener that will register itself as litsener to the OOBAccessorRegistry and will add any registered accessors to the deviceToOOBAccessorMap.
            // At the end of the scope, it will unregister itself from the OOBAccessorRegistry.
            OOBAccessorRegistrationListener listener(device, GetDeviceToOOBAccessorMap());
            RegisterOOBAccessors(device, OOBAccessorRegistry::Instance());
        }
    }

    template <typename TDevice>
    static void BeforeDeviceUnregistration(TDevice & device)
    {
        // check if the device has OOB accessors that could be registered
        if constexpr (detail::HasOOBAccessors<TDevice>::value)
        {
            auto & deviceToOOBAccessorMap = GetDeviceToOOBAccessorMap();

            // Unregister all OOB accessors associated with the device being unregistered
            for (auto [devicePtr, accessorPtr] : deviceToOOBAccessorMap)
            {
                if (devicePtr == &device && accessorPtr != nullptr)
                {
                    LogErrorOnFailure(OOBAccessorRegistry::Instance().Unregister(*accessorPtr));
                }
            }

            // Remove corresponding entries from the map
            deviceToOOBAccessorMap.erase(std::remove_if(deviceToOOBAccessorMap.begin(), deviceToOOBAccessorMap.end(),
                                                        [&device](const auto & pair) { return pair.first == &device; }),
                                         deviceToOOBAccessorMap.end());
        }
    }

private:
    class OOBAccessorRegistrationListener : public OOBAccessorRegisteredCallback
    {
    public:
        OOBAccessorRegistrationListener(DeviceInterface & device, std::vector<std::pair<DeviceInterface *, OOBAccessor *>> & deviceToOOBAccessorMap) :
            mDevice(device),
            mDeviceToOOBAccessorMap(deviceToOOBAccessorMap)
        {
            OOBAccessorRegistry::Instance().AddOOBAccessorRegisteredCallback(*this);
        }
        ~OOBAccessorRegistrationListener() { OOBAccessorRegistry::Instance().RemoveOOBAccessorRegisteredCallback(*this); }

        // Add the registered accessor to the map with the current device as the key, if a device is currently being registered.
        void OnRegistered(OOBAccessor & accessor) override
        {
            mDeviceToOOBAccessorMap.push_back({ &mDevice, &accessor });
        }

    private:
        DeviceInterface & mDevice;
        std::vector<std::pair<DeviceInterface *, OOBAccessor *>> & mDeviceToOOBAccessorMap;
    };
    static std::vector<std::pair<DeviceInterface *, OOBAccessor *>> & GetDeviceToOOBAccessorMap()
    {
        static std::vector<std::pair<DeviceInterface *, OOBAccessor *>> sDeviceToOOBAccessorMap;
        return sDeviceToOOBAccessorMap;
    }
};

} // namespace chip::app
