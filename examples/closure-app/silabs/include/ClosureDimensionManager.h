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

// This is an application level delegate to handle Closure Dimension commands according to the specific business logic.
class ClosureDimensionManager : public ClosureDimension::Delegate
{
public:
    ClosureDimensionDelegate(EndpointId endpoint) : mEndpoint(endpoint) {}
    CHIP_ERROR HandleSetTarget(const Optional<Percent100ths> & pos, const Optional<TargetLatchEnum> & latch,
                               const Optional<Globals::ThreeLevelAutoEnum> & speed) override;

    CHIP_ERROR HandleStep(const StepDirectionEnum & direction, const uint16_t & numberOfSteps,
                          const Optional<Globals::ThreeLevelAutoEnum> & speed) override;

private:
    EndpointId mEndpoint;
};

class ClosureDimensionManager
{
public:
    enum Action_t
    {
        LOCK_ACTION = 0,
        UNLOCK_ACTION,
        UNLATCH_ACTION,

        INVALID_ACTION
    } Action;

    typedef void (*Callback_fn_initiated)(Action_t, int32_t aActor);
    typedef void (*Callback_fn_completed)(Action_t);
    void SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB);

    ClosureDimensionManager(EndpointId endpoint) :
        mEndpoint(endpoint), mContext(mEndpoint), mDelegate(mEndpoint), mLogic(mDelegate, mContext), mInterface(mEndpoint, mLogic)
    {}
    CHIP_ERROR Init()
    {
        ReturnErrorOnFailure(mLogic.Init(kConformance));
        ReturnErrorOnFailure(mInterface.Init());
        return CHIP_NO_ERROR;
    }

    bool SupportsLimitMonitoring() const;
    int32_t GetLimitRangeMin() const;
    int32_t GetLimitRangeMax() const;
    uint16_t GetCurrentPosition() const;
    void MoveToPosition(uint16_t position);
    void SetSpeed(Globals::ThreeLevelAutoEnum speedMode);
    ClusterLogic getLogic(){
        return mLogic;
    }
    ClusterConformance getConformance(){
        return kConformance;
    }
    CHIP_ERROR HandleStepCommand(const StepDirectionEnum & direction, const uint16_t & numberOfSteps,
        const Optional<Globals::ThreeLevelAutoEnum> & speed){

        return mDelegate.HandleStep(direction,numberOfSteps,speed);
    }

private:
    static ClosureDimensionManager sManager;
    const ClusterConformance kConformance = { .featureMap = 0, .supportsOverflow = false };

    EndpointId mEndpoint;
    MatterContext mContext;
    ClosureDimensionDelegate mDelegate;
    ClusterLogic mLogic;
    Interface mInterface;

    int32_t mLimitMin = 0;
    int32_t mLimitMax = 10000;
    uint16_t mCurrentPosition = 0;
    Globals::ThreeLevelAutoEnum mCurrentSpeed = Globals::ThreeLevelAutoEnum::kAuto;
    Callback_fn_initiated mActionInitiated_CB;
    Callback_fn_completed mActionCompleted_CB;
};

inline ClosureDimensionManager & ClosureCtrlMgr()
{
    return ClosureDimensionManager::sClosureCtrlMgr;
}
} // namespace ClosureDimension
} // namespace Clusters
} // namespace app
} // namespace chip
