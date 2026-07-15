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
#include <device/api/SingleEndpoint.h>
#include <lib/support/TimerDelegate.h>

namespace chip {
namespace app {

/// Generic smoke + CO alarm device. It wires up the clusters but is agnostic about behavior: the
/// SmokeCoAlarmDelegate is injected, so concrete subclasses decide how the alarm actually behaves
/// (see LoggingOnlySmokeCoAlarm for the example/no-hardware implementation).
class SmokeCoAlarm : public SingleEndpoint
{
public:
    using ConcentrationCluster = Clusters::ConcentrationMeasurement::ConcentrationMeasurementCluster;

    SmokeCoAlarm(TimerDelegate & timerDelegate, Clusters::SmokeCoAlarmDelegate & smokeCoAlarmDelegate);
    ~SmokeCoAlarm() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    ConcentrationCluster & GetCoConcentrationCluster();
    Clusters::SmokeCoAlarmCluster & GetSmokeCoAlarmCluster();
    Clusters::IdentifyCluster & GetIdentifyCluster();

protected:
    TimerDelegate & mTimerDelegate;
    Clusters::SmokeCoAlarmDelegate & mSmokeCoAlarmDelegate;
    ConcentrationCluster::Config mCoConfig;
    Clusters::SmokeCoAlarmCluster::Config mSmokeConfig;

    LazyRegisteredServerCluster<ConcentrationCluster> mCoMeasurementCluster;
    LazyRegisteredServerCluster<Clusters::SmokeCoAlarmCluster> mSmokeCoAlarmCluster;
    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
};

} // namespace app
} // namespace chip
