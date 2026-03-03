/*
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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
#pragma once
#include <crypto/PSAKeyAllocator.h>
#include "mcux_psa_s2xx_key_locations.h"


namespace chip {
namespace DeviceLayer {
class S200KeyAllocator : public chip::Crypto::DefaultPSAKeyAllocator
{
public:
    void UpdateKeyAttributes(psa_key_attributes_t & attrs) override
    {
        if (psa_get_key_lifetime(&attrs) == PSA_KEY_LIFETIME_PERSISTENT)
        {
            psa_set_key_lifetime(&attrs, PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(
                                 PSA_KEY_LIFETIME_PERSISTENT, PSA_KEY_LOCATION_S200_KEY_STORAGE_NON_EL2GO));
        }
    }

private:
};

} // namespace DeviceLayer
} // namespace chip