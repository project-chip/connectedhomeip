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

#include "Messages.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Messages;

CHIP_ERROR PresentMessagesRequestCommand::Invoke(const char * messageText, std::function<void(CHIP_ERROR)> responseCallback)
{
    Messages::Commands::PresentMessagesRequest::Type request;
    uint8_t buf[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5 };

    request.messageID   = ByteSpan(buf, sizeof(buf));
    request.messageText = CharSpan::fromCharString(messageText);
    request.priority    = MessagePriorityEnum(static_cast<uint8_t>(0));
    request.startTime   = DataModel::Nullable<uint32_t>(static_cast<uint32_t>(0));
    request.duration    = DataModel::Nullable<uint64_t>(static_cast<uint64_t>(60 * 1000));

    return MediaCommandBase::Invoke(request, responseCallback);
}
