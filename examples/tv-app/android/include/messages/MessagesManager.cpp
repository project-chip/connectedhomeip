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

using namespace chip::app;
using namespace chip::app::Clusters::Messages;

// Commands
void MessagesManager::HandlePresentMessagesRequest(
    const chip::ByteSpan & messageId, const MessagePriorityEnum & priority,
    const chip::BitMask<MessageControlBitmap> & messageControl, const chip::app::DataModel::Nullable<uint32_t> & startTime,
    const chip::app::DataModel::Nullable<uint64_t> & duration, const chip::CharSpan & messageText,
    const chip::Optional<chip::app::DataModel::DecodableList<MessageResponseOption>> & responses)
{
    // TODO: Present Message
}

void MessagesManager::HandleCancelMessagesRequest(const chip::app::DataModel::DecodableList<chip::ByteSpan> & messageIds)
{
    // TODO: Cancel Message
}

// Attributes
CHIP_ERROR MessagesManager::HandleGetMessages(chip::app::AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeEmptyList();
}

CHIP_ERROR MessagesManager::HandleGetActiveMessageIds(chip::app::AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeEmptyList();
}

// Global Attributes
uint32_t MessagesManager::GetFeatureMap(chip::EndpointId endpoint)
{
    return 1;
}
