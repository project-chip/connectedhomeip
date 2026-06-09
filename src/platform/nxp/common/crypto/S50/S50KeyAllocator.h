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
#include "els_pkc_driver.h"
#include <crypto/PSAKeyAllocator.h>

namespace chip {
namespace DeviceLayer {
class S50KeyAllocator : public chip::Crypto::DefaultPSAKeyAllocator
{
public:
    void UpdateKeyAttributes(psa_key_attributes_t & attrs) override
    {
        using namespace chip::Crypto;
        psa_key_id_t keyId          = psa_get_key_id(&attrs);
        psa_key_lifetime_t lifetime = psa_get_key_lifetime(&attrs);
        psa_key_type_t keyType      = psa_get_key_type(&attrs);

        if (psa_get_key_lifetime(&attrs) == PSA_KEY_LIFETIME_PERSISTENT)
        {
            psa_set_key_lifetime(&attrs,
                                 PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(PSA_KEY_LIFETIME_PERSISTENT,
                                                                                PSA_CRYPTO_ELS_PKC_LOCATION_S50_RFC3394_STORAGE));
        }

        /* NON_EL2GO storage for CASE Ephemeral Keys and pending operational keys
        + Ignoring PSA_ALG_ECDH since ECDH keys are not supported in S50 and should not be allocated in this storage.
        */
        if ((lifetime == PSA_KEY_LIFETIME_VOLATILE) && PSA_KEY_TYPE_IS_ECC_KEY_PAIR(keyType) &&
            (PSA_ECC_FAMILY_SECP_R1 == PSA_KEY_TYPE_ECC_GET_FAMILY(keyType)) &&
            (psa_get_key_bits(&attrs) == kP256_PrivateKey_Length * 8) && (psa_get_key_algorithm(&attrs) != PSA_ALG_ECDH))
        {
            psa_set_key_lifetime(&attrs,
                                 PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(PSA_KEY_LIFETIME_VOLATILE,
                                                                                PSA_CRYPTO_ELS_PKC_LOCATION_S50_RFC3394_STORAGE));
            return;
        }
    }

private:
};

} // namespace DeviceLayer
} // namespace chip
