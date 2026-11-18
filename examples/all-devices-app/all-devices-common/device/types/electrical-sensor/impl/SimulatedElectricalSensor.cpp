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

#include "SimulatedElectricalSensor.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;

namespace {
const static Clusters::detail::Structs::MeasurementAccuracyRangeStruct::Type kAccuracyRange{
    .rangeMin = 0, .rangeMax = 10, .percentMax = MakeOptional(uint8_t{ 100 })
};

const static Clusters::detail::Structs::MeasurementAccuracyStruct::Type kAccuracy{
    .measurementType  = Clusters::detail::MeasurementTypeEnum::kElectricalEnergy,
    .measured         = false,
    .minMeasuredValue = 0,
    .maxMeasuredValue = 10,
    .accuracyRanges   = Span(&kAccuracyRange, 1)
};
} // namespace

namespace {

BitMask<ElectricalPowerMeasurement::Feature> PowerMeasurementFeaturesForRole(SimulatedElectricalSensorDelegate::Role role)
{
    using Feature = ElectricalPowerMeasurement::Feature;
    switch (role)
    {
    case SimulatedElectricalSensorDelegate::Role::kGrid:
        return BitMask<Feature>(Feature::kAlternatingCurrent);
    case SimulatedElectricalSensorDelegate::Role::kBattery:
        return BitMask<Feature>(Feature::kDirectCurrent);
    case SimulatedElectricalSensorDelegate::Role::kGeneric:
    default:
        return BitMask<Feature>(Feature::kAlternatingCurrent, Feature::kPolyphasePower, Feature::kDirectCurrent,
                                Feature::kHarmonics, Feature::kPowerQuality);
    }
}

ElectricalPowerMeasurement::ElectricalPowerMeasurementCluster::OptionalAttributesSet
PowerMeasurementOptionalAttributesForRole(SimulatedElectricalSensorDelegate::Role role)
{
    using OptionalAttributesSet = ElectricalPowerMeasurement::ElectricalPowerMeasurementCluster::OptionalAttributesSet;

    switch (role)
    {
    case SimulatedElectricalSensorDelegate::Role::kGrid: {
        OptionalAttributesSet attributes(OptionalAttributesSet::All());
        attributes.Set<ElectricalPowerMeasurement::Attributes::NeutralCurrent::Id>(false);
        return attributes;
    }
    case SimulatedElectricalSensorDelegate::Role::kBattery: {
        OptionalAttributesSet attributes;
        attributes.Set<ElectricalPowerMeasurement::Attributes::Ranges::Id>();
        attributes.Set<ElectricalPowerMeasurement::Attributes::Voltage::Id>();
        attributes.Set<ElectricalPowerMeasurement::Attributes::ActiveCurrent::Id>();
        return attributes;
    }
    case SimulatedElectricalSensorDelegate::Role::kGeneric:
    default:
        return OptionalAttributesSet(OptionalAttributesSet::All());
    }
}

} // namespace

SimulatedElectricalSensorDelegate::SimulatedElectricalSensorDelegate(TimerDelegate & timerDelegate,
                                                                     TestEventTriggerDelegate & testEventTriggerDelegate,
                                                                     Role role) :
    mRole(role),
    mFakeReadings(timerDelegate), mTestEventTriggerDelegate(testEventTriggerDelegate)
{}

