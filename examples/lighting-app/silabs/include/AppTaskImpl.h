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
 * @brief CRTP base for AppTask, exposing override hooks for customizable APIs.
 *
 * Each public method dispatches to `Derived::*Impl()`. Overrides are optional: default
 * `*Impl()` implementations in the private section forward to `AppTask`. Override in
 * Derived only for the behaviors you want to customize.
 *
 * @tparam Derived The derived class type (CRTP pattern)
 */
template <typename Derived>
class AppTaskImpl : public AppTask
{
public:
    // External triggers — entry points that pass context into AppTask (e.g. PostEvent). Optional override: *Impl().
    CHIP_ERROR AppInit() override { CRTP_OPTIONAL_DISPATCH(AppTaskImpl, Derived, AppInitImpl); }

    CHIP_ERROR InitLight() { CRTP_OPTIONAL_DISPATCH(AppTaskImpl, Derived, InitLightImpl); }

    static void ButtonEventHandler(uint8_t button, uint8_t btnAction)
    {
        CRTP_OPTIONAL_STATIC_DISPATCH(AppTaskImpl, Derived, ButtonEventHandlerImpl, button, btnAction);
    }

    static void OnTriggerOffWithEffect(OnOffEffect * effect)
    {
        CRTP_OPTIONAL_STATIC_DISPATCH(AppTaskImpl, Derived, OnTriggerOffWithEffectImpl, effect);
    }

    // AppTask-thread event handlers (queued events, timers, data model). Optional override: *Impl().
    static void LightActionEventHandler(AppEvent * aEvent)
    {
        CRTP_OPTIONAL_STATIC_DISPATCH(AppTaskImpl, Derived, LightActionEventHandlerImpl, aEvent);
    }

    static void LightTimerEventHandler(void * timerCbArg)
    {
        CRTP_OPTIONAL_STATIC_DISPATCH(AppTaskImpl, Derived, LightTimerEventHandlerImpl, timerCbArg);
    }

#if (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
    static void LightControlEventHandler(AppEvent * aEvent)
    {
        CRTP_OPTIONAL_STATIC_DISPATCH(AppTaskImpl, Derived, LightControlEventHandlerImpl, aEvent);
    }
#endif

    void DMPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
    {
        CRTP_OPTIONAL_VOID_DISPATCH(AppTaskImpl, Derived, DMPostAttributeChangeCallbackImpl, attributePath, type, size, value);
    }

private:
    friend Derived;

    /** Default implementations — override in Derived to customize. */

    // External triggers (*Impl)
    CHIP_ERROR AppInitImpl() { return AppTask::AppInit(); }

    CHIP_ERROR InitLightImpl() { return AppTask::InitLight(); }

    void ButtonEventHandlerImpl(uint8_t button, uint8_t btnAction) { AppTask::ButtonEventHandler(button, btnAction); }

    void OnTriggerOffWithEffectImpl(OnOffEffect * effect) { AppTask::OnTriggerOffWithEffect(effect); }

    // AppTask-thread event handlers (*Impl)
    void LightActionEventHandlerImpl(AppEvent * aEvent) { AppTask::LightActionEventHandler(aEvent); }

    void LightTimerEventHandlerImpl(void * timerCbArg) { AppTask::LightTimerEventHandler(timerCbArg); }

#if (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
    void LightControlEventHandlerImpl(AppEvent * aEvent) { AppTask::LightControlEventHandler(aEvent); }
#endif

    void DMPostAttributeChangeCallbackImpl(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                           uint8_t * value)
    {
        AppTask::DMPostAttributeChangeCallback(attributePath, type, size, value);
    }
};
