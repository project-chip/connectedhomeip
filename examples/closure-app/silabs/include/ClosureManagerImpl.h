/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "ClosureManager.h"
#include "CRTPHelpers.h"

/**
 * @brief CRTP override layer for `ClosureManager`.
 *
 * Each public entry point dispatches to a corresponding `Derived::*Impl()` hook.
 * The default `*Impl()` (declared private below) forwards to `ClosureManager`, so
 * `Derived` only needs to override the hooks whose behavior it wants to
 * customize.
 *
 * Customer code overrides these hooks in `CustomerAppManager`; see the app README
 * ("How to Override APIs").
 *
 * @tparam Derived The CRTP derived class (`CustomerAppManager`).
 */
template <typename Derived>
class ClosureManagerImpl : public ClosureManager
{
public:
    // ClosureManager bring up
    void Init() override { CRTP_OPTIONAL_VOID_DISPATCH(ClosureManagerImpl, Derived, InitImpl); }

    // Cluster calibrate command handler
    chip::Protocols::InteractionModel::Status OnCalibrateCommand() override
    {
        CRTP_OPTIONAL_DISPATCH(ClosureManagerImpl, Derived, OnCalibrateCommandImpl);
    }

    // Cluster MoveTo command handler
    chip::Protocols::InteractionModel::Status
    OnMoveToCommand(const chip::Optional<chip::app::Clusters::ClosureControl::TargetPositionEnum> position,
                    const chip::Optional<bool> latch,
                    const chip::Optional<chip::app::Clusters::Globals::ThreeLevelAutoEnum> speed) override
    {
        CRTP_OPTIONAL_DISPATCH_ARGS(ClosureManagerImpl, Derived, OnMoveToCommandImpl, position, latch, speed);
    }

    // Cluster Stop command handler
    chip::Protocols::InteractionModel::Status OnStopCommand() override
    {
        CRTP_OPTIONAL_DISPATCH(ClosureManagerImpl, Derived, OnStopCommandImpl);
    }

    // Panel SetTarget command handler
    chip::Protocols::InteractionModel::Status
    OnSetTargetCommand(const chip::Optional<chip::Percent100ths> & position, const chip::Optional<bool> & latch,
                       const chip::Optional<chip::app::Clusters::Globals::ThreeLevelAutoEnum> & speed,
                       const chip::EndpointId endpointId) override
    {
        CRTP_OPTIONAL_DISPATCH_ARGS(ClosureManagerImpl, Derived, OnSetTargetCommandImpl, position, latch, speed, endpointId);
    }

    // Panel Step command handler
    chip::Protocols::InteractionModel::Status
    OnStepCommand(const chip::app::Clusters::ClosureDimension::StepDirectionEnum & direction, const uint16_t & numberOfSteps,
                  const chip::Optional<chip::app::Clusters::Globals::ThreeLevelAutoEnum> & speed,
                  const chip::EndpointId & endpointId) override
    {
        CRTP_OPTIONAL_DISPATCH_ARGS(ClosureManagerImpl, Derived, OnStepCommandImpl, direction, numberOfSteps, speed, endpointId);
    }

    // Notify endpoints when a closure action completes
    void HandleClosureActionComplete(Action_t action)
    {
        CRTP_OPTIONAL_VOID_DISPATCH(ClosureManagerImpl, Derived, HandleClosureActionCompleteImpl, action);
    }

    // Advance closure endpoints toward their MoveTo target
    void HandleClosureMotionAction()
    {
        CRTP_OPTIONAL_VOID_DISPATCH(ClosureManagerImpl, Derived, HandleClosureMotionActionImpl);
    }

    // Unlatch the closure endpoint before continuing motion
    void HandleClosureUnlatchAction()
    {
        CRTP_OPTIONAL_VOID_DISPATCH(ClosureManagerImpl, Derived, HandleClosureUnlatchActionImpl);
    }

    // Compute the next panel position step toward the target
    bool GetPanelNextPosition(const chip::app::Clusters::ClosureDimension::GenericDimensionStateStruct & currentState,
                              const chip::app::Clusters::ClosureDimension::GenericDimensionStateStruct & targetState,
                              chip::app::DataModel::Nullable<chip::Percent100ths> & nextPosition)
    {
        CRTP_OPTIONAL_DISPATCH_ARGS(ClosureManagerImpl, Derived, GetPanelNextPositionImpl, currentState, targetState, nextPosition);
    }

