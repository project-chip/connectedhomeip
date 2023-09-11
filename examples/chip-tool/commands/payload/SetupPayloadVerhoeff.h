/*
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include "../common/Command.h"
#include <setup_payload/SetupPayload.h>

class SetupPayloadVerhoeffVerify : public Command
{
public:
    SetupPayloadVerhoeffVerify() : Command("verhoeff-verify")
    {
        AddArgument("payload", &mSetupCode);
        AddArgument("position", 0, UINT8_MAX, &mPos);
    }
    CHIP_ERROR Run() override;

private:
    char * mSetupCode;
    uint8_t mPos;
    bool Verify(std::string codeString);
};

class SetupPayloadVerhoeffGenerate : public Command
{
public:
    SetupPayloadVerhoeffGenerate() : Command("verhoeff-generate") { AddArgument("payload", &mSetupCode); }
    CHIP_ERROR Run() override;

private:
    char * mSetupCode;
    CHIP_ERROR GenerateChar(std::string codeString, char & generatedChar);
};
