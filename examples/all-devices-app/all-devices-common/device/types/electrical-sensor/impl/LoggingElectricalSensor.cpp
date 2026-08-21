#include "LoggingElectricalSensor.h"

#include <app/clusters/electrical-energy-measurement-server/EnergyReportingTestEventTriggerHandler.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;

namespace {
static FakeReadings gFakeReadings;
}

void SetTestEventTrigger_FakeReadingsLoadStart()
{
    int64_t power_mW              = 1'000'000; // Fake load 1000 W
    uint32_t powerRandomness_mW   = 20'000;    // randomness 20W
    int64_t voltage_mV            = 230'000;   // Fake Voltage 230V
    uint32_t voltageRandomness_mV = 1'000;     // randomness 1V
    int64_t current_mA            = 4'348;     // Fake Current (at 1kW@230V = 4.3478 Amps)
    uint32_t currentRandomness_mA = 500;       // randomness 500mA
    uint8_t interval_s            = 2;         // 2s updates
    bool reset                    = true;
    gFakeReadings.StartFakeReadings(power_mW, powerRandomness_mW, voltage_mV, voltageRandomness_mV, current_mA,
                                    currentRandomness_mA, interval_s, reset);
}

void SetTestEventTrigger_FakeReadingsGeneratorStart()
{
    int64_t power_mW              = -3'000'000; // Fake Generator -3000 W
    uint32_t powerRandomness_mW   = 20'000;     // randomness 20W
    int64_t voltage_mV            = 230'000;    // Fake Voltage 230V
    uint32_t voltageRandomness_mV = 1'000;      // randomness 1V
    int64_t current_mA            = -13'043;    // Fake Current (at -3kW@230V = -13.0434 Amps)
    uint32_t currentRandomness_mA = 500;        // randomness 500mA
    uint8_t interval_s            = 5;          // 5s updates
    bool reset                    = true;
    gFakeReadings.StartFakeReadings(power_mW, powerRandomness_mW, voltage_mV, voltageRandomness_mV, current_mA,
                                    currentRandomness_mA, interval_s, reset);
}

void SetTestEventTrigger_FakeReadingsStop()
{
    gFakeReadings.StopFakeReadings();
}

bool HandleEnergyReportingTestEventTrigger(uint64_t eventTrigger)
{
    EnergyReportingTrigger trigger = static_cast<EnergyReportingTrigger>(eventTrigger);

    switch (trigger)
    {
    case EnergyReportingTrigger::kFakeReadingsStop:
        ChipLogProgress(Support, "[EnergyReporting-Test-Event] => Stop Fake load");
        SetTestEventTrigger_FakeReadingsStop();
        break;
    case EnergyReportingTrigger::kFakeReadingsLoadStart_1kW_2s:
        ChipLogProgress(Support, "[EnergyReporting-Test-Event] => Start Fake load 1kW @2s Import");
        SetTestEventTrigger_FakeReadingsLoadStart();
        break;
    case EnergyReportingTrigger::kFakeReadingsGenStart_3kW_5s:
        ChipLogProgress(Support, "[EnergyReporting-Test-Event] => Start Fake generator 3kW @5s Export");
        SetTestEventTrigger_FakeReadingsGeneratorStart();
        break;

    default:
        return false;
    }

    return true;
}

LoggingElectricalSensor::LoggingElectricalSensor(TimerDelegate & timerDelegate) :
    ElectricalSensor(timerDelegate, *this, *this, *this)
{
    gFakeReadings.SetTimerDelegate(timerDelegate);
};

CHIP_ERROR LoggingElectricalSensor::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                             EndpointComposition composition)
{
    ReturnErrorOnFailure(ElectricalSensor::Register(endpoint, provider, composition));
    gFakeReadings.SetEEMCluster(ElectricalEnergyMeasurementCluster());
    return CHIP_NO_ERROR;
}

// define instead of constexpr static auto because the latter failes on Darwin.
#define MSG "%s device: %s cluster: %s called"
// for the index format specifier we use the largest unsigned type a size_t can be, and cast everything to it.
#define MSG_WITH_INDEX "%s device: %s cluster: %s called with index %llu"

constexpr static auto device                   = "ElectricalSensor";
constexpr static auto energyMeasurementCluster = "ElectricalEnergyMeasurement";
constexpr static auto powerMeasurementCluster  = "ElectricalPowerMeasurement";
constexpr static auto powerTopologyCluster     = "PowerTopology";

// Clusters::ElectricalEnergyMeasurement::Delegate implementation

