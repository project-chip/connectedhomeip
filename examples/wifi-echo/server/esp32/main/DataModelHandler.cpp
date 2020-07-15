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
#include <datamodel/ZCLCommand.h>

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

void InitDataModelHandler() {}

void HandleDataModelMessage(ClusterServer & server, System::PacketBuffer * buffer)
{
    ZCLCommand cmd(buffer);
    cmd.StartDecode();

    if (server.HandleCommand(cmd) != CHIP_NO_ERROR)
    {
        ESP_LOGI(TAG, "Data model processing success!");
    }
    else
    {
        ESP_LOGI(TAG, "Data model processing failure");
    }
    cmd.EndDecode();
    System::PacketBuffer::Free((chip::System::PacketBuffer *) cmd.mBuffer);
}
