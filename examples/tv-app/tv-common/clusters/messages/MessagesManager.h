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
#include <vector>

/**
 * Note on memory management:
 *
 * The CachedMessage contains strings and objects
 * allocated when created (via HandlePresentMessagesRequest) and freed
 * in the destructor (via HandleCancelMessagesRequest).
 *
 */

struct CachedMessageOption
{
    uint32_t mId;
    std::string mLabel;
    chip::app::Clusters::Messages::MessageResponseOption mOption;

    CachedMessageOption(uint32_t id, std::string label) :
        mId(id), mLabel(label),
        mOption{ chip::Optional<uint32_t>(mId), chip::Optional<chip::CharSpan>(chip::CharSpan::fromCharString(mLabel.c_str())) }
    {
        // ChipLogProgress(Controller, "CachedMessageOption constructor id=%d label=%s", mId, mLabel.c_str());
    }

    chip::app::Clusters::Messages::MessageResponseOption GetMessageOption() { return mOption; }

    ~CachedMessageOption()
    {
        // ChipLogProgress(Controller, "CachedMessageOption destructor");
    }
};

struct CachedMessage
{
    const chip::app::Clusters::Messages::MessagePriorityEnum mPriority;
    const chip::BitMask<chip::app::Clusters::Messages::MessageControlBitmap> mMessageControl;
    const chip::app::DataModel::Nullable<uint32_t> mStartTime;
    const chip::app::DataModel::Nullable<uint16_t> mDuration;

    std::string mMessageText;

    uint8_t * messageIdBuffer = nullptr;
    chip::ByteSpan mMessageId;

    CachedMessage(const chip::ByteSpan & messageId, const chip::app::Clusters::Messages::MessagePriorityEnum & priority,
                  const chip::BitMask<chip::app::Clusters::Messages::MessageControlBitmap> & messageControl,
                  const chip::app::DataModel::Nullable<uint32_t> & startTime,
                  const chip::app::DataModel::Nullable<uint16_t> & duration, std::string messageText) :
        mPriority(priority),
        mMessageControl(messageControl), mStartTime(startTime), mDuration(duration), mMessageText(messageText)
    {
        messageIdBuffer = new uint8_t[messageId.size()];
        VerifyOrReturn(messageIdBuffer != nullptr, ChipLogProgress(Controller, "CachedMessage messageIdBuffer alloc failed"));
        memcpy(messageIdBuffer, messageId.data(), messageId.size());
        mMessageId = chip::ByteSpan(messageIdBuffer, messageId.size());
    }

    void AddOption(CachedMessageOption * option)
    {
        mOptions.push_back(*option);
        mResponseOptions.push_back(option->GetMessageOption());
    }

    chip::app::Clusters::Messages::Structs::MessageStruct::Type GetMessage()
    {
        chip::app::DataModel::List<chip::app::Clusters::Messages::MessageResponseOption> options(mResponseOptions.data(),
                                                                                                 mResponseOptions.size());
        chip::app::Clusters::Messages::Structs::MessageStruct::Type message{
            mMessageId,
            mPriority,
            mMessageControl,
            mStartTime,
            mDuration,
            chip::CharSpan::fromCharString(mMessageText.c_str()),
            chip::Optional<chip::app::DataModel::List<chip::app::Clusters::Messages::MessageResponseOption>>(options)
        };
        return message;
    }

    ~CachedMessage()
    {
        if (messageIdBuffer != nullptr)
        {
            delete[] messageIdBuffer;
        }
    }

protected:
    std::vector<chip::app::Clusters::Messages::MessageResponseOption> mResponseOptions;
    std::list<CachedMessageOption> mOptions;
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
