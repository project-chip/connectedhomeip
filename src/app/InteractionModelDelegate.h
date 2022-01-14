/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      This file defines the delegate the SDK consumer needs to implement to receive notifications from the interaction model.
 *
 */

#pragma once

#include <app/AttributePathParams.h>
#include <app/ClusterInfo.h>
#include <app/MessageDef/StatusIB.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <messaging/ExchangeContext.h>
#include <protocols/interaction_model/Constants.h>
#include <protocols/secure_channel/Constants.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace app {

static constexpr System::Clock::Timeout kImMessageTimeout = System::Clock::Seconds16(12);

class ReadClient;
class WriteClient;
class CommandSender;
class ReadHandler;

/**
 * @brief
 *   This class defines the API for a delegate that an SDK consumer can use to interface with the interaction model.
 */
class InteractionModelDelegate
{
public:
    /**
     * Notification that Subscription has been established successfully and application can do further work in handler.
     */
    virtual CHIP_ERROR SubscriptionEstablished(const ReadHandler * apReadHandler) { return CHIP_ERROR_NOT_IMPLEMENTED; }

    /**
     * Notification that Subscription has been terminated in handler side.
     */
    virtual CHIP_ERROR SubscriptionTerminated(const ReadHandler * apReadHandler) { return CHIP_ERROR_NOT_IMPLEMENTED; }

    virtual ~InteractionModelDelegate() = default;
};

} // namespace app
} // namespace chip
