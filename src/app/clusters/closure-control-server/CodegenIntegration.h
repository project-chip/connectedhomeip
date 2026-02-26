/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-enums.h>
#include <app/clusters/closure-control-server/ClosureControlCluster.h>
#include <app/clusters/closure-control-server/ClosureControlClusterDelegate.h>
#include <app/clusters/closure-control-server/ClosureControlClusterMatterContext.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureControl {

ClosureControlCluster & GetInstance(EndpointId endpointId);

void MatterClosureControlSetDelegate(ClosureControlClusterDelegate & delegate);

void MatterClosureControlSetConformance(const ClusterConformance & conformance);

void MatterClosureControlSetInitParams(const ClusterInitParameters & initParams);

} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip
