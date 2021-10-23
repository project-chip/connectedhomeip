/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      This file defines message types for TempZCL protocol.
 *
 */

#pragma once

#include <lib/core/CHIPCore.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>

namespace chip {
namespace Protocols {
namespace TempZCL {

/**
 * TempZCL Protocol Message Types
 */
enum class MsgType : uint8_t
{
    TempZCLRequest  = 0x01,
    TempZCLResponse = 0x02
};

} // namespace TempZCL

template <>
struct MessageTypeTraits<TempZCL::MsgType>
{
    static constexpr const Protocols::Id & ProtocolId() { return TempZCL::Id; }
};

} // namespace Protocols
} // namespace chip
