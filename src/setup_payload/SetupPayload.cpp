/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *      The implementation of the Setup Payload. Currently only needed to
 *      verify the validity of a Setup Payload
 */

#include "SetupPayload.h"

#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <lib/core/TLV.h>
#include <lib/core/TLVData.h>
#include <lib/core/TLVUtilities.h>
#include <lib/support/CodeUtils.h>
#include <utility>

namespace chip {

// Check the Setup Payload for validity
//
// `vendor_id` and `product_id` are allowed all of uint16_t
bool PayloadContents::isValidQRCodePayload(ValidationMode mode) const
{
    // 3-bit value specifying the QR code payload version.
    VerifyOrReturnValue(version < (1 << kVersionFieldLengthInBits), false);

    VerifyOrReturnValue(static_cast<uint8_t>(commissioningFlow) < (1 << kCommissioningFlowFieldLengthInBits), false);

    // Device Commissioning Flow
    // Even in ValidationMode::kConsume we can only handle modes that we understand.
    // 0: Standard commissioning flow: such a device, when uncommissioned, always enters commissioning mode upon power-up, subject
    // to the rules in [ref_Announcement_Commencement]. 1: User-intent commissioning flow: user action required to enter
    // commissioning mode. 2: Custom commissioning flow: interaction with a vendor-specified means is needed before commissioning.
    // 3: Reserved
    VerifyOrReturnValue(commissioningFlow == CommissioningFlow::kStandard ||
                            commissioningFlow == CommissioningFlow::kUserActionRequired ||
                            commissioningFlow == CommissioningFlow::kCustom,
                        false);

    // General discriminator validity is enforced by the SetupDiscriminator class, but it can't be short for QR a code.
    VerifyOrReturnValue(!discriminator.IsShortDiscriminator(), false);

    // RendevouzInformation must be present for a QR code.
    VerifyOrReturnValue(rendezvousInformation.HasValue(), false);
    if (mode == ValidationMode::kProduce)
    {
        chip::RendezvousInformationFlags valid(RendezvousInformationFlag::kBLE, RendezvousInformationFlag::kOnNetwork,
                                               RendezvousInformationFlag::kSoftAP, RendezvousInformationFlag::kWiFiPAF);
        VerifyOrReturnValue(rendezvousInformation.Value().HasOnly(valid), false);
    }

    return CheckPayloadCommonConstraints();
}

bool PayloadContents::isValidManualCode(ValidationMode mode) const
{
    // No additional constraints apply to Manual Pairing Codes.
    // (If the payload has a long discriminator it will be converted automatically.)
    return CheckPayloadCommonConstraints();
}

bool PayloadContents::IsValidSetupPIN(uint32_t setupPIN)
{
    // SHALL be restricted to the values 0x0000001 to 0x5F5E0FE (00000001 to 99999998 in decimal), excluding the invalid Passcode
    // values.
    if (setupPIN == kSetupPINCodeUndefinedValue || setupPIN > kSetupPINCodeMaximumValue || setupPIN == 11111111 ||
        setupPIN == 22222222 || setupPIN == 33333333 || setupPIN == 44444444 || setupPIN == 55555555 || setupPIN == 66666666 ||
        setupPIN == 77777777 || setupPIN == 88888888 || setupPIN == 12345678 || setupPIN == 87654321)
    {
        return false;
    }

    return true;
}

bool PayloadContents::CheckPayloadCommonConstraints() const
{
    // Validation rules in this method apply to all validation modes.

    // Even in ValidationMode::kConsume we don't understand how to handle any payload version other than 0.
    VerifyOrReturnValue(version == 0, false);

    VerifyOrReturnValue(IsValidSetupPIN(setUpPINCode), false);

    // VendorID must be unspecified (0) or in valid range expected.
    VerifyOrReturnValue((vendorID == VendorId::Unspecified) || IsVendorIdValidOperationally(vendorID), false);

    // A value of 0x0000 SHALL NOT be assigned to a product since Product ID = 0x0000 is used for these specific cases:
    //  * To announce an anonymized Product ID as part of device discovery
    //  * To indicate an OTA software update file applies to multiple Product IDs equally.
    //  * To avoid confusion when presenting the Onboarding Payload for ECM with multiple nodes
    // In these special cases the vendorID must be 0 (Unspecified)
    VerifyOrReturnValue(productID != 0 || vendorID == VendorId::Unspecified, false);

    return true;
}

bool PayloadContents::operator==(const PayloadContents & input) const
{
    return (this->version == input.version && this->vendorID == input.vendorID && this->productID == input.productID &&
            this->commissioningFlow == input.commissioningFlow && this->rendezvousInformation == input.rendezvousInformation &&
            this->discriminator == input.discriminator && this->setUpPINCode == input.setUpPINCode);
}

CHIP_ERROR SetupPayload::addOptionalVendorData(uint8_t tag, std::string data)
{
    OptionalQRCodeInfo info;
    info.tag  = tag;
    info.type = optionalQRCodeInfoTypeString;
    info.data = std::move(data);

    return addOptionalVendorData(info);
}

CHIP_ERROR SetupPayload::addOptionalVendorData(uint8_t tag, int32_t data)
{
    OptionalQRCodeInfo info;
    info.tag   = tag;
    info.type  = optionalQRCodeInfoTypeInt32;
    info.int32 = data;

    return addOptionalVendorData(info);
}

std::vector<OptionalQRCodeInfo> SetupPayload::getAllOptionalVendorData() const
{
    std::vector<OptionalQRCodeInfo> returnedOptionalInfo;
    for (auto & entry : optionalVendorData)
    {
        returnedOptionalInfo.push_back(entry.second);
    }
    return returnedOptionalInfo;
}

CHIP_ERROR SetupPayload::removeOptionalVendorData(uint8_t tag)
{
    VerifyOrReturnError(optionalVendorData.find(tag) != optionalVendorData.end(), CHIP_ERROR_KEY_NOT_FOUND);
    optionalVendorData.erase(tag);

    return CHIP_NO_ERROR;
}

CHIP_ERROR SetupPayload::addSerialNumber(std::string serialNumber)
{
    OptionalQRCodeInfoExtension info;
    info.tag  = kSerialNumberTag;
    info.type = optionalQRCodeInfoTypeString;
    info.data = std::move(serialNumber);

    return addOptionalExtensionData(info);
}

CHIP_ERROR SetupPayload::addSerialNumber(uint32_t serialNumber)
{
    OptionalQRCodeInfoExtension info;
    info.tag    = kSerialNumberTag;
    info.type   = optionalQRCodeInfoTypeUInt32;
    info.uint32 = serialNumber;

    return addOptionalExtensionData(info);
}

CHIP_ERROR SetupPayload::getSerialNumber(std::string & outSerialNumber) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    OptionalQRCodeInfoExtension info;
    ReturnErrorOnFailure(getOptionalExtensionData(kSerialNumberTag, info));

