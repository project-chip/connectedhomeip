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
#include <lib/core/CHIPTLV.h>
#include <lib/core/CHIPTLVData.hpp>
#include <lib/core/CHIPTLVUtilities.hpp>
#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <lib/support/CodeUtils.h>
#include <utility>

namespace chip {

// Spec 5.1.4.2 CHIPCommon tag numbers are in the range [0x00, 0x7F]
bool SetupPayload::IsCommonTag(uint8_t tag)
{
    return tag < 0x80;
}

// Spec 5.1.4.1 Manufacture-specific tag numbers are in the range [0x80, 0xFF]
bool SetupPayload::IsVendorTag(uint8_t tag)
{
    return !IsCommonTag(tag);
}

// Check the Setup Payload for validity
//
// `vendor_id` and `product_id` are allowed all of uint16_t
bool PayloadContents::isValidQRCodePayload() const
{
    // 3-bit value specifying the QR code payload version.
    if (version >= 1 << kVersionFieldLengthInBits)
    {
        return false;
    }

    if (static_cast<uint8_t>(commissioningFlow) > static_cast<uint8_t>((1 << kCommissioningFlowFieldLengthInBits) - 1))
    {
        return false;
    }

    // Device Commissioning Flow
    // 0: Standard commissioning flow: such a device, when uncommissioned, always enters commissioning mode upon power-up, subject
    // to the rules in [ref_Announcement_Commencement]. 1: User-intent commissioning flow: user action required to enter
    // commissioning mode. 2: Custom commissioning flow: interaction with a vendor-specified means is needed before commissioning.
    // 3: Reserved
    if (commissioningFlow != CommissioningFlow::kStandard && commissioningFlow != CommissioningFlow::kUserActionRequired &&
        commissioningFlow != CommissioningFlow::kCustom)
    {
        return false;
    }

    chip::RendezvousInformationFlags allvalid(RendezvousInformationFlag::kBLE, RendezvousInformationFlag::kOnNetwork,
                                              RendezvousInformationFlag::kSoftAP);
    if (!rendezvousInformation.HasValue() || !rendezvousInformation.Value().HasOnly(allvalid))
    {
        return false;
    }

    // Discriminator validity is enforced by the SetupDiscriminator class.

    if (setUpPINCode >= 1 << kSetupPINCodeFieldLengthInBits)
    {
        return false;
    }

    return CheckPayloadCommonConstraints();
}

bool PayloadContents::isValidManualCode() const
{
    // Discriminator validity is enforced by the SetupDiscriminator class.

    if (setUpPINCode >= 1 << kSetupPINCodeFieldLengthInBits)
    {
        return false;
    }

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
    // A version not equal to 0 would be invalid for v1 and would indicate new format (e.g. version 2)
    if (version != 0)
    {
        return false;
    }

    if (!IsValidSetupPIN(setUpPINCode))
    {
        return false;
    }

    // VendorID must be unspecified (0) or in valid range expected.
    if (!IsVendorIdValidOperationally(vendorID) && (vendorID != VendorId::Unspecified))
    {
        return false;
    }

    // A value of 0x0000 SHALL NOT be assigned to a product since Product ID = 0x0000 is used for these specific cases:
    //  * To announce an anonymized Product ID as part of device discovery
    //  * To indicate an OTA software update file applies to multiple Product IDs equally.
    //  * To avoid confusion when presenting the Onboarding Payload for ECM with multiple nodes
    if (productID == 0 && vendorID != VendorId::Unspecified)
    {
        return false;
    }

    return true;
}

bool PayloadContents::operator==(PayloadContents & input) const
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

CHIP_ERROR SetupPayload::getOptionalVendorData(uint8_t tag, OptionalQRCodeInfo & info)
{
    VerifyOrReturnError(optionalVendorData.find(tag) != optionalVendorData.end(), CHIP_ERROR_KEY_NOT_FOUND);
    info = optionalVendorData[tag];

    return CHIP_NO_ERROR;
}

CHIP_ERROR SetupPayload::getOptionalExtensionData(uint8_t tag, OptionalQRCodeInfoExtension & info) const
{
    const auto it = optionalExtensionData.find(tag);
    VerifyOrReturnError(it != optionalExtensionData.end(), CHIP_ERROR_KEY_NOT_FOUND);
    info = it->second;
    return CHIP_NO_ERROR;
}

optionalQRCodeInfoType SetupPayload::getNumericTypeFor(uint8_t tag)
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

std::vector<OptionalQRCodeInfoExtension> SetupPayload::getAllOptionalExtensionData()
{
    std::vector<OptionalQRCodeInfoExtension> returnedOptionalInfo;
    for (auto & entry : optionalExtensionData)
    {
        returnedOptionalInfo.push_back(entry.second);
    }
    return returnedOptionalInfo;
}

bool SetupPayload::operator==(SetupPayload & input)
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
