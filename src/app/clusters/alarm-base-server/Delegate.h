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

#pragma once

#include <app/clusters/alarm-base-server/alarm-base-cluster-objects.h>

namespace chip::app::Clusters::AlarmBase {

/**
 * Optional application delegate for AlarmBase command handling.
 *
 * The cluster calls these methods while handling incoming commands, before it
 * updates Matter attributes. Return true to allow the cluster to apply the
 * change; return false to reject the command (the cluster responds with Failure
 * and leaves attributes unchanged).
 *
 * Implementations that only use the Matter attribute model may return true
 * unconditionally. Products with physical alarms, annunciators, or latched
 * hardware state should use these hooks to perform or refuse the corresponding
 * device-side action before the cluster updates Mask or State.
 */
class Delegate
{
public:
    Delegate()          = default;
    virtual ~Delegate() = default;

    /**
     * Called when a ModifyEnabledAlarms command is received, before the Mask
     * attribute is updated.
     *
     * @param mask  Proposed new value for the Mask attribute: a bitmap of alarm
     *              bits that shall be enabled (not suppressed). Each set bit
     *              corresponds to an alarm type defined by the derived cluster
     *              (for example DoorError on Dishwasher Alarm). This is the same
     *              value the command argument carries and what SetMask()
     *              would store if the command succeeds.
     *
     * The application may configure hardware or internal policy to match the
     * requested enable/suppress behavior (for example enabling or disabling
     * monitoring for specific fault sources). Return false if the device cannot
     * honor the request; in that case the cluster does not change Mask or State.
     *
     * If this returns true, the cluster updates Mask and may adjust State so
     * that active alarms remain consistent with the new mask.
     */
    virtual bool ModifyEnabledAlarms(AlarmMap mask) { return true; }

    /**
     * Called when a Reset command is received, before the State attribute is
     * updated.
     *
     * @param alarms  Bitmap of alarm bits the client asked to reset. Each set
     *                bit identifies a latched/active alarm in State that the
     *                client wants cleared.
     *
     * The application should clear the corresponding physical or logical alarm
     * condition when possible (for example silencing an annunciator, clearing a
     * fault latch in hardware, or updating persisted fault state). Return false
     * if the device cannot reset one or more of the requested alarms; in that
     * case the cluster does not change State.
     *
     * If this returns true, the cluster clears the requested bits in State via
     * ResetLatchedAlarms() and may emit a Notify event.
     */
    virtual bool ResetAlarms(AlarmMap alarms) { return true; }
};

} // namespace chip::app::Clusters::AlarmBase
