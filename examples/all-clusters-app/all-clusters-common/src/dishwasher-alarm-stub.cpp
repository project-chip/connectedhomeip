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
     *   This method for a server whether is able to enable a currently suppressed alarm,
     *   or suppress a currently enabled alarm
     *   @param[in] mask The value of modify Alarm mask
     *   @return true
     *   @return false
     */
    bool ModifyEnableAlarmsCallback(const chip::BitMask<AlarmMap> mask);

    /**
     *   @brief
     *   This method for a server whether the alarm definition requires manual intervention
     *   @param[in] alarms The value of reset alarm
     *   @return true
     *   @return false
     */
    bool ResetAlarmsCallback(const chip::BitMask<AlarmMap> alarms);

    ~DishwasherAlarmDelegate() = default;
};


bool DishwasherAlarmDelegate::ModifyEnableAlarmsCallback(const chip::BitMask<AlarmMap> mask)
{
    // placeholder implementation
    return true;
}

bool DishwasherAlarmDelegate::ResetAlarmsCallback(const chip::BitMask<AlarmMap> alarms)
{
    // placeholder implementation
    return false;
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
    static chip::app::Clusters::DishwasherAlarm::DishwasherAlarmDelegate delegate;
    chip::app::Clusters::DishwasherAlarm::SetDefaultDelegate(kDemoEndpointId, &delegate);
}
