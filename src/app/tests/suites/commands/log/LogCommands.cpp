/*
 *   Copyright (c) 2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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
