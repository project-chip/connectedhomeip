/*
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

#include <messaging/ExchangeHolder.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {

/**
 * Interface for sending InvokeResponseMessage(s).
 *
 * Provides information about the associated exchange context.
 *
 * Design Rationale: This interface enhances unit testability and allows applications to
 * customize CommandResponder behavior with stubs.
 */
class CommandResponderInterface
{
public:
    virtual ~CommandResponderInterface() = default;

    /* ExchangeContext related getter methods */
    virtual Messaging::ExchangeContext * GetExchangeContext() const = 0;
    virtual Access::SubjectDescriptor GetSubjectDescriptor() const  = 0;
    virtual FabricIndex GetAccessingFabricIndex() const             = 0;
    virtual bool IsForGroup() const                                 = 0;
    virtual GroupId GetGroupId() const                              = 0;

    /* Exchange methods that CommandHandler's may need to perform while processing request */
    virtual void FlushAcksRightNow() = 0;

    /* CommandResponder core methods */
    virtual void AddInvokeResponseToSend(System::PacketBufferHandle && aPacket) = 0;
    virtual void ResponseDropped()                                              = 0;
};

} // namespace app
} // namespace chip
