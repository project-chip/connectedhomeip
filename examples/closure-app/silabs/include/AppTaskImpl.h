/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    Copyright (c) 2025 Google LLC.
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
    // Common AppTask bring up
    CHIP_ERROR AppInit() override { CRTP_OPTIONAL_DISPATCH(AppTaskImpl, Derived, AppInitImpl); }

    // Handle button press
    static void ButtonEventHandler(uint8_t button, uint8_t btnAction)
    {
        CRTP_OPTIONAL_STATIC_DISPATCH(AppTaskImpl, Derived, ButtonEventHandlerImpl, button, btnAction);
    }

    // AppTask thread handler for the closure button action event
    static void ClosureButtonActionEventHandler(AppEvent * aEvent)
    {
        CRTP_OPTIONAL_STATIC_DISPATCH(AppTaskImpl, Derived, ClosureButtonActionEventHandlerImpl, aEvent);
    }

    // Data model hook invoked when a cluster attribute changes
    void DMPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
    {
        CRTP_OPTIONAL_VOID_DISPATCH(AppTaskImpl, Derived, DMPostAttributeChangeCallbackImpl, attributePath, type, size, value);
    }

    // Data model hook for ClosureControl cluster attribute changes
    void DMClosureControlClusterAttributeChangedCallback(const chip::app::ConcreteAttributePath & attributePath)
    {
        CRTP_OPTIONAL_VOID_DISPATCH(AppTaskImpl, Derived, DMClosureControlClusterAttributeChangedCallbackImpl, attributePath);
    }

    // Data model hook for ClosureDimension cluster attribute changes
    void DMClosureDimensionClusterAttributeChangedCallback(const chip::app::ConcreteAttributePath & attributePath)
    {
        CRTP_OPTIONAL_VOID_DISPATCH(AppTaskImpl, Derived, DMClosureDimensionClusterAttributeChangedCallbackImpl, attributePath);
    }

private:
    friend Derived;

    // Default *Impl() hooks, each forwards to the matching AppTask method
    // Override the corresponding hook in CustomerAppTask to customize behavior

    CHIP_ERROR AppInitImpl() { return AppTask::AppInit(); }

    void ButtonEventHandlerImpl(uint8_t button, uint8_t btnAction) { AppTask::ButtonEventHandler(button, btnAction); }

    void ClosureButtonActionEventHandlerImpl(AppEvent * aEvent) { AppTask::ClosureButtonActionEventHandler(aEvent); }

    void DMPostAttributeChangeCallbackImpl(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                           uint8_t * value)
    {
        AppTask::DMPostAttributeChangeCallback(attributePath, type, size, value);
    }

    void DMClosureControlClusterAttributeChangedCallbackImpl(const chip::app::ConcreteAttributePath & attributePath)
    {
        AppTask::DMClosureControlClusterAttributeChangedCallback(attributePath);
    }

    void DMClosureDimensionClusterAttributeChangedCallbackImpl(const chip::app::ConcreteAttributePath & attributePath)
    {
        AppTask::DMClosureDimensionClusterAttributeChangedCallback(attributePath);
    }
};
