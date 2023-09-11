/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
     *   A notification that the Mask attribute will be changed.  When this happens, some previously suppressed
     *   alarms may need to be enabled, and previously enabled alarms may need to be suppressed.
     *   @param[in] mask The new value of the Mask attribute.
     *   @return The cluster will do this update if ModifyEnabledAlarmsCallback() returns true.
     *   the cluster will not do this update if ModifyEnabledAlarmsCallback() returns false.
     */
    virtual bool ModifyEnabledAlarmsCallback(const BitMask<AlarmMap> mask) = 0;

    /**
     *   @brief
     *   A notification that resets active alarms (if possible)
     *   @param[in] alarms The value of reset alarms
     *   @return The cluster will reset active alarms if ResetAlarmsCallback() returns true.
     *   The cluster will not reset active alarms if ResetAlarmsCallback() returns false.
     */
    virtual bool ResetAlarmsCallback(const BitMask<AlarmMap> alarms) = 0;

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