    // Advance a panel endpoint toward its SetTarget position
    void HandlePanelSetTargetAction(chip::EndpointId endpointId)
    {
        CRTP_OPTIONAL_VOID_DISPATCH(ClosureManagerImpl, Derived, HandlePanelSetTargetActionImpl, endpointId);
    }

    // Unlatch a panel endpoint before SetTarget motion
    void HandlePanelUnlatchAction(chip::EndpointId endpointId)
    {
        CRTP_OPTIONAL_VOID_DISPATCH(ClosureManagerImpl, Derived, HandlePanelUnlatchActionImpl, endpointId);
    }

    // Advance a panel endpoint by one Step action
    void HandlePanelStepAction(chip::EndpointId endpointId)
    {
        CRTP_OPTIONAL_VOID_DISPATCH(ClosureManagerImpl, Derived, HandlePanelStepActionImpl, endpointId);
    }

private:
    friend Derived;

    // Default *Impl() hooks, each forwards to the matching ClosureManager method
    // Override the corresponding hook in CustomerAppManager to customize behavior

    void InitImpl() { ClosureManager::Init(); }

    chip::Protocols::InteractionModel::Status OnCalibrateCommandImpl() { return ClosureManager::OnCalibrateCommand(); }

    chip::Protocols::InteractionModel::Status
    OnMoveToCommandImpl(const chip::Optional<chip::app::Clusters::ClosureControl::TargetPositionEnum> position,
                        const chip::Optional<bool> latch,
                        const chip::Optional<chip::app::Clusters::Globals::ThreeLevelAutoEnum> speed)
    {
        return ClosureManager::OnMoveToCommand(position, latch, speed);
    }

    chip::Protocols::InteractionModel::Status OnStopCommandImpl() { return ClosureManager::OnStopCommand(); }

    chip::Protocols::InteractionModel::Status
    OnSetTargetCommandImpl(const chip::Optional<chip::Percent100ths> & position, const chip::Optional<bool> & latch,
                           const chip::Optional<chip::app::Clusters::Globals::ThreeLevelAutoEnum> & speed,
                           const chip::EndpointId endpointId)
    {
        return ClosureManager::OnSetTargetCommand(position, latch, speed, endpointId);
    }

    chip::Protocols::InteractionModel::Status
    OnStepCommandImpl(const chip::app::Clusters::ClosureDimension::StepDirectionEnum & direction, const uint16_t & numberOfSteps,
                      const chip::Optional<chip::app::Clusters::Globals::ThreeLevelAutoEnum> & speed,
                      const chip::EndpointId & endpointId)
    {
        return ClosureManager::OnStepCommand(direction, numberOfSteps, speed, endpointId);
    }

    void HandleClosureActionCompleteImpl(Action_t action) { ClosureManager::HandleClosureActionComplete(action); }

    void HandleClosureMotionActionImpl() { ClosureManager::HandleClosureMotionAction(); }

    void HandleClosureUnlatchActionImpl() { ClosureManager::HandleClosureUnlatchAction(); }

    bool GetPanelNextPositionImpl(const chip::app::Clusters::ClosureDimension::GenericDimensionStateStruct & currentState,
                                  const chip::app::Clusters::ClosureDimension::GenericDimensionStateStruct & targetState,
                                  chip::app::DataModel::Nullable<chip::Percent100ths> & nextPosition)
    {
        return ClosureManager::GetPanelNextPosition(currentState, targetState, nextPosition);
    }

    void HandlePanelSetTargetActionImpl(chip::EndpointId endpointId) { ClosureManager::HandlePanelSetTargetAction(endpointId); }

    void HandlePanelUnlatchActionImpl(chip::EndpointId endpointId) { ClosureManager::HandlePanelUnlatchAction(endpointId); }

    void HandlePanelStepActionImpl(chip::EndpointId endpointId) { ClosureManager::HandlePanelStepAction(endpointId); }
};
