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

#include <app/TimerDelegates.h>
#include <devices/boolean-state-sensor/BooleanStateSensorDevice.h>
#include <functional>
#include <lib/core/CHIPError.h>
#include <map>
#include <devices/Types.h>

namespace chip::app {

/**
 * This is a factory class made to be used to create any valid device type as part of the 
 * all-devices-app. This class is meant to abstract away some details of device specific code,
 * and to have more generic implementation code being used in main to create a device. The keys
 * in the device registry map are the command line arguments used to start the respective device.
 * Create devices by fetching the instance of this class and passing in the device type argument
 * i.e. DeviceFactory::GetInstance().Create(deviceTypeName)
 */
class DeviceFactory
{
public:
    using DeviceCreator = std::function<std::unique_ptr<BaseDevice>()>;

    static DeviceFactory & GetInstance()
    {
        static DeviceFactory instance;
        return instance;
    }

    bool IsValidDevice(const std::string & deviceTypeArg) { return mRegistry.find(deviceTypeArg) != mRegistry.end(); }

    std::unique_ptr<BaseDevice> Create(const std::string & deviceTypeArg)
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
        mRegistry["contact-sensor"]      = []() { return std::make_unique<BooleanStateSensorDevice>(std::make_unique<DefaultTimerDelegate>(), Device::Type::kContactSensor); };
        mRegistry["water-leak-detector"] = []() { return std::make_unique<BooleanStateSensorDevice>(std::make_unique<DefaultTimerDelegate>(), Device::Type::kWaterLeakDetector); };
    }
};

} // namespace chip::app
