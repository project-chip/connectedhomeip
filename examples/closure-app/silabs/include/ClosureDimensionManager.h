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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/closure-dimension-server/closure-dimension-delegate.h>
#include <app/clusters/closure-dimension-server/closure-dimension-server.h>
#include <app/clusters/closure-dimension-server/closure-dimension-cluster-logic.h>
#include <app/clusters/closure-dimension-server/closure-dimension-matter-context.h>

#include <lib/core/CHIPError.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureDimension {

using Protocols::InteractionModel::Status;

// This is an application level delegate to handle Closure Dimension commands according to the specific business logic.
class ClosureDimensionDelegate : public DelegateBase
{
public:
    enum Action_t
    {
        MOVE_ACTION = 0,
        MOVE_AND_LATCH_ACTION,
        STEP_ACTION,
        TARGET_CHANGE_ACTION,

        INVALID_ACTION
    } Action;

    typedef void (*Callback_fn_initiated)(Action_t);
    typedef void (*Callback_fn_completed)(Action_t);
    void SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB);

    Callback_fn_initiated mActionInitiated_CB;
    Callback_fn_completed mActionCompleted_CB;

    const uint32_t kExampleMotionCountDown     = 5;
    const uint32_t kExampleStepCountDown       = 3000;

    ClosureDimensionDelegate(EndpointId endpoint) : mEndpoint(endpoint), gLogic(nullptr) {}

    Status HandleSetTarget(const Optional<Percent100ths> & pos, const Optional<TargetLatchEnum> & latch,
                               const Optional<Globals::ThreeLevelAutoEnum> & speed) override;

    Status HandleStep(const StepDirectionEnum & direction, const uint16_t & numberOfSteps,
                          const Optional<Globals::ThreeLevelAutoEnum> & speed) override;

    Status HandleMotion(bool latchNeeded, bool motionNeeded, bool newTarget);

    CHIP_ERROR Init();

    void SetLogic(ClusterLogic* logic)
    {
        gLogic = logic;
    }

    ClusterLogic* getLogic() const
    {
        return gLogic;
    }

    bool IsDeviceMoving() const
    {
        return isMoving;
    }

    void SetDeviceMoving(bool moving)
    {
        isMoving = moving;
    }

    Action_t GetAction() const
    {
        return mAction;
    }

    void SetAction(Action_t action)
    {
        mAction = action;
    }

    StepDirectionEnum GetTargetDirection()
    {
        return mTargetDirection;
    }

    void SetTargetDirection(StepDirectionEnum direction)
    {
        mTargetDirection = direction;
    }

private:
    bool isMoving = false;
    bool isManualLatch = false;
    StepDirectionEnum mTargetDirection = StepDirectionEnum::kUnknownEnumValue;
    Action_t mAction = INVALID_ACTION;
    EndpointId mEndpoint;
    ClusterLogic* gLogic;
};

class ClosureDimensionManager
{
public:


    ClosureDimensionManager(EndpointId endpoint) :
        mEndpoint(endpoint), mContext(mEndpoint), mDelegate(mEndpoint), mLogic(mDelegate, mContext), mInterface(mEndpoint, mLogic)
    {
        mDelegate.SetLogic(&mLogic);
    }

    CHIP_ERROR Init()
    {
        ChipLogProgress(AppServer, "ClosureDimensionManager::Init start");
        ReturnErrorOnFailure(mLogic.Init(kConformance));
        ReturnErrorOnFailure(mInterface.Init());
        ReturnErrorOnFailure(mDelegate.Init());
        ChipLogProgress(AppServer, "ClosureDimensionManager::Init end");
        return CHIP_NO_ERROR;
    }

    ClosureDimensionDelegate& getDelegate()
    {
        return mDelegate;
    }

private:
    const ClusterConformance kConformance = { .featureMap = 255, .supportsOverflow = true };

    EndpointId mEndpoint;
    MatterContext mContext;
    ClosureDimensionDelegate mDelegate;
    ClusterLogic mLogic;
    Interface mInterface;
};
} // namespace ClosureDimension
} // namespace Clusters
} // namespace app
} // namespace chip
