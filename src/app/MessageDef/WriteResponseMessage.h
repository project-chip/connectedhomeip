/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "AttributeStatusIBs.h"
#include "MessageBuilder.h"
#include "MessageParser.h"
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
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
    /**
     *  @brief Roughly verify the message is correctly formed
     *   1) all mandatory tags are present
     *   2) all elements have expected data type
     *   3) any tag can only appear once
     *   4) At the top level of the structure, unknown tags are ignored for forward compatibility
     *  @note The main use of this function is to print out what we're
     *    receiving during protocol development and debugging.
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR CheckSchemaValidity() const;

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
     *  @return A reference to *this
     */
    WriteResponseMessage::Builder & EndOfWriteResponseMessage();

private:
    AttributeStatusIBs::Builder mWriteResponses;
};
} // namespace WriteResponseMessage
} // namespace app
} // namespace chip
