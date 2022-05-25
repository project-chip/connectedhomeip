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
#include <lib/support/StringBuilder.h>
#include <setup_payload/ManualSetupPayloadParser.h>
#include <setup_payload/QRCodeSetupPayloadParser.h>
#include <setup_payload/SetupPayload.h>

using namespace ::chip;

namespace {

#if CHIP_PROGRESS_LOGGING

const char * CustomFlowString(CommissioningFlow flow)
{
    switch (flow)
    {
    case CommissioningFlow::kStandard:
        return "STANDARD";
    case CommissioningFlow::kUserActionRequired:
        return "USER ACTION REQUIRED";
    case CommissioningFlow::kCustom:
        return "CUSTOM";
    }

    return "???";
}

#endif // CHIP_PROGRESS_LOGGING

} // namespace

CHIP_ERROR SetupPayloadParseCommand::Run()
{
    std::string codeString(mCode);
    SetupPayload payload;

    ReturnErrorOnFailure(Parse(codeString, payload));
    ReturnErrorOnFailure(Print(payload));

    return CHIP_NO_ERROR;
}

CHIP_ERROR SetupPayloadParseCommand::Parse(std::string codeString, chip::SetupPayload & payload)
{
    bool isQRCode = IsQRCode(codeString);

    ChipLogDetail(SetupPayload, "Parsing %sRepresentation: %s", isQRCode ? "base38" : "decimal", codeString.c_str());

    return isQRCode ? QRCodeSetupPayloadParser(codeString).populatePayload(payload)
                    : ManualSetupPayloadParser(codeString).populatePayload(payload);
}

CHIP_ERROR SetupPayloadParseCommand::Print(chip::SetupPayload payload)
{
    ChipLogProgress(SetupPayload, "Version:       %u", payload.version);
    ChipLogProgress(SetupPayload, "VendorID:      %u", payload.vendorID);
    ChipLogProgress(SetupPayload, "ProductID:     %u", payload.productID);
    ChipLogProgress(SetupPayload, "Custom flow:   %u    (%s)", to_underlying(payload.commissioningFlow),
                    CustomFlowString(payload.commissioningFlow));
    {
        StringBuilder<128> humanFlags;

        if (payload.rendezvousInformation.HasAny())
        {
            if (payload.rendezvousInformation.Has(RendezvousInformationFlag::kSoftAP))
            {
                humanFlags.Add("Soft-AP");
            }
            if (payload.rendezvousInformation.Has(RendezvousInformationFlag::kBLE))
            {
                if (!humanFlags.Empty())
                {
                    humanFlags.Add(", ");
                }
                humanFlags.Add("BLE");
            }
            if (payload.rendezvousInformation.Has(RendezvousInformationFlag::kOnNetwork))
            {
                if (!humanFlags.Empty())
                {
                    humanFlags.Add(", ");
                }
                humanFlags.Add("On IP network");
            }
        }
        else
        {
            humanFlags.Add("NONE");
        }

        ChipLogProgress(SetupPayload, "Capabilities:  0x%02X (%s)", payload.rendezvousInformation.Raw(), humanFlags.c_str());
    }
    ChipLogProgress(SetupPayload, "Discriminator: %u", payload.discriminator);
    ChipLogProgress(SetupPayload, "Passcode:      %u", payload.setUpPINCode);

    std::string serialNumber;
    if (payload.getSerialNumber(serialNumber) == CHIP_NO_ERROR)
    {
        ChipLogProgress(SetupPayload, "SerialNumber: %s", serialNumber.c_str());
    }

    std::vector<OptionalQRCodeInfo> optionalVendorData = payload.getAllOptionalVendorData();
    for (const OptionalQRCodeInfo & info : optionalVendorData)
    {
        bool isTypeString = info.type == optionalQRCodeInfoTypeString;
        bool isTypeInt32  = info.type == optionalQRCodeInfoTypeInt32;
        VerifyOrReturnError(isTypeString || isTypeInt32, CHIP_ERROR_INVALID_ARGUMENT);

        if (isTypeString)
        {
            ChipLogProgress(SetupPayload, "OptionalQRCodeInfo: tag=%u,string value=%s", info.tag, info.data.c_str());
        }
        else
        {
            ChipLogProgress(SetupPayload, "OptionalQRCodeInfo: tag=%u,int value=%u", info.tag, info.int32);
        }
    }

    return CHIP_NO_ERROR;
}

bool SetupPayloadParseCommand::IsQRCode(std::string codeString)
{
    return codeString.rfind(kQRCodePrefix) == 0;
}
