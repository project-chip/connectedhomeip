/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "KMUKeyAllocator.h"
#include <crypto/PSASessionKeystore.h>

namespace chip {
namespace DeviceLayer {

class KMUSessionKeystore : public chip::Crypto::PSASessionKeystore
{
public:
#if CHIP_CONFIG_ENABLE_ICD_CIP
    // Override PSASessionKeystore methods
    CHIP_ERROR PersistICDKey(chip::Crypto::Symmetric128BitsKeyHandle & key) override
    {
        CHIP_ERROR err        = CHIP_NO_ERROR;
        psa_key_id_t newKeyId = PSA_KEY_ID_NULL;
        psa_key_attributes_t attrs;
        uint8_t keyEx[chip::Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES];
        size_t keyExSize = sizeof(keyEx);

        psa_get_key_attributes(key.As<psa_key_id_t>(), &attrs);

        if (psa_get_key_lifetime(&attrs) ==
            PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(PSA_KEY_PERSISTENCE_DEFAULT, PSA_KEY_LOCATION_CRACEN_KMU))
        {
            psa_reset_key_attributes(&attrs);
            return CHIP_NO_ERROR;
        }

        newKeyId = chip::Crypto::GetPSAKeyAllocator().AllocateICDKeyId();
        VerifyOrExit(PSA_KEY_ID_NULL != newKeyId, err = CHIP_ERROR_INTERNAL);

        psa_set_key_id(&attrs, newKeyId);
        chip::Crypto::GetPSAKeyAllocator().UpdateKeyAttributes(attrs);

        // Remove EXPORT and COPY from the key usage flags for the new key since it should be forbidden to export the KMU key.
        psa_set_key_usage_flags(&attrs, psa_get_key_usage_flags(&attrs) & ~(PSA_KEY_USAGE_EXPORT | PSA_KEY_USAGE_COPY));

        // We cannot use psa_copy_key here because the source and target keys are stored in different locations.
        // Instead we need to export this key and import once again.
        VerifyOrExit(psa_export_key(key.As<psa_key_id_t>(), keyEx, sizeof(keyEx), &keyExSize) == PSA_SUCCESS,
                     err = CHIP_ERROR_INTERNAL);
        VerifyOrExit(psa_import_key(&attrs, keyEx, keyExSize, &newKeyId) == PSA_SUCCESS, err = CHIP_ERROR_INTERNAL);

    exit:
        DestroyKey(key);
        chip::Crypto::ClearSecret(keyEx, sizeof(keyEx));
        psa_reset_key_attributes(&attrs);

        if (err == CHIP_NO_ERROR)
        {
            auto & KeyId = key.AsMutable<psa_key_id_t>();
            KeyId        = newKeyId;
        }

        return err;
    }
#endif // CHIP_CONFIG_ENABLE_ICD_CIP
};

} // namespace DeviceLayer
} // namespace chip
