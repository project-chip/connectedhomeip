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

    CHIP_ERROR InitLight() { CRTP_OPTIONAL_DISPATCH(AppTaskImpl, Derived, InitLightImpl); }

    // Platform button callback, posts light toggle or base application events.
    static void ButtonEventHandler(uint8_t button, uint8_t btnAction)
    {
        CRTP_OPTIONAL_STATIC_DISPATCH(AppTaskImpl, Derived, ButtonEventHandlerImpl, button, btnAction);
    }

    // OnOff cluster callback, arms a timed off with effect transition.
    static void OnTriggerOffWithEffect(OnOffEffect * effect)
    {
        CRTP_OPTIONAL_STATIC_DISPATCH(AppTaskImpl, Derived, OnTriggerOffWithEffectImpl, effect);
    }

    // Toggles light on/off, updates LED/display, and schedules OnOff cluster sync on the Matter thread.
    static void LightActionEventHandler(AppEvent * aEvent)
    {
        CRTP_OPTIONAL_STATIC_DISPATCH(AppTaskImpl, Derived, LightActionEventHandlerImpl, aEvent);
    }

    // Timer callback, completes an off with effect transition.
    static void LightTimerEventHandler(void * timerCbArg)
    {
        CRTP_OPTIONAL_STATIC_DISPATCH(AppTaskImpl, Derived, LightTimerEventHandlerImpl, timerCbArg);
    }

#if (defined(SL_MATTER_RGB_LED_ENABLED) && SL_MATTER_RGB_LED_ENABLED == 1)
    // Updates RGB LED from queued ColorControl changes.
    static void LightControlEventHandler(AppEvent * aEvent)
    {
        CRTP_OPTIONAL_STATIC_DISPATCH(AppTaskImpl, Derived, LightControlEventHandlerImpl, aEvent);
    }
#endif

    // Matter stack callback after a server attribute change, syncs OnOff/LevelControl/ColorControl to LED and display.
    void DMPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
    {
        CRTP_OPTIONAL_VOID_DISPATCH(AppTaskImpl, Derived, DMPostAttributeChangeCallbackImpl, attributePath, type, size, value);
    }

private:
    friend Derived;

    // Default *Impl() hooks, each forwards to the matching AppTask method
    // Override the corresponding hook in CustomerAppTask to customize behavior

    CHIP_ERROR AppInitImpl() { return AppTask::AppInit(); }

    CHIP_ERROR InitLightImpl() { return AppTask::InitLight(); }

    void ButtonEventHandlerImpl(uint8_t button, uint8_t btnAction) { AppTask::ButtonEventHandler(button, btnAction); }

    void OnTriggerOffWithEffectImpl(OnOffEffect * effect) { AppTask::OnTriggerOffWithEffect(effect); }

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
