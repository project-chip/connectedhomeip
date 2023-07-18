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

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandResponseHelper.h>
#include <app/util/af.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DishwasherAlarm {

/** @brief
 *  Defines methods for implementing application-specific logic for the DishwasherAlarm Cluster.
 */
class Delegate
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
    virtual bool ModifyEnableAlarmsCallback(const chip::BitMask<AlarmMap> mask) = 0;

    /**
     *   @brief
     *   This method for a server whether the alarm definition requires manual intervention
     *   @param[in] alarms The value of reset alarm
     *   @return true
     *   @return false
     */
    virtual bool ResetAlarmsCallback(const chip::BitMask<AlarmMap> alarms) = 0;

    Delegate(EndpointId endpoint) : mEndpoint(endpoint) {}

    Delegate() = default;

    virtual ~Delegate() = default;

protected:
    EndpointId mEndpoint = 0;
};

} // namespace DishwasherAlarm
} // namespace Clusters
} // namespace app
} // namespace chip
