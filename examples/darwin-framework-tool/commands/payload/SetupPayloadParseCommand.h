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

#pragma once

#import <Matter/Matter.h>
#include <commands/common/Command.h>

class SetupPayloadParseCommand : public Command {
public:
    SetupPayloadParseCommand()
        : Command("parse-setup-payload")
    {
        AddArgument("payload", &mCode);
    }
    CHIP_ERROR Run() override;
    static bool IsQRCode(NSString * codeString);

private:
    char * mCode;
    CHIP_ERROR Print(MTRSetupPayload * payload);

    // Will log the given string and given error (as progress if success, error
    // if failure).
    void LogNSError(const char * logString, NSError * error);
};
