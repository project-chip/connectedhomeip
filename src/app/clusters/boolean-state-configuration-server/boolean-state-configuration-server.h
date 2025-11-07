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
 * @file API declarations for boolean state configuration cluster.
 */

#pragma once

#include <app/clusters/boolean-state-configuration-server/boolean-state-configuration-cluster.h>
#include <app/clusters/boolean-state-configuration-server/boolean-state-configuration-delegate.h>
#include <lib/core/CHIPError.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace BooleanStateConfiguration {

// Get access to the underlying cluster registered on the given endpoint
BooleanStateConfigurationCluster * FindClusterOnEndpoint(EndpointId endpointId);

////////////////////////////////////////////////////////////////////////////////////
// The methods below are DEPRECATED. Please interact with the cluster directly
// via `FindClusterOnEndpoint` (for code generated builds)
////////////////////////////////////////////////////////////////////////////////////
inline void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    // NOTE: this will only work AFTER cluster startup (i.e. emberAfClusterInit)
    if (auto cluster = FindClusterOnEndpoint(endpoint); cluster != nullptr)
    {
        cluster->SetDelegate(delegate);
    }
}

inline Delegate * GetDefaultDelegate(EndpointId endpoint)
{
    if (auto cluster = FindClusterOnEndpoint(endpoint); cluster != nullptr)
    {
        return cluster->GetDelegate();
    }
    return nullptr;
}

inline CHIP_ERROR SetAlarmsActive(EndpointId ep, chip::BitMask<AlarmModeBitmap> alarms)
{
    auto cluster = FindClusterOnEndpoint(ep);
    VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_NO_ENDPOINT);
    auto status = cluster->SetAlarmsActive(alarms);
    return (status == Protocols::InteractionModel::Status::Success) ? CHIP_NO_ERROR : CHIP_ERROR_IM_GLOBAL_STATUS_VALUE(status);
}

inline CHIP_ERROR SetAllEnabledAlarmsActive(EndpointId ep)
{
    auto cluster = FindClusterOnEndpoint(ep);
    VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_NO_ENDPOINT);
    auto status = cluster->SetAllEnabledAlarmsActive();
    return (status == Protocols::InteractionModel::Status::Success) ? CHIP_NO_ERROR : CHIP_ERROR_IM_GLOBAL_STATUS_VALUE(status);
}

inline CHIP_ERROR ClearAllAlarms(EndpointId ep)
{
    auto cluster = FindClusterOnEndpoint(ep);
    VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_NO_ENDPOINT);
    cluster->ClearAllAlarms();
    return CHIP_NO_ERROR;
}

inline CHIP_ERROR SuppressAlarms(EndpointId ep, chip::BitMask<BooleanStateConfiguration::AlarmModeBitmap> alarms)
{
    auto cluster = FindClusterOnEndpoint(ep);
    VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_NO_ENDPOINT);
    auto status = cluster->SuppressAlarms(alarms);
    return (status == Protocols::InteractionModel::Status::Success) ? CHIP_NO_ERROR : CHIP_ERROR_IM_GLOBAL_STATUS_VALUE(status);
}

inline CHIP_ERROR SetCurrentSensitivityLevel(EndpointId ep, uint8_t level)
{
    auto cluster = FindClusterOnEndpoint(ep);
    VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_NO_ENDPOINT);
    return cluster->SetCurrentSensitivityLevel(level);
}

inline CHIP_ERROR EmitSensorFault(EndpointId ep, chip::BitMask<BooleanStateConfiguration::SensorFaultBitmap> fault)
{
    auto cluster = FindClusterOnEndpoint(ep);
    VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_NO_ENDPOINT);
    cluster->EmitSensorFault(fault);
    return CHIP_NO_ERROR;
}

inline bool HasFeature(EndpointId ep, Feature feature)
{
    auto cluster = FindClusterOnEndpoint(ep);
    VerifyOrReturnValue(cluster != nullptr, false);
    return cluster->GetFeatures().Has(feature);
}

} // namespace BooleanStateConfiguration
} // namespace Clusters
} // namespace app
} // namespace chip
