/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "AppTaskBase.h"

namespace chip {
namespace NXP {
namespace App {
class AppTaskZephyr : public AppTaskBase
{
public:
    virtual ~AppTaskZephyr() = default;

    /**
     * \brief Entry point of AppTaskBase.
     *
     * This function:
     *              - Init matter stack and other needed components
     *              - Dispatch event comming from the app event queue
     *
     */
    virtual CHIP_ERROR Start() override;

    /**
     * \brief Send event to the event queue.
     *
     */
    void PostEvent(const AppEvent & event);

    /**
     * \brief Return a pointer to the NXP Wifi Driver instance.
     *
     * \return NXP Wifi Driver instance pointeur
     */
#if defined(CONFIG_CHIP_WIFI)
    virtual chip::DeviceLayer::NetworkCommissioning::WiFiDriver * GetWifiDriverInstance(void) override;
#endif

private:
    void DispatchEvent(const AppEvent & event);
};
} // namespace App
} // namespace NXP
} // namespace chip
