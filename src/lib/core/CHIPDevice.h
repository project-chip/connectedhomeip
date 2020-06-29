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
 *      This file contains definitions for the CHIP Device Interface
 *
 *      Applications will typically use this class to communicate with
 *      the CHIP Stack. This is a singleton object.
 */

#ifndef CHIP_DEVICE_H_
#define CHIP_DEVICE_H_

#include <core/CHIPCore.h>
#include <core/CHIPError.h>
#include <platform/CHIPDeviceLayer.h>

#include <support/DLLUtil.h>

#include <stdarg.h>
#include <stdlib.h>

namespace chip {
namespace DeviceLayer {

class DLL_EXPORT Device
{
public:
    Device(const Device &)             = delete;
    Device(const Device &&)            = delete;
    Device & operator=(const Device &) = delete;

    static Device& GetInstance()
    {
        static Device instance;
        return instance;
    }

    Device& SetVendorID(uint16_t VID)
    {
        vendorID = VID;
        return *this;
    }

    uint16_t GetVendorID()
    {
        return vendorID;
    }

    Device& SetProductID(uint16_t PID)
    {
        productID = PID;
        return *this;
    }

    uint16_t SetProductID()
    {
        return productID;
    }

    CHIP_ERROR Init(PlatformManager::EventHandlerFunct handler);

 private:
    uint16_t vendorID;
    uint16_t productID;
    Device() {}
};

} // namespace DeviceLayer
} // namespace chip

#endif /* CHIP_DEVICE_H_ */
