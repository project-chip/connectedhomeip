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

#include <app/clusters/power-topology-server/CodegenIntegration.h>

#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>

namespace chip::app::Clusters::PowerTopology {

/// Create and register the Power Topology cluster on `endpointId` with the TreeTopology feature.
/// Call from ApplicationInit().
///
/// Power Topology is mandatory on the Electrical Distribution Enclosure device type (0x0517). Of
/// the NODE/TREE/SET choice, TREE is the one that describes an enclosure: it provides power to
/// itself and its child endpoints, which is what the contained breakers are.
CHIP_ERROR PowerTopologyInit(EndpointId endpointId);

/// Unregister and destroy the cluster. Call from ApplicationShutdown().
void PowerTopologyShutdown();

} // namespace chip::app::Clusters::PowerTopology
