/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2021 Google LLC.
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

#include <stdbool.h>
#include <stdint.h>

#include "AppEvent.h"
#include <app/server/Server.h>
#include <platform/NetworkCommissioning.h>

namespace chip {
namespace NXP {
namespace App {
class AppTaskBase
{
public:
    virtual ~AppTaskBase() = default;

    /**
     * \brief Entry point of AppTaskBase. This function should be overridden.
     *
     * This function should be called by the main function in order to initialize the matter stack and all other components
     * required.
     * Should be overriden depending on the OS, for example FreeRTOS start the AppTaskMain task, but Zephyr didn't need to start any
     * task.
     *
     */
    virtual CHIP_ERROR Start() = 0;

    /**
     * \brief This function could be overridden in order to execute code at the beginning of the InitServer function.
     *
     * This function is called at the begging of the InitServer function.
     *
     */
    virtual void PreInitMatterServerInstance(void){};

    /**
     * \brief This function could be overridden in order to execute code at the end of the InitServer function.
     *
     * Example of usage: all-cluster-apps example disable last fixed endpoint after InitServer function execution.
     *
     */
    virtual void PostInitMatterServerInstance(void){};

    /**
     * \brief This function could be overridden in order to execute code before matter stack init function.
     *
     * Example of usage: if some initialization has to be done before the matter stack initialization.
     *
     */
    virtual void PreInitMatterStack(void){};

    /**
     * \brief This function could be overridden in order to execute code after matter stack init function.
     *
     * Example of usage: The laundry whasher example requires the TemperatureControl cluster initialization, this initialization is
     * done after matter stack init. Developper can override this function to add cluster initialization/customization.
     *
     */
    virtual void PostInitMatterStack(void){};

    /**
     * \brief This function could be overridden in order to register features.
     *
     * Example of usage: Could be overridden in order to register matter CLI or button features.
     *
     * \return CHIP_ERROR
     *
     */
    virtual CHIP_ERROR AppMatter_Register(void) { return CHIP_NO_ERROR; };

    /**
     * \brief This function could be overridden in order to register custom CLI commands.
     *
     * Example of usage: Laundry washer application used additionnal CLI commands.
     *
     */
    virtual void AppMatter_RegisterCustomCliCommands(void){};

    /**
     * \brief This function could be overridden in order to dispatch event.
     *
     * Example of usage: FreeRtos dispatch event using the event handler.
     *
     */
    virtual void DispatchEvent(const AppEvent & event){};

    /**
     * \brief Return a pointer to the NXP Wifi Driver instance.
     *
     * \return NXP Wifi Driver instance pointeur
     */
#if CONFIG_CHIP_WIFI || CHIP_DEVICE_CONFIG_ENABLE_WPA
    virtual chip::DeviceLayer::NetworkCommissioning::WiFiDriver * GetWifiDriverInstance(void) = 0;
#endif

    /**
     * \brief Stack initializations.
     *
     * Init matter stack and all other components (openthread, wifi, cli ...).
     *
     */
    CHIP_ERROR Init();

    /**
     * \brief Initialize the ZCL Data Model and start server.
     *
     * Call by Init function to initialize the ZCL Data Model and start server.
     *
     */
    static void InitServer(intptr_t arg);

    /* Commissioning handlers */
    virtual void StartCommissioningHandler(void){};
    virtual void StopCommissioningHandler(void){};
    virtual void SwitchCommissioningStateHandler(void){};
    virtual void FactoryResetHandler(void){};

private:
    inline static chip::CommonCaseDeviceServerInitParams initParams;
};

/**
 * Returns the application-specific implementation of the AppTaskBase object.
 *
 * Applications can use this to gain access to features of the AppTaskBase.
 */
extern AppTaskBase & GetAppTask();
} // namespace App
} // namespace NXP
} // namespace chip
