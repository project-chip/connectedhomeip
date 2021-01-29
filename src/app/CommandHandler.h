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
 *      This file defines object for a CHIP IM Invoke Command Handler
 *
 */

#pragma once

#ifndef _CHIP_INTERACTION_MODEL_COMMAND_HANDLER_H
#define _CHIP_INTERACTION_MODEL_COMMAND_HANDLER_H

#include <app/Command.h>
#include <core/CHIPCore.h>
#include <core/CHIPTLVDebug.hpp>
#include <map>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <support/CodeUtils.h>
#include <support/DLLUtil.h>
#include <support/logging/CHIPLogging.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace app {

class DLL_EXPORT CommandHandler : public Command
{
public:
    CHIP_ERROR SendCommandResponse();
    void OnMessageReceived(Messaging::ExchangeContext * ec, const PacketHeader & packetHeader, uint32_t protocolId, uint8_t msgType,
                           System::PacketBufferHandle payload);

private:
    CHIP_ERROR ProcessCommandDataElement(CommandDataElement::Parser & aCommandElement) override;
};
} // namespace app
} // namespace chip

#endif // _CHIP_INTERACTION_MODEL_COMMAND_HANDLER_H
