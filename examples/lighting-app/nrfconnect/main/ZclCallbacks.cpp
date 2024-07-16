/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "AppTask.h"
#include "PWMDevice.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OnOff;

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    ClusterId clusterId     = attributePath.mClusterId;
    AttributeId attributeId = attributePath.mAttributeId;

    if (clusterId == OnOff::Id && attributeId == OnOff::Attributes::OnOff::Id)
    {
        ChipLogProgress(Zcl, "Cluster OnOff: attribute OnOff set to %u", *value);
        AppTask::Instance().GetPWMDevice().InitiateAction(*value ? PWMDevice::ON_ACTION : PWMDevice::OFF_ACTION,
                                                          static_cast<int32_t>(AppEventType::Lighting), value);
    }
    else if (clusterId == LevelControl::Id && attributeId == LevelControl::Attributes::CurrentLevel::Id)
    {
        ChipLogProgress(Zcl, "Cluster LevelControl: attribute CurrentLevel set to %u", *value);
        if (AppTask::Instance().GetPWMDevice().IsTurnedOn())
        {
            AppTask::Instance().GetPWMDevice().InitiateAction(PWMDevice::LEVEL_ACTION, static_cast<int32_t>(AppEventType::Lighting),
                                                              value);
        }
        else
        {
            ChipLogDetail(Zcl, "LED is off. Try to use move-to-level-with-on-off instead of move-to-level");
        }
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
void emberAfOnOffClusterInitCallback(EndpointId endpoint)
{
    Protocols::InteractionModel::Status status;
    bool storedValue;

    // Read storedValue on/off value
    status = Attributes::OnOff::Get(endpoint, &storedValue);
    if (status == Protocols::InteractionModel::Status::Success)
    {
        // Set actual state to the cluster state that was last persisted
        AppTask::Instance().GetPWMDevice().InitiateAction(storedValue ? PWMDevice::ON_ACTION : PWMDevice::OFF_ACTION,
                                                          static_cast<int32_t>(AppEventType::Lighting),
                                                          reinterpret_cast<uint8_t *>(&storedValue));
    }

    AppTask::Instance().UpdateClusterState();
}
