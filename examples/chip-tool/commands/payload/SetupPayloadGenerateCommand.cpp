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
#include <lib/core/TLV.h>
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
        payload.discriminator.SetLongValue(mDiscriminator.Value());
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

    if (mExistingPayload.HasValue())
    {
        CHIP_ERROR err = QRCodeSetupPayloadParser(mExistingPayload.Value()).populatePayload(payload);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(chipTool, "Invalid existing payload: %" CHIP_ERROR_FORMAT, err.Format());
            return err;
        }
    }

    ConfigurePayload(payload);

    if (mRendezvous.HasValue())
    {
        payload.rendezvousInformation.Emplace().SetRaw(mRendezvous.Value());
    }
    else if (!payload.rendezvousInformation.HasValue())
    {
        // Default to not having anything in the discovery capabilities.
        payload.rendezvousInformation.SetValue(RendezvousInformationFlag::kNone);
    }

    if (mTLVBytes.HasValue())
    {
        CHIP_ERROR err = PopulatePayloadTLVFromBytes(payload, mTLVBytes.Value());
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(chipTool, "Unable to populate payload TLV: %" CHIP_ERROR_FORMAT, err.Format());
            return err;
        }
    }

    QRCodeSetupPayloadGenerator generator(payload);
    generator.SetAllowInvalidPayload(mAllowInvalidPayload.ValueOr(false));

    std::string code;
    ReturnErrorOnFailure(generator.payloadBase38RepresentationWithAutoTLVBuffer(code));
    // CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE includes various prefixes we don't
    // control (timestamps, process ids, etc).  Let's assume (hope?) that
    // those prefixes use up no more than half the total available space.
    constexpr size_t chunkSize = CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE / 2;
    while (code.size() > chunkSize)
    {
        ChipLogProgress(chipTool, "QR Code: %s", code.substr(0, chunkSize).c_str());
        code = code.substr(chunkSize);
    }
    ChipLogProgress(chipTool, "QR Code: %s", code.c_str());

    return CHIP_NO_ERROR;
}

CHIP_ERROR SetupPayloadGenerateQRCodeCommand::PopulatePayloadTLVFromBytes(SetupPayload & payload, const ByteSpan & tlvBytes)
{
    // First clear out all the existing TVL bits from the payload.  Ignore
    // errors here, because we don't care if those bits are not present.
    (void) payload.removeSerialNumber();

    auto existingVendorData = payload.getAllOptionalVendorData();
    for (auto & data : existingVendorData)
    {
        (void) payload.removeOptionalVendorData(data.tag);
    }

    if (tlvBytes.empty())
    {
        // Used to just clear out the existing TLV.
        return CHIP_NO_ERROR;
    }

    TLV::TLVReader reader;
    reader.Init(tlvBytes);

    // Data is a TLV structure.
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

    TLV::TLVType outerType;
    ReturnErrorOnFailure(reader.EnterContainer(outerType));

    CHIP_ERROR err;
    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        TLV::Tag tag = reader.GetTag();
        if (!TLV::IsContextTag(tag))
        {
            ChipLogError(chipTool, "Unexpected non-context TLV tag.");
            return CHIP_ERROR_INVALID_TLV_TAG;
        }

        uint8_t tagNum = static_cast<uint8_t>(TLV::TagNumFromTag(tag));
        if (tagNum < 0x80)
        {
            // Matter-common tag.
            if (tagNum != kSerialNumberTag)
            {
                ChipLogError(chipTool, "No support yet for Matter-common tags other than serial number");
                return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
            }

            // Serial number can be a string or an unsigned integer.
            if (reader.GetType() == TLV::kTLVType_UTF8String)
            {
                CharSpan data;
                ReturnErrorOnFailure(reader.Get(data));
                ReturnErrorOnFailure(payload.addSerialNumber(std::string(data.data(), data.size())));
                continue;
            }

            if (reader.GetType() == TLV::kTLVType_UnsignedInteger)
            {
                uint32_t value;
                ReturnErrorOnFailure(reader.Get(value));
                ReturnErrorOnFailure(payload.addSerialNumber(value));
                continue;
            }

            ChipLogError(chipTool, "Unexpected type for serial number: %d", to_underlying(reader.GetType()));
            return CHIP_ERROR_WRONG_TLV_TYPE;
        }

        // Vendor tag.  We support strings and signed integers.
        if (reader.GetType() == TLV::kTLVType_UTF8String)
        {
            CharSpan data;
            ReturnErrorOnFailure(reader.Get(data));
            ReturnErrorOnFailure(payload.addOptionalVendorData(tagNum, std::string(data.data(), data.size())));
            continue;
        }

        if (reader.GetType() == TLV::kTLVType_SignedInteger)
        {
            int32_t value;
            ReturnErrorOnFailure(reader.Get(value));
            ReturnErrorOnFailure(payload.addOptionalVendorData(tagNum, value));
            continue;
        }

        ChipLogError(chipTool, "Unexpected type for vendor data: %d", to_underlying(reader.GetType()));
        return CHIP_ERROR_WRONG_TLV_TYPE;
    }

    VerifyOrReturnError(err == CHIP_END_OF_TLV, err);

    ReturnErrorOnFailure(reader.ExitContainer(outerType));
    ReturnErrorOnFailure(reader.VerifyEndOfContainer());

    return CHIP_NO_ERROR;
}

CHIP_ERROR SetupPayloadGenerateManualCodeCommand::Run()
{
    SetupPayload payload;

    if (mExistingPayload.HasValue())
    {
        CHIP_ERROR err = ManualSetupPayloadParser(mExistingPayload.Value()).populatePayload(payload);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(chipTool, "Invalid existing payload: %" CHIP_ERROR_FORMAT, err.Format());
            return err;
        }
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
