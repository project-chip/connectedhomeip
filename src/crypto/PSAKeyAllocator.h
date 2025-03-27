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

#if CHIP_HAVE_CONFIG_H
#include <crypto/CryptoBuildConfig.h>
#endif // CHIP_HAVE_CONFIG_H

#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/TypeTraits.h>

namespace chip {
namespace Crypto {

static_assert(PSA_KEY_ID_USER_MIN <= CHIP_CONFIG_CRYPTO_PSA_KEY_ID_BASE && CHIP_CONFIG_CRYPTO_PSA_KEY_ID_END <= PSA_KEY_ID_USER_MAX,
              "Matter specific PSA key range doesn't fit within PSA allowed range");

// Each ICD client requires storing two keys- AES and HMAC
static constexpr uint32_t kMaxICDClientKeys = 2 * CHIP_CONFIG_CRYPTO_PSA_ICD_MAX_CLIENTS;

static_assert(kMaxICDClientKeys >= CHIP_CONFIG_ICD_CLIENTS_SUPPORTED_PER_FABRIC * CHIP_CONFIG_MAX_FABRICS,
              "Number of allocated ICD key slots is lower than maximum number of supported ICD clients");

/**
 * @brief Defines subranges of the PSA key identifier space used by Matter.
 */
enum class KeyIdBase : psa_key_id_t
{
    Minimum          = CHIP_CONFIG_CRYPTO_PSA_KEY_ID_BASE,
    Operational      = Minimum, ///< Base of the PSA key ID range for Node Operational Certificate private keys
    DACPrivKey       = Operational + kMaxValidFabricIndex + 1,
    ICDKeyRangeStart = DACPrivKey + 1,
    Maximum          = ICDKeyRangeStart + kMaxICDClientKeys,
};

static_assert(to_underlying(KeyIdBase::Minimum) >= CHIP_CONFIG_CRYPTO_PSA_KEY_ID_BASE &&
                  to_underlying(KeyIdBase::Maximum) <= CHIP_CONFIG_CRYPTO_PSA_KEY_ID_END,
              "PSA key ID base out of allowed range");

/**
 * @brief Finds first free persistent Key slot ID within range.
 *
 * @param[out] keyId Key ID handler to which free ID will be set.
 * @param[in]  start Starting ID in search range.
 * @param[in]  range Search range.
 *
 * @retval CHIP_NO_ERROR               On success.
 * @retval CHIP_ERROR_INTERNAL         On PSA crypto API error.
 * @retval CHIP_ERROR_NOT_FOUND        On no free Key ID within range.
 * @retval CHIP_ERROR_INVALID_ARGUMENT On search arguments out of PSA allowed range.
 */
CHIP_ERROR FindFreeKeySlotInRange(psa_key_id_t & keyId, psa_key_id_t start, uint32_t range);

/**
 * @brief Calculates PSA key ID for Node Operational Certificate private key for the given fabric.
 */
constexpr psa_key_id_t MakeOperationalKeyId(FabricIndex fabricIndex)
{
    return to_underlying(KeyIdBase::Operational) + static_cast<psa_key_id_t>(fabricIndex);
}

/**
 * @brief Interface for PSA key allocation.
 *
 * The PSA Key Allocator interface provides an abstraction that allows the application to
 * allocate PSA keys in a secure environment. This class uses a concept that isolates the
 * application from the actual key material. The secure location may vary depending on the
 * cryptographic hardware used. Using this class a platform can implement this interface to
 * allocate keys in the specific secure location.
 *
 * In some cases key attributes must be redefined to match the specific requirements of the
 * secure location and the cryptographic hardware.
 *
 * This class keeps the static instance and uses it in the all the places where the key
 * should be persisted. You can add the usage of this class anywhere in the code by calling the
 * GetPSAKeyAllocator() function.
 *
 * If the static instance is not set, the default implementation is used.
 *
 * To change the static instance of the PSAKeyAllocator, you can call the SetPSAKeyAllocator function.
 */
class PSAKeyAllocator
{
public:
    /**
     * @brief Destructor for PSAKeyAllocator.
     */
    virtual ~PSAKeyAllocator() = default;

    /**
     * @brief Get the Device Attestation Key (DAC) ID.
     *
     * @return psa_key_id_t The DAC key ID.
     */
    virtual psa_key_id_t GetDacKeyId() = 0;

