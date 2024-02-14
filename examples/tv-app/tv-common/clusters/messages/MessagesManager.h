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

#pragma once

#include <app/clusters/messages-server/messages-server.h>

#include <iostream>
#include <list>

/**
 * Note on memory management:
 *
 * The CachedMessage contains strings and objects
 * allocated when created (via HandlePresentMessagesRequest) and freed
 * in the destructor (via HandleCancelMessagesRequest).
 *
 */

struct CachedMessage
{
    chip::ByteSpan mMessageId;

    const chip::app::Clusters::Messages::MessagePriorityEnum mPriority;
    const chip::BitMask<chip::app::Clusters::Messages::MessageControlBitmap> mMessageControl;
    const chip::app::DataModel::Nullable<uint32_t> mStartTime;
    const chip::app::DataModel::Nullable<uint16_t> mDuration;

    std::string mMessageText;

    chip::app::Clusters::Messages::MessageResponseOption * mOptionArray = nullptr;
    size_t mOptionArraySize                                             = 0;

    CachedMessage(uint8_t * messageId, size_t messageIdSize, const chip::app::Clusters::Messages::MessagePriorityEnum & priority,
                  const chip::BitMask<chip::app::Clusters::Messages::MessageControlBitmap> & messageControl,
                  const chip::app::DataModel::Nullable<uint32_t> & startTime,
                  const chip::app::DataModel::Nullable<uint16_t> & duration, std::string messageText) :
        mMessageId(messageId, messageIdSize),
        mPriority(priority), mMessageControl(messageControl), mStartTime(startTime), mDuration(duration), mMessageText(messageText)
    {}

    CHIP_ERROR SetOptionArraySize(size_t size)
    {
        mOptionArray = new chip::app::Clusters::Messages::MessageResponseOption[size];
        if (mOptionArray == nullptr)
        {
            ChipLogProgress(Controller, "HandlePresentMessagesRequest MessageResponseOption alloc failed");
            return CHIP_ERROR_NO_MEMORY;
        }
        mOptionArraySize = size;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetOption(size_t index, chip::Optional<uint32_t> id, chip::Optional<chip::CharSpan> label)
    {
        if (index >= mOptionArraySize)
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }
        mOptionArray[index].messageResponseID = id;
        mOptionArray[index].label             = label;
        return CHIP_NO_ERROR;
    }

    chip::app::Clusters::Messages::Structs::MessageStruct::Type GetMessage()
    {
        chip::app::Clusters::Messages::Structs::MessageStruct::Type message{
            mMessageId,
            mPriority,
            mMessageControl,
            mStartTime,
            mDuration,
            chip::CharSpan::fromCharString(mMessageText.c_str()),
            chip::Optional<chip::app::DataModel::List<chip::app::Clusters::Messages::MessageResponseOption>>(
                chip::app::DataModel::List<chip::app::Clusters::Messages::MessageResponseOption>(mOptionArray, mOptionArraySize))
        };
        return message;
    }

    ~CachedMessage()
    {
        ChipLogProgress(Controller, "CachedMessage destructor start");
        delete[] mMessageId.data();
        for (size_t i = 0; i < mOptionArraySize; i++)
        {
            if (mOptionArray[i].label.HasValue())
            {
                delete[] mOptionArray[i].label.Value().data();
            }
        }
        delete[] mOptionArray;
        ChipLogProgress(Controller, "CachedMessage destructor done");
    }
};

class MessagesManager : public chip::app::Clusters::Messages::Delegate
{
public:
    // Commands
    void HandlePresentMessagesRequest(
        const chip::ByteSpan & messageId, const chip::app::Clusters::Messages::MessagePriorityEnum & priority,
        const chip::BitMask<chip::app::Clusters::Messages::MessageControlBitmap> & messageControl,
        const chip::app::DataModel::Nullable<uint32_t> & startTime, const chip::app::DataModel::Nullable<uint16_t> & duration,
        const chip::CharSpan & messageText,
        const chip::Optional<
            chip::app::DataModel::DecodableList<chip::app::Clusters::Messages::Structs::MessageResponseOptionStruct::Type>> &
            responses) override;
    void HandleCancelMessagesRequest(const chip::app::DataModel::DecodableList<chip::ByteSpan> & messageIds) override;

    // Attributes
    CHIP_ERROR HandleGetMessages(chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR HandleGetActiveMessageIds(chip::app::AttributeValueEncoder & aEncoder) override;

    // Global Attributes
    uint32_t GetFeatureMap(chip::EndpointId endpoint) override;

protected:
    std::list<CachedMessage> mCachedMessages;
};
