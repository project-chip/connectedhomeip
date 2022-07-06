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

#include "../common/Command.h"
#include <setup_payload/SetupPayload.h>

class SetupPayloadGenerateCommand : public Command
{
public:
    SetupPayloadGenerateCommand(const char * name) : Command(name)
    {
        AddArgument("existing-payload", &mExistingPayload, "An existing setup payload to modify based on the other arguments.");
        AddArgument("discriminator", 0, UINT16_MAX, &mDiscriminator);
        AddArgument("setup-pin-code", 0, UINT32_MAX, &mSetUpPINCode);
        AddArgument("version", 0, UINT8_MAX, &mVersion);
        AddArgument("vendor-id", 0, UINT16_MAX, &mVendorId);
        AddArgument("product-id", 0, UINT16_MAX, &mProductId);
        AddArgument("commissioning-mode", 0, UINT8_MAX, &mCommissioningMode);
        AddArgument("allow-invalid-payload", 0, 1, &mAllowInvalidPayload);
    }

protected:
    void ConfigurePayload(chip::SetupPayload & payload);

    chip::Optional<uint16_t> mDiscriminator;
    chip::Optional<uint32_t> mSetUpPINCode;
    chip::Optional<uint8_t> mVersion;
    chip::Optional<uint16_t> mVendorId;
    chip::Optional<uint16_t> mProductId;
    chip::Optional<char *> mExistingPayload;
    chip::Optional<uint8_t> mCommissioningMode;
    chip::Optional<bool> mAllowInvalidPayload;
};

class SetupPayloadGenerateQRCodeCommand : public SetupPayloadGenerateCommand
{
public:
    SetupPayloadGenerateQRCodeCommand() : SetupPayloadGenerateCommand("generate-qrcode")
    {
        AddArgument("rendezvous", 0, UINT8_MAX, &mRendezvous);
        AddArgument(
            "tlvBytes", &mTLVBytes,
            "Pre-encoded TLV for the optional part of the payload.  A nonempty value should be passed as \"hex:\" followed by the "
            "bytes in hex encoding.  Passing an empty string to override the TLV in an existing payload is allowed.");
    }
    CHIP_ERROR Run() override;

private:
    static CHIP_ERROR PopulatePayloadTLVFromBytes(chip::SetupPayload & payload, const chip::ByteSpan & tlvBytes);

    chip::Optional<uint8_t> mRendezvous;
    chip::Optional<chip::ByteSpan> mTLVBytes;
};

class SetupPayloadGenerateManualCodeCommand : public SetupPayloadGenerateCommand
{
public:
    SetupPayloadGenerateManualCodeCommand() : SetupPayloadGenerateCommand("generate-manualcode")
    {
        AddArgument("force-short-code", 0, 1, &mForceShortCode);
    }
    CHIP_ERROR Run() override;

private:
    chip::Optional<bool> mForceShortCode;
};
