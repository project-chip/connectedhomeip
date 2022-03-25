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

CHIP_ERROR LogCommands::Log(const char * message)
{
    ChipLogDetail(chipTool, "%s", message);
    return ContinueOnChipMainThread(CHIP_NO_ERROR);
}

CHIP_ERROR LogCommands::UserPrompt(const char * message, const char * expectedValue)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    std::string line;
    ChipLogDetail(chipTool, "USER_PROMPT: %s", message);
    if (expectedValue == nullptr)
    {
        return ContinueOnChipMainThread(err);
    }

    std::getline(std::cin, line);
    if (line != expectedValue)
    {
        err = CHIP_ERROR_INVALID_ARGUMENT;
    }
    return ContinueOnChipMainThread(err);
}
