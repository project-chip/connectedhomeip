/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/af-enums.h>
#include <app/clusters/operational-state-server/operational-state-server.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace MicrowaveOvenControl {


/** @brief Set Operational State instance from the application layer.
 *
 * Set operational state instance
 *
 * @param endpoint    Endpoint that is being initialized
 * @param aInstance    Instance of Operaiotn State
 */
void SetOPInstance(EndpointId aEndpoint, OperationalState::Instance * aInstance);

/** @brief Get Operational State instance for Microwave Oven Control Cluster to know what the current state is.
 *
 * Get operational state instance
 *
 * @param endpoint    Endpoint that is being initialized
 * @return Return a pointer of operational state instance which is being initialized.
 */
OperationalState::Instance * GetOPInstance(EndpointId aEndpoint);


} // namespace MicrowaveOvenControl
} // namespace Clusters
} // namespace app
} // namespace chip