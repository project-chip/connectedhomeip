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

#include "SetupPayloadGenerateCommand.h"
#include <setup_payload/ManualSetupPayloadGenerator.h>
#include <setup_payload/ManualSetupPayloadParser.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/QRCodeSetupPayloadParser.h>
#include <setup_payload/SetupPayload.h>

using namespace ::chip;

void SetupPayloadGenerateCommand::ConfigurePayload(SetupPayload & payload)
{
    if (mDiscriminator.HasValue())
    {
        payload.discriminator = mDiscriminator.Value();
    }

    if (mSetUpPINCode.HasValue())
    {
        payload.setUpPINCode = mSetUpPINCode.Value();
    }

    if (mVersion.HasValue())
    {
        payload.version = mVersion.Value();
    }

    if (mVendorId.HasValue())
    {
        payload.vendorID = mVendorId.Value();
    }

    if (mProductId.HasValue())
    {
        payload.productID = mProductId.Value();
    }

    if (mCommissioningMode.HasValue())
    {
        payload.commissioningFlow = static_cast<CommissioningFlow>(mCommissioningMode.Value());
    }
}

CHIP_ERROR SetupPayloadGenerateQRCodeCommand::Run()
{
    SetupPayload payload;

    if (mPayload.HasValue())
    {
        QRCodeSetupPayloadParser(mPayload.Value()).populatePayload(payload);
    }

    ConfigurePayload(payload);

    if (mRendezvous.HasValue())
    {
        payload.rendezvousInformation.SetRaw(mRendezvous.Value());
    }

    QRCodeSetupPayloadGenerator generator(payload);
    generator.SetAllowInvalidPayload(mAllowInvalidPayload.ValueOr(false));

    std::string code;
    ReturnErrorOnFailure(generator.payloadBase38Representation(code));
    ChipLogProgress(chipTool, "QR Code: %s", code.c_str());

    return CHIP_NO_ERROR;
}

CHIP_ERROR SetupPayloadGenerateManualCodeCommand::Run()
{
    SetupPayload payload;

    if (mPayload.HasValue())
    {
        ManualSetupPayloadParser(mPayload.Value()).populatePayload(payload);
    }

    ConfigurePayload(payload);

    ManualSetupPayloadGenerator generator(payload);
    generator.SetAllowInvalidPayload(mAllowInvalidPayload.ValueOr(false));
    generator.SetForceShortCode(mForceShortCode.ValueOr(false));

    std::string code;
    ReturnErrorOnFailure(generator.payloadDecimalStringRepresentation(code));
    ChipLogProgress(chipTool, "Manual Code: %s", code.c_str());

    return CHIP_NO_ERROR;
}
