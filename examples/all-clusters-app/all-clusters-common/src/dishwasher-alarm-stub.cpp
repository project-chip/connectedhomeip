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

namespace chip {
namespace app {
namespace Clusters {
namespace DishwasherAlarm {

class DishwasherAlarmDelegate : public Delegate
{
public:
    /**
     *   @brief
     *   A notification that the Mask attribute has changed.  When this happens, some previously suppressed
     *   alarms may need to be enabled, and previously enabled alarms may need to be suppressed.
     *   @param[in] mask The new value of the Mask attribute.
     *   @return The cluster will do this update if ModifyEnabledAlarmsCallback() returns true.
     *   The cluster will not do this update if ModifyEnabledAlarmsCallback() returns false.
     */
    bool ModifyEnabledAlarmsCallback(const chip::BitMask<AlarmMap> mask) override;

    /**
     *   @brief
     *   A notification that resets active and latched alarms (if possible)
     *   @param[in] alarms The value of reset alarms
     *   @return The cluster will do this update if ResetAlarmsCallback() returns true.
     *   The cluster will not do this update if ResetAlarmsCallback() returns false.
     */
    bool ResetAlarmsCallback(const chip::BitMask<AlarmMap> alarms) override;

    ~DishwasherAlarmDelegate() = default;
};

bool DishwasherAlarmDelegate::ModifyEnabledAlarmsCallback(const chip::BitMask<AlarmMap> mask)
{
    // placeholder implementation
    return true;
}

bool DishwasherAlarmDelegate::ResetAlarmsCallback(const chip::BitMask<AlarmMap> alarms)
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
constexpr chip::EndpointId kDemoEndpointId = 1;

void MatterDishwasherAlarmServerInit()
{
    using namespace chip::app::Clusters;
    using namespace chip::app::Clusters::DishwasherAlarm;

    static DishwasherAlarm::DishwasherAlarmDelegate delegate;
    chip::app::Clusters::DishwasherAlarm::SetDefaultDelegate(kDemoEndpointId, &delegate);

    chip::BitMask<AlarmMap> supported;
    supported.SetField(AlarmMap::kInflowError, 1);
    supported.SetField(AlarmMap::kDrainError, 1);
    supported.SetField(AlarmMap::kTempTooLow, 1);
    DishwasherAlarmServer::Instance().SetSupportedValue(kDemoEndpointId, supported);

    chip::BitMask<AlarmMap> mask;
    mask.SetField(AlarmMap::kInflowError, 1);
    mask.SetField(AlarmMap::kTempTooLow, 1);
    DishwasherAlarmServer::Instance().SetMaskValue(kDemoEndpointId, mask);

    chip::BitMask<AlarmMap> state;
    state.SetField(AlarmMap::kInflowError, 1);
    state.SetField(AlarmMap::kTempTooLow, 1);
    DishwasherAlarmServer::Instance().SetStateValue(kDemoEndpointId, state);

    chip::BitMask<AlarmMap> latch;
    latch.SetField(AlarmMap::kInflowError, 1);
    latch.SetField(AlarmMap::kTempTooLow, 1);
    DishwasherAlarmServer::Instance().SetMaskValue(kDemoEndpointId, latch);
}
