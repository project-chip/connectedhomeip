/*
 *
 *    Copyright (c) 2024-2025 Project CHIP Authors
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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <clusters/Thermostat/Metadata.h>
#include <lib/support/Assertions.h>
#include <protocols/interaction_model/StatusCode.h>

#include "Setpoint.h"
#include "Temperature.h"
#include "ThermostatCluster.h"
#include "ThermostatClusterSetpoints.h"

#include "Setpoints.h"

using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::app::Clusters::Thermostat::Commands;
using namespace chip::Protocols::InteractionModel;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

Status ThermostatAutoSetpoints::Delegate::GetMinDeadband(temperature & minDeadband) const
{
    return Status::UnsupportedAttribute;
}

Status ThermostatAutoSetpoints::LoadDeadband(temperature & deadband) {
    auto status = mDelegate.GetMinDeadband(deadband);
    VerifyOrReturnValue(status == Status::Success, status);
    if (deadband < kMinDeadBand * 10 || deadband > kMaxDeadBand * 10)
    {
        ChipLogError(Zcl, "Invalid value for Deadband: %d", deadband);
        return Status::ConstraintError;
    }
    return Status::Success;
}

Status ThermostatAutoSetpoints::LoadSetpoints(Setpoints & setpoints) {
    temperature deadband;
    auto status = LoadDeadband(deadband);
    VerifyOrReturnValue(status == Status::Success, status);
    setpoints.deadBand = deadband;
    return Status::Success;
}

std::optional<DataModel::ActionReturnStatus> ThermostatAutoSetpoints::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                                  AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case MinSetpointDeadBand::Id: {
        temperature deadband;
        auto status = LoadDeadband(deadband);
        VerifyOrReturnValue(status == Status::Success, status);
        return encoder.Encode(static_cast<int8_t>(deadband / 10));
    }
    default:
        return std::nullopt;
    }
}

std::optional<DataModel::ActionReturnStatus> ThermostatAutoSetpoints::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                                  AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
    case MinSetpointDeadBand::Id:
        {
            int16_t db;
            ReturnErrorOnFailure(decoder.Decode(db));
            if (db < kMinDeadBand || db > kMaxDeadBand)
            {
                ChipLogError(Zcl, "Invalid value for Deadband: %d", db);
                return Status::ConstraintError;
            }
            // Note: for backwards compatibility, writes to this attribute are allowed (as long as the value is valid) but ignored
            return Status::Success;
        }
    default:
        return std::nullopt;
    }
}

CHIP_ERROR ThermostatAutoSetpoints::Attributes(const ConcreteClusterPath & path,
                                               ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    return builder.AppendElements({ MinSetpointDeadBand::kMetadataEntry });
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
