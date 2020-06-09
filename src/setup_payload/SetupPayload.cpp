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

using namespace chip;
using namespace std;
using namespace chip::TLV;

namespace chip {

bool IsCHIPTag(uint8_t tag)
{
    return tag >= 1 << kRawVendorTagLengthInBits;
}

bool IsVendorTag(uint8_t tag)
{
    return tag < 1 << kRawVendorTagLengthInBits;
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

    size_t rendezvousInfoAllowedFieldLengthInBits = kRendezvousInfoFieldLengthInBits - kRendezvousInfoReservedFieldLengthInBits;
    if (rendezvousInformation >= 1 << rendezvousInfoAllowedFieldLengthInBits)
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

    if (version == 0 && rendezvousInformation == 0 && discriminator == 0 && setUpPINCode == 0)
    {
        return false;
    }

    return true;
}

bool SetupPayload::isValidManualCode()
{
    if (discriminator >= 1 << kManualSetupDiscriminatorFieldLengthInBits)
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

CHIP_ERROR SetupPayload::addVendorOptionalData(OptionalQRCodeInfo info)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(info.tag < 1 << kRawVendorTagLengthInBits, err = CHIP_ERROR_INVALID_ARGUMENT);
    vendorOptionalData[info.tag] = info;
exit:
    return err;
}

CHIP_ERROR SetupPayload::addVendorOptionalData(uint8_t tag, string data)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    OptionalQRCodeInfo info;
    info.type = optionalQRCodeInfoTypeString;
    info.tag  = tag;
    info.data = data;
    err       = addVendorOptionalData(info);
    SuccessOrExit(err);
exit:
    return err;
}

CHIP_ERROR SetupPayload::addVendorOptionalData(uint8_t tag, int data)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    OptionalQRCodeInfo info;
    info.type    = optionalQRCodeInfoTypeInt;
    info.tag     = tag;
    info.integer = data;
    err          = addVendorOptionalData(info);
    SuccessOrExit(err);
exit:
    return err;
}

CHIP_ERROR SetupPayload::removeVendorOptionalData(uint8_t tag)
{
    if (vendorOptionalData.find(tag) == vendorOptionalData.end())
    {
        return CHIP_ERROR_KEY_NOT_FOUND;
    }
    vendorOptionalData.erase(tag);
    return CHIP_NO_ERROR;
}

vector<OptionalQRCodeInfo> SetupPayload::getAllVendorOptionalData()
{
    vector<OptionalQRCodeInfo> returnedOptionalInfo;
    for (map<uint8_t, OptionalQRCodeInfo>::iterator it = vendorOptionalData.begin(); it != vendorOptionalData.end(); ++it)
    {
        returnedOptionalInfo.push_back(it->second);
    }
    return returnedOptionalInfo;
}

CHIP_ERROR SetupPayload::addCHIPOptionalData(CHIPQRCodeInfo info)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(info.tag >= 1 << kRawVendorTagLengthInBits, err = CHIP_ERROR_INVALID_ARGUMENT);
    chipOptionalData[info.tag] = info;
exit:
    return err;
}

CHIP_ERROR SetupPayload::addSerialNumber(string serialNumber)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CHIPQRCodeInfo info;
    info.type       = chipQRCodeInfoTypeString;
    info.stringData = serialNumber;
    info.tag        = kSerialNumberTag;
    err             = addCHIPOptionalData(info);
    SuccessOrExit(err);
exit:
    return err;
}

CHIP_ERROR SetupPayload::addSerialNumber(uint32_t serialNumber)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CHIPQRCodeInfo info;
    info.type          = chipQRCodeInfoTypeUInt32;
    info.unsignedInt32 = serialNumber;
    info.tag           = kSerialNumberTag;
    err                = addCHIPOptionalData(info);
    SuccessOrExit(err);
exit:
    return err;
}

CHIP_ERROR SetupPayload::retrieveSerialNumber(string & outSerialNumber)
{
    CHIP_ERROR err                            = CHIP_NO_ERROR;
    map<uint8_t, CHIPQRCodeInfo>::iterator it = chipOptionalData.find(kSerialNumberTag);
    CHIPQRCodeInfo info;
    VerifyOrExit(it != chipOptionalData.end(), err = CHIP_ERROR_KEY_NOT_FOUND);

    info = it->second;
    switch (info.type)
    {
    case (chipQRCodeInfoTypeString):
        outSerialNumber = info.stringData;
        break;
    case (chipQRCodeInfoTypeUInt32):
        outSerialNumber = to_string(info.unsignedInt32);
        break;
    default:
        err = CHIP_ERROR_INVALID_ARGUMENT;
    }
exit:
    return err;
}

CHIP_ERROR SetupPayload::removeSerialNumber(void)
{
    chipOptionalData.erase(kSerialNumberTag);
    return CHIP_NO_ERROR;
}

vector<CHIPQRCodeInfo> SetupPayload::getAllCHIPOptionalData()
{
    vector<CHIPQRCodeInfo> returnedOptionalInfo;
    for (map<uint8_t, CHIPQRCodeInfo>::iterator it = chipOptionalData.begin(); it != chipOptionalData.end(); ++it)
    {
        returnedOptionalInfo.push_back(it->second);
    }
    return returnedOptionalInfo;
}

CHIP_ERROR SetupPayload::removeCHIPOptionalData(uint8_t tag)
{
    if (chipOptionalData.find(tag) == chipOptionalData.end())
    {
        return CHIP_ERROR_KEY_NOT_FOUND;
    }
    chipOptionalData.erase(tag);
    return CHIP_NO_ERROR;
}

bool SetupPayload::operator==(const SetupPayload & input)
{
    return this->version == input.version && this->vendorID == input.vendorID && this->productID == input.productID &&
        this->requiresCustomFlow == input.requiresCustomFlow && this->rendezvousInformation == input.rendezvousInformation &&
        this->discriminator == input.discriminator && this->setUpPINCode == input.setUpPINCode;
}

} // namespace chip