    switch (info.type)
    {
    case (optionalQRCodeInfoTypeString):
        outSerialNumber = info.data;
        break;
    case (optionalQRCodeInfoTypeUInt32):
        outSerialNumber = std::to_string(info.uint32);
        break;
    default:
        err = CHIP_ERROR_INVALID_ARGUMENT;
        break;
    }

    return err;
}

CHIP_ERROR SetupPayload::removeSerialNumber()
{
    VerifyOrReturnError(optionalExtensionData.find(kSerialNumberTag) != optionalExtensionData.end(), CHIP_ERROR_KEY_NOT_FOUND);
    optionalExtensionData.erase(kSerialNumberTag);

    return CHIP_NO_ERROR;
}

CHIP_ERROR SetupPayload::addOptionalVendorData(const OptionalQRCodeInfo & info)
{
    VerifyOrReturnError(IsVendorTag(info.tag), CHIP_ERROR_INVALID_ARGUMENT);
    optionalVendorData[info.tag] = info;

    return CHIP_NO_ERROR;
}

CHIP_ERROR SetupPayload::addOptionalExtensionData(const OptionalQRCodeInfoExtension & info)
{
    VerifyOrReturnError(IsCommonTag(info.tag), CHIP_ERROR_INVALID_ARGUMENT);
    optionalExtensionData[info.tag] = info;

    return CHIP_NO_ERROR;
}

