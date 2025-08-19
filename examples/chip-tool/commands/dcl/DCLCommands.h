/*
 *   Copyright (c) 2024 Project CHIP Authors
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

#include "../common/Command.h"

#include "DCLClient.h"
#include "DisplayTermsAndConditions.h"

class DCLCommandBase : public Command
{
public:
    DCLCommandBase(const char * name) : Command(name) {}

    void AddArguments()
    {
        AddArgument("hostname", &mHostName,
                    "Hostname of the DCL server to fetch information from. Defaults to 'on.dcl.csa-iot.org'.");
        AddArgument("port", 0, UINT16_MAX, &mPort, "Port number for connecting to the DCL server. Defaults to '443'.");
    }

    CHIP_ERROR Run()
    {
        auto client = chip::tool::dcl::DCLClient(mHostName, mPort);
        return RunCommand(client);

        return CHIP_NO_ERROR;
    }

    virtual CHIP_ERROR RunCommand(chip::tool::dcl::DCLClient & client) = 0;

private:
    chip::Optional<char *> mHostName;
    chip::Optional<uint16_t> mPort;
};

class DCLPayloadCommandBase : public DCLCommandBase
{
public:
    DCLPayloadCommandBase(const char * name) : DCLCommandBase(name)
    {
        AddArgument("payload", &mPayload);
        DCLCommandBase::AddArguments();
    }

protected:
    char * mPayload;
};

class DCLIdsCommandBase : public DCLCommandBase
{
public:
    DCLIdsCommandBase(const char * name) : DCLCommandBase(name)
    {
        AddArgument("vendor-id", 0, UINT16_MAX, &mVendorId);
        AddArgument("product-id", 0, UINT16_MAX, &mProductId);
        DCLCommandBase::AddArguments();
    }

protected:
    uint16_t mVendorId;
    uint16_t mProductId;
};

class DCLModelByPayloadCommand : public DCLPayloadCommandBase
{
public:
    DCLModelByPayloadCommand() : DCLPayloadCommandBase("model-by-payload") {}

    CHIP_ERROR RunCommand(chip::tool::dcl::DCLClient & client)
    {
        Json::Value model;
        ReturnErrorOnFailure(client.Model(mPayload, model));
        VerifyOrReturnError(model != Json::nullValue, CHIP_NO_ERROR);

        ChipLogProgress(chipTool, "%s", model.toStyledString().c_str());
        return CHIP_NO_ERROR;
    }
};

class DCLModelCommand : public DCLIdsCommandBase
{
public:
    DCLModelCommand() : DCLIdsCommandBase("model") {}

    CHIP_ERROR RunCommand(chip::tool::dcl::DCLClient & client)
    {
        Json::Value model;
        ReturnErrorOnFailure(client.Model(static_cast<chip::VendorId>(mVendorId), mProductId, model));
        VerifyOrReturnError(model != Json::nullValue, CHIP_NO_ERROR);

        ChipLogProgress(chipTool, "%s", model.toStyledString().c_str());
        return CHIP_NO_ERROR;
    }
};

class DCLTCByPayloadCommand : public DCLPayloadCommandBase
{
public:
    DCLTCByPayloadCommand() : DCLPayloadCommandBase("tc-by-payload") {}

    CHIP_ERROR RunCommand(chip::tool::dcl::DCLClient & client)
    {
        Json::Value tc;
        ReturnErrorOnFailure(client.TermsAndConditions(mPayload, tc));
        VerifyOrReturnError(tc != Json::nullValue, CHIP_NO_ERROR);

        ChipLogProgress(chipTool, "%s", tc.toStyledString().c_str());
        return CHIP_NO_ERROR;
    }
};

class DCLTCCommand : public DCLIdsCommandBase
{
public:
    DCLTCCommand() : DCLIdsCommandBase("tc") {}

    CHIP_ERROR RunCommand(chip::tool::dcl::DCLClient & client)
    {
        Json::Value tc;
        ReturnErrorOnFailure(client.TermsAndConditions(static_cast<chip::VendorId>(mVendorId), mProductId, tc));
        VerifyOrReturnError(tc != Json::nullValue, CHIP_NO_ERROR);

        ChipLogProgress(chipTool, "%s", tc.toStyledString().c_str());
        return CHIP_NO_ERROR;
    }
};

class DCLTCDisplayByPayloadCommand : public DCLPayloadCommandBase
{
public:
    DCLTCDisplayByPayloadCommand() : DCLPayloadCommandBase("tc-display-by-payload")
    {
        AddArgument("country-code", &mCountryCode,
                    "The country code to retrieve terms and conditions for. Defaults to the country configured in the DCL.");
        AddArgument("language-code", &mLanguageCode,
                    "The language code to retrieve terms and conditions for. Defaults to the language configured for the chosen "
                    "country in the DCL.");
    }

    CHIP_ERROR RunCommand(chip::tool::dcl::DCLClient & client)
    {
        Json::Value tc;
        ReturnErrorOnFailure(client.TermsAndConditions(mPayload, tc));
        VerifyOrReturnError(tc != Json::nullValue, CHIP_NO_ERROR);

        uint16_t version      = 0;
        uint16_t userResponse = 0;
        ReturnErrorOnFailure(chip::tool::dcl::DisplayTermsAndConditions(tc, version, userResponse, mCountryCode, mLanguageCode));

        ChipLogProgress(chipTool, "\nTerms and conditions\n\tRevision    : %u\n\tUserResponse: %u", version, userResponse);
        return CHIP_NO_ERROR;
    }

private:
    chip::Optional<char *> mCountryCode;
    chip::Optional<char *> mLanguageCode;
};

class DCLTCDisplayCommand : public DCLIdsCommandBase
{
public:
    DCLTCDisplayCommand() : DCLIdsCommandBase("tc-display")
    {
        AddArgument("country-code", &mCountryCode,
                    "The country code to retrieve terms and conditions for. Defaults to the country configured in the DCL.");
        AddArgument("language-code", &mLanguageCode,
                    "The language code to retrieve terms and conditions for. Defaults to the language configured for the chosen "
                    "country in the DCL.");
    }
    CHIP_ERROR RunCommand(chip::tool::dcl::DCLClient & client)
    {
        Json::Value tc;
        ReturnErrorOnFailure(client.TermsAndConditions(static_cast<chip::VendorId>(mVendorId), mProductId, tc));
        VerifyOrReturnError(tc != Json::nullValue, CHIP_NO_ERROR);

        uint16_t version      = 0;
        uint16_t userResponse = 0;
        ReturnErrorOnFailure(chip::tool::dcl::DisplayTermsAndConditions(tc, version, userResponse, mCountryCode, mLanguageCode));

        ChipLogProgress(chipTool, "\nTerms and conditions\n\tRevision    : %u\n\tUserResponse: %u", version, userResponse);
        return CHIP_NO_ERROR;
    }

private:
    chip::Optional<char *> mCountryCode;
    chip::Optional<char *> mLanguageCode;
};
