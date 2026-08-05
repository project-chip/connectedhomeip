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

// This app enables every EPALM feature so the full alarm surface can be exercised. Rating
// attributes are left null (the app performs no real measurement); alarm State starts clear and is
// driven by the test-event-trigger below.
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
