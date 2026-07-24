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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/ConcreteAttributePath.h>
#include <app/util/attribute-storage.h>
#include <protocols/interaction_model/Constants.h>

#include "Setpoint.h"
#include "Setpoints.h"

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

/**
Validates a proposed change to a setpoint attribute.  Returns a constraint error if
the change is not allowed, otherwise returns Success.

In the event that the change is allowed, this method may also adjust the values of other setpoints to
maintain the setpoint rules.  The setpoint attributes that are changed by this
operation are returned in the changedAttributes set.

@param setpoints the Setpoints object to use for validation
@param attributeId the attributeId of the setpoint attribute being changed
@param value the proposed value of the setpoint attribute
@param changedAttributes the set of attributes that were changed by this operation

@return Success if the change is allowed, otherwise ConstraintError

@note This is temporary until the conversion to a code-driven cluster.
*/
Protocols::InteractionModel::Status ValidateSetpointChange(Setpoints & setpoints, chip::AttributeId attributeId, temperature value,
                                                           SetpointAttributes & changedAttributes);

/*
Handles the successful write of a setpoint attribute. It re-runs ValidateSetpointChange
and looks for any associated attribute changes. If any other attributes were affected, it writes
the first one to storage. This will re-invoke the PreAttributeChangedCallback for that
attribute, which kicks off the process again.

Eventually, all changed attributes will have been saved in the correct order.

@param attributePath the attribute path of the setpoint attribute that was written

@note This is temporary until the conversion to a code-driven cluster.
*/
void HandleSetpointWrite(const ConcreteAttributePath & attributePath);

/*
Handles the SetpointRaiseLower command. It will adjust the active setpoints either up or down
based on the values provided in commandData.

@param endpointId the endpoint id of the device
@param commandData the command data for the SetpointRaiseLower command

@return Success if the command was handled successfully, otherwise ConstraintError
*/
Protocols::InteractionModel::Status SetpointRaiseLower(const EndpointId endpointId,
                                                       const Commands::SetpointRaiseLower::DecodableType & commandData);
} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
