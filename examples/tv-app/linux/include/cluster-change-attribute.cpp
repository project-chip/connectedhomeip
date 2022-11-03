/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace ::chip::app::Clusters;

enum TvCommand
{
    PowerToggle,
    MuteToggle
};

void runTvCommand(TvCommand command)
{
    switch (command)
    {
    case PowerToggle:
        // TODO: Insert your code here to send power toggle command
        break;
    case MuteToggle:
        // TODO: Insert your code here to send mute toggle command
        break;

    default:
        break;
    }
}

void MatterAfPostAttributeChangeCallback(const app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                         uint8_t * value)
{
    if (attributePath.mClusterId == OnOff::Id && attributePath.mAttributeId == OnOff::Attributes::OnOff::Id)
    {
        ChipLogProgress(Zcl, "Received on/off command for cluster id: " ChipLogFormatMEI, ChipLogValueMEI(OnOff::Id));

        if (attributePath.mEndpointId == 0)
        {
            ChipLogProgress(Zcl, "Execute POWER_TOGGLE");
            runTvCommand(PowerToggle);
        }
        else if (attributePath.mEndpointId == 1)
        {
            ChipLogProgress(Zcl, "Execute MUTE_TOGGLE");
            runTvCommand(MuteToggle);
        }
    }
}
