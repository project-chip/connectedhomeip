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
#include <memory>
#include <string>
#include <vector>

// Completion always coincides with removal from mCachedMessages (see
// MessagesManager::CompleteMessage), so there's no separate "complete" state to observe --
// only these two are ever set.
enum class MessageState : uint8_t
{
    kQueued,
    kPresented,
};

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
        mMessageUri(message.mMessageUri), mOptions(message.mOptions), mState(message.mState)
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
                  const chip::app::DataModel::Nullable<uint64_t> & duration, std::string messageText, std::string languageCode,
                  std::string messageUri) :
        mPriority(priority),
        mMessageControl(messageControl), mStartTime(startTime), mDuration(duration), mMessageText(messageText),
        mLanguageCode(languageCode), mMessageUri(messageUri)
    {
        memcpy(mMessageIdBuffer, messageId.data(), sizeof(mMessageIdBuffer));
    }

    bool MessageIdMatches(const chip::ByteSpan & id) const { return chip::ByteSpan(mMessageIdBuffer).data_equal(id); }

    void AddOption(CachedMessageOption option)
    {
        mOptions.push_back(option);
        mResponseOptions.push_back(option.GetMessageOption());
    }

    chip::ByteSpan GetMessageId() const { return chip::ByteSpan(mMessageIdBuffer); }
    const chip::BitMask<chip::app::Clusters::Messages::MessageControlBitmap> & GetMessageControl() const { return mMessageControl; }
    const chip::app::DataModel::Nullable<uint32_t> & GetStartTime() const { return mStartTime; }
    const chip::app::DataModel::Nullable<uint64_t> & GetDuration() const { return mDuration; }
    MessageState GetState() const { return mState; }
    void SetState(MessageState state) { mState = state; }

    chip::app::Clusters::Messages::Structs::MessageStruct::Type GetMessage() const
    {
        chip::app::Clusters::Messages::Structs::MessageStruct::Type message{ chip::ByteSpan(mMessageIdBuffer),
                                                                             mPriority,
                                                                             mMessageControl,
                                                                             mStartTime,
                                                                             mDuration,
                                                                             chip::CharSpan::fromCharString(mMessageText.c_str()) };
        if (mResponseOptions.size() > 0)
        {
            chip::app::DataModel::List<const chip::app::Clusters::Messages::Structs::MessageResponseOptionStruct::Type> options(
                mResponseOptions.data(), mResponseOptions.size());
            message.responses = chip::MakeOptional(options);
        }
        if (!mLanguageCode.empty())
        {
            message.languageCode = chip::MakeOptional(chip::CharSpan(mLanguageCode.data(), mLanguageCode.size()));
        }
        if (!mMessageUri.empty())
        {
            message.messageURI = chip::MakeOptional(chip::CharSpan(mMessageUri.data(), mMessageUri.size()));
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
    MessageState mState = MessageState::kQueued;
};

class MessagesManager : public chip::app::Clusters::Messages::Delegate
{
public:
    ~MessagesManager() override;

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

    // Not part of the Delegate contract. ZCLCallbacks.cpp calls this once, right
    // after SetDefaultDelegate(), so events can be logged against the right endpoint.
    void SetEndpointId(chip::EndpointId endpoint) { mEndpointId = endpoint; }

    // Not part of the Delegate contract. Read-only introspection for the `messages
    // list` shell command.
    void LogCachedMessages() const;

private:
    enum class MessageTimerType : uint8_t
    {
        kPresent,
        kComplete,
    };

    struct MessageTimerContext
    {
        MessagesManager * manager;
        uint8_t messageId[chip::app::Clusters::Messages::kMessageIdLength];
        MessageTimerType type;
    };

    static constexpr uint32_t kClockRecheckIntervalSeconds = 30;

    std::list<CachedMessage>::iterator FindCachedMessage(const chip::ByteSpan & messageId);

    // Computes the delay until `messageId`'s StartTime and (re-)schedules the
    // present-timer for it, or presents it immediately if StartTime has already
    // passed (or is null). Also used to recheck once the real time becomes synced.
    void ScheduleOrPresentMessage(const chip::ByteSpan & messageId);
    void PresentMessage(CachedMessage & message);
    void CompleteMessage(const chip::ByteSpan & messageId);

    void StartMessageTimer(const chip::ByteSpan & messageId, MessageTimerType type, uint32_t delayMs);
    void CancelMessageTimers(const chip::ByteSpan & messageId);
    static void OnMessageTimerExpired(chip::System::Layer * systemLayer, void * context);

    chip::EndpointId mEndpointId = chip::kInvalidEndpointId;
    std::list<CachedMessage> mCachedMessages;
    std::vector<std::shared_ptr<MessageTimerContext>> mTimerContexts;
};

// Defined in ZCLCallbacks.cpp; returns the live Messages delegate instance for
// use by the `messages` shell command.
MessagesManager * GetMessagesManager();
