/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *    Copyright 2024 NXP
 *    All rights reserved.
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

#include <lib/core/CHIPError.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemLayer.h>

namespace chip {
namespace DeviceLayer {
class EthManager
{
public:
    static EthManager & Instance()
    {
        static EthManager sInstance;
        return sInstance;
    }
    CHIP_ERROR Init();
};

} // namespace DeviceLayer
} // namespace chip
