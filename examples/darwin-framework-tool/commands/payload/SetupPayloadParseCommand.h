/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
