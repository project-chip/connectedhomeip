/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <devices/contact-sensor/ContactSensorDevice.h>
#include <devices/water-leak-detector/WaterLeakDetectorDevice.h>
#include <functional>
#include <lib/core/CHIPError.h>
#include <map>

namespace chip::app {

class DeviceFactory
{
public:
    using DeviceCreator = std::function<std::unique_ptr<Device>()>;

    static DeviceFactory & GetInstance()
    {
        static DeviceFactory instance;
        return instance;
    }

    bool IsValidDevice(const std::string & deviceTypeArg) { return mRegistry.find(deviceTypeArg) != mRegistry.end(); }

    std::unique_ptr<Device> Create(const std::string & deviceTypeArg)
    {
        if (IsValidDevice(deviceTypeArg))
        {
            return mRegistry.find(deviceTypeArg)->second();
        }
        return nullptr;
    }

private:
    std::map<std::string, DeviceCreator> mRegistry;

    DeviceFactory()
    {
        mRegistry["contact-sensor"]      = []() { return std::make_unique<ContactSensorDevice>(); };
        mRegistry["water-leak-detector"] = []() { return std::make_unique<WaterLeakDetectorDevice>(); };
    }
};

} // namespace chip::app