Nullable<int64_t> LoggingElectricalSensor::GetCumulativeEnergyImported() /* override */
{
    ChipLogProgress(DeviceLayer, MSG, device, energyMeasurementCluster, "GetCumulativeEnergyImported");
    return MakeNullable(gFakeReadings.GetCumulativeEnergyImported());
}
Nullable<int64_t> LoggingElectricalSensor::GetCumulativeEnergyExported() /* override */
{
    ChipLogProgress(DeviceLayer, MSG, device, energyMeasurementCluster, "GetCumulativeEnergyExported");
    return MakeNullable(gFakeReadings.GetCumulativeEnergyExported());
}
Nullable<int64_t> LoggingElectricalSensor::GetPeriodicEnergyImported() /* override */
{
    ChipLogProgress(DeviceLayer, MSG, device, energyMeasurementCluster, "GetPeriodicEnergyImported");
    return MakeNullable(gFakeReadings.GetPeriodicEnergyImported());
}
Nullable<int64_t> LoggingElectricalSensor::GetPeriodicEnergyExported() /* override */
{
    ChipLogProgress(DeviceLayer, MSG, device, energyMeasurementCluster, "GetPeriodicEnergyExported");
    return MakeNullable(gFakeReadings.GetPeriodicEnergyExported());
}

// Clusters::ElectricalPowerMeasurement::Delegate implementation

Clusters::ElectricalPowerMeasurement::PowerModeEnum LoggingElectricalSensor::GetPowerMode() /* override */
{
    ChipLogProgress(DeviceLayer, MSG, device, powerMeasurementCluster, "GetPowerMode");
    return Clusters::ElectricalPowerMeasurement::PowerModeEnum::kAc;
}
uint8_t LoggingElectricalSensor::GetNumberOfMeasurementTypes() /* override */
{
    ChipLogProgress(DeviceLayer, MSG, device, powerMeasurementCluster, "GetNumberOfMeasurementTypes");
    return 1;
}

CHIP_ERROR LoggingElectricalSensor::StartAccuracyRead() /* override */
{
    ChipLogProgress(DeviceLayer, MSG, device, powerMeasurementCluster, "StartAccuracyRead");
    return CHIP_NO_ERROR;
}
CHIP_ERROR LoggingElectricalSensor::GetAccuracyByIndex(
    uint8_t index, Clusters::ElectricalPowerMeasurement::Structs::MeasurementAccuracyStruct::Type & val) /* override */
{
    ChipLogProgress(DeviceLayer, MSG_WITH_INDEX, device, powerMeasurementCluster, "GetAccuracyByIndex",
                    static_cast<unsigned long long>(index));
    static Clusters::ElectricalPowerMeasurement::Structs::MeasurementAccuracyRangeStruct::Type accuracyRange = { .rangeMin = 0,
                                                                                                                 .rangeMax = 10 };
    val = { .measurementType  = Clusters::ElectricalPowerMeasurement::MeasurementTypeEnum::kActivePower,
            .measured         = false,
            .minMeasuredValue = 0,
            .maxMeasuredValue = 10,
            .accuracyRanges   = Span(&accuracyRange, 1) };
    return index == 0 ? CHIP_NO_ERROR : CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}
CHIP_ERROR LoggingElectricalSensor::EndAccuracyRead() /* override */
{
    ChipLogProgress(DeviceLayer, MSG, device, powerMeasurementCluster, "EndAccuracyRead");
    return CHIP_NO_ERROR;
}

CHIP_ERROR LoggingElectricalSensor::StartRangesRead() /* override */
{
    ChipLogProgress(DeviceLayer, MSG, device, powerMeasurementCluster, "StartRangesRead");
    return CHIP_NO_ERROR;
}
CHIP_ERROR LoggingElectricalSensor::GetRangeByIndex(
    uint8_t index, Clusters::ElectricalPowerMeasurement::Structs::MeasurementRangeStruct::Type &) /* override */
{
    ChipLogProgress(DeviceLayer, MSG_WITH_INDEX, device, powerMeasurementCluster, "GetRangeByIndex",
                    static_cast<unsigned long long>(index));
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}
CHIP_ERROR LoggingElectricalSensor::EndRangesRead() /* override */
{
    ChipLogProgress(DeviceLayer, MSG, device, powerMeasurementCluster, "EndRangesRead");
    return CHIP_NO_ERROR;
}

CHIP_ERROR LoggingElectricalSensor::StartHarmonicCurrentsRead() /* override */
{
    ChipLogProgress(DeviceLayer, MSG, device, powerMeasurementCluster, "StartHarmonicCurrentsRead");
    return CHIP_NO_ERROR;
}
CHIP_ERROR LoggingElectricalSensor::GetHarmonicCurrentsByIndex(
    uint8_t index, Clusters::ElectricalPowerMeasurement::Structs::HarmonicMeasurementStruct::Type &) /* override */
{
    ChipLogProgress(DeviceLayer, MSG_WITH_INDEX, device, powerMeasurementCluster, "GetHarmonicCurrentsByIndex",
                    static_cast<unsigned long long>(index));
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}
CHIP_ERROR LoggingElectricalSensor::EndHarmonicCurrentsRead() /* override */
{
    ChipLogProgress(DeviceLayer, MSG, device, powerMeasurementCluster, "EndHarmonicCurrentsRead");
    return CHIP_NO_ERROR;
}

