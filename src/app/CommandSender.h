/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *    @file
 *      This file defines objects for a CHIP IM Invoke Command Sender
 *
 */

#pragma once

#include <core/CHIPCore.h>
#include <core/CHIPTLVDebug.hpp>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <support/CodeUtils.h>
#include <support/DLLUtil.h>
#include <support/logging/CHIPLogging.h>
#include <system/SystemPacketBuffer.h>

#include <app/Command.h>

#define COMMON_STATUS_SUCCESS 0

namespace chip {
namespace app {

class CommandSender : public Command, public Messaging::ExchangeDelegate
{
public:
    // TODO: issue #6792 - the secure session parameter should be made non-optional and passed by reference.
    // Once SendCommandRequest returns successfully, the CommandSender will
    // handle calling Shutdown on itself once it decides it's done with waiting
    // for a response (i.e. times out or gets a response).
    //
    // If SendCommandRequest is never called, or the call fails, the API
    // consumer is responsible for calling Shutdown on the CommandSender.
    CHIP_ERROR SendCommandRequest(NodeId aNodeId, Transport::AdminId aAdminId, SecureSessionHandle * secureSession = nullptr);

private:
    // ExchangeDelegate interface implementation.  Private so people won't
    // accidentally call it on us when we're not being treated as an actual
    // ExchangeDelegate.
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                                 const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext) override;

    CHIP_ERROR ProcessCommandDataElement(CommandDataElement::Parser & aCommandElement) override;
};

} // namespace app
} // namespace chip
