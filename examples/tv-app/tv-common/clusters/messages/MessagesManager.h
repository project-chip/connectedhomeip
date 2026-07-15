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
#include <string>
#include <vector>

struct CachedMessageOption
{
    CachedMessageOption(uint32_t id, std::string label) :
        mLabel(label), mOption{ chip::MakeOptional(id), chip::MakeOptional(chip::CharSpan::fromCharString(mLabel.c_str())) }
    {}

    CachedMessageOption(const CachedMessageOption & option) :
        mLabel(option.mLabel),
        mOption{ option.mOption.messageResponseID, chip::MakeOptional(chip::CharSpan::fromCharString(mLabel.c_str())) }
    {}

    CachedMessageOption & operator=(const CachedMessageOption & option) = delete;

    chip::app::Clusters::Messages::Structs::MessageResponseOptionStruct::Type GetMessageOption() { return mOption; }

    ~CachedMessageOption() {}

protected:
    std::string mLabel;
    chip::app::Clusters::Messages::Structs::MessageResponseOptionStruct::Type mOption;
};

struct CachedMessage
{
    CachedMessage(const CachedMessage & message) :
        mPriority(message.mPriority), mMessageControl(message.mMessageControl), mStartTime(message.mStartTime),
        mDuration(message.mDuration), mMessageText(message.mMessageText), mLanguageCode(message.mLanguageCode),
        mMessageUri(message.mMessageUri), mOptions(message.mOptions)
    {
        memcpy(mMessageIdBuffer, message.mMessageIdBuffer, sizeof(mMessageIdBuffer));

        for (CachedMessageOption & entry : mOptions)
        {
            mResponseOptions.push_back(entry.GetMessageOption());
        }
    }

    CachedMessage & operator=(const CachedMessage & message) = delete;

    CachedMessage(const chip::ByteSpan & messageId, const chip::app::Clusters::Messages::MessagePriorityEnum & priority,
                  const chip::BitMask<chip::app::Clusters::Messages::MessageControlBitmap> & messageControl,
                  const chip::app::DataModel::Nullable<uint32_t> & startTime,
                  const chip::app::DataModel::Nullable<uint64_t> & duration, std::string messageText,
                  std::string languageCode, std::string messageUri) :
        mPriority(priority),
        mMessageControl(messageControl), mStartTime(startTime), mDuration(duration), mMessageText(messageText),
        mLanguageCode(languageCode), mMessageUri(messageUri)
    {
        memcpy(mMessageIdBuffer, messageId.data(), sizeof(mMessageIdBuffer));
    }

    bool MessageIdMatches(const chip::ByteSpan & id) { return chip::ByteSpan(mMessageIdBuffer).data_equal(id); }

    void AddOption(CachedMessageOption option)
    {
        mOptions.push_back(option);
        mResponseOptions.push_back(option.GetMessageOption());
    }

    chip::app::Clusters::Messages::Structs::MessageStruct::Type GetMessage()
    {
        chip::app::Clusters::Messages::Structs::MessageStruct::Type message{ chip::ByteSpan(mMessageIdBuffer),
                                                                             mPriority,
                                                                             mMessageControl,
                                                                             mStartTime,
                                                                             mDuration,
                                                                             chip::CharSpan::fromCharString(mMessageText.c_str()) };
        if (mResponseOptions.size() > 0)
        {
            chip::app::DataModel::List<chip::app::Clusters::Messages::Structs::MessageResponseOptionStruct::Type> options(
                mResponseOptions.data(), mResponseOptions.size());
            message.responses = chip::MakeOptional(options);
        }
        if (!mLanguageCode.empty())
        {
            message.languageCode = chip::MakeOptional(chip::CharSpan::fromCharString(mLanguageCode.c_str()));
        }
        if (!mMessageUri.empty())
        {
            message.messageURI = chip::MakeOptional(chip::CharSpan::fromCharString(mMessageUri.c_str()));
        }
        return message;
    }

    ~CachedMessage() {}

protected:
    const chip::app::Clusters::Messages::MessagePriorityEnum mPriority;
    const chip::BitMask<chip::app::Clusters::Messages::MessageControlBitmap> mMessageControl;
    const chip::app::DataModel::Nullable<uint32_t> mStartTime;
    const chip::app::DataModel::Nullable<uint64_t> mDuration;

    std::string mMessageText;
    std::string mLanguageCode;
    std::string mMessageUri;
    uint8_t mMessageIdBuffer[chip::app::Clusters::Messages::kMessageIdLength];

    std::vector<chip::app::Clusters::Messages::Structs::MessageResponseOptionStruct::Type> mResponseOptions;
    std::list<CachedMessageOption> mOptions;
};

class MessagesManager : public chip::app::Clusters::Messages::Delegate
{
public:
    // Commands
    CHIP_ERROR HandlePresentMessagesRequest(
        const chip::ByteSpan & messageId, const chip::app::Clusters::Messages::MessagePriorityEnum & priority,
        const chip::BitMask<chip::app::Clusters::Messages::MessageControlBitmap> & messageControl,
        const chip::app::DataModel::Nullable<uint32_t> & startTime, const chip::app::DataModel::Nullable<uint64_t> & duration,
        const chip::CharSpan & messageText,
        const chip::Optional<
            chip::app::DataModel::DecodableList<chip::app::Clusters::Messages::Structs::MessageResponseOptionStruct::Type>> &
            responses,
        const chip::Optional<chip::CharSpan> & languageCode, const chip::Optional<chip::CharSpan> & messageUri) override;
    CHIP_ERROR HandleCancelMessagesRequest(const chip::app::DataModel::DecodableList<chip::ByteSpan> & messageIds) override;

    // Attributes
    CHIP_ERROR HandleGetMessages(chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR HandleGetActiveMessageIds(chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR HandleGetSupportedLanguageCodes(chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR HandleGetSupportedMimeTypes(chip::app::AttributeValueEncoder & aEncoder) override;

    // Global Attributes
    uint32_t GetFeatureMap(chip::EndpointId endpoint) override;

protected:
    std::list<CachedMessage> mCachedMessages;
};
