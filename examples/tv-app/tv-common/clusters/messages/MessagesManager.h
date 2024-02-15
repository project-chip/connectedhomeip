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

struct CachedMessageOption
{
    constexpr static size_t kResponseIdMin  = 1;
    constexpr static size_t kLabelMaxLength = 32;

    CachedMessageOption(uint32_t id, std::string label) :
        mLabel(label), mOption{ chip::MakeOptional(id), chip::MakeOptional(chip::CharSpan::fromCharString(mLabel.c_str())) }
    {}

    CachedMessageOption(const CachedMessageOption & option) :
        mLabel(option.mLabel), mOption{ option.mOption.messageResponseID,
                                        chip::MakeOptional(chip::CharSpan::fromCharString(mLabel.c_str())) } {};

    CachedMessageOption & operator=(const CachedMessageOption & option) { return *this; };

    chip::app::Clusters::Messages::MessageResponseOption GetMessageOption() { return mOption; }

    ~CachedMessageOption() {}

protected:
    std::string mLabel;
    chip::app::Clusters::Messages::MessageResponseOption mOption;
};

struct CachedMessage
{
    constexpr static size_t kMessageIdLength = 16;
    constexpr static size_t kMaxOptionCount  = 4;

    chip::ByteSpan mMessageId;

    CachedMessage(const CachedMessage & message) :
        mPriority(message.mPriority), mMessageControl(message.mMessageControl), mStartTime(message.mStartTime),
        mDuration(message.mDuration), mMessageText(message.mMessageText), mOptions(message.mOptions)
    {
        memcpy(mMessageIdBuffer, message.mMessageIdBuffer, kMessageIdLength);
        mMessageId = chip::ByteSpan(mMessageIdBuffer, kMessageIdLength);

        for (CachedMessageOption & entry : mOptions)
        {
            mResponseOptions.push_back(entry.GetMessageOption());
        }
    };

    CachedMessage & operator=(const CachedMessage & message) { return *this; };

    CachedMessage(const chip::ByteSpan & messageId, const chip::app::Clusters::Messages::MessagePriorityEnum & priority,
                  const chip::BitMask<chip::app::Clusters::Messages::MessageControlBitmap> & messageControl,
                  const chip::app::DataModel::Nullable<uint32_t> & startTime,
                  const chip::app::DataModel::Nullable<uint16_t> & duration, std::string messageText) :
        mPriority(priority),
        mMessageControl(messageControl), mStartTime(startTime), mDuration(duration), mMessageText(messageText)
    {
        if (messageId.size() != kMessageIdLength)
        {
            ChipLogProgress(Zcl, "CachedMessage bad message id size, ignoring");
            return;
        }
        memcpy(mMessageIdBuffer, messageId.data(), kMessageIdLength);
        mMessageId = chip::ByteSpan(mMessageIdBuffer, kMessageIdLength);
    }

    void AddOption(CachedMessageOption option)
    {
        mOptions.push_back(option);
        mResponseOptions.push_back(option.GetMessageOption());
    }

    chip::app::Clusters::Messages::Structs::MessageStruct::Type GetMessage()
    {
        if (mResponseOptions.size() > 0)
        {
            chip::app::DataModel::List<chip::app::Clusters::Messages::MessageResponseOption> options(mResponseOptions.data(),
                                                                                                     mResponseOptions.size());
            chip::app::Clusters::Messages::Structs::MessageStruct::Type message{ mMessageId,
                                                                                 mPriority,
                                                                                 mMessageControl,
                                                                                 mStartTime,
                                                                                 mDuration,
                                                                                 chip::CharSpan::fromCharString(
                                                                                     mMessageText.c_str()),
                                                                                 chip::MakeOptional(options) };
            return message;
        }
        else
        {
            chip::app::Clusters::Messages::Structs::MessageStruct::Type message{
                mMessageId, mPriority, mMessageControl, mStartTime, mDuration, chip::CharSpan::fromCharString(mMessageText.c_str())
            };
            return message;
        }
    }

    ~CachedMessage() {}

protected:
    const chip::app::Clusters::Messages::MessagePriorityEnum mPriority;
    const chip::BitMask<chip::app::Clusters::Messages::MessageControlBitmap> mMessageControl;
    const chip::app::DataModel::Nullable<uint32_t> mStartTime;
    const chip::app::DataModel::Nullable<uint16_t> mDuration;

    std::string mMessageText;
    uint8_t mMessageIdBuffer[kMessageIdLength];

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
