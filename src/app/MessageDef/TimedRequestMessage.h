/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
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
namespace TimedRequestMessage {
enum class Tag : uint8_t
{
    kTimeoutMs = 0,
};

class Parser : public MessageParser
{
public:
#if CHIP_CONFIG_IM_PRETTY_PRINT
    CHIP_ERROR PrettyPrint() const;
#endif

    /**
     *  @brief Get Timeout value. Next() must be called before accessing them.
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetTimeoutMs(uint16_t * const apTimeoutMs) const;
};

class Builder : public MessageBuilder
{
public:
    /**
     *  @brief Timeout value, sent by a client to a server, if there is a preceding successful Timed Request action,
     *  the following action SHALL be received before the end of the Timeout interval.
     */
    TimedRequestMessage::Builder & TimeoutMs(const uint16_t aTimeoutMs);
};
} // namespace TimedRequestMessage
} // namespace app
} // namespace chip
