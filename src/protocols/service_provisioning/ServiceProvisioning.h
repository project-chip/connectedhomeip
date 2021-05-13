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
 *      This file defines message types for ServiceProvisioning protocol.
 *
 */

#pragma once

#include <core/CHIPCore.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <support/CodeUtils.h>
#include <support/DLLUtil.h>
#include <support/logging/CHIPLogging.h>

namespace chip {
namespace Protocols {
namespace ServiceProvisioning {

/**
 * ServiceProvisioning Protocol Message Types
 */
enum class MsgType : uint8_t
{
    ServiceProvisioningRequest = 0x01,
};

} // namespace ServiceProvisioning

template <>
struct MessageTypeTraits<ServiceProvisioning::MsgType>
{
    static constexpr const Protocols::Id & ProtocolId() { return ServiceProvisioning::Id; }
};

} // namespace Protocols
} // namespace chip
