/*
 *
 *    <COPYRIGHT>
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

namespace chip {

// Check the Setup Payload for validity
//
// `vendor_id` and `product_id` are allowed all of uint16_t
// `requiresCustomFlow` is not checked since it is a bool
bool SetupPayload::isValid()
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
    // make sure the rendezvousInformation only uses allowed values
    // i.e it must be > 0 and set to 1 or a power of 2.
    uint16_t rezInfo = rendezvousInformation;
    if (rezInfo == 0 || (rezInfo != 1 && (rezInfo & (rezInfo - 1)) != 0))
    {
        return false;
    }
    return true;
}

} // namespace chip