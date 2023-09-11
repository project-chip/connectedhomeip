/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "LevelManager.h"
#include "OnOffManager.h"
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace ::chip::app::Clusters;

void MatterPostAttributeChangeCallback(const app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    if (attributePath.mClusterId == OnOff::Id && attributePath.mAttributeId == OnOff::Attributes::OnOff::Id)
    {
        bool onoff = static_cast<bool>(*value);

        ChipLogProgress(Zcl, "Received on/off command endpoint %d value = %d", static_cast<int>(attributePath.mEndpointId), onoff);

        OnOffManager().PostOnOffChanged(attributePath.mEndpointId, onoff);
    }
    else if (attributePath.mClusterId == LevelControl::Id &&
             attributePath.mAttributeId == LevelControl::Attributes::CurrentLevel::Id)
    {
        uint8_t level = *value;

        ChipLogProgress(Zcl, "Received level command endpoint %d value = %d", static_cast<int>(attributePath.mEndpointId), level);

        LevelManager().PostLevelChanged(attributePath.mEndpointId, level);
    }
}
