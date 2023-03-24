/**
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

#pragma once
#include "EventPathIBs.h"
#include "MessageBuilder.h"
#include "MessageParser.h"
#include <app/AppBuildConfig.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace SubscribeResponseMessage {
enum class Tag : uint8_t
{
    kSubscriptionId = 0,
    kMaxInterval    = 2,
};

class Parser : public MessageParser
{
public:
#if CHIP_CONFIG_IM_PRETTY_PRINT
    CHIP_ERROR PrettyPrint() const;
#endif // CHIP_CONFIG_IM_PRETTY_PRINT
    /**
     *  @brief Get Subscription ID. Next() must be called before accessing them.
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetSubscriptionId(SubscriptionId * const apSubscriptionId) const;

    /**
     *  @brief Get Final MaxIntervalCeilingSeconds. Next() must be called before accessing them.
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetMaxInterval(uint16_t * const apMaxInterval) const;
};

class Builder : public MessageBuilder
{
public:
    /**
     *  @brief final subscription Id for the subscription back to the client.s.
     */
    SubscribeResponseMessage::Builder & SubscriptionId(const chip::SubscriptionId SubscriptionId);

    /**
     *  @brief Final Max Interval for the subscription back to the clients.
     */
    SubscribeResponseMessage::Builder & MaxInterval(const uint16_t aMaxInterval);

    /**
     *  @brief Mark the end of this SubscribeResponseMessage
     */
    SubscribeResponseMessage::Builder & EndOfSubscribeResponseMessage();
};
} // namespace SubscribeResponseMessage
} // namespace app
} // namespace chip
