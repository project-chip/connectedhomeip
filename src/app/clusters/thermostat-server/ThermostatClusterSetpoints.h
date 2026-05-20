/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/ConcreteAttributePath.h>
#include <app/util/attribute-storage.h>
#include <protocols/interaction_model/Constants.h>

#include "Setpoint.h"
#include "Setpoints.h"

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

Protocols::InteractionModel::Status HandleSetpointChange(Setpoints & setpoints, chip::AttributeId attributeId, temperature value,
                                                         SetpointAttributes & changedAttributes);

Protocols::InteractionModel::Status SetpointRaiseLower(const EndpointId endpointId,
                                                       const Commands::SetpointRaiseLower::DecodableType & commandData);
} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
