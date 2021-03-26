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
    CHIP_ERROR SendCommandRequest(NodeId aNodeId, Transport::AdminId aAdminId);

    void OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                           const PayloadHeader & aPayloadHeader, System::PacketBufferHandle aPayload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext) override;

private:
    CHIP_ERROR ProcessCommandDataElement(CommandDataElement::Parser & aCommandElement) override;
};

} // namespace app
} // namespace chip
