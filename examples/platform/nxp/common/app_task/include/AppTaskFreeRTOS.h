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
class AppTaskFreeRTOS : public AppTaskBase
{
public:
    virtual ~AppTaskFreeRTOS() = default;

    /**
     * \brief Entry point of AppTaskBase.
     *
     * This function:
     *              - Create application message queue
     *              - Create the dedicated application task
     *
     */
    virtual CHIP_ERROR Start() override;

    /**
     * \brief Application task.
     *
     * This function:
     *              - Init matter stack and other needed components
     *              - Dispatch event comming from the app event queue
     *
     */
    static void AppTaskMain(void * pvParameter);

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
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    virtual chip::DeviceLayer::NetworkCommissioning::WiFiDriver * GetWifiDriverInstance(void) override;
#endif

    /**
     * \brief This function register matter CLI and button features.
     *
     * \return CHIP_ERROR
     *
     */
    virtual CHIP_ERROR AppMatter_Register(void) override;

    /* Functions that would be called in the Matter task context */
    static void StartCommissioning(intptr_t arg);
    static void StopCommissioning(intptr_t arg);
    static void SwitchCommissioningState(intptr_t arg);

    /* Commissioning handlers */
    virtual void StartCommissioningHandler(void) override;
    virtual void StopCommissioningHandler(void) override;
    virtual void SwitchCommissioningStateHandler(void) override;

    /* FactoryResetHandler */
    virtual void FactoryResetHandler(void) override;

private:
    void DispatchEvent(const AppEvent & event);
};
} // namespace App
} // namespace NXP
} // namespace chip
