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
#include "MessageBuilder.h"
#include "MessageParser.h"
#include <app/AppConfig.h>
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
