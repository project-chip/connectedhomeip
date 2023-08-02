/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <lib/support/logging/CHIPLogging.h>

#include "AppTask.h"
#include "ContactSensorManager.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/EventLogging.h>
#include <app/util/af-types.h>
#include <app/util/af.h>

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::app::Clusters;
using namespace ::chip::app::Clusters::BooleanState;

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & path, uint8_t type, uint16_t size, uint8_t * value)
{
    ChipLogProgress(Zcl, "MatterPostAttributeChangeCallback, value:%d\n", *value);
    if (path.mClusterId != BooleanState::Id)
    {
        ChipLogProgress(Zcl, "Unknown cluster ID: " ChipLogFormatMEI, ChipLogValueMEI(path.mClusterId));
        return;
    }

    if (path.mAttributeId != BooleanState::Attributes::StateValue::Id)
    {
        ChipLogProgress(Zcl, "Unknown attribute ID: " ChipLogFormatMEI, ChipLogValueMEI(path.mAttributeId));
        return;
    }

    AppTask & task = GetAppTask();
    // If the callback is called after the cluster attribute was changed due to pressing a button,
    // set the sync value to false. Both LED and attribute were updated at this point.
    // On the other hand, if the cluster attribute was changed due to a cluster command,
    // forward the request to AppTask in order to update the LED state.
    if (task.IsSyncClusterToButtonAction())
    {
        task.SetSyncClusterToButtonAction(false);
    }
    else
    {
        task.PostContactActionRequest(*value ? ContactSensorManager::Action::kSignalDetected
                                             : ContactSensorManager::Action::kSignalLost);
    }
}

/** @brief OnOff Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 * TODO Issue #3841
 * emberAfOnOffClusterInitCallback happens before the stack initialize the cluster
 * attributes to the default value.
 * The logic here expects something similar to the deprecated Plugins callback
 * emberAfPluginOnOffClusterServerPostInitCallback.
 *
 */
void emberAfBooleanStateClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "emberAfBooleanStateClusterInitCallback\n");
    GetAppTask().UpdateClusterState();
}

void logBooleanStateEvent(bool state)
{
    EventNumber eventNumber;
    Events::StateChange::Type event{ state };
    if (CHIP_NO_ERROR != LogEvent(event, 1, eventNumber))
    {
        ChipLogProgress(Zcl, "booleanstate: failed to reacord state-change event");
    }
}
