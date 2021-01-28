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

#include <core/CHIPCore.h>
#include <core/CHIPTLV.h>
#include <core/CHIPTLVData.hpp>
#include <core/CHIPTLVUtilities.hpp>
#include <support/CodeUtils.h>
#include <support/RandUtils.h>
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
// `requiresCustomFlow` is not checked since it is a bool
bool SetupPayload::isValidQRCodePayload()
{
    if (version >= 1 << kVersionFieldLengthInBits)
    {
        return false;
    }

    if (rendezvousInformation > RendezvousInformationFlags::kAllMask)
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

    if (version == 0 && rendezvousInformation == RendezvousInformationFlags::kNone && discriminator == 0 && setUpPINCode == 0)
    {
        return false;
    }

    return true;
}

bool SetupPayload::isValidManualCode()
{
    // The discriminator for manual setup code is 4 least significant bits
    // in a regular 12 bit discriminator. Let's make sure that the provided
    // discriminator fits within 12 bits (kPayloadDiscriminatorFieldLengthInBits).
    // The manual setup code generator will only use 4 least significant bits from
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
    CHIP_ERROR err = CHIP_NO_ERROR;
    OptionalQRCodeInfo info;
    info.tag  = tag;
    info.type = optionalQRCodeInfoTypeString;
    info.data = std::move(data);

    err = addOptionalVendorData(info);
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR SetupPayload::addOptionalVendorData(uint8_t tag, int32_t data)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    OptionalQRCodeInfo info;
    info.tag   = tag;
    info.type  = optionalQRCodeInfoTypeInt32;
    info.int32 = data;

    err = addOptionalVendorData(info);
    SuccessOrExit(err);

exit:
    return err;
}

std::vector<OptionalQRCodeInfo> SetupPayload::getAllOptionalVendorData()
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
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(optionalVendorData.find(tag) != optionalVendorData.end(), err = CHIP_ERROR_KEY_NOT_FOUND);
    optionalVendorData.erase(tag);

exit:
    return err;
}

CHIP_ERROR SetupPayload::addSerialNumber(std::string serialNumber)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    OptionalQRCodeInfoExtension info;
    info.tag  = kSerialNumberTag;
    info.type = optionalQRCodeInfoTypeString;
    info.data = std::move(serialNumber);

    err = addOptionalExtensionData(info);
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR SetupPayload::addSerialNumber(uint32_t serialNumber)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    OptionalQRCodeInfoExtension info;
    info.tag    = kSerialNumberTag;
    info.type   = optionalQRCodeInfoTypeUInt32;
    info.uint32 = serialNumber;

    err = addOptionalExtensionData(info);
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR SetupPayload::getSerialNumber(std::string & outSerialNumber)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    OptionalQRCodeInfoExtension info;
    err = getOptionalExtensionData(kSerialNumberTag, info);
    SuccessOrExit(err);

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
    }

exit:
    return err;
}

CHIP_ERROR SetupPayload::removeSerialNumber()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(optionalExtensionData.find(kSerialNumberTag) != optionalExtensionData.end(), err = CHIP_ERROR_KEY_NOT_FOUND);
    optionalExtensionData.erase(kSerialNumberTag);

exit:
    return err;
}

CHIP_ERROR SetupPayload::addOptionalVendorData(const OptionalQRCodeInfo & info)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(IsVendorTag(info.tag), err = CHIP_ERROR_INVALID_ARGUMENT);
    optionalVendorData[info.tag] = info;

exit:
    return err;
}

CHIP_ERROR SetupPayload::addOptionalExtensionData(const OptionalQRCodeInfoExtension & info)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(IsCHIPTag(info.tag), err = CHIP_ERROR_INVALID_ARGUMENT);
    optionalExtensionData[info.tag] = info;

exit:
    return err;
}

CHIP_ERROR SetupPayload::getOptionalVendorData(uint8_t tag, OptionalQRCodeInfo & info)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(optionalVendorData.find(tag) != optionalVendorData.end(), err = CHIP_ERROR_KEY_NOT_FOUND);
    info = optionalVendorData[tag];

exit:
    return err;
}

CHIP_ERROR SetupPayload::getOptionalExtensionData(uint8_t tag, OptionalQRCodeInfoExtension & info)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(optionalExtensionData.find(tag) != optionalExtensionData.end(), err = CHIP_ERROR_KEY_NOT_FOUND);
    info = optionalExtensionData[tag];

exit:
    return err;
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
    bool isIdentical = true;
    std::vector<OptionalQRCodeInfo> inputOptionalVendorData;
    std::vector<OptionalQRCodeInfoExtension> inputOptionalExtensionData;

    VerifyOrExit(this->version == input.version && this->vendorID == input.vendorID && this->productID == input.productID &&
                     this->requiresCustomFlow == input.requiresCustomFlow &&
                     this->rendezvousInformation == input.rendezvousInformation && this->discriminator == input.discriminator &&
                     this->setUpPINCode == input.setUpPINCode,
                 isIdentical = false);

    inputOptionalVendorData = input.getAllOptionalVendorData();
    VerifyOrExit(optionalVendorData.size() == inputOptionalVendorData.size(), isIdentical = false);

    for (const OptionalQRCodeInfo & inputInfo : inputOptionalVendorData)
    {
        OptionalQRCodeInfo info;
        CHIP_ERROR err = getOptionalVendorData(inputInfo.tag, info);
        VerifyOrExit(err == CHIP_NO_ERROR, isIdentical = false);
        VerifyOrExit(inputInfo.type == info.type, isIdentical = false);
        VerifyOrExit(inputInfo.data == info.data, isIdentical = false);
        VerifyOrExit(inputInfo.int32 == info.int32, isIdentical = false);
    }

    inputOptionalExtensionData = input.getAllOptionalExtensionData();
    VerifyOrExit(optionalExtensionData.size() == inputOptionalExtensionData.size(), isIdentical = false);

    for (const OptionalQRCodeInfoExtension & inputInfo : inputOptionalExtensionData)
    {
        OptionalQRCodeInfoExtension info;
        CHIP_ERROR err = getOptionalExtensionData(inputInfo.tag, info);
        VerifyOrExit(err == CHIP_NO_ERROR, isIdentical = false);
        VerifyOrExit(inputInfo.type == info.type, isIdentical = false);
        VerifyOrExit(inputInfo.data == info.data, isIdentical = false);
        VerifyOrExit(inputInfo.int32 == info.int32, isIdentical = false);
        VerifyOrExit(inputInfo.int64 == info.int64, isIdentical = false);
        VerifyOrExit(inputInfo.uint32 == info.uint32, isIdentical = false);
        VerifyOrExit(inputInfo.uint64 == info.uint64, isIdentical = false);
    }

exit:
    return isIdentical;
}

} // namespace chip
