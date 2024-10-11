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
#include <string>

#include <pw_unit_test/framework.h>

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

/**
 * Implementation of OperationalKeystore for testing purposes.
 *
 * Not all methods of OperationalKeystore are implemented, only the currently needed.
 * Currently this implementation supports only one fabric and one operational key.
 *
 * The Validate method can be used to validate the provided P256SerializedKeypair with the
 * stored kP256SerializedKeypairRaw data.
 */
class TestOperationalKeystore final : public Crypto::OperationalKeystore
{
public:
    constexpr static uint8_t kP256SerializedKeypairRaw[] = {
        0x4,  0xd0, 0x99, 0xde, 0xd1, 0x15, 0xea, 0xcf, 0x8f, 0x13, 0xde, 0xaf, 0x74, 0x65, 0xf3, 0x10, 0x3a, 0x75, 0x94, 0x51,
        0x37, 0x3c, 0xc,  0x9a, 0x25, 0xc7, 0xad, 0xb4, 0x31, 0x39, 0x62, 0xec, 0x12, 0xa3, 0xdf, 0x28, 0x5f, 0x2c, 0x86, 0x47,
        0x2d, 0x1f, 0x5d, 0x45, 0x1d, 0x9f, 0xbc, 0xe8, 0x47, 0xf2, 0x1f, 0x40, 0x17, 0x61, 0x2b, 0x9a, 0x4e, 0x68, 0x9c, 0xe9,
        0x9e, 0xb7, 0x45, 0xdc, 0xcd, 0xb,  0x90, 0xd0, 0x24, 0xa5, 0x6d, 0x64, 0x97, 0x62, 0x75, 0x42, 0x91, 0x74, 0xfc, 0xfe,
        0xcb, 0x1,  0x6c, 0xc,  0x74, 0x6f, 0x39, 0x9f, 0x5,  0x96, 0x1b, 0xe6, 0x4a, 0x97, 0xe5, 0x84, 0x72
    };

    bool HasOpKeypairForFabric(FabricIndex fabricIndex) const override { return fabricIndex == mUsedFabricIndex; };

    CHIP_ERROR NewOpKeypairForFabric(FabricIndex fabricIndex, MutableByteSpan & outCertificateSigningRequest) override
    {
        // Only one Fabric is supported
        if (mUsedFabricIndex != 0)
        {
            ChipLogError(Test,
                         "The TestOperationalKeystore has been initialized already, please use RemoveOpKeypairForFabric or remove "
                         "the object to store a new fabric");
            return CHIP_ERROR_INVALID_FABRIC_INDEX;
        }
        mUsedFabricIndex = fabricIndex;
        (void) outCertificateSigningRequest;

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ExportOpKeypairForFabric(FabricIndex fabricIndex, Crypto::P256SerializedKeypair & outKeypair) override
    {
        VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

        // Simulate not existing value while the fabric index is valid.
        if (fabricIndex != mUsedFabricIndex)
        {
            return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
        }

        if (outKeypair.Capacity() != sizeof(kP256SerializedKeypairRaw))
        {
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }

        memcpy(outKeypair.Bytes(), kP256SerializedKeypairRaw, outKeypair.Capacity());
        outKeypair.SetLength(sizeof(kP256SerializedKeypairRaw));

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR RemoveOpKeypairForFabric(FabricIndex fabricIndex) override
    {
        mUsedFabricIndex = 0;
        return CHIP_NO_ERROR;
    }

    bool ValidateKeypair(Crypto::P256SerializedKeypair & keypair)
    {
        return (keypair.Length() == sizeof(kP256SerializedKeypairRaw) &&
                memcmp(keypair.ConstBytes(), kP256SerializedKeypairRaw, keypair.Length()) == 0);
    }

    // Not implemented methods, they are not used in any tests yet.
    bool HasPendingOpKeypair() const override { return false; }
    CHIP_ERROR ActivateOpKeypairForFabric(FabricIndex fabricIndex, const Crypto::P256PublicKey & nocPublicKey) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    CHIP_ERROR CommitOpKeypairForFabric(FabricIndex fabricIndex) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR MigrateOpKeypairForFabric(FabricIndex fabricIndex, OperationalKeystore & operationalKeystore) const override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    void RevertPendingKeypair() override {}
    CHIP_ERROR SignWithOpKeypair(FabricIndex fabricIndex, const ByteSpan & message,
                                 Crypto::P256ECDSASignature & outSignature) const override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    Crypto::P256Keypair * AllocateEphemeralKeypairForCASE() override { return nullptr; }
    void ReleaseEphemeralKeypair(Crypto::P256Keypair * keypair) override {}

private:
    FabricIndex mUsedFabricIndex = 0;
};

struct TestPersistentStorageOpKeyStore : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestPersistentStorageOpKeyStore, TestBasicLifeCycle)
{
    TestPersistentStorageDelegate storageDelegate;
    PersistentStorageOperationalKeystore opKeystore;