CHIP_ERROR LoggingElectricalSensor::StartHarmonicPhasesRead() /* override */
{
    ChipLogProgress(DeviceLayer, MSG, device, powerMeasurementCluster, "StartHarmonicPhasesRead");
    return CHIP_NO_ERROR;
}
CHIP_ERROR LoggingElectricalSensor::GetHarmonicPhasesByIndex(
    uint8_t index, Clusters::ElectricalPowerMeasurement::Structs::HarmonicMeasurementStruct::Type &) /* override */
{
    ChipLogProgress(DeviceLayer, MSG_WITH_INDEX, device, powerMeasurementCluster, "GetHarmonicPhasesByIndex",
                    static_cast<unsigned long long>(index));
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}
CHIP_ERROR LoggingElectricalSensor::EndHarmonicPhasesRead() /* override */
{
    ChipLogProgress(DeviceLayer, MSG, device, powerMeasurementCluster, "EndHarmonicPhasesRead");
    return CHIP_NO_ERROR;
}

Nullable<int64_t> LoggingElectricalSensor::GetVoltage() /* override */
{
    ChipLogProgress(DeviceLayer, MSG, device, powerMeasurementCluster, "GetVoltage");
    return MakeNullable(gFakeReadings.GetVoltage());
}
Nullable<int64_t> LoggingElectricalSensor::GetActiveCurrent() /* override */
{
    ChipLogProgress(DeviceLayer, MSG, device, powerMeasurementCluster, "GetActiveCurrent");
    return MakeNullable(gFakeReadings.GetActiveCurrent());
}
Nullable<int64_t> LoggingElectricalSensor::GetReactiveCurrent() /* override */
{
    ChipLogProgress(DeviceLayer, MSG, device, powerMeasurementCluster, "GetReactiveCurrent");
    return NullNullable;
}
Nullable<int64_t> LoggingElectricalSensor::GetApparentCurrent() /* override */
{
    ChipLogProgress(DeviceLayer, MSG, device, powerMeasurementCluster, "GetApparentCurrent");
    return NullNullable;
}
Nullable<int64_t> LoggingElectricalSensor::GetActivePower() /* override */
{
    ChipLogProgress(DeviceLayer, MSG, device, powerMeasurementCluster, "GetActivePower");
    return MakeNullable(gFakeReadings.GetActivePower());
}
Nullable<int64_t> LoggingElectricalSensor::GetReactivePower() /* override */
{
    ChipLogProgress(DeviceLayer, MSG, device, powerMeasurementCluster, "GetReactivePower");
    return NullNullable;
}
Nullable<int64_t> LoggingElectricalSensor::GetApparentPower() /* override */
{
    ChipLogProgress(DeviceLayer, MSG, device, powerMeasurementCluster, "GetApparentPower");
    return NullNullable;
}
Nullable<int64_t> LoggingElectricalSensor::GetRMSVoltage() /* override */
{
    ChipLogProgress(DeviceLayer, MSG, device, powerMeasurementCluster, "GetRMSVoltage");
    return NullNullable;
}
Nullable<int64_t> LoggingElectricalSensor::GetRMSCurrent() /* override */
{
    ChipLogProgress(DeviceLayer, MSG, device, powerMeasurementCluster, "GetRMSCurrent");
    return NullNullable;
}
Nullable<int64_t> LoggingElectricalSensor::GetRMSPower() /* override */
{
    ChipLogProgress(DeviceLayer, MSG, device, powerMeasurementCluster, "GetRMSPower");
    return NullNullable;
}
Nullable<int64_t> LoggingElectricalSensor::GetFrequency() /* override */
{
    ChipLogProgress(DeviceLayer, MSG, device, powerMeasurementCluster, "GetFrequency");
    return NullNullable;
}
Nullable<int64_t> LoggingElectricalSensor::GetPowerFactor() /* override */
{
    ChipLogProgress(DeviceLayer, MSG, device, powerMeasurementCluster, "GetPowerFactor");
    return NullNullable;
}
Nullable<int64_t> LoggingElectricalSensor::GetNeutralCurrent() /* override */
{
    ChipLogProgress(DeviceLayer, MSG, device, powerMeasurementCluster, "GetNeutralCurrent");
    return NullNullable;
}

// Clusters::PowerTopology::Delegate implementation

CHIP_ERROR LoggingElectricalSensor::GetAvailableEndpointAtIndex(size_t index, EndpointId & endpointId) /* override */
{
    ChipLogProgress(DeviceLayer, MSG_WITH_INDEX, device, powerTopologyCluster, "GetAvailableEndpointAtIndex",
                    static_cast<unsigned long long>(index));
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}
CHIP_ERROR LoggingElectricalSensor::GetActiveEndpointAtIndex(size_t index, EndpointId & endpointId) /* override */
{
    ChipLogProgress(DeviceLayer, MSG_WITH_INDEX, device, powerTopologyCluster, "GetActiveEndpointAtIndex",
                    static_cast<unsigned long long>(index));
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}
#undef MSG
#undef MSG_WITH_INDEX
