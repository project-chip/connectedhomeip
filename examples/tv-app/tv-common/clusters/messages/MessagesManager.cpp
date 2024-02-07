/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "MessagesManager.h"

using namespace std;
using namespace chip::app;
using namespace chip::app::Clusters::Messages;

// Commands
void MessagesManager::HandlePresentMessagesRequest(
    const chip::ByteSpan & messageId, const MessagePriorityEnum & priority,
    const chip::BitMask<MessageControlBitmap> & messageControl, const chip::app::DataModel::Nullable<uint32_t> & startTime,
    const chip::app::DataModel::Nullable<uint16_t> & duration, const chip::CharSpan & messageText,
    const chip::Optional<chip::app::DataModel::DecodableList<MessageResponseOption>> & responses)
{
    Message message;
    // TODO: Enable id
    // message.messageID = messageId;
    message.priority       = priority;
    message.messageControl = messageControl;
    message.startTime      = startTime;
    message.duration       = duration;
    // TODO: Enable text
    // message.messageText = messageText;
    // TODO: Convert to  Optional<chip::app::DataModel::List<const
    // chip::app::Clusters::Messages::Structs::MessageResponseOptionStruct::Type>> message.responses = responses;

    // mMessages.push_back(message);
    mMessages.push_back({nullptr, priority, messageControl, startTime, duration, nullptr});
    // Add your code to present Message
}

void MessagesManager::HandleCancelMessagesRequest(const chip::app::DataModel::DecodableList<chip::ByteSpan> & messageIds)
{
    // TODO: Cancel Message
}

// Attributes
CHIP_ERROR MessagesManager::HandleGetMessages(chip::app::AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (Message & entry : mMessages)
        {
            ReturnErrorOnFailure(encoder.Encode(entry));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR MessagesManager::HandleGetActiveMessageIds(chip::app::AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (Message & entry : mMessages)
        {
            ReturnErrorOnFailure(encoder.Encode(entry.messageID));
        }
        return CHIP_NO_ERROR;
    });
}

// Global Attributes
uint32_t MessagesManager::GetFeatureMap(chip::EndpointId endpoint)
{
    return 1;
}
