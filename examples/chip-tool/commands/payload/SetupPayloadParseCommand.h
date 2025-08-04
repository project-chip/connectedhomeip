/*
 *   Copyright (c) 2020 Project CHIP Authors
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

#pragma once

#include "../common/Command.h"
#include <setup_payload/SetupPayload.h>

#include <string>

class SetupPayloadParseCommand : public Command
{
public:
    SetupPayloadParseCommand() : Command("parse-setup-payload") { AddArgument("payload", &mCode); }
    CHIP_ERROR Run() override;
    static bool IsQRCode(std::string codeString);

private:
    char * mCode;
    CHIP_ERROR Parse(std::string codeString, chip::SetupPayload & payload);
    CHIP_ERROR Print(chip::SetupPayload payload);
};
