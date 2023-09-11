/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/AppBuildConfig.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include "CommandDataIB.h"
#include "CommandStatusIB.h"
#include "StructBuilder.h"
#include "StructParser.h"

namespace chip {
namespace app {
namespace InvokeResponseIB {
enum class Tag : uint8_t
{
    kCommand = 0,
    kStatus  = 1,
};

class Parser : public StructParser
{
public:
#if CHIP_CONFIG_IM_PRETTY_PRINT
    CHIP_ERROR PrettyPrint() const;
#endif // CHIP_CONFIG_IM_PRETTY_PRINT

    /**
     *  @brief Get a parser for a Command.
     *
     *  @param [in] apCommand    A pointer to the CommandDataIB parser.
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetCommand(CommandDataIB::Parser * const apCommand) const;

    /**
     *  @brief Get a parser for a Status.
     *
     *  @param [in] apStatus    A pointer to the CommandStatusIB parser.
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetStatus(CommandStatusIB::Parser * const apStatus) const;
};

class Builder : public StructBuilder
{
public:
    /**
     *  @brief Initialize a CommandDataIB::Builder for writing into the TLV stream
     *
     *  @return A reference to CommandDataIB::Builder
     */
    CommandDataIB::Builder & CreateCommand();

    /**
     *  @return A reference to CommandDataIB::Builder
     */
    CommandDataIB::Builder & GetCommand() { return mCommand; }

    /**
     *  @return A reference to CommandStatusIB::Builder
     */
    CommandStatusIB::Builder & GetStatus() { return mStatus; }

    /**
     *  @brief Initialize a CommandStatusIB::Builder for writing into the TLV stream
     *
     *  @return A reference to CommandStatusIB::Builder
     */
    CommandStatusIB::Builder & CreateStatus();

    /**
     *  @brief Mark the end of this InvokeCommand
     *
     *  @return The builder's final status.
     */
    CHIP_ERROR EndOfInvokeResponseIB();

private:
    CommandDataIB::Builder mCommand;
    CommandStatusIB::Builder mStatus;
};
} // namespace InvokeResponseIB
} // namespace app
} // namespace chip
