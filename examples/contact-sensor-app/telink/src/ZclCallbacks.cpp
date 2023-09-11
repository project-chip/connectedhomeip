/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "AppTask.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app::Clusters;

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    ClusterId clusterId     = attributePath.mClusterId;
    AttributeId attributeId = attributePath.mAttributeId;
    ChipLogProgress(Zcl, "Cluster callback: " ChipLogFormatMEI, ChipLogValueMEI(clusterId));

    if (clusterId == BooleanState::Id && attributeId == BooleanState::Attributes::StateValue::Id)
    {
        ChipLogProgress(Zcl, "Cluster BooleanState: attribute StateValue set to %u", *value);
        AppTask & task = GetAppTask();
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
}

void emberAfBooleanStateClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "emberAfBooleanStateClusterInitCallback");
    GetAppTask().UpdateClusterState();
}
