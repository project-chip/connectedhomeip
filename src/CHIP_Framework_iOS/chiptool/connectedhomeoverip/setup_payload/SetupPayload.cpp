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
#include <vector>

//#include <support/CodeUtils.h>

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
    if (rendezvousInformation >= 1 << kRendezvousInfoFieldLengthInBits)
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

bool SetupPayload::operator==(const SetupPayload & input)
{
    return this->version == input.version && this->vendorID == input.vendorID && this->productID == input.productID &&
        this->requiresCustomFlow == input.requiresCustomFlow && this->rendezvousInformation == input.rendezvousInformation &&
        this->discriminator == input.discriminator && this->setUpPINCode == input.setUpPINCode;
}

} // namespace chip
