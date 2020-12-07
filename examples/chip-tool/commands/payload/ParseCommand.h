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

class ParseCommand : public Command
{
public:
    ParseCommand() : Command("parse") { AddArgument("code", &mCode); }
    CHIP_ERROR Run(PersistentStorage & storage, NodeId localId, NodeId remoteId) override;

private:
    char * mCode;
    CHIP_ERROR Parse(std::string codeString, chip::SetupPayload & payload);
    CHIP_ERROR Print(chip::SetupPayload payload);
    bool IsQRCode(std::string codeString);
    const std::string QRCODE_PREFIX = "CH:";
};