    /**
     * @brief Get the Node Operational Certificate key ID for a given fabric index.
     *
     * @param fabricIndex The fabric index for which the operational key ID is requested.
     * @return psa_key_id_t The operational key ID.
     */
    virtual psa_key_id_t GetOpKeyId(FabricIndex fabricIndex) = 0;

    /**
     * @brief Allocate a new Intermittently Connected Devices (ICD) key ID.
     *
     * This method is used to allocate both AES-CCM and HMAC (SHA-256) keys independently.
     * The caller is responsible for storing the key ID in non-volatile memory
     * and setting the appropriate key type.
     *
     * @return psa_key_id_t The newly allocated ICD key ID.
     */
    virtual psa_key_id_t AllocateICDKeyId() = 0;

    /**
     * @brief Update the key attributes before storing the key.
     *
     * In some cases the key attributes must be redefined to match the specific requirements of the
     * secure location and the cryptographic hardware. This method allows the platform to update the
     * key attributes before storing the key.
     *
     * Read the current key attributes to determine the key type, algorithm, and usage flags. Update
     * the key attributes as needed.
     *
     * @param attrs Reference to the key attributes structure to be updated.
     */
    virtual void UpdateKeyAttributes(psa_key_attributes_t & attrs) = 0;

    // Allow setting and getting the static instance of the PSAKeyAllocator by external functions
    friend PSAKeyAllocator & GetPSAKeyAllocator();
    friend void SetPSAKeyAllocator(PSAKeyAllocator * keyAllocator);

private:
    static PSAKeyAllocator * sInstance;
};

/**
 * @brief Default implementation of PSAKeyAllocator.
 *
 * This default implementation allocates key IDs according to the KeyIdBase enum.
 * The operational key ID is calculated as the base operational key ID plus the fabric index.
 * The DAC key ID is calculated as the base DAC key ID.
 * The ICD key ID is allocated from the range starting from the ICDKeyRangeStart.
 * The key attributes are not updated.
 */
class DefaultPSAKeyAllocator : public PSAKeyAllocator
{
public:
    // implementations of the PSAKeyAllocator interface
    psa_key_id_t GetDacKeyId() override { return to_underlying(KeyIdBase::DACPrivKey); }
    psa_key_id_t GetOpKeyId(FabricIndex fabricIndex) override { return MakeOperationalKeyId(fabricIndex); }
    psa_key_id_t AllocateICDKeyId() override
    {
        psa_key_id_t newKeyId = PSA_KEY_ID_NULL;
        if (CHIP_NO_ERROR !=
            Crypto::FindFreeKeySlotInRange(newKeyId, to_underlying(KeyIdBase::ICDKeyRangeStart), kMaxICDClientKeys))
        {
            newKeyId = PSA_KEY_ID_NULL;
        }
        return newKeyId;
    }
    void UpdateKeyAttributes(psa_key_attributes_t & attrs) override
    {
        // Do nothing
    }
};

/**
 * @brief Static function to get the instance of PSAKeyAllocator.
 *
 * If the static instance is not set, the default implementation is returned.
 *
 * @return PSAKeyAllocator reference to the instance of PSAKeyAllocator.
 */
inline PSAKeyAllocator & GetPSAKeyAllocator()
{
    if (!PSAKeyAllocator::sInstance)
    {
        static DefaultPSAKeyAllocator defaultAllocator;
        return defaultAllocator;
    }
    return *PSAKeyAllocator::sInstance;
}

/**
 * @brief Set the static implementation of the PSAKeyAllocator.
 *
 * Providing nullptr as an argument will revert to the default implementation.
 *
 * @param keyAllocator Pointer to the PSAKeyAllocator instance to be set.
 */
inline void SetPSAKeyAllocator(PSAKeyAllocator * keyAllocator)
{
    PSAKeyAllocator::sInstance = keyAllocator;
}

// Initialize the static global PSAKeyAllocator instance
// To avoid the need for an additional source file, we initialize the static instance here using the 'inline' keyword.
// This is possible due to a C++17 feature, which is available starting from the 201606L standard.
// The functionality is verified in the TestPSAOpKeyStore/ TestKeyAllocation test case.
inline PSAKeyAllocator * PSAKeyAllocator::sInstance = nullptr;

} // namespace Crypto
} // namespace chip