    FabricIndex kFabricIndex    = 111;
    FabricIndex kBadFabricIndex = static_cast<FabricIndex>(kFabricIndex + 10u);

    // Failure before Init of ActivateOpKeypairForFabric
    P256PublicKey placeHolderPublicKey;
    CHIP_ERROR err = opKeystore.ActivateOpKeypairForFabric(kFabricIndex, placeHolderPublicKey);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(storageDelegate.GetNumKeys(), 0u);

    // Failure before Init of NewOpKeypairForFabric
    uint8_t unusedCsrBuf[kMIN_CSR_Buffer_Size];
    MutableByteSpan unusedCsrSpan{ unusedCsrBuf };
    err = opKeystore.NewOpKeypairForFabric(kFabricIndex, unusedCsrSpan);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);

    // Failure before Init of CommitOpKeypairForFabric
    err = opKeystore.CommitOpKeypairForFabric(kFabricIndex);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);

    // Failure before Init of RemoveOpKeypairForFabric
    err = opKeystore.RemoveOpKeypairForFabric(kFabricIndex);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);

    // Success after Init
    err = opKeystore.Init(&storageDelegate);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Can generate a key and get a CSR
    uint8_t csrBuf[kMIN_CSR_Buffer_Size];
    MutableByteSpan csrSpan{ csrBuf };
    err = opKeystore.NewOpKeypairForFabric(kFabricIndex, csrSpan);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(opKeystore.HasPendingOpKeypair());
    EXPECT_FALSE(opKeystore.HasOpKeypairForFabric(kFabricIndex));

    P256PublicKey csrPublicKey1;
    err = VerifyCertificateSigningRequest(csrSpan.data(), csrSpan.size(), csrPublicKey1);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(csrPublicKey1.Matches(csrPublicKey1));

    // Can regenerate a second CSR and it has different PK
    csrSpan = MutableByteSpan{ csrBuf };
    err     = opKeystore.NewOpKeypairForFabric(kFabricIndex, csrSpan);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(opKeystore.HasPendingOpKeypair());

    // Cannot NewOpKeypair for a different fabric if one already pending
    uint8_t badCsrBuf[kMIN_CSR_Buffer_Size];
    MutableByteSpan badCsrSpan = MutableByteSpan{ badCsrBuf };
    err                        = opKeystore.NewOpKeypairForFabric(kBadFabricIndex, badCsrSpan);
    EXPECT_EQ(err, CHIP_ERROR_INVALID_FABRIC_INDEX);
    EXPECT_TRUE(opKeystore.HasPendingOpKeypair());

    P256PublicKey csrPublicKey2;
    err = VerifyCertificateSigningRequest(csrSpan.data(), csrSpan.size(), csrPublicKey2);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_FALSE(csrPublicKey1.Matches(csrPublicKey2));

    // Fail to generate CSR for invalid fabrics
    csrSpan = MutableByteSpan{ csrBuf };
    err     = opKeystore.NewOpKeypairForFabric(kUndefinedFabricIndex, csrSpan);
    EXPECT_EQ(err, CHIP_ERROR_INVALID_FABRIC_INDEX);

    csrSpan = MutableByteSpan{ csrBuf };
    err     = opKeystore.NewOpKeypairForFabric(kMaxValidFabricIndex + 1, csrSpan);
    EXPECT_EQ(err, CHIP_ERROR_INVALID_FABRIC_INDEX);

    // No storage done by NewOpKeypairForFabric
    EXPECT_EQ(storageDelegate.GetNumKeys(), 0u);
    EXPECT_FALSE(opKeystore.HasOpKeypairForFabric(kFabricIndex));

    // Even after error, the previous valid pending keypair stays valid.
    EXPECT_TRUE(opKeystore.HasPendingOpKeypair());

    // Activating with mismatching fabricIndex and matching public key fails
    err = opKeystore.ActivateOpKeypairForFabric(kBadFabricIndex, csrPublicKey2);
    EXPECT_EQ(err, CHIP_ERROR_INVALID_FABRIC_INDEX);
    EXPECT_EQ(storageDelegate.GetNumKeys(), 0u);
    EXPECT_TRUE(opKeystore.HasPendingOpKeypair());
    EXPECT_FALSE(opKeystore.HasOpKeypairForFabric(kFabricIndex));

    // Activating with matching fabricIndex and mismatching public key fails
    err = opKeystore.ActivateOpKeypairForFabric(kFabricIndex, csrPublicKey1);
    EXPECT_EQ(err, CHIP_ERROR_INVALID_PUBLIC_KEY);
    EXPECT_EQ(storageDelegate.GetNumKeys(), 0u);
    EXPECT_TRUE(opKeystore.HasPendingOpKeypair());
    EXPECT_FALSE(opKeystore.HasOpKeypairForFabric(kFabricIndex));

    uint8_t message[] = { 1, 2, 3, 4 };
    P256ECDSASignature sig1;
    // Before successful activation, cannot sign
    err = opKeystore.SignWithOpKeypair(kFabricIndex, ByteSpan{ message }, sig1);
    EXPECT_EQ(err, CHIP_ERROR_INVALID_FABRIC_INDEX);

    // Activating with matching fabricIndex and matching public key succeeds
    err = opKeystore.ActivateOpKeypairForFabric(kFabricIndex, csrPublicKey2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Activating does not store, and keeps pending
    EXPECT_EQ(storageDelegate.GetNumKeys(), 0u);
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
    EXPECT_EQ(storageDelegate.GetNumKeys(), 0u);
    EXPECT_TRUE(opKeystore.HasPendingOpKeypair());
    EXPECT_TRUE(opKeystore.HasOpKeypairForFabric(kFabricIndex));

    // Committing key resets pending state and adds storage
    std::string opKeyStorageKey = DefaultStorageKeyAllocator::FabricOpKey(kFabricIndex).KeyName();
    err                         = opKeystore.CommitOpKeypairForFabric(kFabricIndex);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_FALSE(opKeystore.HasPendingOpKeypair());
    EXPECT_EQ(storageDelegate.GetNumKeys(), 1u);
    EXPECT_TRUE(storageDelegate.HasKey(opKeyStorageKey));

    // Exporting a key
    P256SerializedKeypair serializedKeypair;
    EXPECT_EQ(opKeystore.ExportOpKeypairForFabric(kFabricIndex, serializedKeypair), CHIP_NO_ERROR);

    // Exporting a key from the bad fabric index
    EXPECT_EQ(opKeystore.ExportOpKeypairForFabric(kBadFabricIndex, serializedKeypair),
              CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

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
    EXPECT_EQ(storageDelegate.GetNumKeys(), 0u);
    EXPECT_FALSE(storageDelegate.HasKey(opKeyStorageKey));

    opKeystore.Finish();
}

TEST_F(TestPersistentStorageOpKeyStore, TestEphemeralKeys)
{
    chip::TestPersistentStorageDelegate storage;

    PersistentStorageOperationalKeystore opKeyStore;
    EXPECT_EQ(opKeyStore.Init(&storage), CHIP_NO_ERROR);

    Crypto::P256ECDSASignature sig;
    uint8_t message[] = { 'm', 's', 'g' };

    Crypto::P256Keypair * ephemeralKeypair = opKeyStore.AllocateEphemeralKeypairForCASE();
    ASSERT_NE(ephemeralKeypair, nullptr);
    EXPECT_EQ(ephemeralKeypair->Initialize(Crypto::ECPKeyTarget::ECDSA), CHIP_NO_ERROR);

    EXPECT_EQ(ephemeralKeypair->ECDSA_sign_msg(message, sizeof(message), sig), CHIP_NO_ERROR);
    EXPECT_EQ(ephemeralKeypair->Pubkey().ECDSA_validate_msg_signature(message, sizeof(message), sig), CHIP_NO_ERROR);

    opKeyStore.ReleaseEphemeralKeypair(ephemeralKeypair);

    opKeyStore.Finish();
}

TEST_F(TestPersistentStorageOpKeyStore, TestMigrationKeys)
{

    chip::TestPersistentStorageDelegate storageDelegate;
    TestOperationalKeystore testOperationalKeystore;
    PersistentStorageOperationalKeystore opKeyStore;
    FabricIndex kFabricIndex    = 111;
    std::string opKeyStorageKey = DefaultStorageKeyAllocator::FabricOpKey(kFabricIndex).KeyName();

    opKeyStore.Init(&storageDelegate);

    // Failure on invalid Fabric indexes
    EXPECT_EQ(opKeyStore.MigrateOpKeypairForFabric(kUndefinedFabricIndex, testOperationalKeystore),
              CHIP_ERROR_INVALID_FABRIC_INDEX);
    EXPECT_EQ(opKeyStore.MigrateOpKeypairForFabric(kMaxValidFabricIndex + 1, testOperationalKeystore),
              CHIP_ERROR_INVALID_FABRIC_INDEX);

    // The key does not exists in the any of the Operational Keystores
    EXPECT_FALSE(storageDelegate.HasKey(opKeyStorageKey));
    EXPECT_FALSE(testOperationalKeystore.HasOpKeypairForFabric(kFabricIndex));
    EXPECT_EQ(opKeyStore.MigrateOpKeypairForFabric(kFabricIndex, testOperationalKeystore),
              CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    // Create a key in the old Operational Keystore
    uint8_t csrBuf[kMIN_CSR_Buffer_Size];
    MutableByteSpan csrSpan{ csrBuf };
    EXPECT_EQ(testOperationalKeystore.NewOpKeypairForFabric(kFabricIndex, csrSpan), CHIP_NO_ERROR);

    // Migrate the key to the PersistentStorageOperationalKeystore
    EXPECT_EQ(opKeyStore.MigrateOpKeypairForFabric(kFabricIndex, testOperationalKeystore), CHIP_NO_ERROR);
    EXPECT_EQ(storageDelegate.GetNumKeys(), 1u);
    EXPECT_TRUE(storageDelegate.HasKey(opKeyStorageKey));
    EXPECT_FALSE(testOperationalKeystore.HasOpKeypairForFabric(kFabricIndex));

    // Verify the migration
    P256SerializedKeypair serializedKeypair;
    EXPECT_EQ(opKeyStore.ExportOpKeypairForFabric(kFabricIndex, serializedKeypair), CHIP_NO_ERROR);
    EXPECT_TRUE(testOperationalKeystore.ValidateKeypair(serializedKeypair));

    // Verify that migration method returns success when there is no OpKey stored in the old keystore, but already exists in PSA
    // ITS.
    EXPECT_EQ(opKeyStore.MigrateOpKeypairForFabric(kFabricIndex, testOperationalKeystore), CHIP_NO_ERROR);

    // The key already exists in ITS, but there is an another attempt to migrate the new key.
    // The key should not be overwritten, but the key from the previous persistent keystore should be removed.
    MutableByteSpan csrSpan2{ csrBuf };
    EXPECT_EQ(testOperationalKeystore.NewOpKeypairForFabric(kFabricIndex, csrSpan2), CHIP_NO_ERROR);
    EXPECT_TRUE(testOperationalKeystore.HasOpKeypairForFabric(kFabricIndex));
    EXPECT_EQ(opKeyStore.MigrateOpKeypairForFabric(kFabricIndex, testOperationalKeystore), CHIP_NO_ERROR);
    EXPECT_FALSE(testOperationalKeystore.HasOpKeypairForFabric(kFabricIndex));

    opKeyStore.Finish();
}

} // namespace
