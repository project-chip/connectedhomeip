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

#include <electrical-protection-alarm-stub.h>

#include <app/clusters/electrical-protection-alarm-server/ElectricalProtectionAlarmTestEventTriggerHandler.h>
#include <clusters/ElectricalProtectionAlarm/Structs.h>
#include <lib/support/CodeUtils.h>

#include <memory>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ElectricalProtectionAlarm;

namespace {

BitMask<AlarmBitmap> AllSupportedAlarms()
{
    BitMask<AlarmBitmap> bits;
    bits.Set(AlarmBitmap::kShortCircuitFault)
        .Set(AlarmBitmap::kOverLoadFault)
        .Set(AlarmBitmap::kOverVoltageFault)
        .Set(AlarmBitmap::kVoltageSurgeFault)
        .Set(AlarmBitmap::kResidualCurrentFault)
        .Set(AlarmBitmap::kArcFault)
        .Set(AlarmBitmap::kSelfTest);
    return bits;
}

// Representative ratings for a 100 A / 240 V residential panel breaker. Currents are amperage-mA
// and voltages are voltage-mV per the cluster definition; ResponseTime is in nanoseconds. These are
// illustrative fixed values, not measurements: the point is that every rating attribute encodes a
// populated struct so the attribute tests exercise their structure validation instead of taking the
// null path.
constexpr int64_t kRatedCurrentMa        = 100000;   // 100 A
constexpr int64_t kUltimateMaxCurrentMa  = 10000000; // 10 kA interrupting capacity
constexpr int64_t kServiceMaxCurrentMa   = 6000000;  // 6 kA service short-circuit rating
constexpr int64_t kMaxOperatingVoltageMv = 253000;   // 240 V +5%
constexpr int64_t kOverVoltageTripMv     = 275000;
constexpr uint64_t kResponseTimeNs       = 25;   // typical MOV clamping response
constexpr uint64_t kEnergyAbsorptionJ    = 1200; // surge energy absorption capability

Structs::ArcFaultRatingsStruct::Type MakeArcFaultRating()
{
    Structs::ArcFaultRatingsStruct::Type r;
    r.seriesArcCurrentSensitivity   = MakeOptional<int64_t>(5000);  // 5 A
    r.parallelArcCurrentSensitivity = MakeOptional<int64_t>(30000); // 30 A
    BitMask<ArcCauseBitmap> causes;
    causes.Set(ArcCauseBitmap::kSeries).Set(ArcCauseBitmap::kParallelToNeutral).Set(ArcCauseBitmap::kParallelToGround);
    r.supportedArcCauses = MakeOptional(causes);
    return r;
}

Structs::OverLoadRatingsStruct::Type MakeOverLoadRating()
{
    Structs::OverLoadRatingsStruct::Type r;
    BitMask<CurrentTripMechanismBitmap> mech;
    mech.Set(CurrentTripMechanismBitmap::kThermal).Set(CurrentTripMechanismBitmap::kMagnetic);
    r.tripCurrent        = MakeOptional(kRatedCurrentMa);
    r.tripCurve          = MakeOptional(CurrentTripCurveEnum::kTypeC);
    r.tripMechanism      = MakeOptional(mech);
    r.ultimateMaxCurrent = MakeOptional(kUltimateMaxCurrentMa);
    r.serviceMaxCurrent  = MakeOptional(kServiceMaxCurrentMa);
    return r;
}

Structs::OverVoltageRatingsStruct::Type MakeOverVoltageRating()
{
    Structs::OverVoltageRatingsStruct::Type r;
    BitMask<VoltageTripMechanismBitmap> mech;
    mech.Set(VoltageTripMechanismBitmap::kMov);
    r.tripMechanism                 = MakeOptional(mech);
    r.tripVoltage                   = MakeOptional(kOverVoltageTripMv);
    r.maxContinuousOperatingVoltage = MakeOptional(kMaxOperatingVoltageMv);
    r.responseTime                  = MakeOptional(kResponseTimeNs);
    return r;
}

Structs::SurgeProtectionRatingsStruct::Type MakeSurgeProtectionRating()
{
    Structs::SurgeProtectionRatingsStruct::Type r;
    BitMask<VoltageTripMechanismBitmap> mech;
    mech.Set(VoltageTripMechanismBitmap::kMov);
    BitMask<SurgeProtectionClassBitmap> cls;
    cls.Set(SurgeProtectionClassBitmap::kClassII);
    BitMask<SurgeProtectionTypeBitmap> type;
    type.Set(SurgeProtectionTypeBitmap::kType2);
    r.tripMechanism                  = MakeOptional(mech);
    r.protectionClass                = MakeOptional(cls);
    r.protectionType                 = MakeOptional(type);
    r.maxContinuousOperatingVoltage  = MakeOptional(kOverVoltageTripMv);
    r.maxVoltageProtection           = MakeOptional<int64_t>(1200000); // 1.2 kV let-through
    r.maxTemporaryVoltage            = MakeOptional<int64_t>(335000);
    r.nominalDischargeCurrent        = MakeOptional<int64_t>(20000000); // 20 kA
    r.maximumDischargeCurrent        = MakeOptional<int64_t>(40000000); // 40 kA
    r.ratedShortCircuitCurrent       = MakeOptional(kUltimateMaxCurrentMa);
    r.ratedShortTimeWithstandCurrent = MakeOptional(kServiceMaxCurrentMa);
    r.energyAbsorptionCapability     = MakeOptional(kEnergyAbsorptionJ);
    r.responseTime                   = MakeOptional(kResponseTimeNs);
    return r;
}

Structs::ShortCircuitRatingsStruct::Type MakeShortCircuitRating()
{
    Structs::ShortCircuitRatingsStruct::Type r;
    BitMask<CurrentTripMechanismBitmap> mech;
    mech.Set(CurrentTripMechanismBitmap::kMagnetic);
    r.tripCurrent        = MakeOptional<int64_t>(1000000); // 1 kA magnetic trip
    r.tripMechanism      = MakeOptional(mech);
    r.tripCurve          = MakeOptional(CurrentTripCurveEnum::kTypeC);
    r.ultimateMaxCurrent = MakeOptional(kUltimateMaxCurrentMa);
    r.serviceMaxCurrent  = MakeOptional(kServiceMaxCurrentMa);
    r.maxCurrent         = MakeOptional(kUltimateMaxCurrentMa);
    return r;
}

Structs::ResidualCurrentFaultRatingsStruct::Type MakeResidualCurrentRating()
{
    Structs::ResidualCurrentFaultRatingsStruct::Type r;
    BitMask<CurrentTripMechanismBitmap> mech;
    mech.Set(CurrentTripMechanismBitmap::kElectronic);
    BitMask<TrippingCharacteristicsBitmap> characteristic;
    characteristic.Set(TrippingCharacteristicsBitmap::kSelective);
    r.currentSensitivity     = MakeOptional<int64_t>(30); // 30 mA
    r.tripMechanism          = MakeOptional(mech);
    r.voltageDependent       = MakeOptional(false);
    r.groundFaultClass       = MakeOptional(GroundFaultClassEnum::kClassA);
    r.waveform               = MakeOptional(CurrentWaveformEnum::kAc);
    r.trippingCharacteristic = MakeOptional(characteristic);
    r.ultimateMaxCurrent     = MakeOptional(kUltimateMaxCurrentMa);
    r.serviceMaxCurrent      = MakeOptional(kServiceMaxCurrentMa);
    return r;
}

// This app enables every EPALM feature so the full alarm surface can be exercised. Alarm State
// starts clear and is driven by the test-event-trigger below.
ElectricalProtectionAlarmCluster::StartupConfiguration MakeDefaultConfig()
{
    ElectricalProtectionAlarmCluster::StartupConfiguration config;
    config.featureMap.Set(Feature::kShortCircuit)
        .Set(Feature::kOverLoad)
        .Set(Feature::kOverVoltage)
        .Set(Feature::kSurgeProtection)
        .Set(Feature::kResidualCurrent)
        .Set(Feature::kArcFault)
        .Set(Feature::kSelfTest);
    config.supported = AllSupportedAlarms();
    config.mask      = AllSupportedAlarms();

    BitMask<ArcCauseBitmap> arcCause;
    arcCause.Set(ArcCauseBitmap::kSeries).Set(ArcCauseBitmap::kParallelToNeutral);
    config.arcCause              = DataModel::MakeNullable(arcCause);
    config.overLoadRating        = DataModel::MakeNullable(MakeOverLoadRating());
    config.overVoltageRating     = DataModel::MakeNullable(MakeOverVoltageRating());
    config.surgeProtectionRating = DataModel::MakeNullable(MakeSurgeProtectionRating());
    config.shortCircuitRating    = DataModel::MakeNullable(MakeShortCircuitRating());
    config.residualCurrentRating = DataModel::MakeNullable(MakeResidualCurrentRating());
    config.arcFaultRating        = DataModel::MakeNullable(MakeArcFaultRating());
    return config;
}

std::unique_ptr<ElectricalProtectionAlarm::Instance> gInstance;

} // namespace

