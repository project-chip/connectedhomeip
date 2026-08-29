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

#include <lib/support/CodeUtils.h>

#include <memory>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ElectricalAlarm;

namespace {

// The app performs no real measurement, so every request is approved and nothing is written to
// hardware. SetElectricalAlarmThresholdsCallback is left to the base implementation, which also
// approves, and is unreachable here anyway because this instance does not enable ADJUST.
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

CHIP_ERROR ElectricalAlarmInit(EndpointId endpointId)
{
    VerifyOrReturnError(gInstance == nullptr, CHIP_ERROR_INCORRECT_STATE);

    // OverCurrent is the only alarm class the breaker endpoint claims. The device type makes it
    // mandatory once this cluster is present, and one alarm class is enough to satisfy the
    // cluster's own at-least-one feature choice.
    gInstance = std::make_unique<Instance>(endpointId, &gDelegate, BitMask<Feature>(Feature::kOverCurrent));
    VerifyOrReturnError(gInstance != nullptr, CHIP_ERROR_NO_MEMORY);

    CHIP_ERROR err = gInstance->Init();
    if (err != CHIP_NO_ERROR)
    {
        gInstance.reset();
        return err;
    }

    // Supported defaults to empty; declare the one alarm this endpoint can raise so Mask and State
    // have something to narrow against.
    Protocols::InteractionModel::Status status =
        gInstance->Cluster().SetSupportedValue(BitMask<AlarmBitmap>(AlarmBitmap::kOverCurrent));
    if (status != Protocols::InteractionModel::Status::Success)
    {
        gInstance.reset();
        return CHIP_ERROR_INTERNAL;
    }
    return CHIP_NO_ERROR;
}

void ElectricalAlarmShutdown()
{
    // ~Instance() unregisters.
    gInstance.reset();
}

} // namespace chip::app::Clusters::ElectricalAlarm
