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
    CHIP_ERROR AppInit() override { CRTP_OPTIONAL_DISPATCH(AppTaskImpl, Derived, AppInitImpl); }

    CHIP_ERROR InitLightSwitch(chip::EndpointId lightSwitchEndpoint, chip::EndpointId genericSwitchEndpoint)
    {
        CRTP_OPTIONAL_DISPATCH_ARGS(AppTaskImpl, Derived, InitLightSwitchImpl, lightSwitchEndpoint, genericSwitchEndpoint);
    }

    static void ButtonEventHandler(uint8_t button, uint8_t btnAction)
    {
        CRTP_OPTIONAL_STATIC_DISPATCH(AppTaskImpl, Derived, ButtonEventHandlerImpl, button, btnAction);
    }

    static void AppEventHandler(AppEvent * aEvent)
    {
        CRTP_OPTIONAL_STATIC_DISPATCH(AppTaskImpl, Derived, AppEventHandlerImpl, aEvent);
    }

    static void InitBindingHandler(intptr_t arg)
    {
        CRTP_OPTIONAL_STATIC_DISPATCH(AppTaskImpl, Derived, InitBindingHandlerImpl, arg);
    }

    static void LightSwitchChangedHandler(const chip::app::Clusters::Binding::TableEntry & binding,
                                          chip::OperationalDeviceProxy * peer_device, void * context)
    {
        CRTP_OPTIONAL_STATIC_DISPATCH(AppTaskImpl, Derived, LightSwitchChangedHandlerImpl, binding, peer_device, context);
    }

    void DMPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
    {
        CRTP_OPTIONAL_VOID_DISPATCH(AppTaskImpl, Derived, DMPostAttributeChangeCallbackImpl, attributePath, type, size, value);
    }

private:
    friend Derived;

    CHIP_ERROR AppInitImpl() { return AppTask::AppInit(); }

    CHIP_ERROR InitLightSwitchImpl(chip::EndpointId lightSwitchEndpoint, chip::EndpointId genericSwitchEndpoint)
    {
        return AppTask::InitLightSwitch(lightSwitchEndpoint, genericSwitchEndpoint);
    }

    void ButtonEventHandlerImpl(uint8_t button, uint8_t btnAction) { AppTask::ButtonEventHandler(button, btnAction); }

    void AppEventHandlerImpl(AppEvent * aEvent) { AppTask::AppEventHandler(aEvent); }

    void InitBindingHandlerImpl(intptr_t arg) { AppTask::InitBindingHandler(arg); }

    void LightSwitchChangedHandlerImpl(const chip::app::Clusters::Binding::TableEntry & binding,
                                       chip::OperationalDeviceProxy * peer_device, void * context)
    {
        AppTask::LightSwitchChangedHandler(binding, peer_device, context);
    }

    void DMPostAttributeChangeCallbackImpl(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                           uint8_t * value)
    {
        AppTask::DMPostAttributeChangeCallback(attributePath, type, size, value);
    }
};
