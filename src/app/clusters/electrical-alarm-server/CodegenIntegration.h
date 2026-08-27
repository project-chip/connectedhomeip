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

#include <app/clusters/electrical-alarm-server/ElectricalAlarmCluster.h>
#include <app/clusters/electrical-alarm-server/electrical-alarm-delegate.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalAlarm {

class Instance
{
public:
    Instance(EndpointId aEndpointId, Delegate * aDelegate, BitMask<Feature> aFeatures) :
        mCluster(ElectricalAlarmCluster::Config{
            .endpointId = aEndpointId,
            .delegate   = aDelegate,
            .features   = aFeatures,
        })
    {}
    ~Instance() { Shutdown(); }

    CHIP_ERROR Init();
    void Shutdown();

private:
    RegisteredServerCluster<ElectricalAlarmCluster> mCluster;
};

} // namespace ElectricalAlarm
} // namespace Clusters
} // namespace app
} // namespace chip
