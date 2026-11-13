/**
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "Setpoints.h"
#include "Temperature.h"
#include "ThermostatClusterAttributes.h"
#include "ThermostatClusterBase.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeValueEncoder.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model-provider/OperationTypes.h>
#include <lib/support/BitMask.h>

#include <optional>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

class ThermostatSetpointsBase
{
public:
    ThermostatSetpointsBase(ThermostatClusterBase & cluster) : mCluster(cluster) {}
    virtual ~ThermostatSetpointsBase() = default;

    virtual Protocols::InteractionModel::Status LoadSetpoints(Setpoints & setpoints) = 0;

    const OptionalAttributes & GetOptionalAttributes() const;
    const BitFlags<Thermostat::Feature> & Features() const;

    /**
     * @param[in] initiatedByOperationalSetpointWrite True if this save was triggered by a direct write to (or
     *            SetpointRaiseLower command targeting) one of the four operational setpoints, as opposed to a
     *            setpoint limit write that incidentally clamped an operational setpoint via Setpoints::Fix().
     *            Only changes with this set to true are reported through the SetpointChange* tracking attributes.
     * @param[in] initiatingAttributeId The specific operational setpoint attribute that was directly written (or
     *            targeted by a single-mode SetpointRaiseLower command), if any. Used to disambiguate which
     *            setpoint's delta to report when the operation also cascades into a second operational setpoint
     *            via Setpoints::Fix() (e.g. an auto-mode deadband adjustment). Left as std::nullopt when more than
     *            one operational setpoint is intentionally targeted by the same operation (SetpointRaiseLower with
     *            mode kBoth), in which case an explicit fixed-order policy is used instead.
     */
    virtual Protocols::InteractionModel::Status SaveSetpoints(const Setpoints & setpoints, SetpointAttributes changedAttributes,
                                                              bool initiatedByOperationalSetpointWrite,
                                                              std::optional<AttributeId> initiatingAttributeId) = 0;
    void GenerateSetpointEvent(AttributeId attributeId, temperature oldTemp, temperature newTemp) const;
    void NotifyAttributesChanged(const SetpointAttributes & changedAttributes);

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler);

protected:
    ThermostatClusterBase & mCluster;

    virtual Setpoints GetSetpoints();

    /**
     * @brief Updates the SetpointChangeSource, SetpointChangeAmount and SetpointChangeSourceTimestamp attributes
     *        whenever one of the operational setpoints (Occupied/UnoccupiedHeating/CoolingSetpoint) changes value.
     *        Each attribute is only computed and reported if its corresponding OptionalAttributes flag is set.
     *
     * @param[in] oldSetpoints The setpoints prior to the change being applied.
     * @param[in] newSetpoints The setpoints after the change has been applied.
     * @param[in] changedAttributes The set of setpoint attributes that were changed by this operation.
     * @param[in] initiatedByOperationalSetpointWrite See SaveSetpoints(). A limit write that merely clamps an
     *            operational setpoint via Setpoints::Fix() must not be reported as a setpoint change.
     * @param[in] initiatingAttributeId See SaveSetpoints().
     */
    void UpdateSetpointChangeAttributes(const Setpoints & oldSetpoints, const Setpoints & newSetpoints,
                                        const SetpointAttributes & changedAttributes, bool initiatedByOperationalSetpointWrite,
                                        std::optional<AttributeId> initiatingAttributeId);

    /**
     * @brief True if attributeId is one of the four operational setpoints (Occupied/UnoccupiedHeating/CoolingSetpoint),
     *        as opposed to e.g. a setpoint limit attribute.
     */
    static bool IsOperationalSetpointAttribute(AttributeId attributeId);

    SetpointChangeSourceEnum mSetpointChangeSource = SetpointChangeSourceEnum::kManual;
    DataModel::Nullable<int16_t> mSetpointChangeAmount;
    uint32_t mSetpointChangeSourceTimestamp = 0;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
