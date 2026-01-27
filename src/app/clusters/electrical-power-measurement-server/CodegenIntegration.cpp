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
#include <app/clusters/electrical-power-measurement-server/CodegenIntegration.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalPowerMeasurement {

ElectricalPowerMeasurementCluster::OptionalAttributesSet
Instance::FromLegacyOptionalAttributes(BitMask<OptionalAttributes> aOptionalAttributes)
{
    ElectricalPowerMeasurementCluster::OptionalAttributesSet attrs;
    attrs.Set<Attributes::Ranges::Id>(aOptionalAttributes.Has(OptionalAttributes::kOptionalAttributeRanges));
    attrs.Set<Attributes::Voltage::Id>(aOptionalAttributes.Has(OptionalAttributes::kOptionalAttributeVoltage));
    attrs.Set<Attributes::ActiveCurrent::Id>(aOptionalAttributes.Has(OptionalAttributes::kOptionalAttributeActiveCurrent));
    attrs.Set<Attributes::ReactiveCurrent::Id>(aOptionalAttributes.Has(OptionalAttributes::kOptionalAttributeReactiveCurrent));
    attrs.Set<Attributes::ApparentCurrent::Id>(aOptionalAttributes.Has(OptionalAttributes::kOptionalAttributeApparentCurrent));
    attrs.Set<Attributes::ReactivePower::Id>(aOptionalAttributes.Has(OptionalAttributes::kOptionalAttributeReactivePower));
    attrs.Set<Attributes::ApparentPower::Id>(aOptionalAttributes.Has(OptionalAttributes::kOptionalAttributeApparentPower));
    attrs.Set<Attributes::RMSVoltage::Id>(aOptionalAttributes.Has(OptionalAttributes::kOptionalAttributeRMSVoltage));
    attrs.Set<Attributes::RMSCurrent::Id>(aOptionalAttributes.Has(OptionalAttributes::kOptionalAttributeRMSCurrent));
    attrs.Set<Attributes::RMSPower::Id>(aOptionalAttributes.Has(OptionalAttributes::kOptionalAttributeRMSPower));
    attrs.Set<Attributes::Frequency::Id>(aOptionalAttributes.Has(OptionalAttributes::kOptionalAttributeFrequency));
    attrs.Set<Attributes::PowerFactor::Id>(aOptionalAttributes.Has(OptionalAttributes::kOptionalAttributePowerFactor));
    attrs.Set<Attributes::NeutralCurrent::Id>(aOptionalAttributes.Has(OptionalAttributes::kOptionalAttributeNeutralCurrent));

    return attrs;
}

CHIP_ERROR Instance::Init()
{
    return CodegenDataModelProvider::Instance().Registry().Register(mCluster.Registration());
}
void Instance::Shutdown()
{
    RETURN_SAFELY_IGNORED CodegenDataModelProvider::Instance().Registry().Unregister(&(mCluster.Cluster()));
}

} // namespace ElectricalPowerMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip

void MatterElectricalPowerMeasurementClusterInitCallback(chip::EndpointId endpoint) {}

void MatterElectricalPowerMeasurementClusterShutdownCallback(chip::EndpointId endpoint) {}
