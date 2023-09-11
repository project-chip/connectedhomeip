/*
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include "../common/Command.h"
#include <setup_payload/SetupPayload.h>

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
