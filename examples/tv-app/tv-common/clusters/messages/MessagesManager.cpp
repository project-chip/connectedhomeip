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
    ChipLogProgress(Zcl, "HandlePresentMessagesRequest message:%s", std::string(messageText.data(), messageText.size()).c_str());

    VerifyOrReturn(messageId.size() == CachedMessage::kMessageIdLength,
                   ChipLogProgress(Zcl, "HandlePresentMessagesRequest invalid message id length:%zu", messageId.size()));

    auto cachedMessage = CachedMessage(messageId, priority, messageControl, startTime, duration,
                                       std::string(messageText.data(), messageText.size()));
    if (responses.HasValue())
    {
        size_t size    = 0;
        CHIP_ERROR err = responses.Value().ComputeSize(&size);
        VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogProgress(Zcl, "HandlePresentMessagesRequest size check failed"));

        VerifyOrReturn(size <= CachedMessage::kMaxOptionCount,
                       ChipLogProgress(Zcl, "HandlePresentMessagesRequest too many options"));

        auto iter = responses.Value().begin();
        while (iter.Next())
        {
            auto & response = iter.GetValue();

            VerifyOrReturn(response.messageResponseID.HasValue() && response.label.HasValue(),
                           ChipLogProgress(Zcl, "HandlePresentMessagesRequest missing response id or label"));

            VerifyOrReturn(response.messageResponseID.Value() >= CachedMessageOption::kResponseIdMin,
                           ChipLogProgress(Zcl, "HandlePresentMessagesRequest responseID value check failed"));

            VerifyOrReturn(response.label.Value().size() <= CachedMessageOption::kLabelMaxLength,
                           ChipLogProgress(Zcl, "HandlePresentMessagesRequest label length check failed"));

            CachedMessageOption option(response.messageResponseID.Value(),
                                       std::string(response.label.Value().data(), response.label.Value().size()));

            cachedMessage.AddOption(option);
        }
        VerifyOrReturn(iter.GetStatus() == CHIP_NO_ERROR, ChipLogProgress(Zcl, "HandlePresentMessagesRequest TLV parsing error"));
    }

    mCachedMessages.push_back(cachedMessage);

    // Add your code to present Message
    ChipLogProgress(Zcl, "HandlePresentMessagesRequest complete");
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
    uint32_t featureMap = 15; // all features enabled by default
    Attributes::FeatureMap::Get(endpoint, &featureMap);
    return featureMap;
}
