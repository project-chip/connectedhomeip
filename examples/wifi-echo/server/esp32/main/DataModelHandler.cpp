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
#include <datamodel/Command.h>
#include <system/SystemPacketBuffer.h>

#include "DataModelHandler.h"
#include "LEDWidget.h"

extern "C" {
#include "chip-zcl/chip-zcl-codec.h"
#include "chip-zcl/chip-zcl.h"
#include "gen/gen-cluster-id.h"
#include "gen/gen-types.h"
}

using namespace ::chip;
using namespace ::chip::DataModel;

static const char * TAG = "data_model_server";

void InitDataModelHandler()
{
    chipZclEndpointInit();
}

static void DecodeHeader(ChipZclBuffer_t * buffer, Command * cmd)
{
    ChipZclCodec_t codec;
    uint8_t mask = 0;
    chipZclCodecDecodeStart(&codec, buffer);
    chipZclCodecDecode(&codec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &mask, sizeof(mask), NULL);
    chipZclCodecDecode(&codec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &(cmd->mEndpointId), sizeof(cmd->mEndpointId), NULL);

    chipZclCodecDecode(&codec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &(cmd->mId), sizeof(cmd->mId), NULL);
    chipZclCodecDecode(&codec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &(cmd->mDirection), sizeof(cmd->mDirection), NULL);
    cmd->mType = kCmdTypeGlobal;
    if (mask & 0x01)
    {
        cmd->mType = kCmdTypeCluster;
        chipZclCodecDecode(&codec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &(cmd->mClusterId), sizeof(cmd->mClusterId), NULL);
    }
    if (mask & 0x02)
    {
        cmd->mType = kCmdTypeMfg;
        chipZclCodecDecode(&codec, CHIP_ZCL_STRUCT_TYPE_INTEGER, &(cmd->mMfgCode), sizeof(cmd->mMfgCode), NULL);
    }
    chipZclCodecDecodeEnd(&codec);
}

void HandleDataModelMessage(ClusterServer & server, System::PacketBuffer * buffer)
{
    Command cmd;
    DecodeHeader((ChipZclBuffer_t *) buffer, &cmd);
    if (server.HandleCommand(cmd) != CHIP_NO_ERROR)
    {
        ESP_LOGI(TAG, "Data model processing success!");
    }
    else
    {
        ESP_LOGI(TAG, "Data model processing failure");
    }
    System::PacketBuffer::Free(buffer);
}
