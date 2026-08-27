#include "SimulatedElectricalSensor.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;

namespace {
    const static detail::Structs::MeasurementAccuracyRangeStruct::Type kAccuracyRange{
        .rangeMin = 0, .rangeMax = 10, .percentMax = MakeOptional(uint8_t{ 100 })
    };

    const static detail::Structs::MeasurementAccuracyStruct::Type kAccuracy{
        .measurementType  = detail::MeasurementTypeEnum::kElectricalEnergy,
        .measured         = false,
        .minMeasuredValue = 0,
        .maxMeasuredValue = 10,
        .accuracyRanges   = Span(&kAccuracyRange, 1)
    };
}

SimulatedElectricalSensor::SimulatedElectricalSensor(TimerDelegate & timerDelegate, TestEventTriggerDelegate & testEventTriggerDelegate) :
    ElectricalSensor({
        .electricalEnergyDelegate = *this,
        .electricalEnergyMeasurementFeatureFlags =  BitMask<ElectricalEnergyMeasurement::Feature>(
            ElectricalEnergyMeasurement::Feature::kImportedEnergy, ElectricalEnergyMeasurement::Feature::kExportedEnergy,
            ElectricalEnergyMeasurement::Feature::kCumulativeEnergy, ElectricalEnergyMeasurement::Feature::kPeriodicEnergy,
            ElectricalEnergyMeasurement::Feature::kApparentEnergy, ElectricalEnergyMeasurement::Feature::kReactiveEnergy
        ),
        .electricalEnergyMeasurementOptionalAttributes = ElectricalEnergyMeasurementClusterT::OptionalAttributesSet(
            ElectricalEnergyMeasurementClusterT::OptionalAttributesSet::All()),
        .electricalEnergyMeasurementAccuracyStruct = kAccuracy,

        .electricalPowerDelegate = *this,
        .electricalPowerMeasurementFeatureFlags = BitMask<ElectricalPowerMeasurement::Feature>(
            ElectricalPowerMeasurement::Feature::kAlternatingCurrent, ElectricalPowerMeasurement::Feature::kPolyphasePower,
            ElectricalPowerMeasurement::Feature::kDirectCurrent, ElectricalPowerMeasurement::Feature::kHarmonics,
            ElectricalPowerMeasurement::Feature::kPowerQuality
        ),
        .electricalPowerMeasurementOptionalAttributes = ElectricalPowerMeasurementClusterT::OptionalAttributesSet(
            ElectricalPowerMeasurementClusterT::OptionalAttributesSet::All()),

        .powerTopologyDelegate = *this,
        .powerTopologyFeatures = BitMask<PowerTopology::Feature>(PowerTopology::Feature::kNodeTopology, PowerTopology::Feature::kElectricalCircuit),

        .timerDelegate = timerDelegate
    }), mFakeReadings(timerDelegate), mTestEventTriggerDelegate(testEventTriggerDelegate)
{};

CHIP_ERROR SimulatedElectricalSensor::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                             EndpointComposition composition)
{
    ReturnErrorOnFailure(ElectricalSensor::Register(endpoint, provider, composition));
    mFakeReadings.SetEEMCluster(&ElectricalEnergyMeasurementCluster());
    ReturnErrorOnFailure(mTestEventTriggerDelegate.AddHandler(&mFakeReadings));
    return CHIP_NO_ERROR;
}

void SimulatedElectricalSensor::Unregister(CodeDrivenDataModelProvider & provider)
{
    mTestEventTriggerDelegate.RemoveHandler(&mFakeReadings);
    mFakeReadings.SetEEMCluster(nullptr);
    ElectricalSensor::Unregister(provider);
}

CHIP_ERROR SimulatedElectricalSensor::GetAccuracyByIndex(
    uint8_t index, ElectricalPowerMeasurement::Structs::MeasurementAccuracyStruct::Type & val)
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
