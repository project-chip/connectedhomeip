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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/reporting/reporting.h>
#include <app/server-cluster/DefaultServerCluster.h>

namespace chip::app::Clusters {
namespace ThreadBorderRouterManagement {

class ThreadBorderRouterManagementCluster : public DefaultServerCluster
{
public:
    struct Config
    {
        constexpr Config(EndpointId endpoint) : endpointId(endpoint) {}
        EndpointId endpointId;
    };

    ThreadBorderRouterManagementCluster(const Config & config) : DefaultServerCluster({ config.endpointId, ThreadBorderRouterManagement::Id }) {}

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
};

} // namespace ThreadBorderRouterManagement
} // namespace chip::app::Clusters
