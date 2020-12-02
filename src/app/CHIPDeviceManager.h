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
 *      This file contains definitions for the CHIP DeviceManager Interface
 *
 *      This object will co-ordinate multiple activities such as
 *      initialisation, rendezvous, session mgmt and other such
 *      activities within the CHIP stack. This is a singleton object.
 */

#pragma once

#include <core/CHIPCore.h>
#include <core/CHIPError.h>

#include <support/DLLUtil.h>

#include <stdarg.h>
#include <stdlib.h>

namespace chip {
namespace DeviceManager {

/**
 * @brief
 *   A common class that drives other components of the CHIP stack
 */
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

    /**
     * @brief
     *   Initialise CHIPDeviceManager
     *
     */
    CHIP_ERROR Init();

private:
    CHIPDeviceManager() {}
};

} // namespace DeviceManager
} // namespace chip
