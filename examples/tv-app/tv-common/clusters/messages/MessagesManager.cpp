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

#include <algorithm>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TypeTraits.h>
#include <platform/CHIPDeviceLayer.h>
#include <string>
#include <vector>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::Messages;
using Message               = chip::app::Clusters::Messages::Structs::MessageStruct::Type;
using MessageResponseOption = chip::app::Clusters::Messages::Structs::MessageResponseOptionStruct::Type;

MessagesManager::~MessagesManager()
{
    for (auto & context : mTimerContexts)
    {
        DeviceLayer::SystemLayer().CancelTimer(OnMessageTimerExpired, context.get());
    }
}

// Commands
CHIP_ERROR MessagesManager::HandlePresentMessagesRequest(
    const ByteSpan & messageId, const MessagePriorityEnum & priority, const BitMask<MessageControlBitmap> & messageControl,
    const DataModel::Nullable<uint32_t> & startTime, const DataModel::Nullable<uint64_t> & duration, const CharSpan & messageText,
    const Optional<DataModel::DecodableList<MessageResponseOption>> & responses, const Optional<CharSpan> & languageCode,
    const Optional<CharSpan> & messageUri, FabricIndex fabricIndex)
{
    ChipLogProgress(Zcl, "HandlePresentMessagesRequest message:%s", std::string(messageText.data(), messageText.size()).c_str());

    auto cachedMessage = CachedMessage(
        messageId, priority, messageControl, startTime, duration, std::string(messageText.data(), messageText.size()),
        (languageCode.HasValue() && !languageCode.Value().empty())
            ? std::string(languageCode.Value().data(), languageCode.Value().size())
            : std::string(),
        (messageUri.HasValue() && !messageUri.Value().empty()) ? std::string(messageUri.Value().data(), messageUri.Value().size())
                                                               : std::string(),
        fabricIndex);
    if (responses.HasValue())
    {
        auto iter = responses.Value().begin();
        while (iter.Next())
        {
            auto & response = iter.GetValue();

            CachedMessageOption option(response.messageResponseID.Value(),
                                       std::string(response.label.Value().data(), response.label.Value().size()));

            cachedMessage.AddOption(option);
        }
    }

    // Re-presenting an already-queued MessageID replaces the prior entry (and its pending
    // timers) instead of silently duplicating it, keeping MessageID usable as a real key.
    CancelMessageTimers(messageId);
    auto existing = FindCachedMessage(messageId);
    if (existing != mCachedMessages.end())
    {
        if (existing->GetState() == MessageState::kPresented)
        {
            CompleteMessage(messageId);
        }
        else
        {
            mCachedMessages.erase(existing);
        }
    }

    mCachedMessages.push_back(cachedMessage);
    LogErrorOnFailure(LogMessageQueuedEvent(mEndpointId, messageId));

    ScheduleOrPresentMessage(messageId);

    ChipLogProgress(Zcl, "HandlePresentMessagesRequest complete");
    return CHIP_NO_ERROR;
}

CHIP_ERROR MessagesManager::HandleCancelMessagesRequest(const DataModel::DecodableList<ByteSpan> & messageIds)
{
    auto iter = messageIds.begin();
    while (iter.Next())
    {
        auto & id = iter.GetValue();

        CancelMessageTimers(id);
        auto it = FindCachedMessage(id);
        if (it == mCachedMessages.end())
        {
            continue;
        }
        if (it->GetState() == MessageState::kPresented)
        {
            CompleteMessage(id);
        }
        else
        {
            mCachedMessages.erase(it);
        }
    }
    return CHIP_NO_ERROR;
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
            if (entry.GetState() == MessageState::kPresented)
            {
                ReturnErrorOnFailure(encoder.Encode(entry.GetMessage().messageID));
            }
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR MessagesManager::HandleGetSupportedLanguageCodes(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
        ReturnErrorOnFailure(encoder.Encode("en-US"_span));
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR MessagesManager::HandleGetSupportedMimeTypes(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
        ReturnErrorOnFailure(encoder.Encode("audio/mpeg"_span));
        ReturnErrorOnFailure(encoder.Encode("audio/ogg"_span));
        return CHIP_NO_ERROR;
    });
}

