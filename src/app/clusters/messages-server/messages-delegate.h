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

#include <app/AttributeAccessInterface.h>
#include <app/CommandResponseHelper.h>
#include <app/util/af.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Messages {

using MessageResponseOption = chip::app::Clusters::Messages::Structs::MessageResponseOptionStruct::Type;

class Delegate
{
public:
    // Commands
    virtual void
    HandlePresentMessagesRequest(const ByteSpan & messageId, const MessagePriorityEnum & priority,
                                 const chip::BitMask<MessageControlBitmap> & messageControl,
                                 const DataModel::Nullable<uint32_t> & startTime, const DataModel::Nullable<uint16_t> & duration,
                                 const CharSpan & messageText,
                                 const chip::Optional<DataModel::DecodableList<MessageResponseOption>> & responses) = 0;
    virtual void HandleCancelMessagesRequest(const DataModel::DecodableList<chip::ByteSpan> & messageIds)           = 0;

    // Attributes
    virtual CHIP_ERROR HandleGetMessages(app::AttributeValueEncoder & aEncoder)         = 0;
    virtual CHIP_ERROR HandleGetActiveMessageIds(app::AttributeValueEncoder & aEncoder) = 0;

    // Global Attributes
    bool HasFeature(chip::EndpointId endpoint, Feature feature);
    virtual uint32_t GetFeatureMap(chip::EndpointId endpoint) = 0;

    virtual ~Delegate() = default;
};

} // namespace Messages
} // namespace Clusters
} // namespace app
} // namespace chip
