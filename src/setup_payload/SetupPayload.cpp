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
#include <lib/support/CodeUtils.h>
#include <lib/support/RandUtils.h>
#include <utility>

namespace chip {

bool IsCHIPTag(uint8_t tag)
{
    return tag >= (1 << kRawVendorTagLengthInBits);
}

bool IsVendorTag(uint8_t tag)
{
    return tag < (1 << kRawVendorTagLengthInBits);
}

// Check the Setup Payload for validity
//
// `vendor_id` and `product_id` are allowed all of uint16_t
bool SetupPayload::isValidQRCodePayload()
{
    if (version >= 1 << kVersionFieldLengthInBits)
    {
        return false;
    }

    if (static_cast<uint8_t>(commissioningFlow) > static_cast<uint8_t>((1 << kCommissioningFlowFieldLengthInBits) - 1))
    {
        return false;
    }

    chip::RendezvousInformationFlags allvalid(RendezvousInformationFlag::kBLE, RendezvousInformationFlag::kOnNetwork,
                                              RendezvousInformationFlag::kSoftAP);
    if (!rendezvousInformation.HasOnly(allvalid))
    {
        return false;
    }

    if (discriminator >= 1 << kPayloadDiscriminatorFieldLengthInBits)
    {
        return false;
    }

    if (setUpPINCode >= 1 << kSetupPINCodeFieldLengthInBits)
    {
        return false;
    }

    if (version == 0 && !rendezvousInformation.HasAny(allvalid) && discriminator == 0 && setUpPINCode == 0)
    {
        return false;
    }

    return true;
}

bool SetupPayload::isValidManualCode()
{
    // The discriminator for manual setup code is 4 most significant bits
    // in a regular 12 bit discriminator. Let's make sure that the provided
    // discriminator fits within 12 bits (kPayloadDiscriminatorFieldLengthInBits).
    // The manual setup code generator will only use 4 most significant bits from
    // it.
    if (discriminator >= 1 << kPayloadDiscriminatorFieldLengthInBits)
    {
        return false;
    }
    if (setUpPINCode >= 1 << kSetupPINCodeFieldLengthInBits)
    {
        return false;
    }

    if (setUpPINCode == 0)
    {
        return false;
    }

    return true;
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
    VerifyOrReturnError(IsCHIPTag(info.tag), CHIP_ERROR_INVALID_ARGUMENT);
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

    VerifyOrReturnError(this->version == input.version && this->vendorID == input.vendorID && this->productID == input.productID &&
                            this->commissioningFlow == input.commissioningFlow &&
                            this->rendezvousInformation == input.rendezvousInformation &&
                            this->discriminator == input.discriminator && this->setUpPINCode == input.setUpPINCode,
                        false);

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
