/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <inttypes.h>

#include <pw_unit_test/framework.h>

#include <crypto/PSAKeyAllocator.h>
#include <crypto/PSAOperationalKeystore.h>
#include <crypto/PersistentStorageOperationalKeystore.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/Span.h>
#include <lib/support/TestPersistentStorageDelegate.h>

using namespace chip;
using namespace chip::Crypto;

namespace {

struct TestPSAOpKeyStore : public ::testing::Test
{
    static void SetUpTestSuite()
    {
        ASSERT_EQ(CHIP_NO_ERROR, chip::Platform::MemoryInit());

#if CHIP_CRYPTO_PSA
        psa_crypto_init();
#endif
    }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestPSAOpKeyStore, TestBasicLifeCycle)
{
    PSAOperationalKeystore opKeystore;

    FabricIndex kFabricIndex    = 111;
    FabricIndex kBadFabricIndex = static_cast<FabricIndex>(kFabricIndex + 10u);

    // Can generate a key and get a CSR
    uint8_t csrBuf[kMIN_CSR_Buffer_Size];
    MutableByteSpan csrSpan{ csrBuf };
    CHIP_ERROR err = opKeystore.NewOpKeypairForFabric(kFabricIndex, csrSpan);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(opKeystore.HasPendingOpKeypair());
    EXPECT_FALSE(opKeystore.HasOpKeypairForFabric(kFabricIndex));

    P256PublicKey csrPublicKey1;
    err = VerifyCertificateSigningRequest(csrSpan.data(), csrSpan.size(), csrPublicKey1);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Can regenerate a second CSR and it has different PK
    csrSpan = MutableByteSpan{ csrBuf };
    err     = opKeystore.NewOpKeypairForFabric(kFabricIndex, csrSpan);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(opKeystore.HasPendingOpKeypair());

    P256PublicKey csrPublicKey2;
    err = VerifyCertificateSigningRequest(csrSpan.data(), csrSpan.size(), csrPublicKey2);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_FALSE(csrPublicKey1.Matches(csrPublicKey2));

    // Cannot NewOpKeypair for a different fabric if one already pending
    uint8_t badCsrBuf[kMIN_CSR_Buffer_Size];
    MutableByteSpan badCsrSpan{ badCsrBuf };
    err = opKeystore.NewOpKeypairForFabric(kBadFabricIndex, badCsrSpan);
    EXPECT_EQ(err, CHIP_ERROR_INVALID_FABRIC_INDEX);
    EXPECT_TRUE(opKeystore.HasPendingOpKeypair());

    // Fail to generate CSR for invalid fabrics
    csrSpan = MutableByteSpan{ csrBuf };
    err     = opKeystore.NewOpKeypairForFabric(kUndefinedFabricIndex, csrSpan);
    EXPECT_EQ(err, CHIP_ERROR_INVALID_FABRIC_INDEX);

    csrSpan = MutableByteSpan{ csrBuf };
    err     = opKeystore.NewOpKeypairForFabric(kMaxValidFabricIndex + 1, csrSpan);
    EXPECT_EQ(err, CHIP_ERROR_INVALID_FABRIC_INDEX);

    // No storage done by NewOpKeypairForFabric
    EXPECT_FALSE(opKeystore.HasOpKeypairForFabric(kFabricIndex));

    // Even after error, the previous valid pending keypair stays valid.
    EXPECT_TRUE(opKeystore.HasPendingOpKeypair());

    // Activating with mismatching fabricIndex and matching public key fails
    err = opKeystore.ActivateOpKeypairForFabric(kBadFabricIndex, csrPublicKey2);
    EXPECT_EQ(err, CHIP_ERROR_INVALID_FABRIC_INDEX);
    EXPECT_TRUE(opKeystore.HasPendingOpKeypair());
    EXPECT_FALSE(opKeystore.HasOpKeypairForFabric(kFabricIndex));

    // Activating with matching fabricIndex and mismatching public key fails
    err = opKeystore.ActivateOpKeypairForFabric(kFabricIndex, csrPublicKey1);
    EXPECT_EQ(err, CHIP_ERROR_INVALID_PUBLIC_KEY);
    EXPECT_TRUE(opKeystore.HasPendingOpKeypair());
    EXPECT_FALSE(opKeystore.HasOpKeypairForFabric(kFabricIndex));

    // Before successful activation, cannot sign
    uint8_t message[] = { 1, 2, 3, 4 };
    P256ECDSASignature sig1;
    err = opKeystore.SignWithOpKeypair(kFabricIndex, ByteSpan{ message }, sig1);
    EXPECT_EQ(err, CHIP_ERROR_INVALID_FABRIC_INDEX);

    // Activating with matching fabricIndex and matching public key succeeds
    err = opKeystore.ActivateOpKeypairForFabric(kFabricIndex, csrPublicKey2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Activating does not store, and keeps pending
    EXPECT_TRUE(opKeystore.HasPendingOpKeypair());
    EXPECT_TRUE(opKeystore.HasOpKeypairForFabric(kFabricIndex));
    EXPECT_FALSE(opKeystore.HasOpKeypairForFabric(kBadFabricIndex));

    // Can't sign for wrong fabric after activation
    P256ECDSASignature sig2;
    err = opKeystore.SignWithOpKeypair(kBadFabricIndex, ByteSpan{ message }, sig2);
    EXPECT_EQ(err, CHIP_ERROR_INVALID_FABRIC_INDEX);

    // Can sign after activation
    err = opKeystore.SignWithOpKeypair(kFabricIndex, ByteSpan{ message }, sig2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Signature matches pending key
    err = csrPublicKey2.ECDSA_validate_msg_signature(message, sizeof(message), sig2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Signature does not match a previous pending key
    err = csrPublicKey1.ECDSA_validate_msg_signature(message, sizeof(message), sig2);
    EXPECT_EQ(err, CHIP_ERROR_INVALID_SIGNATURE);

    // Committing with mismatching fabric fails, leaves pending
    err = opKeystore.CommitOpKeypairForFabric(kBadFabricIndex);
    EXPECT_EQ(err, CHIP_ERROR_INVALID_FABRIC_INDEX);
    EXPECT_TRUE(opKeystore.HasPendingOpKeypair());
    EXPECT_TRUE(opKeystore.HasOpKeypairForFabric(kFabricIndex));

    // Committing key resets pending state
    err = opKeystore.CommitOpKeypairForFabric(kFabricIndex);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_FALSE(opKeystore.HasPendingOpKeypair());
    EXPECT_TRUE(opKeystore.HasOpKeypairForFabric(kFabricIndex));

    // Verify if the key is not exportable - the PSA_KEY_USAGE_EXPORT psa flag should not be set
    P256SerializedKeypair serializedKeypair;
    EXPECT_EQ(opKeystore.ExportOpKeypairForFabric(kFabricIndex, serializedKeypair), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    // After committing, signing works with the key that was pending
    P256ECDSASignature sig3;
    uint8_t message2[] = { 10, 11, 12, 13 };
    err                = opKeystore.SignWithOpKeypair(kFabricIndex, ByteSpan{ message2 }, sig3);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = csrPublicKey2.ECDSA_validate_msg_signature(message2, sizeof(message2), sig3);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Let's remove the opkey for a fabric, it disappears
    err = opKeystore.RemoveOpKeypairForFabric(kFabricIndex);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_FALSE(opKeystore.HasPendingOpKeypair());
    EXPECT_FALSE(opKeystore.HasOpKeypairForFabric(kFabricIndex));
}

TEST_F(TestPSAOpKeyStore, TestEphemeralKeys)
{
    PSAOperationalKeystore opKeyStore;

    Crypto::P256ECDSASignature sig;
    uint8_t message[] = { 'm', 's', 'g' };

    Crypto::P256Keypair * ephemeralKeypair = opKeyStore.AllocateEphemeralKeypairForCASE();
    EXPECT_NE(nullptr, ephemeralKeypair);
    EXPECT_EQ(ephemeralKeypair->Initialize(Crypto::ECPKeyTarget::ECDSA), CHIP_NO_ERROR);

    EXPECT_EQ(ephemeralKeypair->ECDSA_sign_msg(message, sizeof(message), sig), CHIP_NO_ERROR);
    EXPECT_EQ(ephemeralKeypair->Pubkey().ECDSA_validate_msg_signature(message, sizeof(message), sig), CHIP_NO_ERROR);

    opKeyStore.ReleaseEphemeralKeypair(ephemeralKeypair);
}

TEST_F(TestPSAOpKeyStore, TestMigrationKeys)
{
    chip::TestPersistentStorageDelegate storage;
    PSAOperationalKeystore psaOpKeyStore;
    PersistentStorageOperationalKeystore persistentOpKeyStore;
    constexpr FabricIndex kFabricIndex = 111;

    // Failure before Init of MoveOpKeysFromPersistentStorageToITS
    EXPECT_EQ(psaOpKeyStore.MigrateOpKeypairForFabric(kFabricIndex, persistentOpKeyStore), CHIP_ERROR_INCORRECT_STATE);

    // Initialize both operational key stores
    EXPECT_EQ(persistentOpKeyStore.Init(&storage), CHIP_NO_ERROR);

    // Failure on invalid Fabric indexes
    EXPECT_EQ(psaOpKeyStore.MigrateOpKeypairForFabric(kUndefinedFabricIndex, persistentOpKeyStore),
              CHIP_ERROR_INVALID_FABRIC_INDEX);
    EXPECT_EQ(psaOpKeyStore.MigrateOpKeypairForFabric(kMaxValidFabricIndex + 1, persistentOpKeyStore),
              CHIP_ERROR_INVALID_FABRIC_INDEX);

    // Failure on the key migration, while the key does not exist in the any keystore.
    EXPECT_EQ(storage.GetNumKeys(), 0u);
    EXPECT_FALSE(storage.HasKey(DefaultStorageKeyAllocator::FabricOpKey(kFabricIndex).KeyName()));
    EXPECT_FALSE(psaOpKeyStore.HasOpKeypairForFabric(kFabricIndex));
    EXPECT_EQ(psaOpKeyStore.MigrateOpKeypairForFabric(kFabricIndex, persistentOpKeyStore),
              CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    auto generateAndStore = [&](FabricIndex index, MutableByteSpan & buf, P256PublicKey & pubKey) {
        EXPECT_FALSE(persistentOpKeyStore.HasPendingOpKeypair());
        EXPECT_EQ(persistentOpKeyStore.NewOpKeypairForFabric(kFabricIndex, buf), CHIP_NO_ERROR);
        EXPECT_EQ(VerifyCertificateSigningRequest(buf.data(), buf.size(), pubKey), CHIP_NO_ERROR);
        EXPECT_TRUE(persistentOpKeyStore.HasPendingOpKeypair());
        EXPECT_EQ(persistentOpKeyStore.ActivateOpKeypairForFabric(kFabricIndex, pubKey), CHIP_NO_ERROR);
        EXPECT_EQ(storage.GetNumKeys(), 0u);
        EXPECT_EQ(persistentOpKeyStore.CommitOpKeypairForFabric(kFabricIndex), CHIP_NO_ERROR);
        EXPECT_FALSE(persistentOpKeyStore.HasPendingOpKeypair());
        EXPECT_EQ(storage.GetNumKeys(), 1u);
        EXPECT_TRUE(storage.HasKey(DefaultStorageKeyAllocator::FabricOpKey(kFabricIndex).KeyName()));
    };

    // Save a key to the old persistent storage
    uint8_t csrBuf[kMIN_CSR_Buffer_Size];
    MutableByteSpan csrSpan{ csrBuf };
    P256PublicKey csrPublicKey1;
    generateAndStore(kFabricIndex, csrSpan, csrPublicKey1);

    // Migrate key to PSA ITS
    EXPECT_EQ(psaOpKeyStore.MigrateOpKeypairForFabric(kFabricIndex, persistentOpKeyStore), CHIP_NO_ERROR);
    EXPECT_TRUE(psaOpKeyStore.HasOpKeypairForFabric(kFabricIndex));

    // Verify the migrated keys
    P256ECDSASignature sig1;
    uint8_t message1[] = { 10, 11, 12, 13 };
    EXPECT_EQ(psaOpKeyStore.SignWithOpKeypair(kFabricIndex, ByteSpan{ message1 }, sig1), CHIP_NO_ERROR);

    // To verify use the public key generated by the old persistent storage
    EXPECT_EQ(csrPublicKey1.ECDSA_validate_msg_signature(message1, sizeof(message1), sig1), CHIP_NO_ERROR);

    // After migration there should be no old keys anymore
    EXPECT_EQ(storage.GetNumKeys(), 0u);
    EXPECT_FALSE(storage.HasKey(DefaultStorageKeyAllocator::FabricOpKey(kFabricIndex).KeyName()));

    // Verify that migration method returns success when there is no OpKey stored in the old keystore, but already exists in PSA
    // ITS.
    EXPECT_EQ(psaOpKeyStore.MigrateOpKeypairForFabric(kFabricIndex, persistentOpKeyStore), CHIP_NO_ERROR);

    // The key already exists in ITS, but there is an another attempt to migrate the new key.
    // The key should not be overwritten, but the key from the previous persistent keystore should be removed.
    MutableByteSpan csrSpan2{ csrBuf };
    generateAndStore(kFabricIndex, csrSpan2, csrPublicKey1);
    EXPECT_EQ(psaOpKeyStore.MigrateOpKeypairForFabric(kFabricIndex, persistentOpKeyStore), CHIP_NO_ERROR);
    EXPECT_EQ(storage.GetNumKeys(), 0u);
    EXPECT_FALSE(storage.HasKey(DefaultStorageKeyAllocator::FabricOpKey(kFabricIndex).KeyName()));

    // Finalize
    persistentOpKeyStore.Finish();
}

TEST_F(TestPSAOpKeyStore, TestKeyAllocation)
{
    const psa_key_id_t kBaseTestKeyId    = 256;
    const psa_key_id_t kICDBaseTestKeyId = 1024;

    class TestKeyAllocator : public PSAKeyAllocator
    {
    public:
        psa_key_id_t GetDacKeyId() override
        {
            // Return a constant number 256 in this test case
            return 256;
        }
        psa_key_id_t GetOpKeyId(FabricIndex fabricIndex) override
        {
            // Return a number 256 + fabricIndex in this test case to have a different value than in DefaultPSAKeyAllocator
            return 256 + fabricIndex;
        }
        psa_key_id_t AllocateICDKeyId() override
        {
            psa_key_id_t newKeyId = PSA_KEY_ID_NULL;
            if (CHIP_NO_ERROR != Crypto::FindFreeKeySlotInRange(newKeyId, kICDBaseTestKeyId, kMaxICDClientKeys))
            {
                newKeyId = PSA_KEY_ID_NULL;
            }
            return newKeyId;
        }
        void UpdateKeyAttributes(psa_key_attributes_t & attrs) override
        {
            psa_set_key_type(&attrs, PSA_KEY_TYPE_AES);
            psa_set_key_bits(&attrs, kP256_PrivateKey_Length);
            psa_set_key_algorithm(&attrs, PSA_ALG_AEAD_WITH_AT_LEAST_THIS_LENGTH_TAG(PSA_ALG_CCM, 8));
            psa_set_key_usage_flags(&attrs, PSA_KEY_USAGE_COPY);
            psa_set_key_lifetime(&attrs, PSA_KEY_LIFETIME_VOLATILE);
        }
    };

    // Create a key with the following attributes:
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
    psa_set_key_bits(&attributes, kP256_PrivateKey_Length * 8);
    psa_set_key_algorithm(&attributes, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_MESSAGE);
    psa_set_key_lifetime(&attributes, PSA_KEY_LIFETIME_PERSISTENT);
    psa_set_key_id(&attributes, GetPSAKeyAllocator().GetOpKeyId(1));

    // Check if the default key allocator returns the expected values
    EXPECT_EQ(GetPSAKeyAllocator().GetDacKeyId(), to_underlying(KeyIdBase::DACPrivKey));
    EXPECT_EQ(GetPSAKeyAllocator().GetOpKeyId(1), to_underlying(KeyIdBase::Operational) + 1);
    EXPECT_EQ(GetPSAKeyAllocator().GetOpKeyId(255), to_underlying(KeyIdBase::Operational) + 255);
    EXPECT_NE(GetPSAKeyAllocator().AllocateICDKeyId(), PSA_KEY_ID_NULL);

    // Check whether attributes are not changed after using the default Allocator
    GetPSAKeyAllocator().UpdateKeyAttributes(attributes);

    EXPECT_EQ(psa_get_key_type(&attributes), PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
    EXPECT_EQ(psa_get_key_bits(&attributes), kP256_PrivateKey_Length * 8);
    EXPECT_EQ(psa_get_key_algorithm(&attributes), PSA_ALG_ECDSA(PSA_ALG_SHA_256));
    EXPECT_EQ(psa_get_key_usage_flags(&attributes), PSA_KEY_USAGE_SIGN_MESSAGE);
    EXPECT_EQ(psa_get_key_lifetime(&attributes), PSA_KEY_LIFETIME_PERSISTENT);

    // Set the new testing Key Allocator and check if it returns the expected values, different than the default ones.
    static TestKeyAllocator testKeyAllocator;
    SetPSAKeyAllocator(&testKeyAllocator);

    EXPECT_EQ(GetPSAKeyAllocator().GetDacKeyId(), kBaseTestKeyId);
    EXPECT_EQ(GetPSAKeyAllocator().GetOpKeyId(1), kBaseTestKeyId + 1);
    EXPECT_EQ(GetPSAKeyAllocator().GetOpKeyId(255), kBaseTestKeyId + 255);
    EXPECT_EQ(GetPSAKeyAllocator().AllocateICDKeyId(), kICDBaseTestKeyId);
    EXPECT_NE(GetPSAKeyAllocator().AllocateICDKeyId(), PSA_KEY_ID_NULL);

    // Test changing the key attributes

    // Use the KeyAllocator to update the key attributes
    GetPSAKeyAllocator().UpdateKeyAttributes(attributes);

    // Check if the attributes were changed
    EXPECT_EQ(psa_get_key_type(&attributes), PSA_KEY_TYPE_AES);
    EXPECT_EQ(psa_get_key_bits(&attributes), kP256_PrivateKey_Length);
    EXPECT_EQ(psa_get_key_algorithm(&attributes), PSA_ALG_AEAD_WITH_AT_LEAST_THIS_LENGTH_TAG(PSA_ALG_CCM, 8));
    EXPECT_EQ(psa_get_key_usage_flags(&attributes), PSA_KEY_USAGE_COPY);
    EXPECT_EQ(psa_get_key_lifetime(&attributes), PSA_KEY_LIFETIME_VOLATILE);

    // Go back to the default Key Allocator and check if the values are still good
    SetPSAKeyAllocator(nullptr);

    // Check if the default key allocator returns the expected values
    EXPECT_EQ(GetPSAKeyAllocator().GetDacKeyId(), to_underlying(KeyIdBase::DACPrivKey));
    EXPECT_EQ(GetPSAKeyAllocator().GetOpKeyId(1), to_underlying(KeyIdBase::Operational) + 1);
    EXPECT_EQ(GetPSAKeyAllocator().GetOpKeyId(255), to_underlying(KeyIdBase::Operational) + 255);
    EXPECT_NE(GetPSAKeyAllocator().AllocateICDKeyId(), PSA_KEY_ID_NULL);

    // Check whether attributes are not changed after using the default Allocator
    GetPSAKeyAllocator().UpdateKeyAttributes(attributes);

    // Attributes should be still the same as previously
    EXPECT_EQ(psa_get_key_type(&attributes), PSA_KEY_TYPE_AES);
    EXPECT_EQ(psa_get_key_bits(&attributes), kP256_PrivateKey_Length);
    EXPECT_EQ(psa_get_key_algorithm(&attributes), PSA_ALG_AEAD_WITH_AT_LEAST_THIS_LENGTH_TAG(PSA_ALG_CCM, 8));
    EXPECT_EQ(psa_get_key_usage_flags(&attributes), PSA_KEY_USAGE_COPY);
    EXPECT_EQ(psa_get_key_lifetime(&attributes), PSA_KEY_LIFETIME_VOLATILE);
}

} // namespace
