/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/ambient-context-sensing-server/AmbientContextSensingCluster.h>
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <device/api/SingleEndpoint.h>
#include <lib/support/TimerDelegate.h>

namespace chip::app {

class AmbientContextSensor : public SingleEndpoint
{
public:
    using AmbientContextSensingConfig = Clusters::AmbientContextSensingCluster::Config;

    AmbientContextSensor(AmbientContextSensingConfig config, TimerDelegate & timerDelegate,
                         Clusters::AmbientContextSensing::AmbientContextSensingDelegate & delegate);
    ~AmbientContextSensor() override = default;

    // DeviceInterface pure virtual lifecycle hooks
    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    Clusters::IdentifyCluster & IdentifyCluster() { return mIdentifyCluster.Cluster(); }
    Clusters::AmbientContextSensingCluster & AmbientContextSensingCluster() { return mAmbientContextSensingCluster.Cluster(); }

protected:
    AmbientContextSensingConfig mConfig;
    TimerDelegate & mTimerDelegate;
    Clusters::AmbientContextSensing::AmbientContextSensingDelegate & mDelegate;
    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::AmbientContextSensingCluster> mAmbientContextSensingCluster;
};

} // namespace chip::app