namespace chip::app::Clusters::ElectricalProtectionAlarm {

CHIP_ERROR ElectricalProtectionAlarmInit(EndpointId endpointId)
{
    VerifyOrReturnError(gInstance == nullptr, CHIP_ERROR_INCORRECT_STATE);

    gInstance = std::make_unique<Instance>(endpointId, MakeDefaultConfig());
    VerifyOrReturnError(gInstance != nullptr, CHIP_ERROR_NO_MEMORY);

    CHIP_ERROR err = gInstance->Init();
    if (err != CHIP_NO_ERROR)
    {
        gInstance.reset();
    }
    return err;
}

void ElectricalProtectionAlarmShutdown()
{
    // ~Instance() unregisters.
    gInstance.reset();
}

} // namespace chip::app::Clusters::ElectricalProtectionAlarm

bool HandleElectricalProtectionAlarmTestEventTrigger(uint64_t eventTrigger)
{
    VerifyOrReturnValue(gInstance != nullptr, false);
    auto & cluster = gInstance->Cluster();

    BitMask<AlarmBitmap> bit;
    switch (static_cast<ElectricalProtectionAlarmTrigger>(eventTrigger))
    {
    case ElectricalProtectionAlarmTrigger::kClearAll:
        return cluster.ClearAllAlarms() == CHIP_NO_ERROR;
    case ElectricalProtectionAlarmTrigger::kSetShortCircuitFault:
        bit.Set(AlarmBitmap::kShortCircuitFault);
        break;
    case ElectricalProtectionAlarmTrigger::kSetOverLoadFault:
        bit.Set(AlarmBitmap::kOverLoadFault);
        break;
    case ElectricalProtectionAlarmTrigger::kSetOverVoltageFault:
        bit.Set(AlarmBitmap::kOverVoltageFault);
        break;
    case ElectricalProtectionAlarmTrigger::kSetVoltageSurgeFault:
        bit.Set(AlarmBitmap::kVoltageSurgeFault);
        break;
    case ElectricalProtectionAlarmTrigger::kSetResidualCurrentFault:
        bit.Set(AlarmBitmap::kResidualCurrentFault);
        break;
    case ElectricalProtectionAlarmTrigger::kSetArcFault:
        bit.Set(AlarmBitmap::kArcFault);
        break;
    case ElectricalProtectionAlarmTrigger::kSetSelfTest:
        bit.Set(AlarmBitmap::kSelfTest);
        break;
    default:
        return false;
    }

    return cluster.ActivateAlarms(bit) == CHIP_NO_ERROR;
}
