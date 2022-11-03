/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "EventPathIBs.h"
#include "MessageBuilder.h"
#include "MessageParser.h"
#include <app/AppBuildConfig.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
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
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    /**
     *  @brief Roughly verify the message is correctly formed
     *   1) all mandatory tags are present
     *   2) all elements have expected data type
     *   3) any tag can only appear once
     *   4) At the top level of the structure, unknown tags are ignored for forward compatibility
     *  @note The main use of this function is to print out what we're
     *    receiving during protocol development and debugging.
     *    The encoding rule has changed in IM encoding spec so this
     *    check is only "roughly" conformant now.
     */
    CHIP_ERROR CheckSchemaValidity() const;
#endif

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
