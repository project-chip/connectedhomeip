/**
 *
 *    Copyright (c) 2023-2026 Project CHIP Authors
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
 *
 */
#include "refrigerator-alarm-server.h"

#include <app/clusters/alarm-base-server/AlarmBaseCluster.h>
#include <app/clusters/refrigerator-alarm-server/CodegenIntegration.h>
#include <app/clusters/alarm-base-server/alarm-base-cluster-objects.h>
#include <lib/support/BitFlags.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::RefrigeratorAlarm;
using chip::Protocols::InteractionModel::Status;

namespace {

AlarmBaseCluster * GetCluster(EndpointId endpoint)
{
    return FindClusterOnEndpoint(endpoint);
}

BitMask<AlarmMap> ToAlarmMap(AlarmBase::AlarmMap map)
{
    return BitMask<AlarmMap>(map.Raw());
}

AlarmBase::AlarmMap FromAlarmMap(const BitMask<AlarmMap> map)
{
    return AlarmBase::AlarmMap(map.Raw());
}

} // namespace

RefrigeratorAlarmServer RefrigeratorAlarmServer::instance;

RefrigeratorAlarmServer & RefrigeratorAlarmServer::Instance()
{
    return instance;
}

Status RefrigeratorAlarmServer::GetMaskValue(EndpointId endpoint, BitMask<AlarmMap> * mask)
{
    AlarmBaseCluster * cluster = GetCluster(endpoint);
    VerifyOrReturnError(cluster != nullptr, Status::UnsupportedEndpoint);

    AlarmBase::AlarmMap value;
    Status status = cluster->GetMaskValue(&value);
    if (status == Status::Success && mask != nullptr)
    {
        *mask = ToAlarmMap(value);
    }
    return status;
}

Status RefrigeratorAlarmServer::GetStateValue(EndpointId endpoint, BitMask<AlarmMap> * state)
{
    AlarmBaseCluster * cluster = GetCluster(endpoint);
    VerifyOrReturnError(cluster != nullptr, Status::UnsupportedEndpoint);

    AlarmBase::AlarmMap value;
    Status status = cluster->GetStateValue(&value);
    if (status == Status::Success && state != nullptr)
    {
        *state = ToAlarmMap(value);
    }
    return status;
}

Status RefrigeratorAlarmServer::GetSupportedValue(EndpointId endpoint, BitMask<AlarmMap> * supported)
{
    AlarmBaseCluster * cluster = GetCluster(endpoint);
    VerifyOrReturnError(cluster != nullptr, Status::UnsupportedEndpoint);

    AlarmBase::AlarmMap value;
    Status status = cluster->GetSupportedValue(&value);
    if (status == Status::Success && supported != nullptr)
    {
        *supported = ToAlarmMap(value);
    }
    return status;
}

Status RefrigeratorAlarmServer::SetMaskValue(EndpointId endpoint, const BitMask<AlarmMap> mask)
{
    AlarmBaseCluster * cluster = GetCluster(endpoint);
    VerifyOrReturnError(cluster != nullptr, Status::UnsupportedEndpoint);
    return cluster->SetMaskValue(FromAlarmMap(mask));
}

Status RefrigeratorAlarmServer::SetStateValue(EndpointId endpoint, const BitMask<AlarmMap> newState)
{
    AlarmBaseCluster * cluster = GetCluster(endpoint);
    VerifyOrReturnError(cluster != nullptr, Status::UnsupportedEndpoint);
    return cluster->SetStateValue(FromAlarmMap(newState));
}

Status RefrigeratorAlarmServer::SetSupportedValue(EndpointId endpoint, const BitMask<AlarmMap> supported)
{
    AlarmBaseCluster * cluster = GetCluster(endpoint);
    VerifyOrReturnError(cluster != nullptr, Status::UnsupportedEndpoint);
    return cluster->SetSupportedValue(FromAlarmMap(supported));
}
