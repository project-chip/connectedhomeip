/*
 *
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

#include <app/clusters/electrical-protection-alarm-server/ElectricalProtectionAlarmCluster.h>

#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalProtectionAlarm {

/// Owns an ElectricalProtectionAlarmCluster and its registration.
///
/// Electrical Protection Alarm is a code-driven cluster, so the generated dispatch calls
/// MatterElectricalProtectionAlarmClusterInitCallback() for every endpoint that enables it. That
/// callback is intentionally a no-op: the application owns the cluster instead, because the
/// supported alarms, mask and feature map are application data that the generated static
/// configuration cannot supply.
///
/// Applications construct one Instance per endpoint hosting the cluster, call Init() during
/// application init, and let the destructor (or an explicit Shutdown()) unregister it.
class Instance
{
public:
    Instance(EndpointId endpointId, const ElectricalProtectionAlarmCluster::StartupConfiguration & config) :
        mCluster(endpointId, config)
    {}
    ~Instance() { Shutdown(); }

    Instance(const Instance &)             = delete;
    Instance(Instance &&)                  = delete;
    Instance & operator=(const Instance &) = delete;
    Instance & operator=(Instance &&)      = delete;

    CHIP_ERROR Init();
    void Shutdown();

    ElectricalProtectionAlarmCluster & Cluster() { return mCluster.Cluster(); }

private:
    RegisteredServerCluster<ElectricalProtectionAlarmCluster> mCluster;
    bool mRegistered = false;
};

} // namespace ElectricalProtectionAlarm
} // namespace Clusters
} // namespace app
} // namespace chip
