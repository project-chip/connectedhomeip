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
#include <math.h>

namespace chip {

// Check the Setup Payload for validity
//
// `vendor_id` and `product_id` are allowed all of uint16_t
// `requiresCustomFlow` is not checked since it is a bool
bool SetupPayload::isValid()
{
    if (!isManualPayloadSetup && version >= 1 << kVersionFieldLengthInBits)
    {
        return false;
    }
    if (!isManualPayloadSetup && rendezvousInformation >= 1 << kRendezvousInfoFieldLengthInBits)
    {
        return false;
    }
    if (isManualPayloadSetup && discriminator >= 1 << kManualSetupDiscriminatorFieldLengthInBits)
    {
        return false;
    } else if (discriminator >= 1 << kPayloadDiscriminatorFieldLengthInBits)
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

bool SetupPayload::operator==(const SetupPayload & input)
{
    return this->version == input.version && this->vendorID == input.vendorID && this->productID == input.productID &&
        this->requiresCustomFlow == input.requiresCustomFlow && this->rendezvousInformation == input.rendezvousInformation &&
        this->discriminator == input.discriminator && this->setUpPINCode == input.setUpPINCode;
}


size_t SetupPayload::manualSetupShortCodeCharLength()
{
    size_t numBits = 1 + kSetupPINCodeFieldLengthInBits + kManualSetupDiscriminatorFieldLengthInBits;
    return ceil(log10(pow(2, numBits)));
}

size_t SetupPayload::manualSetupLongCodeCharLength()
{
    return manualSetupShortCodeCharLength() + manualSetupVendorIdCharLength() + manualSetupProductIdCharLength();
}

size_t SetupPayload::manualSetupVendorIdCharLength()
{
    return ceil(log10(pow(2, kVendorIDFieldLengthInBits)));
}

size_t SetupPayload::manualSetupProductIdCharLength()
{
    return ceil(log10(pow(2, kProductIDFieldLengthInBits)));
}

} // namespace chip
