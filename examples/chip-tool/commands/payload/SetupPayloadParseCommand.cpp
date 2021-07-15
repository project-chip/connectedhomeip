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

#include "SetupPayloadParseCommand.h"
#include <lib/support/TypeTraits.h>
#include <setup_payload/ManualSetupPayloadParser.h>
#include <setup_payload/QRCodeSetupPayloadParser.h>
#include <setup_payload/SetupPayload.h>

using namespace ::chip;

CHIP_ERROR SetupPayloadParseCommand::Run()
{
    std::string codeString(mCode);
    SetupPayload payload;

    CHIP_ERROR err = CHIP_NO_ERROR;
    err            = Parse(codeString, payload);
    SuccessOrExit(err);

    err = Print(payload);
    SuccessOrExit(err);
exit:
    if (err == CHIP_NO_ERROR)
    {
        SetCommandExitStatus(CHIP_NO_ERROR);
    }
    return err;
}

CHIP_ERROR SetupPayloadParseCommand::Print(chip::SetupPayload payload)
{
    std::string serialNumber;
    std::vector<OptionalQRCodeInfo> optionalVendorData;
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogProgress(SetupPayload, "CommissioningFlow: %" PRIu8, to_underlying(payload.commissioningFlow));
    ChipLogProgress(SetupPayload, "VendorID: %u", payload.vendorID);
    ChipLogProgress(SetupPayload, "Version: %u", payload.version);
    ChipLogProgress(SetupPayload, "ProductID: %u", payload.productID);
    ChipLogProgress(SetupPayload, "Discriminator: %u", payload.discriminator);
    ChipLogProgress(SetupPayload, "SetUpPINCode: %u", payload.setUpPINCode);
    ChipLogProgress(SetupPayload, "RendezvousInformation: %u", payload.rendezvousInformation.Raw());

    if (payload.getSerialNumber(serialNumber) == CHIP_NO_ERROR)
    {
        ChipLogProgress(SetupPayload, "SerialNumber: %s", serialNumber.c_str());
    }

    optionalVendorData = payload.getAllOptionalVendorData();
    for (const OptionalQRCodeInfo & info : optionalVendorData)
    {
        if (info.type == optionalQRCodeInfoTypeString)
        {
            ChipLogProgress(SetupPayload, "OptionalQRCodeInfo: tag=%u,string value=%s", info.tag, info.data.c_str());
        }
        else if (info.type == optionalQRCodeInfoTypeInt32)
        {
            ChipLogProgress(SetupPayload, "OptionalQRCodeInfo: tag=%u,int value=%u", info.tag, info.int32);
        }
        else
        {
            err = CHIP_ERROR_INVALID_ARGUMENT;
        }
    }

    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR SetupPayloadParseCommand::Parse(std::string codeString, chip::SetupPayload & payload)
{

    CHIP_ERROR err = CHIP_NO_ERROR;
    if (IsQRCode(codeString))
    {
        ChipLogDetail(SetupPayload, "Parsing base38Representation: %s", codeString.c_str());
        err = QRCodeSetupPayloadParser(codeString).populatePayload(payload);
    }
    else
    {
        ChipLogDetail(SetupPayload, "Parsing decimalRepresentation: %s", codeString.c_str());
        err = ManualSetupPayloadParser(codeString).populatePayload(payload);
    }

    return err;
}

bool SetupPayloadParseCommand::IsQRCode(std::string codeString)
{
    return codeString.rfind(kQRCodePrefix) == 0;
}
