/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using chip::Protocols::InteractionModel::Status;
using namespace chip::app;
using namespace chip::app::Clusters;

#ifdef MATTER_DM_PLUGIN_DISHWASHER_ALARM_SERVER
#include <app/clusters/dishwasher-alarm-server/dishwasher-alarm-server.h>
using namespace chip::app::Clusters::DishwasherAlarm;

class DishwasherAlarmDelegate : public Delegate
{
public:
    /**
     *   @brief
     *   A notification that the Mask attribute will be changed.  When this happens, some previously suppressed
     *   alarms may need to be enabled, and previously enabled alarms may need to be suppressed.
     *   @param[in] mask The new value of the Mask attribute.
     *   @return The cluster will do this update if ModifyEnabledAlarmsCallback() returns true.
     *   The cluster will not do this update if ModifyEnabledAlarmsCallback() returns false.
     */
    bool ModifyEnabledAlarmsCallback(const BitMask<AlarmMap> mask) override;

    /**
     *   @brief
     *   A notification that resets active alarms (if possible)
     *   @param[in] alarms The value of reset alarms
     *   @return The cluster will reset active alarms if ResetAlarmsCallback() returns true.
     *   The cluster will not reset active alarms if ResetAlarmsCallback() returns false.
     */
    bool ResetAlarmsCallback(const BitMask<AlarmMap> alarms) override;

    ~DishwasherAlarmDelegate() = default;
};

bool DishwasherAlarmDelegate::ModifyEnabledAlarmsCallback(const BitMask<AlarmMap> mask)
{
    // placeholder implementation
    return true;
}

bool DishwasherAlarmDelegate::ResetAlarmsCallback(const BitMask<AlarmMap> alarms)
{
    // placeholder implementation
    return true;
}

namespace {

using namespace chip::app::Clusters::DishwasherAlarm::Attributes;

void VerifyDishwasherAlarmFixedAttributes(EndpointId endpoint)
{
    BitMask<AlarmMap> expectedSupported;
    BitMask<AlarmMap> expectedLatch;
    BitMask<AlarmMap> actualSupported;
    BitMask<AlarmMap> actualLatch;

    VerifyOrDie(Supported::GetDefault(endpoint, &expectedSupported) == Status::Success);
    VerifyOrDie(Latch::GetDefault(endpoint, &expectedLatch) == Status::Success);
    VerifyOrDie(DishwasherAlarmServer::Instance().GetSupportedValue(endpoint, &actualSupported) == Status::Success);
    VerifyOrDie(DishwasherAlarmServer::Instance().GetLatchValue(endpoint, &actualLatch) == Status::Success);
    VerifyOrDie(actualSupported == expectedSupported);
    VerifyOrDie(actualLatch == expectedLatch);
}

} // namespace

/*
 * An example to present device's endpointId
 */
static constexpr EndpointId kDemoEndpointId = 1;

void MatterDishwasherAlarmServerInit()
{
    using namespace app::Clusters;
    using namespace app::Clusters::DishwasherAlarm;

    static DishwasherAlarm::DishwasherAlarmDelegate delegate;
    DishwasherAlarm::SetDefaultDelegate(kDemoEndpointId, &delegate);

    VerifyDishwasherAlarmFixedAttributes(kDemoEndpointId);

    // Set Mask attribute = 0x0 = 0
    // Bit Name              Value
    // 0   InflowError       0
    // 1   DrainError        0
    // 2   DoorError         0
    // 3   TempTooLow        0
    // 4   TempTooHigh       0
    // 5   WaterLevelError   0
    BitMask<AlarmMap> mask;
    mask.SetField(AlarmMap::kInflowError, 0);
    mask.SetField(AlarmMap::kDrainError, 0);
    mask.SetField(AlarmMap::kDoorError, 0);
    mask.SetField(AlarmMap::kTempTooLow, 0);
    mask.SetField(AlarmMap::kTempTooHigh, 0);
    mask.SetField(AlarmMap::kWaterLevelError, 0);
    DishwasherAlarmServer::Instance().SetMaskValue(kDemoEndpointId, mask);

    // Set State attribute = 0x00
    // Bit Name              Value
    // 0   InflowError       0
    // 1   DrainError        0
    // 2   DoorError         0
    // 3   TempTooLow        0
    // 4   TempTooHigh       0
    // 5   WaterLevelError   0
    BitMask<AlarmMap> state;
    state.SetField(AlarmMap::kInflowError, 0);
    state.SetField(AlarmMap::kDrainError, 0);
    state.SetField(AlarmMap::kDoorError, 0);
    state.SetField(AlarmMap::kTempTooLow, 0);
    state.SetField(AlarmMap::kTempTooHigh, 0);
    state.SetField(AlarmMap::kWaterLevelError, 0);
    DishwasherAlarmServer::Instance().SetStateValue(kDemoEndpointId, state);
}

#endif // MATTER_DM_PLUGIN_DISHWASHER_ALARM_SERVER
