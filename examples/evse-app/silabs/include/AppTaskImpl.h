/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#include "AppEvent.h"
#include "AppTask.h"
#include "CRTPHelpers.h"

/**
 * @brief CRTP override layer for `AppTask`.
 *
 * Each public entry point dispatches to a corresponding `Derived::*Impl()` hook.
 * The default `*Impl()` (declared private below) forwards to `AppTask`, so
 * `Derived` only needs to override the hooks whose behavior it wants to
 * customize.
 *
 * Customer code overrides these hooks in `CustomerAppTask`; see the app README
 * ("How to Override APIs").
 *
 * @tparam Derived The CRTP derived class (`CustomerAppTask`).
 */
template <typename Derived>
class AppTaskImpl : public AppTask
{
public:
    // Common AppTask bring up
    CHIP_ERROR AppInit() override { CRTP_OPTIONAL_DISPATCH(AppTaskImpl, Derived, AppInitImpl); }

    // Application bring up, invoked from AppInit
    void ApplicationInit() { CRTP_OPTIONAL_VOID_DISPATCH(AppTaskImpl, Derived, ApplicationInitImpl); }

    // Handle button press
    static void ButtonEventHandler(uint8_t button, uint8_t btnAction)
    {
        CRTP_OPTIONAL_STATIC_DISPATCH(AppTaskImpl, Derived, ButtonEventHandlerImpl, button, btnAction);
    }

    // AppTask thread event handler for energy management button actions
    static void EnergyManagementActionEventHandler(AppEvent * aEvent)
    {
        CRTP_OPTIONAL_STATIC_DISPATCH(AppTaskImpl, Derived, EnergyManagementActionEventHandlerImpl, aEvent);
    }

private:
    friend Derived;

    // Default *Impl() hooks, each forwards to the matching AppTask method
    // Override the corresponding hook in CustomerAppTask to customize behavior

    CHIP_ERROR AppInitImpl() { return AppTask::AppInit(); }

    void ApplicationInitImpl() { AppTask::ApplicationInit(); }

    void ButtonEventHandlerImpl(uint8_t button, uint8_t btnAction) { AppTask::ButtonEventHandler(button, btnAction); }

    void EnergyManagementActionEventHandlerImpl(AppEvent * aEvent) { AppTask::EnergyManagementActionEventHandler(aEvent); }
};
