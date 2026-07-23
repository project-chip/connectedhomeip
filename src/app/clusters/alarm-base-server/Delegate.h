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
 * When a delegate method returns false, the cluster will not apply the requested change.
 */
class Delegate
{
public:
    Delegate()          = default;
    virtual ~Delegate() = default;

    /**
     * Called before applying a ModifyEnabledAlarms command. Return true to allow the cluster
     * to update the Mask attribute.
     */
    virtual bool ModifyEnabledAlarms(AlarmMap mask) { return true; }

    /**
     * Called before applying a Reset command. Return true to allow the cluster to clear the
     * requested bits in the State attribute.
     */
    virtual bool ResetAlarms(AlarmMap alarms) { return true; }
};

} // namespace chip::app::Clusters::AlarmBase
