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

#include "AttributeStatusIBs.h"
#include "MessageBuilder.h"
#include "MessageParser.h"
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace WriteResponseMessage {
enum class Tag : uint8_t
{
    kWriteResponses = 0,
};

class Parser : public MessageParser
{
public:
#if CHIP_CONFIG_IM_PRETTY_PRINT
    CHIP_ERROR PrettyPrint() const;
#endif // CHIP_CONFIG_IM_PRETTY_PRINT
    /**
     *  @brief Get a TLVReader for the AttributeStatusIBs. Next() must be called before accessing them.
     *
     *  @param [in] apWriteResponses    A pointer to apWriteResponses
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetWriteResponses(AttributeStatusIBs::Parser * const apWriteResponses) const;
};

class Builder : public MessageBuilder
{
public:
    /**
     *  @brief Initialize a AttributeStatusIBs::Builder for writing into the TLV stream
     *
     *  @return A reference to AttributeStatusIBs::Builder
     */
    AttributeStatusIBs::Builder & CreateWriteResponses();

    /**
     *  @brief Get reference to AttributeStatusIBs::Builder
     *
     *  @return A reference to AttributeStatusIBs::Builder
     */
    AttributeStatusIBs::Builder & GetWriteResponses();

    /**
     *  @brief Mark the end of this WriteResponseMessage
     *
     *  @return The builder's final status.
     */
    CHIP_ERROR EndOfWriteResponseMessage();

private:
    AttributeStatusIBs::Builder mWriteResponses;
};
} // namespace WriteResponseMessage
} // namespace app
} // namespace chip
