/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app/clusters/smoke-co-alarm-server/SmokeCoAlarmCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

namespace chip {
namespace app {
namespace Clusters {

/**
 * @brief Application-facing wrapper for SmokeCoAlarmCluster.
 *
 * Owns the cluster instance and manages its lifetime with the data model provider.
 * The application creates one SmokeCoAlarmServer per endpoint, calls Init() to
 * register with the provider, and destroys the object to unregister.
 */
class SmokeCoAlarmServer
{
public:
    SmokeCoAlarmServer() = default;
    ~SmokeCoAlarmServer();

    static SmokeCoAlarmServer & Instance()
    {
        static SmokeCoAlarmServer sInstance;
        return sInstance;
    }

    void SetInoperativeWhenUnmounted(bool v);
    CHIP_ERROR Init(EndpointId endpointId, const SmokeCoAlarmCluster::Config & config, SmokeCoAlarmDelegate * delegate = nullptr);
    SmokeCoAlarmCluster & Cluster();

    static constexpr size_t kPriorityOrderLength = SmokeCoAlarmCluster::kPriorityOrderLength;

    bool RequestSelfTest(EndpointId endpoint);

    // Endpoint-taking overloads for backwards compatibility with old call sites.
    // Returns false (or no-ops for void) if endpoint does not match the one passed to Init().
    void SetExpressedStateByPriority(EndpointId, const std::array<SmokeCoAlarm::ExpressedStateEnum, kPriorityOrderLength> & o);
    bool SetSmokeState(EndpointId, SmokeCoAlarm::AlarmStateEnum v);
    bool SetCOState(EndpointId, SmokeCoAlarm::AlarmStateEnum v);
    void SetBatteryAlert(EndpointId, SmokeCoAlarm::AlarmStateEnum v);
    bool SetDeviceMuted(EndpointId, SmokeCoAlarm::MuteStateEnum v);
    void SetTestInProgress(EndpointId, bool v);
    void SetHardwareFaultAlert(EndpointId, bool v);
    void SetEndOfServiceAlert(EndpointId, SmokeCoAlarm::EndOfServiceEnum v);
    bool SetInterconnectSmokeAlarm(EndpointId, SmokeCoAlarm::AlarmStateEnum v);
    bool SetInterconnectCOAlarm(EndpointId, SmokeCoAlarm::AlarmStateEnum v);
    void SetContaminationState(EndpointId, SmokeCoAlarm::ContaminationStateEnum v);
    void SetSmokeSensitivityLevel(EndpointId, SmokeCoAlarm::SensitivityEnum v);
    void SetExpiryDate(EndpointId, uint32_t v);
    bool SetUnmountedState(EndpointId, bool v);

    chip::BitFlags<SmokeCoAlarm::Feature> GetFeatures() const;
    bool SupportsSmokeAlarm() const;
    bool SupportsCOAlarm() const;

    // Endpoint-taking Get overloads for backwards compatibility with old call sites.
    // Returns false if endpoint does not match the one passed to Init().
    bool GetExpressedState(EndpointId, SmokeCoAlarm::ExpressedStateEnum & v) const;
    bool GetSmokeState(EndpointId, SmokeCoAlarm::AlarmStateEnum & v) const;
    bool GetCOState(EndpointId, SmokeCoAlarm::AlarmStateEnum & v) const;
    bool GetBatteryAlert(EndpointId, SmokeCoAlarm::AlarmStateEnum & v) const;
    bool GetDeviceMuted(EndpointId, SmokeCoAlarm::MuteStateEnum & v) const;
    bool GetTestInProgress(EndpointId, bool & v) const;
    bool GetHardwareFaultAlert(EndpointId, bool & v) const;
    bool GetEndOfServiceAlert(EndpointId, SmokeCoAlarm::EndOfServiceEnum & v) const;
    bool GetInterconnectSmokeAlarm(EndpointId, SmokeCoAlarm::AlarmStateEnum & v) const;
    bool GetInterconnectCOAlarm(EndpointId, SmokeCoAlarm::AlarmStateEnum & v) const;
    bool GetContaminationState(EndpointId, SmokeCoAlarm::ContaminationStateEnum & v) const;
    bool GetSmokeSensitivityLevel(EndpointId, SmokeCoAlarm::SensitivityEnum & v) const;
    bool GetExpiryDate(EndpointId, uint32_t & v) const;
    bool GetUnmountedState(EndpointId, bool & v) const;
    chip::BitFlags<SmokeCoAlarm::Feature> GetFeatures(EndpointId) const;
    bool SupportsSmokeAlarm(EndpointId) const;
    bool SupportsCOAlarm(EndpointId) const;

private:
    EndpointId mEndpointId;
    SmokeCoAlarmCluster::Config mConfig;
    LazyRegisteredServerCluster<SmokeCoAlarmCluster> mCluster;
};

} // namespace Clusters
} // namespace app
} // namespace chip