CHIP_ERROR SetupPayload::getOptionalVendorData(uint8_t tag, OptionalQRCodeInfo & info) const
{
    const auto it = optionalVendorData.find(tag);
    VerifyOrReturnError(it != optionalVendorData.end(), CHIP_ERROR_KEY_NOT_FOUND);
    info = it->second;

    return CHIP_NO_ERROR;
}

CHIP_ERROR SetupPayload::getOptionalExtensionData(uint8_t tag, OptionalQRCodeInfoExtension & info) const
{
    const auto it = optionalExtensionData.find(tag);
    VerifyOrReturnError(it != optionalExtensionData.end(), CHIP_ERROR_KEY_NOT_FOUND);
    info = it->second;
    return CHIP_NO_ERROR;
}

optionalQRCodeInfoType SetupPayload::getNumericTypeFor(uint8_t tag) const
{
    optionalQRCodeInfoType elemType = optionalQRCodeInfoTypeUnknown;

    if (IsVendorTag(tag))
    {
        elemType = optionalQRCodeInfoTypeInt32;
    }
    else if (tag == kSerialNumberTag)
    {
        elemType = optionalQRCodeInfoTypeUInt32;
    }

    return elemType;
}

std::vector<OptionalQRCodeInfoExtension> SetupPayload::getAllOptionalExtensionData() const
{
    std::vector<OptionalQRCodeInfoExtension> returnedOptionalInfo;
    for (auto & entry : optionalExtensionData)
    {
        returnedOptionalInfo.push_back(entry.second);
    }
    return returnedOptionalInfo;
}

bool SetupPayload::operator==(const SetupPayload & input) const
{
    std::vector<OptionalQRCodeInfo> inputOptionalVendorData;
    std::vector<OptionalQRCodeInfoExtension> inputOptionalExtensionData;

    VerifyOrReturnError(PayloadContents::operator==(input), false);

    inputOptionalVendorData = input.getAllOptionalVendorData();
    VerifyOrReturnError(optionalVendorData.size() == inputOptionalVendorData.size(), false);

    for (const OptionalQRCodeInfo & inputInfo : inputOptionalVendorData)
    {
        OptionalQRCodeInfo info;
        CHIP_ERROR err = getOptionalVendorData(inputInfo.tag, info);
        VerifyOrReturnError(err == CHIP_NO_ERROR, false);
        VerifyOrReturnError(inputInfo.type == info.type, false);
        VerifyOrReturnError(inputInfo.data == info.data, false);
        VerifyOrReturnError(inputInfo.int32 == info.int32, false);
    }

    inputOptionalExtensionData = input.getAllOptionalExtensionData();
    VerifyOrReturnError(optionalExtensionData.size() == inputOptionalExtensionData.size(), false);

    for (const OptionalQRCodeInfoExtension & inputInfo : inputOptionalExtensionData)
    {
        OptionalQRCodeInfoExtension info;
        CHIP_ERROR err = getOptionalExtensionData(inputInfo.tag, info);
        VerifyOrReturnError(err == CHIP_NO_ERROR, false);
        VerifyOrReturnError(inputInfo.type == info.type, false);
        VerifyOrReturnError(inputInfo.data == info.data, false);
        VerifyOrReturnError(inputInfo.int32 == info.int32, false);
        VerifyOrReturnError(inputInfo.int64 == info.int64, false);
        VerifyOrReturnError(inputInfo.uint32 == info.uint32, false);
        VerifyOrReturnError(inputInfo.uint64 == info.uint64, false);
    }

    return true;
}

} // namespace chip
