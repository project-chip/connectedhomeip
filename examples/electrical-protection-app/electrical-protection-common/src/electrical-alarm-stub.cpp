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

#include <electrical-alarm-stub.h>

#include <app/clusters/electrical-alarm-server/ElectricalAlarmTestEventTriggerHandler.h>
#include <lib/support/CodeUtils.h>

#include <memory>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ElectricalAlarm;

namespace {

// The app performs no real measurement, so every request is approved and nothing is written to
// hardware. SetElectricalAlarmThresholdsCallback is left to the base implementation, which also
// approves.
class BreakerAlarmDelegate : public ElectricalAlarm::Delegate
{
public:
    bool ModifyEnabledAlarmsCallback(const BitMask<AlarmBitmap> mask) override { return true; }
    bool ResetAlarmsCallback(const BitMask<AlarmBitmap> alarms) override { return true; }
};

BreakerAlarmDelegate gDelegate;
std::unique_ptr<ElectricalAlarm::Instance> gInstance;

} // namespace

namespace chip::app::Clusters::ElectricalAlarm {

CHIP_ERROR ElectricalAlarmInit(EndpointId endpointId, BitMask<Feature> features)
{
    VerifyOrReturnError(gInstance == nullptr, CHIP_ERROR_INCORRECT_STATE);

    gInstance = std::make_unique<Instance>(endpointId, &gDelegate, features);
    VerifyOrReturnError(gInstance != nullptr, CHIP_ERROR_NO_MEMORY);

    CHIP_ERROR err = gInstance->Init();
    if (err != CHIP_NO_ERROR)
    {
        gInstance.reset();
        return err;
    }

    // Supported defaults to empty; declare every alarm so Mask and State have the widest set to
    // narrow against, and so a test script exercising any one alarm bit has a DUT.
    Protocols::InteractionModel::Status status = gInstance->Cluster().SetSupportedValue(kAllAlarms);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        gInstance.reset();
        return CHIP_ERROR_INTERNAL;
    }

    // Mask defaults to empty, which suppresses every alarm and makes State undrivable. Enable them
    // all so a test can activate any alarm it asks for.
    status = gInstance->Cluster().SetMaskValue(kAllAlarms);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        gInstance.reset();
        return CHIP_ERROR_INTERNAL;
    }

    // Latch every alarm when RST is offered, so an activated alarm stays active until Reset clears
    // it. Without this the Reset command has nothing to act on.
    if (features.Has(Feature::kReset))
    {
        status = gInstance->Cluster().SetLatchValue(kAllAlarms);
        if (status != Protocols::InteractionModel::Status::Success)
        {
            gInstance.reset();
            return CHIP_ERROR_INTERNAL;
        }
    }
    return CHIP_NO_ERROR;
}

void ElectricalAlarmShutdown()
{
    // ~Instance() unregisters.
    gInstance.reset();
}

} // namespace chip::app::Clusters::ElectricalAlarm

bool HandleElectricalAlarmTestEventTrigger(uint64_t eventTrigger)
{
    VerifyOrReturnValue(gInstance != nullptr, false);
    auto & cluster = gInstance->Cluster();

    // Activating is additive: OR the requested bit into the current State so a test can raise
    // several alarms in sequence.
    auto activate = [&cluster](AlarmBitmap alarm) {
        BitMask<AlarmBitmap> next = cluster.GetState();
        next.Set(alarm);
        return cluster.SetStateValue(next) == Protocols::InteractionModel::Status::Success;
    };

    switch (static_cast<ElectricalAlarmTrigger>(eventTrigger))
    {
    case ElectricalAlarmTrigger::kClearAll:
        // Models the measured condition going away, not an operator acknowledging it: latched
        // alarms deliberately stay active here and clear only via the Reset command.
        return cluster.SetStateValue(BitMask<AlarmBitmap>()) == Protocols::InteractionModel::Status::Success;
    case ElectricalAlarmTrigger::kSetOverVoltage:
        return activate(AlarmBitmap::kOverVoltage);
    case ElectricalAlarmTrigger::kSetUnderVoltage:
        return activate(AlarmBitmap::kUnderVoltage);
    case ElectricalAlarmTrigger::kSetOverFrequency:
        return activate(AlarmBitmap::kOverFrequency);
    case ElectricalAlarmTrigger::kSetUnderFrequency:
        return activate(AlarmBitmap::kUnderFrequency);
    case ElectricalAlarmTrigger::kSetOverPower:
        return activate(AlarmBitmap::kOverPower);
    case ElectricalAlarmTrigger::kSetUnderPower:
        return activate(AlarmBitmap::kUnderPower);
    case ElectricalAlarmTrigger::kSetOverCurrent:
        return activate(AlarmBitmap::kOverCurrent);
    case ElectricalAlarmTrigger::kSetUnderCurrent:
        return activate(AlarmBitmap::kUnderCurrent);
    case ElectricalAlarmTrigger::kSetPowerImport:
        return activate(AlarmBitmap::kPowerImported);
    case ElectricalAlarmTrigger::kSetPowerExport:
        return activate(AlarmBitmap::kPowerExported);
    default:
        return false;
    }
}
