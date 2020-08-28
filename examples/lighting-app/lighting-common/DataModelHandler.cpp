/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 * @file
 *   This file implements the handler for data model messages.
 */

#include <lib/support/logging/CHIPLogging.h>
#include <support/logging/CHIPLogging.h>
#include <system/SystemPacketBuffer.h>

#include "DataModelHandler.h"
#include "LightingManager.h"

#include "af-types.h"
#include "attribute-storage.h"
#include "gen/attribute-id.h"
#include "gen/cluster-id.h"
#include "gen/znet-bookkeeping.h"
#include "util.h"
#include <app/chip-zcl-zpro-codec.h>

using namespace ::chip;

extern "C" {
void emberAfPostAttributeChangeCallback(uint8_t endpoint, EmberAfClusterId clusterId, EmberAfAttributeId attributeId, uint8_t mask,
                                        uint16_t manufacturerCode, uint8_t type, uint8_t size, uint8_t * value)
{
    if (clusterId != ZCL_ON_OFF_CLUSTER_ID)
    {
        ChipLogProgress(App, "Unknown cluster ID: %d", clusterId);
        return;
    }

    if (attributeId != ZCL_ON_OFF_ATTRIBUTE_ID)
    {
        ChipLogProgress(App, "Unknown attribute ID: %d", attributeId);
        return;
    }

    if (*value)
    {
        LightingMgr().InitiateAction(LightingManager::ON_ACTION);
    }
    else
    {
        LightingMgr().InitiateAction(LightingManager::OFF_ACTION);
    }
}

/**
 * Handle a message that should be processed via our data model processing
 * codepath.
 *
 * @param [in] buffer The buffer holding the message.  This function guarantees
 *                    that it will free the buffer before returning.
 */
void HandleDataModelMessage(const MessageHeader & header, System::PacketBuffer * buffer, SecureSessionMgrBase * mgr)
{
    EmberApsFrame frame;
    bool ok = extractApsFrame(buffer->Start(), buffer->DataLength(), &frame) > 0;
    if (ok)
    {
        ChipLogProgress(App, "APS frame processing success!");
    }
    else
    {
        ChipLogProgress(App, "APS frame processing failure!");
        System::PacketBuffer::Free(buffer);
        return;
    }

    ChipResponseDestination responseDest(header.GetSourceNodeId().Value(), mgr);
    uint8_t * message;
    uint16_t messageLen = extractMessage(buffer->Start(), buffer->DataLength(), &message);
    ok                  = emberAfProcessMessage(&frame,
                               0, // type
                               message, messageLen,
                               &responseDest, // source identifier
                               NULL);

    System::PacketBuffer::Free(buffer);

    if (ok)
    {
        ChipLogProgress(App, "Data model processing success!");
    }
    else
    {
        ChipLogProgress(App, "Data model processing failure!");
    }
}
}

void InitDataModelHandler()
{
    emberAfEndpointConfigure();
    emAfInit();
}
