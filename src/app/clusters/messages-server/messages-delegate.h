/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
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

#include <app-common/zap-generated/cluster-objects.h>

#include <app/AttributeValueEncoder.h>
#include <app/CommandResponseHelper.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Messages {

constexpr static size_t kMessageIdLength               = 16;
constexpr static size_t kMessageTextLengthMax          = 256;
constexpr static size_t kMessageMaxOptionCount         = 4;
constexpr static size_t kMessageResponseIdMin          = 1;
constexpr static size_t kMessageResponseLabelMaxLength = 32;
constexpr static size_t kLanguageCodeLengthMax         = 32;
constexpr static size_t kMessageUriLengthMax           = 256;
constexpr static size_t kSupportedLanguageCodeMaxCount = 32;
constexpr static size_t kSupportedMimeTypeMaxCount     = 64;
constexpr static size_t kSupportedMimeTypeLengthMax    = 256;

class Delegate
{
public:
    // Commands
    virtual CHIP_ERROR HandlePresentMessagesRequest(
        const ByteSpan & messageId, const MessagePriorityEnum & priority,
        const chip::BitMask<MessageControlBitmap> & messageControl, const DataModel::Nullable<uint32_t> & startTime,
        const DataModel::Nullable<uint64_t> & duration, const CharSpan & messageText,
        const chip::Optional<DataModel::DecodableList<chip::app::Clusters::Messages::Structs::MessageResponseOptionStruct::Type>> &
            responses,
        const chip::Optional<CharSpan> & languageCode, const chip::Optional<CharSpan> & messageUri)             = 0;
    virtual CHIP_ERROR HandleCancelMessagesRequest(const DataModel::DecodableList<chip::ByteSpan> & messageIds) = 0;

    // Attributes
    virtual CHIP_ERROR HandleGetMessages(app::AttributeValueEncoder & aEncoder)               = 0;
    virtual CHIP_ERROR HandleGetActiveMessageIds(app::AttributeValueEncoder & aEncoder)       = 0;
    virtual CHIP_ERROR HandleGetSupportedLanguageCodes(app::AttributeValueEncoder & aEncoder) = 0;
    virtual CHIP_ERROR HandleGetSupportedMimeTypes(app::AttributeValueEncoder & aEncoder)     = 0;

    // Global Attributes
    bool HasFeature(chip::EndpointId endpoint, Feature feature);
    virtual uint32_t GetFeatureMap(chip::EndpointId endpoint) = 0;

    virtual ~Delegate() = default;
};

// Logs a MessageNotPresented event for the given endpoint.
CHIP_ERROR LogMessageNotPresentedEvent(chip::EndpointId endpoint, const ByteSpan & messageId, bool removedFromQueue);

} // namespace Messages
} // namespace Clusters
} // namespace app
} // namespace chip
