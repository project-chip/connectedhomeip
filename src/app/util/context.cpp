/**
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

#include "context.h"

#include "common.h"

DataModelHandler gDataModelHandler;

chip::Messaging::ExchangeDelegate * GetDataModelHandler()
{
    return &gDataModelHandler;
}

void DataModelHandler::OnMessageReceived(chip::Messaging::ExchangeContext * ec, const chip::PacketHeader & packetHeader,
                                         uint32_t protocolId, uint8_t msgType, chip::System::PacketBufferHandle payload)
{
    auto contextLock = dmContext.Scoped(ec);
    EmberApsFrame frame;
    bool ok = extractApsFrame(payload->Start(), payload->DataLength(), &frame) > 0;
    if (ok)
    {
        ChipLogProgress(Zcl, "APS frame processing success!");
    }
    else
    {
        ChipLogProgress(Zcl, "APS frame processing failure!");
        return;
    }

    uint8_t * message;
    uint16_t messageLen = extractMessage(payload->Start(), payload->DataLength(), &message);
    ok                  = emberAfProcessMessage(&frame,
                               0, // type
                               message, messageLen,
                               packetHeader.GetSourceNodeId().Value(), // source identifier
                               NULL);

    if (ok)
    {
        ChipLogProgress(Zcl, "Data model processing success!");
    }
    else
    {
        ChipLogProgress(Zcl, "Data model processing failure!");
        return;
    }
}

void DataModelHandler::OnResponseTimeout(chip::Messaging::ExchangeContext * ec) {}
void DataModelHandler::OnExchangeClosing(chip::Messaging::ExchangeContext * ec) {}
