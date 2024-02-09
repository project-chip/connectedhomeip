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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <vector>

using namespace std;
using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::Messages;
using Message = chip::app::Clusters::Messages::Structs::MessageStruct::Type;

// Commands
void MessagesManager::HandlePresentMessagesRequest(const ByteSpan & messageId, const MessagePriorityEnum & priority,
                                                   const BitMask<MessageControlBitmap> & messageControl,
                                                   const DataModel::Nullable<uint32_t> & startTime,
                                                   const DataModel::Nullable<uint16_t> & duration, const CharSpan & messageText,
                                                   const Optional<DataModel::DecodableList<MessageResponseOption>> & responses)
{
    std::vector<char *> needToFree;
    std::vector<MessageResponseOption *> optionToFree;

    uint8_t * messageIdBuffer = nullptr;
    char * messageTextBuffer  = new char[messageText.size()];
    VerifyOrExit(messageTextBuffer != nullptr,
                 ChipLogProgress(Controller, "HandlePresentMessagesRequest messageTextBuffer alloc failed"));
    needToFree.push_back(messageTextBuffer);
    memcpy(messageTextBuffer, messageText.data(), messageText.size());

    ChipLogProgress(Controller, "HandlePresentMessagesRequest message:%s size:%lu", messageTextBuffer, messageText.size());

    messageIdBuffer = new uint8_t[messageId.size()];
    VerifyOrExit(messageIdBuffer != nullptr,
                 ChipLogProgress(Controller, "HandlePresentMessagesRequest messageIdBuffer alloc failed"));
    memcpy(messageIdBuffer, messageId.data(), messageId.size());

    if (responses.HasValue())
    {
        size_t size    = 0;
        CHIP_ERROR err = responses.Value().ComputeSize(&size);
        VerifyOrExit(err == CHIP_NO_ERROR, ChipLogProgress(Controller, "HandlePresentMessagesRequest size check failed"));

        MessageResponseOption * optionArray = new MessageResponseOption[size];
        VerifyOrExit(optionArray != nullptr,
                     ChipLogProgress(Controller, "HandlePresentMessagesRequest MessageResponseOption alloc failed"));
        optionToFree.push_back(optionArray);

        int counter = 0;
        auto iter   = responses.Value().begin();
        while (iter.Next())
        {
            auto & response = iter.GetValue();

            ChipLogProgress(Controller, "HandlePresentMessagesRequest option id:%u", response.messageResponseID.ValueOr(0));

            if (response.messageResponseID.HasValue())
            {
                optionArray[counter].messageResponseID = Optional<uint32_t>(response.messageResponseID.ValueOr(0));
            }

            if (response.label.HasValue())
            {
                char * labelBuffer = new char[response.label.Value().size()];
                VerifyOrExit(labelBuffer != nullptr,
                             ChipLogProgress(Controller, "HandlePresentMessagesRequest label alloc failed"));
                needToFree.push_back(labelBuffer);
                memcpy(labelBuffer, response.label.Value().data(), response.label.Value().size());

                ChipLogProgress(Controller, "HandlePresentMessagesRequest option label:%s size:%lu", labelBuffer,
                                response.label.Value().size());

                optionArray[counter].label = Optional<CharSpan>(CharSpan::fromCharString(labelBuffer));
            }
            counter++;
        }

        Message message{ ByteSpan(messageIdBuffer, messageId.size()),
                         priority,
                         messageControl,
                         startTime,
                         duration,
                         CharSpan::fromCharString(messageTextBuffer),
                         Optional<DataModel::List<MessageResponseOption>>(
                             DataModel::List<MessageResponseOption>(optionArray, size)) };

        mMessages.push_back(message);
    }
    else
    {
        Message message{ ByteSpan(messageIdBuffer, messageId.size()), priority, messageControl, startTime, duration,
                         CharSpan::fromCharString(messageTextBuffer) };
        mMessages.push_back(message);
    }

    // Add your code to present Message
    ChipLogProgress(Controller, "HandlePresentMessagesRequest complete");

    return;
exit:

    ChipLogProgress(Controller, "HandlePresentMessagesRequest error exit");
    // only reach here on errors
    if (messageIdBuffer != nullptr)
    {
        delete[] messageIdBuffer;
    }
    for (MessageResponseOption * memloc : optionToFree)
    {
        delete[] memloc;
    }
    for (char * memloc : needToFree)
    {
        delete[] memloc;
    }
}

void MessagesManager::HandleCancelMessagesRequest(const DataModel::DecodableList<ByteSpan> & messageIds)
{
    auto iter = messageIds.begin();
    while (iter.Next())
    {
        auto & id = iter.GetValue();

        mMessages.remove_if([id](Clusters::Messages::Structs::MessageStruct::Type & entry) {
            if (entry.messageID.data_equal(id))
            {
                // free the memory allocated for this entry
                if (entry.responses.HasValue())
                {
                    for (const MessageResponseOption & option : entry.responses.Value())
                    {
                        if (option.label.HasValue())
                        {
                            delete[] option.label.Value().data();
                        }
                    }
                    delete[] entry.responses.Value().data();
                }
                delete[] entry.messageID.data();
                delete[] entry.messageText.data();
                return true;
            }
            return false;
        });
    }
    return;
}

// Attributes
CHIP_ERROR MessagesManager::HandleGetMessages(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (Message & entry : mMessages)
        {
            ReturnErrorOnFailure(encoder.Encode(entry));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR MessagesManager::HandleGetActiveMessageIds(AttributeValueEncoder & aEncoder)
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
uint32_t MessagesManager::GetFeatureMap(EndpointId endpoint)
{
    uint32_t featureMap = 0;
    Attributes::FeatureMap::Get(endpoint, &featureMap);
    return featureMap;
}
