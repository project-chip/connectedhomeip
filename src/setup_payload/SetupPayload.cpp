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
    optionalData[info.tag] = info;
exit:
    return err;
}

CHIP_ERROR SetupPayload::addCHIPOptionalData(OptionalQRCodeInfo info)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(info.tag >= 1 << kRawVendorTagLengthInBits, err = CHIP_ERROR_INVALID_ARGUMENT);
    optionalData[info.tag] = info;
exit:
    return err;
}

vector<OptionalQRCodeInfo> SetupPayload::getAllOptionalData()
{
    vector<OptionalQRCodeInfo> returnedOptionalInfo;
    for (map<uint64_t, OptionalQRCodeInfo>::iterator it = optionalData.begin(); it != optionalData.end(); ++it)
    {
        returnedOptionalInfo.push_back(it->second);
    }
    return returnedOptionalInfo;
}

CHIP_ERROR SetupPayload::removeOptionalData(uint8_t tag)
{
    if (optionalData.find(tag) == optionalData.end())
    {
        return CHIP_ERROR_KEY_NOT_FOUND;
    }
    optionalData.erase(tag);
    return CHIP_NO_ERROR;
}

bool SetupPayload::operator==(const SetupPayload & input)
{
    return this->version == input.version && this->vendorID == input.vendorID && this->productID == input.productID &&
        this->requiresCustomFlow == input.requiresCustomFlow && this->rendezvousInformation == input.rendezvousInformation &&
        this->discriminator == input.discriminator && this->setUpPINCode == input.setUpPINCode;
}

} // namespace chip