ElectricalSensor::Config
SimulatedElectricalSensorDelegate::GetConfig(TimerDelegate & timerDelegate,
                                             Span<const Clusters::Globals::Structs::SemanticTagStruct::Type> tagList)
{
    return { .electricalEnergyDelegate                = *this,
             .electricalEnergyMeasurementFeatureFlags = BitMask<ElectricalEnergyMeasurement::Feature>(
                 ElectricalEnergyMeasurement::Feature::kImportedEnergy, ElectricalEnergyMeasurement::Feature::kExportedEnergy,
                 ElectricalEnergyMeasurement::Feature::kCumulativeEnergy, ElectricalEnergyMeasurement::Feature::kPeriodicEnergy,
                 ElectricalEnergyMeasurement::Feature::kApparentEnergy, ElectricalEnergyMeasurement::Feature::kReactiveEnergy),
             .electricalEnergyMeasurementOptionalAttributes =
                 ElectricalEnergyMeasurement::ElectricalEnergyMeasurementCluster::OptionalAttributesSet(
                     ElectricalEnergyMeasurement::ElectricalEnergyMeasurementCluster::OptionalAttributesSet::All()),
             .electricalEnergyMeasurementAccuracyStruct = kAccuracy,

             .electricalPowerDelegate                      = *this,
             .electricalPowerMeasurementFeatureFlags       = PowerMeasurementFeaturesForRole(mRole),
             .electricalPowerMeasurementOptionalAttributes = PowerMeasurementOptionalAttributesForRole(mRole),

             .powerTopologyDelegate = *this,
             .powerTopologyFeatures =
                 BitMask<PowerTopology::Feature>(PowerTopology::Feature::kSetTopology, PowerTopology::Feature::kDynamicPowerFlow),

             .timerDelegate = timerDelegate,
             .tagList       = tagList };
}

SimulatedElectricalSensor::SimulatedElectricalSensor(TimerDelegate & timerDelegate,
                                                     TestEventTriggerDelegate & testEventTriggerDelegate,
                                                     Span<const Clusters::Globals::Structs::SemanticTagStruct::Type> tagList,
                                                     Role role) :
    SimulatedElectricalSensorDelegate(timerDelegate, testEventTriggerDelegate, role),
    ElectricalSensor(GetConfig(timerDelegate, tagList))
{}

Clusters::ElectricalPowerMeasurement::PowerModeEnum SimulatedElectricalSensorDelegate::GetPowerMode()
{
    return mRole == Role::kBattery ? Clusters::ElectricalPowerMeasurement::PowerModeEnum::kDc
                                   : Clusters::ElectricalPowerMeasurement::PowerModeEnum::kAc;
}

CHIP_ERROR SimulatedElectricalSensor::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                               EndpointComposition composition)
{
    ReturnErrorOnFailure(ElectricalSensor::Register(endpoint, provider, composition));
    DeviceRegistrationTransaction transaction(*this, provider);
    ReturnErrorOnFailure(SimulatedElectricalSensorDelegate::Register(*this));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void SimulatedElectricalSensor::Unregister(CodeDrivenDataModelProvider & provider)
{
    SimulatedElectricalSensorDelegate::Unregister();
    ElectricalSensor::Unregister(provider);
}

CHIP_ERROR SimulatedElectricalSensorDelegate::Register(ElectricalSensor & sensor)
{
    ReturnErrorOnFailure(mTestEventTriggerDelegate.AddHandler(&mFakeReadings));
    mFakeReadings.SetEndpointId(sensor.GetEndpointId());
    mFakeReadings.SetEEMCluster(&sensor.ElectricalEnergyMeasurementCluster());
    return CHIP_NO_ERROR;
}

void SimulatedElectricalSensorDelegate::Unregister()
{
    mFakeReadings.StopFakeReadings();
    mFakeReadings.SetEEMCluster(nullptr);
    mFakeReadings.SetEndpointId(kInvalidEndpointId);
    mTestEventTriggerDelegate.RemoveHandler(&mFakeReadings);
}

CHIP_ERROR
SimulatedElectricalSensorDelegate::GetAccuracyByIndex(uint8_t index,
                                                      ElectricalPowerMeasurement::Structs::MeasurementAccuracyStruct::Type & val)
{
    static ElectricalPowerMeasurement::Structs::MeasurementAccuracyRangeStruct::Type accuracyRange = { .rangeMin = 0,
                                                                                                       .rangeMax = 10 };
    val = { .measurementType  = ElectricalPowerMeasurement::MeasurementTypeEnum::kActivePower,
            .measured         = false,
            .minMeasuredValue = 0,
            .maxMeasuredValue = 10,
            .accuracyRanges   = Span(&accuracyRange, 1) };
    return index == 0 ? CHIP_NO_ERROR : CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}