// Global Attributes
uint32_t MessagesManager::GetFeatureMap(EndpointId endpoint)
{
    BitMask<Feature> FeatureMap;
    FeatureMap.Set(Feature::kReceivedConfirmation);
    FeatureMap.Set(Feature::kConfirmationResponse);
    FeatureMap.Set(Feature::kConfirmationReply);
    FeatureMap.Set(Feature::kProtectedMessages);
    FeatureMap.Set(Feature::kSpokenMessages);
    FeatureMap.Set(Feature::kAudioMessages);
    FeatureMap.Set(Feature::kMultiModalMessages);

    uint32_t featureMap = FeatureMap.Raw();
    ChipLogProgress(Zcl, "GetFeatureMap featureMap=%d", featureMap);
    // forcing to all features since this implementation supports all
    // Attributes::FeatureMap::Get(endpoint, &featureMap);
    return featureMap;
}

// State machine

std::list<CachedMessage>::iterator MessagesManager::FindCachedMessage(ByteSpan messageId)
{
    return std::find_if(mCachedMessages.begin(), mCachedMessages.end(),
                        [&messageId](CachedMessage & entry) { return entry.MessageIdMatches(messageId); });
}

void MessagesManager::ScheduleOrPresentMessage(ByteSpan messageId)
{
    auto it = FindCachedMessage(messageId);
    if (it == mCachedMessages.end())
    {
        // Already cancelled/completed before this ran.
        return;
    }

    const auto & startTime = it->GetStartTime();
    if (startTime.IsNull())
    {
        PresentOrSuppressMessage(it);
        return;
    }

    uint32_t nowEpochS = 0;
    CHIP_ERROR err     = System::Clock::GetClock_MatterEpochS(nowEpochS);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(Zcl, "MessagesManager: could not read current time (%" CHIP_ERROR_FORMAT "), rechecking in %u s",
                        err.Format(), kClockRecheckIntervalSeconds);
        StartMessageTimer(messageId, MessageTimerType::kPresent, kClockRecheckIntervalSeconds * 1000);
        return;
    }

    if (startTime.Value() > nowEpochS)
    {
        // Cap the delay so `delaySeconds * 1000` can't overflow uint32_t (~49.7 days worth of
        // ms); if StartTime is further out than that, wait the max chunk and let this function
        // re-run and re-check/reschedule, same as the clock-error retry above.
        constexpr uint32_t kMaxSingleTimerDelaySeconds = UINT32_MAX / 1000;
        uint32_t delaySeconds                          = startTime.Value() - nowEpochS;
        if (delaySeconds > kMaxSingleTimerDelaySeconds)
        {
            delaySeconds = kMaxSingleTimerDelaySeconds;
        }
        StartMessageTimer(messageId, MessageTimerType::kPresent, delaySeconds * 1000);
        return;
    }

    PresentOrSuppressMessage(it);
}

void MessagesManager::PresentOrSuppressMessage(std::list<CachedMessage>::iterator it)
{
    if (mDoNotDisturb)
    {
        LogErrorOnFailure(LogMessageNotPresentedEvent(mEndpointId, it->GetMessageId(), true, it->GetFabricIndex()));
        mCachedMessages.erase(it);
        return;
    }
    PresentMessage(*it);
}

void MessagesManager::PresentMessage(CachedMessage & message)
{
    auto messageStruct = message.GetMessage();
    std::string text(messageStruct.messageText.data(), messageStruct.messageText.size());
    const auto & controlBits = message.GetMessageControl();

    if (controlBits.Has(MessageControlBitmap::kSpokenMessage))
    {
        std::string language = (messageStruct.languageCode.HasValue())
            ? std::string(messageStruct.languageCode.Value().data(), messageStruct.languageCode.Value().size())
            : "en-US";
        ChipLogProgress(Zcl, "MessagesManager: [simulated TTS, language=%s] \"%s\"", language.c_str(), text.c_str());
    }
    else if (controlBits.Has(MessageControlBitmap::kAudioMessage))
    {
        std::string uri = (messageStruct.messageURI.HasValue())
            ? std::string(messageStruct.messageURI.Value().data(), messageStruct.messageURI.Value().size())
            : "<no URI>";
        ChipLogProgress(Zcl, "MessagesManager: [simulated audio playback, uri=%s] \"%s\"", uri.c_str(), text.c_str());
    }
    else
    {
        ChipLogProgress(Zcl, "MessagesManager: [presented] \"%s\"", text.c_str());
    }

    message.SetState(MessageState::kPresented);
    LogErrorOnFailure(LogMessagePresentedEvent(mEndpointId, message.GetMessageId()));

    const auto & duration = message.GetDuration();
    if (!duration.IsNull())
    {
        uint64_t durationMs = duration.Value();
        if (durationMs > UINT32_MAX)
        {
            durationMs = UINT32_MAX;
        }
        StartMessageTimer(message.GetMessageId(), MessageTimerType::kComplete, static_cast<uint32_t>(durationMs));
    }
}

