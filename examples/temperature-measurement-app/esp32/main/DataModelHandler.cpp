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

#include "esp_log.h"
#include <system/SystemPacketBuffer.h>
#include <transport/raw/MessageHeader.h>

#include "DataModelHandler.h"

#include "attribute-storage.h"
#include "gen/attribute-id.h"
#include "gen/cluster-id.h"
#include "gen/znet-bookkeeping.h"
#include "util.h"
#include <app/chip-zcl-zpro-codec.h>

using namespace ::chip;

static const char * TAG = "data_model_server";

void InitDataModelHandler()
{
    emberAfEndpointConfigure();
    emAfInit();
}

void HandleDataModelMessage(const PacketHeader & header, System::PacketBuffer * buffer, SecureSessionMgrBase * mgr)
{
    EmberApsFrame frame;
    bool ok = extractApsFrame(buffer->Start(), buffer->DataLength(), &frame) > 0;
    if (ok)
    {
        ESP_LOGI(TAG, "APS frame processing success!");
    }
    else
    {
        ESP_LOGI(TAG, "APS frame processing failure");
        System::PacketBuffer::Free(buffer);
        return;
    }

    uint8_t * message;
    uint16_t messageLen = extractMessage(buffer->Start(), buffer->DataLength(), &message);
    ok                  = emberAfProcessMessage(&frame,
                               0, // type
                               message, messageLen,
                               header.GetSourceNodeId().Value(), // source identifier
                               nullptr);

    System::PacketBuffer::Free(buffer);

    if (ok)
    {
        ESP_LOGI(TAG, "Data model processing success!");
    }
    else
    {
        ESP_LOGI(TAG, "Data model processing failure");
    }
}
