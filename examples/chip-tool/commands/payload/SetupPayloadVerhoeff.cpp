/*
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "SetupPayloadVerhoeff.h"
#include "SetupPayloadParseCommand.h"
#include <setup_payload/SetupPayload.h>
#include <support/verhoeff/Verhoeff.h>

using namespace ::chip;

CHIP_ERROR SetupPayloadVerhoeffVerify::Run()
{
    std::string codeString(mSetupCode);

    bool isQRCode = SetupPayloadParseCommand::IsQRCode(codeString);

    VerifyOrReturnError(codeString.length() > mPos && mPos >= 0, CHIP_ERROR_INVALID_STRING_LENGTH);
    VerifyOrReturnError(!isQRCode, CHIP_ERROR_NOT_IMPLEMENTED);

    ChipLogProgress(SetupPayload, "%s is %sVALID at position %u", codeString.c_str(), Verify(codeString) ? "" : "IN", mPos);

    return CHIP_NO_ERROR;
}

bool SetupPayloadVerhoeffVerify::Verify(std::string codeString)
{
    char checkChar;
    bool result;

    checkChar = codeString.at(mPos);
    codeString.erase(mPos, mPos);
    ChipLogDetail(SetupPayload, "Verifying Manual Code: %s", codeString.c_str());

    result = Verhoeff10::ValidateCheckChar(checkChar, codeString.c_str());

    return result;
}

CHIP_ERROR SetupPayloadVerhoeffGenerate::Run()
{
    std::string codeString(mSetupCode);
    char generatedChar;

    ReturnErrorOnFailure(GenerateChar(codeString, generatedChar));
    ChipLogProgress(SetupPayload, "Generated Char: %c", generatedChar);

    return CHIP_NO_ERROR;
}

CHIP_ERROR SetupPayloadVerhoeffGenerate::GenerateChar(std::string codeString, char & generatedChar)
{
    bool isQRCode = SetupPayloadParseCommand::IsQRCode(codeString);

    VerifyOrReturnError(!isQRCode, CHIP_ERROR_INVALID_ARGUMENT);

    ChipLogDetail(SetupPayload, "Generating Character for: %s", codeString.c_str());
    generatedChar = Verhoeff10::ComputeCheckChar(codeString.c_str());

    return CHIP_NO_ERROR;
}
//