void MessagesManager::CompleteMessage(ByteSpan messageId)
{
    auto it = FindCachedMessage(messageId);
    if (it == mCachedMessages.end())
    {
        return;
    }

    // Auto-dismissed after Duration elapsed with no real user response available
    // (this example app has no UI to collect one).
    LogErrorOnFailure(LogMessageCompleteEvent(mEndpointId, messageId, Optional<DataModel::Nullable<uint32_t>>(),
                                              Optional<DataModel::Nullable<CharSpan>>(),
                                              DataModel::Nullable<FutureMessagePreferenceEnum>()));

    mCachedMessages.erase(it);
}

void MessagesManager::StartMessageTimer(ByteSpan messageId, MessageTimerType type, uint32_t delayMs)
{
    if (messageId.size() != sizeof(MessageTimerContext::messageId))
    {
        ChipLogError(Zcl, "MessagesManager: unexpected message id size %u", static_cast<unsigned>(messageId.size()));
        return;
    }

    auto context     = std::make_shared<MessageTimerContext>();
    context->manager = this;
    context->type    = type;
    memcpy(context->messageId, messageId.data(), sizeof(context->messageId));

    CHIP_ERROR err =
        DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(delayMs), OnMessageTimerExpired, context.get());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "MessagesManager: failed to start timer: %" CHIP_ERROR_FORMAT, err.Format());
        return;
    }
    mTimerContexts.push_back(context);
}

void MessagesManager::CancelMessageTimers(ByteSpan messageId)
{
    for (auto it = mTimerContexts.begin(); it != mTimerContexts.end();)
    {
        if (ByteSpan((*it)->messageId).data_equal(messageId))
        {
            DeviceLayer::SystemLayer().CancelTimer(OnMessageTimerExpired, it->get());
            it = mTimerContexts.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void MessagesManager::OnMessageTimerExpired(System::Layer * systemLayer, void * context)
{
    auto * ctx             = reinterpret_cast<MessageTimerContext *>(context);
    MessagesManager * self = ctx->manager;
    MessageTimerType type  = ctx->type;
    // Copied out to a local buffer *before* erasing below, since that erase may drop the last
    // owning shared_ptr to `ctx` -- it must not be dereferenced again after this point.
    uint8_t messageIdBuffer[kMessageIdLength];
    memcpy(messageIdBuffer, ctx->messageId, sizeof(messageIdBuffer));
    ByteSpan messageId(messageIdBuffer);

    self->mTimerContexts.erase(
        std::remove_if(self->mTimerContexts.begin(), self->mTimerContexts.end(),
                       [ctx](const std::shared_ptr<MessageTimerContext> & entry) { return entry.get() == ctx; }),
        self->mTimerContexts.end());

    if (type == MessageTimerType::kPresent)
    {
        self->ScheduleOrPresentMessage(messageId);
    }
    else
    {
        self->CompleteMessage(messageId);
    }
}

void MessagesManager::LogCachedMessages() const
{
    ChipLogProgress(Zcl, "MessagesManager: do-not-disturb=%s", mDoNotDisturb ? "on" : "off");
    if (mCachedMessages.empty())
    {
        ChipLogProgress(Zcl, "MessagesManager: no cached messages");
        return;
    }
    for (const CachedMessage & entry : mCachedMessages)
    {
        auto messageStruct = entry.GetMessage();
        std::string text(messageStruct.messageText.data(), messageStruct.messageText.size());
        const char * state = entry.GetState() == MessageState::kQueued ? "Queued" : "Presented";
        ChipLogProgress(Zcl, "MessagesManager:   [%s] priority=%u \"%s\"", state, to_underlying(messageStruct.priority),
                        text.c_str());
    }
}
