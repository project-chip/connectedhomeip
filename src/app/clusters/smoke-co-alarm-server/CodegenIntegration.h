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

/**
 * @file
 *   Application-facing server API for the Smoke CO Alarm cluster.
 *
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

    static SmokeCoAlarmServer & Instance() { return sInstance; }

    void SetInoperativeWhenUnmounted(bool v);
    CHIP_ERROR Init(EndpointId endpointId, const SmokeCoAlarm::SmokeCoAlarmCluster::Config & config,
                    SmokeCoAlarm::SmokeCoAlarmDelegate * delegate = nullptr);
    SmokeCoAlarm::SmokeCoAlarmCluster & Cluster();

    static constexpr size_t kPriorityOrderLength = SmokeCoAlarm::SmokeCoAlarmCluster::kPriorityOrderLength;

    bool RequestSelfTest(EndpointId endpoint);
    void HandleRemoteSelfTestRequest(EndpointId endpoint, chip::app::CommandHandler * commandObj,
                                     const chip::app::ConcreteCommandPath & commandPath);

    // Endpoint-taking overloads for backwards compatibility with old call sites.
    // The endpoint param is ignored; Instance() already owns a specific endpoint.
    void SetExpressedStateByPriority(EndpointId, const std::array<SmokeCoAlarm::ExpressedStateEnum, kPriorityOrderLength> & o);
    bool SetSmokeState(EndpointId, SmokeCoAlarm::AlarmStateEnum v);
    bool SetCOState(EndpointId, SmokeCoAlarm::AlarmStateEnum v);
    bool SetBatteryAlert(EndpointId, SmokeCoAlarm::AlarmStateEnum v);
    bool SetDeviceMuted(EndpointId, SmokeCoAlarm::MuteStateEnum v);
    bool SetTestInProgress(EndpointId, bool v);
    bool SetHardwareFaultAlert(EndpointId, bool v);
    bool SetEndOfServiceAlert(EndpointId, SmokeCoAlarm::EndOfServiceEnum v);
    bool SetInterconnectSmokeAlarm(EndpointId, SmokeCoAlarm::AlarmStateEnum v);
    bool SetInterconnectCOAlarm(EndpointId, SmokeCoAlarm::AlarmStateEnum v);
    bool SetContaminationState(EndpointId, SmokeCoAlarm::ContaminationStateEnum v);
    bool SetSmokeSensitivityLevel(EndpointId, SmokeCoAlarm::SensitivityEnum v);
    bool SetExpiryDate(EndpointId, uint32_t v);
    bool SetUnmountedState(EndpointId, bool v);

    chip::BitFlags<SmokeCoAlarm::Feature> GetFeatures() const;
    bool SupportsSmokeAlarm() const;
    bool SupportsCOAlarm() const;

    // Endpoint-taking Get overloads for backwards compatibility with old call sites.
    // The endpoint param is ignored; Instance() already owns a specific endpoint.
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
    SmokeCoAlarm::SmokeCoAlarmCluster::Config mConfig;
    LazyRegisteredServerCluster<SmokeCoAlarm::SmokeCoAlarmCluster> mCluster;

    static SmokeCoAlarmServer sInstance;
};

} // namespace Clusters
} // namespace app
} // namespace chip
