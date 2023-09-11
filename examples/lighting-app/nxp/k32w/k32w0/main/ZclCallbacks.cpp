/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lib/support/logging/CHIPLogging.h>

#include "AppTask.h"
#include "LightingManager.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/util/af-types.h>
#include <app/util/af.h>

using namespace ::chip;
using namespace ::chip::app::Clusters;

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & path, uint8_t type, uint16_t size, uint8_t * value)
{
    if (path.mClusterId == OnOff::Id)
    {
        if (path.mAttributeId != OnOff::Attributes::OnOff::Id)
        {
            ChipLogProgress(Zcl, "Unknown attribute ID: " ChipLogFormatMEI, ChipLogValueMEI(path.mAttributeId));
            return;
        }

        LightingMgr().InitiateAction(0, *value ? LightingManager::TURNON_ACTION : LightingManager::TURNOFF_ACTION);
    }
    else if (path.mClusterId == LevelControl::Id)
    {
        ChipLogProgress(Zcl, "Level Control attribute ID: " ChipLogFormatMEI " Type: %u Value: %u, length %u",
                        ChipLogValueMEI(path.mAttributeId), type, *value, size);

        // WIP Apply attribute change to Light
    }
    else if (path.mClusterId == ColorControl::Id)
    {
        ChipLogProgress(Zcl, "Color Control attribute ID: " ChipLogFormatMEI " Type: %u Value: %u, length %u",
                        ChipLogValueMEI(path.mAttributeId), type, *value, size);

        // WIP Apply attribute change to Light
    }
    else if (path.mClusterId == OnOffSwitchConfiguration::Id)
    {
        ChipLogProgress(Zcl, "OnOff Switch Configuration attribute ID: " ChipLogFormatMEI " Type: %u Value: %u, length %u",
                        ChipLogValueMEI(path.mAttributeId), type, *value, size);

        // WIP Apply attribute change to Light
    }
    else
    {
        ChipLogProgress(Zcl, "Unknown attribute ID: " ChipLogFormatMEI, ChipLogValueMEI(path.mAttributeId));
    }
}
