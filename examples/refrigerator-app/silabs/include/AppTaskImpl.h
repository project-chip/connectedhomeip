/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
 * @brief CRTP base for refrigerator AppTask, exposing override hooks for customizable APIs.
 *
 * Each public method dispatches to `Derived::*Impl()`. Overrides are optional: default
 * `*Impl()` implementations in the private section forward to `AppTask`. Override in
 * `Derived` only for the behaviors you want to customize.
 *
 * @tparam Derived The derived class type (CRTP pattern)
 */
template <typename Derived>
class AppTaskImpl : public AppTask
{
public:
    // Common AppTask bring up
    CHIP_ERROR AppInit() override { CRTP_OPTIONAL_DISPATCH(AppTaskImpl, Derived, AppInitImpl); }

    // Refrigerator specific initialization
    CHIP_ERROR InitRefrigerator() { CRTP_OPTIONAL_DISPATCH(AppTaskImpl, Derived, InitRefrigeratorImpl); }

    // Handle button press
    static void ButtonEventHandler(uint8_t button, uint8_t btnAction)
    {
        CRTP_OPTIONAL_STATIC_DISPATCH(AppTaskImpl, Derived, ButtonEventHandlerImpl, button, btnAction);
    }

    // Data model hook invoked when a cluster attribute changes
    void DMPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
    {
        CRTP_OPTIONAL_VOID_DISPATCH(AppTaskImpl, Derived, DMPostAttributeChangeCallbackImpl, attributePath, type, size, value);
    }

    // Constructs the ModeBase::Instance for the cabinet mode cluster on the given endpoint
    void CabinetModeClusterInit(chip::EndpointId endpointId)
    {
        CRTP_OPTIONAL_VOID_DISPATCH(AppTaskImpl, Derived, CabinetModeClusterInitImpl, endpointId);
    }

    // ModeBase::Delegate hooks. Override matching *Impl() in Derived to customize cabinet mode behavior.
    CHIP_ERROR Init() override { CRTP_OPTIONAL_DISPATCH(AppTaskImpl, Derived, CabinetModeInitImpl); }

    void HandleChangeToMode(uint8_t newMode, chip::app::Clusters::ModeBase::Commands::ChangeToModeResponse::Type & response) override
    {
        CRTP_OPTIONAL_VOID_DISPATCH(AppTaskImpl, Derived, HandleChangeToModeImpl, newMode, response);
    }

    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, chip::MutableCharSpan & label) override
    {
        CRTP_OPTIONAL_DISPATCH_ARGS(AppTaskImpl, Derived, GetModeLabelByIndexImpl, modeIndex, label);
    }

    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override
    {
        CRTP_OPTIONAL_DISPATCH_ARGS(AppTaskImpl, Derived, GetModeValueByIndexImpl, modeIndex, value);
    }

    CHIP_ERROR GetModeTagsByIndex(
        uint8_t modeIndex, chip::app::DataModel::List<chip::app::Clusters::detail::Structs::ModeTagStruct::Type> & tags) override
    {
        CRTP_OPTIONAL_DISPATCH_ARGS(AppTaskImpl, Derived, GetModeTagsByIndexImpl, modeIndex, tags);
    }

private:
    friend Derived;

    // Default *Impl() hooks, each forwards to the matching AppTask method
    // Override the corresponding hook in CustomerAppTask to customize behavior

    CHIP_ERROR AppInitImpl() { return AppTask::AppInit(); }

    CHIP_ERROR InitRefrigeratorImpl() { return AppTask::InitRefrigerator(); }

    void ButtonEventHandlerImpl(uint8_t button, uint8_t btnAction) { AppTask::ButtonEventHandler(button, btnAction); }

    void DMPostAttributeChangeCallbackImpl(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                           uint8_t * value)
    {
        AppTask::DMPostAttributeChangeCallback(attributePath, type, size, value);
    }

    void CabinetModeClusterInitImpl(chip::EndpointId endpointId) { AppTask::CabinetModeClusterInit(endpointId); }

    CHIP_ERROR CabinetModeInitImpl() { return AppTask::Init(); }

    void HandleChangeToModeImpl(uint8_t newMode, chip::app::Clusters::ModeBase::Commands::ChangeToModeResponse::Type & response)
    {
        AppTask::HandleChangeToMode(newMode, response);
    }

    CHIP_ERROR GetModeLabelByIndexImpl(uint8_t modeIndex, chip::MutableCharSpan & label)
    {
        return AppTask::GetModeLabelByIndex(modeIndex, label);
    }

    CHIP_ERROR GetModeValueByIndexImpl(uint8_t modeIndex, uint8_t & value)
    {
        return AppTask::GetModeValueByIndex(modeIndex, value);
    }

    CHIP_ERROR GetModeTagsByIndexImpl(
        uint8_t modeIndex, chip::app::DataModel::List<chip::app::Clusters::detail::Structs::ModeTagStruct::Type> & tags)
    {
        return AppTask::GetModeTagsByIndex(modeIndex, tags);
    }
};
