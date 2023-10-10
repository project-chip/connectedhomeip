/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/dishwasher-alarm-server/dishwasher-alarm-server.h>
#include <app/util/af-enums.h>

using namespace chip;

namespace chip {
namespace app {
namespace Clusters {
namespace DishwasherAlarm {

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
    bool ModifyEnabledAlarmsCallback(const BitMask<AlarmBitmap> mask) override;

    /**
     *   @brief
     *   A notification that resets active alarms (if possible)
     *   @param[in] alarms The value of reset alarms
     *   @return The cluster will reset active alarms if ResetAlarmsCallback() returns true.
     *   The cluster will not reset active alarms if ResetAlarmsCallback() returns false.
     */
    bool ResetAlarmsCallback(const BitMask<AlarmBitmap> alarms) override;

    ~DishwasherAlarmDelegate() = default;
};

bool DishwasherAlarmDelegate::ModifyEnabledAlarmsCallback(const BitMask<AlarmBitmap> mask)
{
    // placeholder implementation
    return true;
}

bool DishwasherAlarmDelegate::ResetAlarmsCallback(const BitMask<AlarmBitmap> alarms)
{
    // placeholder implementation
    return true;
}

} // namespace DishwasherAlarm
} // namespace Clusters
} // namespace app
} // namespace chip

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

    // Set Supported attribute = 0x2F = 47
    // Bit Name              Value
    // 0   InflowError       1
    // 1   DrainError        1
    // 2   DoorError         1
    // 3   TempTooLow        1
    // 4   TempTooHigh       0
    // 5   WaterLevelError   1
    BitMask<AlarmBitmap> supported;
    supported.SetField(AlarmBitmap::kInflowError, 1);
    supported.SetField(AlarmBitmap::kDrainError, 1);
    supported.SetField(AlarmBitmap::kDoorError, 1);
    supported.SetField(AlarmBitmap::kTempTooLow, 1);
    supported.SetField(AlarmBitmap::kWaterLevelError, 1);
    DishwasherAlarmServer::Instance().SetSupportedValue(kDemoEndpointId, supported);

    // Set Mask attribute = 0x2F = 47
    // Bit Name              Value
    // 0   InflowError       1
    // 1   DrainError        1
    // 2   DoorError         1
    // 3   TempTooLow        1
    // 4   TempTooHigh       0
    // 5   WaterLevelError   1
    BitMask<AlarmBitmap> mask;
    mask.SetField(AlarmBitmap::kInflowError, 1);
    mask.SetField(AlarmBitmap::kDrainError, 1);
    mask.SetField(AlarmBitmap::kDoorError, 1);
    mask.SetField(AlarmBitmap::kTempTooLow, 1);
    mask.SetField(AlarmBitmap::kWaterLevelError, 1);
    DishwasherAlarmServer::Instance().SetMaskValue(kDemoEndpointId, mask);

    // Set Latch attribute = 0x03
    // Bit Name              Value
    // 0   InflowError       1
    // 1   DrainError        1
    // 2   DoorError         0
    // 3   TempTooLow        0
    // 4   TempTooHigh       0
    // 5   WaterLevelError   0
    BitMask<AlarmBitmap> latch;
    latch.SetField(AlarmBitmap::kInflowError, 1);
    latch.SetField(AlarmBitmap::kDrainError, 1);
    DishwasherAlarmServer::Instance().SetLatchValue(kDemoEndpointId, latch);

    // Set State attribute = 0x07
    // Bit Name              Value
    // 0   InflowError       1
    // 1   DrainError        1
    // 2   DoorError         1
    // 3   TempTooLow        0
    // 4   TempTooHigh       0
    // 5   WaterLevelError   0
    BitMask<AlarmBitmap> state;
    state.SetField(AlarmBitmap::kInflowError, 1);
    state.SetField(AlarmBitmap::kDrainError, 1);
    state.SetField(AlarmBitmap::kDoorError, 1);
    DishwasherAlarmServer::Instance().SetStateValue(kDemoEndpointId, state);
}
