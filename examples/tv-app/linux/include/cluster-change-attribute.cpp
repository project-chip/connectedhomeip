/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app/util/af.h>

using namespace chip;

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

void emberAfPostAttributeChangeCallback(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId, uint8_t mask,
                                        uint16_t manufacturerCode, uint8_t type, uint16_t size, uint8_t * value)
{
    if (clusterId == ZCL_ON_OFF_CLUSTER_ID && attributeId == ZCL_ON_OFF_ATTRIBUTE_ID)
    {
        ChipLogProgress(Zcl, "Received on/off command for cluster id: %d", clusterId);

        if (endpoint == 0)
        {
            ChipLogProgress(Zcl, "Execute POWER_TOGGLE");
            runTvCommand(PowerToggle);
        }
        else if (endpoint == 1)
        {
            ChipLogProgress(Zcl, "Execute MUTE_TOGGLE");
            runTvCommand(MuteToggle);
        }
    }
}
