/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/concentration-measurement-server/ConcentrationMeasurementCluster.h>
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <app/clusters/smoke-co-alarm-server/SmokeCoAlarmCluster.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <devices/interface/SingleEndpointDevice.h>
#include <lib/support/TimerDelegate.h>

namespace chip {
namespace app {

class AppSmokeCoAlarmDelegate : public Clusters::SmokeCoAlarmDelegate, public TimerContext
{
public:
    static constexpr uint16_t kSelfTestTimeoutSec = 10;

    void Init(TimerDelegate & timerDelegate, Clusters::SmokeCoAlarmCluster & cluster)
    {
        mTimerDelegate = &timerDelegate;
        mCluster       = &cluster;
    }

    void OnSelfTestRequested() override
    {
        ChipLogDetail(NotSpecified, "SmokeCoAlarm: self-test started");
        mTimerDelegate->StartTimer(this, System::Clock::Seconds32(kSelfTestTimeoutSec));
    }

    void TimerFired() override
    {
        mCluster->SetTestInProgress(false);
        mCluster->SetExpressedStateByPriority(sPriorityOrder);
        ChipLogDetail(NotSpecified, "SmokeCoAlarm: self-test complete");
    }

    void OnSmokeSensitivityLevelChanged(Clusters::SmokeCoAlarm::SensitivityEnum newLevel) override
    {
        ChipLogDetail(NotSpecified, "SmokeCoAlarm: smoke sensitivity level changed to %u", to_underlying(newLevel));
    }

    void OnExpressedStateChanged(Clusters::SmokeCoAlarm::ExpressedStateEnum newState) override
    {
        ChipLogDetail(NotSpecified, "SmokeCoAlarm: expressed state changed to %u", to_underlying(newState));
    }

private:
    static const std::array<Clusters::SmokeCoAlarm::ExpressedStateEnum, Clusters::SmokeCoAlarmCluster::kPriorityOrderLength>
        sPriorityOrder;

    TimerDelegate * mTimerDelegate           = nullptr;
    Clusters::SmokeCoAlarmCluster * mCluster = nullptr;
};

class SmokeCoAlarmDevice : public SingleEndpointDevice
{
public:
    using ConcentrationCluster = Clusters::ConcentrationMeasurement::ConcentrationMeasurementCluster;

    SmokeCoAlarmDevice(TimerDelegate & timerDelegate, const ConcentrationCluster::Config & coConfig,
                       const Clusters::SmokeCoAlarmCluster::Config & smokeConfig = {});
    ~SmokeCoAlarmDevice() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointId parentId = kInvalidEndpointId) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    ConcentrationCluster & GetCoConcentrationCluster();
    Clusters::SmokeCoAlarmCluster & GetSmokeCoAlarmCluster();
    Clusters::IdentifyCluster & GetIdentifyCluster();

protected:
    TimerDelegate & mTimerDelegate;
    ConcentrationCluster::Config mCoConfig;
    Clusters::SmokeCoAlarmCluster::Config mSmokeConfig;
    AppSmokeCoAlarmDelegate mSmokeCoAlarmDelegate;

    LazyRegisteredServerCluster<ConcentrationCluster> mCoMeasurementCluster;
    LazyRegisteredServerCluster<Clusters::SmokeCoAlarmCluster> mSmokeCoAlarmCluster;
    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
};

} // namespace app
} // namespace chip
