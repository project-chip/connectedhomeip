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
    std::unique_ptr<char[]> messageTextBufferPtr;
    std::unique_ptr<uint8_t[]> messageIdBufferPtr;

    uint8_t * messageIdBuffer = nullptr;
    // see MessagesManager.h "Note on memory management" for allocation/free rules
    char * messageTextBuffer = new char[messageText.size() + 1];
    VerifyOrExit(messageTextBuffer != nullptr,
                 ChipLogProgress(Controller, "HandlePresentMessagesRequest messageTextBuffer alloc failed"));
    messageTextBufferPtr.reset(messageTextBuffer);

    memcpy(messageTextBuffer, messageText.data(), messageText.size());
    messageTextBuffer[messageText.size()] = '\0';

    ChipLogProgress(Controller, "HandlePresentMessagesRequest message:%s size:%lu", messageTextBuffer, messageText.size());

    // see MessagesManager.h "Note on memory management" for allocation/free rules
    messageIdBuffer = new uint8_t[messageId.size()];
    VerifyOrExit(messageIdBuffer != nullptr,
                 ChipLogProgress(Controller, "HandlePresentMessagesRequest messageIdBuffer alloc failed"));
    memcpy(messageIdBuffer, messageId.data(), messageId.size());
    messageIdBufferPtr.reset(messageIdBuffer);

    {
        messageTextBufferPtr.release();
        messageIdBufferPtr.release();
        CachedMessage * cachedMessage = new CachedMessage(messageIdBuffer, messageId.size(), priority, messageControl, startTime,
                                                          duration, std::string(messageTextBuffer, messageText.size() + 1));
        std::unique_ptr<CachedMessage *> cachedMessagePtr = std::make_unique<CachedMessage *>(cachedMessage);

        if (responses.HasValue())
        {
            size_t size    = 0;
            CHIP_ERROR err = responses.Value().ComputeSize(&size);
            VerifyOrExit(err == CHIP_NO_ERROR, ChipLogProgress(Controller, "HandlePresentMessagesRequest size check failed"));

            // set the array size on the CachedMessage
            VerifyOrExit(cachedMessage->SetOptionArraySize(size) == CHIP_NO_ERROR,
                         ChipLogProgress(Controller, "HandlePresentMessagesRequest SetOptionArraySize failed"));

            size_t counter = 0;
            auto iter      = responses.Value().begin();
            while (iter.Next())
            {
                std::unique_ptr<char[]> labelBufferPtr;
                auto & response = iter.GetValue();

                VerifyOrExit(response.messageResponseID.HasValue() && response.label.HasValue(),
                             ChipLogProgress(Controller, "HandlePresentMessagesRequest missing respond id or label"));

                // see MessagesManager.h "Note on memory management" for allocation/free rules
                char * labelBuffer = new char[response.label.Value().size() + 1];
                VerifyOrExit(labelBuffer != nullptr,
                             ChipLogProgress(Controller, "HandlePresentMessagesRequest label alloc failed"));
                labelBufferPtr.reset(labelBuffer);

                memcpy(labelBuffer, response.label.Value().data(), response.label.Value().size());
                labelBuffer[response.label.Value().size()] = '\0';

                labelBufferPtr.release();
                // set the option fields on the CachedMessage
                cachedMessage->SetOption(counter, Optional<uint32_t>(response.messageResponseID.Value()),
                                         Optional<CharSpan>(CharSpan::fromCharString(labelBuffer)));
                counter++;
            }
        }

        cachedMessagePtr.release();
        mCachedMessages.push_back(*cachedMessage);
    }

    // Add your code to present Message
    ChipLogProgress(Controller, "HandlePresentMessagesRequest complete");

    return;
exit:

    ChipLogProgress(Controller, "HandlePresentMessagesRequest error exit");
}

void MessagesManager::HandleCancelMessagesRequest(const DataModel::DecodableList<ByteSpan> & messageIds)
{
    auto iter = messageIds.begin();
    while (iter.Next())
    {
        auto & id = iter.GetValue();

        mCachedMessages.remove_if([id](CachedMessage & entry) {
            if (entry.mMessageId.data_equal(id))
            {
                return true;
            }
            return false;
        });
        // per spec, the command succeeds even when the message id does not match an existing message
    }
}

// Attributes
CHIP_ERROR MessagesManager::HandleGetMessages(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (CachedMessage & entry : mCachedMessages)
        {
            ReturnErrorOnFailure(encoder.Encode(entry.GetMessage()));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR MessagesManager::HandleGetActiveMessageIds(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (CachedMessage & entry : mCachedMessages)
        {
            ReturnErrorOnFailure(encoder.Encode(entry.GetMessage().messageID));
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
