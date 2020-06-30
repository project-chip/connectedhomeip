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

#ifndef CHIP_DEVICEMANAGER_H_
#define CHIP_DEVICEMANAGER_H_

#include <core/CHIPCore.h>
#include <core/CHIPError.h>
#include <platform/CHIPDeviceLayer.h>

#include <support/DLLUtil.h>

#include <stdarg.h>
#include <stdlib.h>

extern "C" {
#include "chip-zcl/chip-zcl.h"
#include "gen/gen-cluster-id.h"
#include "gen/gen-types.h"
}

namespace chip {
namespace DeviceManager {

class DLL_EXPORT CHIPDeviceManagerCallbacks
{
public:
    virtual void DeviceEventCallback(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);
    virtual void PostAttributeChangeCallback(uint8_t endpoint, ChipZclClusterId clusterId, ChipZclAttributeId attributeId,
                                             uint8_t mask, uint16_t manufacturerCode, uint8_t type, uint8_t size, uint8_t * value)
    {}
    virtual ~CHIPDeviceManagerCallbacks() {}
};

class DLL_EXPORT CHIPDeviceManager
{
public:
    CHIPDeviceManager(const CHIPDeviceManager &)  = delete;
    CHIPDeviceManager(const CHIPDeviceManager &&) = delete;
    CHIPDeviceManager & operator=(const CHIPDeviceManager &) = delete;

    static CHIPDeviceManager & GetInstance()
    {
        static CHIPDeviceManager instance;
        return instance;
    }

    CHIPDeviceManager & SetVendorID(uint16_t VID)
    {
        mVendorID = VID;
        return *this;
    }

    uint16_t GetVendorID() { return mVendorID; }

    CHIPDeviceManager & SetProductID(uint16_t PID)
    {
        mProductID = PID;
        return *this;
    }

    uint16_t SetProductID() { return mProductID; }

    CHIPDeviceManagerCallbacks * GetCHIPDeviceManagerCallbacks() { return mCB; }

    CHIP_ERROR Init(CHIPDeviceManagerCallbacks * cb);
    static void CommonDeviceEventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);

private:
    uint16_t mVendorID;
    uint16_t mProductID;
    CHIPDeviceManagerCallbacks * mCB = nullptr;
    CHIPDeviceManager() {}
};

} // namespace DeviceManager
} // namespace chip

#endif /* CHIP_DEVICEMANAGER_H_ */
