/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "LogCommands.h"
#include <iostream>

CHIP_ERROR LogCommands::Log(const char * identity, const chip::app::Clusters::LogCommands::Commands::Log::Type & value)
{
    ChipLogDetail(chipTool, "%.*s", static_cast<int>(value.message.size()), value.message.data());
    return ContinueOnChipMainThread(CHIP_NO_ERROR);
}

CHIP_ERROR LogCommands::UserPrompt(const char * identity,
                                   const chip::app::Clusters::LogCommands::Commands::UserPrompt::Type & value)
{
    ChipLogDetail(chipTool, "USER_PROMPT: %.*s", static_cast<int>(value.message.size()), value.message.data());

    if (value.expectedValue.HasValue())
    {
        std::string line;
        std::getline(std::cin, line);
        if (line != std::string(value.expectedValue.Value().data(), value.expectedValue.Value().size()))
        {
            return ContinueOnChipMainThread(CHIP_ERROR_INVALID_ARGUMENT);
        }
    }

    return ContinueOnChipMainThread(CHIP_NO_ERROR);
}
