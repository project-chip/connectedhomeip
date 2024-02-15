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

#include <crypto/PSAOperationalKeystore.h>
#include <crypto/PersistentStorageOperationalKeystore.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/Span.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/UnitTestExtendedAssertions.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

using namespace chip;
using namespace chip::Crypto;

namespace {

void TestBasicLifeCycle(nlTestSuite * inSuite, void * inContext)
{
    PSAOperationalKeystore opKeystore;

    FabricIndex kFabricIndex    = 111;
    FabricIndex kBadFabricIndex = static_cast<FabricIndex>(kFabricIndex + 10u);

    // Can generate a key and get a CSR
    uint8_t csrBuf[kMIN_CSR_Buffer_Size];
    MutableByteSpan csrSpan{ csrBuf };
    CHIP_ERROR err = opKeystore.NewOpKeypairForFabric(kFabricIndex, csrSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, opKeystore.HasPendingOpKeypair() == true);
    NL_TEST_ASSERT(inSuite, opKeystore.HasOpKeypairForFabric(kFabricIndex) == false);

    P256PublicKey csrPublicKey1;
    err = VerifyCertificateSigningRequest(csrSpan.data(), csrSpan.size(), csrPublicKey1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Can regenerate a second CSR and it has different PK
    csrSpan = MutableByteSpan{ csrBuf };
    err     = opKeystore.NewOpKeypairForFabric(kFabricIndex, csrSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, opKeystore.HasPendingOpKeypair() == true);

    P256PublicKey csrPublicKey2;
    err = VerifyCertificateSigningRequest(csrSpan.data(), csrSpan.size(), csrPublicKey2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !csrPublicKey1.Matches(csrPublicKey2));

    // Cannot NewOpKeypair for a different fabric if one already pending
    uint8_t badCsrBuf[kMIN_CSR_Buffer_Size];
    MutableByteSpan badCsrSpan{ badCsrBuf };
    err = opKeystore.NewOpKeypairForFabric(kBadFabricIndex, badCsrSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_FABRIC_INDEX);
    NL_TEST_ASSERT(inSuite, opKeystore.HasPendingOpKeypair() == true);

    // Fail to generate CSR for invalid fabrics
    csrSpan = MutableByteSpan{ csrBuf };
    err     = opKeystore.NewOpKeypairForFabric(kUndefinedFabricIndex, csrSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_FABRIC_INDEX);

    csrSpan = MutableByteSpan{ csrBuf };
    err     = opKeystore.NewOpKeypairForFabric(kMaxValidFabricIndex + 1, csrSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_FABRIC_INDEX);

    // No storage done by NewOpKeypairForFabric
    NL_TEST_ASSERT(inSuite, opKeystore.HasOpKeypairForFabric(kFabricIndex) == false);

    // Even after error, the previous valid pending keypair stays valid.
    NL_TEST_ASSERT(inSuite, opKeystore.HasPendingOpKeypair() == true);

    // Activating with mismatching fabricIndex and matching public key fails
    err = opKeystore.ActivateOpKeypairForFabric(kBadFabricIndex, csrPublicKey2);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_FABRIC_INDEX);
    NL_TEST_ASSERT(inSuite, opKeystore.HasPendingOpKeypair() == true);
    NL_TEST_ASSERT(inSuite, opKeystore.HasOpKeypairForFabric(kFabricIndex) == false);

    // Activating with matching fabricIndex and mismatching public key fails
    err = opKeystore.ActivateOpKeypairForFabric(kFabricIndex, csrPublicKey1);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_PUBLIC_KEY);
    NL_TEST_ASSERT(inSuite, opKeystore.HasPendingOpKeypair() == true);
    NL_TEST_ASSERT(inSuite, opKeystore.HasOpKeypairForFabric(kFabricIndex) == false);

    // Before successful activation, cannot sign
    uint8_t message[] = { 1, 2, 3, 4 };
    P256ECDSASignature sig1;
    err = opKeystore.SignWithOpKeypair(kFabricIndex, ByteSpan{ message }, sig1);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_FABRIC_INDEX);

    // Activating with matching fabricIndex and matching public key succeeds
    err = opKeystore.ActivateOpKeypairForFabric(kFabricIndex, csrPublicKey2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Activating does not store, and keeps pending
    NL_TEST_ASSERT(inSuite, opKeystore.HasPendingOpKeypair() == true);
    NL_TEST_ASSERT(inSuite, opKeystore.HasOpKeypairForFabric(kFabricIndex) == true);
    NL_TEST_ASSERT(inSuite, opKeystore.HasOpKeypairForFabric(kBadFabricIndex) == false);

    // Can't sign for wrong fabric after activation
    P256ECDSASignature sig2;
    err = opKeystore.SignWithOpKeypair(kBadFabricIndex, ByteSpan{ message }, sig2);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_FABRIC_INDEX);

    // Can sign after activation
    err = opKeystore.SignWithOpKeypair(kFabricIndex, ByteSpan{ message }, sig2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Signature matches pending key
    err = csrPublicKey2.ECDSA_validate_msg_signature(message, sizeof(message), sig2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Signature does not match a previous pending key
    err = csrPublicKey1.ECDSA_validate_msg_signature(message, sizeof(message), sig2);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_SIGNATURE);

    // Committing with mismatching fabric fails, leaves pending
    err = opKeystore.CommitOpKeypairForFabric(kBadFabricIndex);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_FABRIC_INDEX);
    NL_TEST_ASSERT(inSuite, opKeystore.HasPendingOpKeypair() == true);
    NL_TEST_ASSERT(inSuite, opKeystore.HasOpKeypairForFabric(kFabricIndex) == true);

    // Committing key resets pending state
    err = opKeystore.CommitOpKeypairForFabric(kFabricIndex);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, opKeystore.HasPendingOpKeypair() == false);
    NL_TEST_ASSERT(inSuite, opKeystore.HasOpKeypairForFabric(kFabricIndex) == true);

    // Verify if the key is not exportable - the PSA_KEY_USAGE_EXPORT psa flag should not be set
    P256SerializedKeypair serializedKeypair;
    NL_TEST_ASSERT(inSuite,
                   opKeystore.ExportOpKeypairForFabric(kFabricIndex, serializedKeypair) == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    // After committing, signing works with the key that was pending
    P256ECDSASignature sig3;
    uint8_t message2[] = { 10, 11, 12, 13 };
    err                = opKeystore.SignWithOpKeypair(kFabricIndex, ByteSpan{ message2 }, sig3);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = csrPublicKey2.ECDSA_validate_msg_signature(message2, sizeof(message2), sig3);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Let's remove the opkey for a fabric, it disappears
    err = opKeystore.RemoveOpKeypairForFabric(kFabricIndex);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, opKeystore.HasPendingOpKeypair() == false);
    NL_TEST_ASSERT(inSuite, opKeystore.HasOpKeypairForFabric(kFabricIndex) == false);
}

void TestEphemeralKeys(nlTestSuite * inSuite, void * inContext)
{
    PSAOperationalKeystore opKeyStore;

    Crypto::P256ECDSASignature sig;
    uint8_t message[] = { 'm', 's', 'g' };

    Crypto::P256Keypair * ephemeralKeypair = opKeyStore.AllocateEphemeralKeypairForCASE();
    NL_TEST_ASSERT(inSuite, ephemeralKeypair != nullptr);
    NL_TEST_ASSERT_SUCCESS(inSuite, ephemeralKeypair->Initialize(Crypto::ECPKeyTarget::ECDSA));

    NL_TEST_ASSERT_SUCCESS(inSuite, ephemeralKeypair->ECDSA_sign_msg(message, sizeof(message), sig));
    NL_TEST_ASSERT_SUCCESS(inSuite, ephemeralKeypair->Pubkey().ECDSA_validate_msg_signature(message, sizeof(message), sig));

    opKeyStore.ReleaseEphemeralKeypair(ephemeralKeypair);
}

void TestMigrationKeys(nlTestSuite * inSuite, void * inContext)
{
    chip::TestPersistentStorageDelegate storage;
    PSAOperationalKeystore psaOpKeyStore;
    PersistentStorageOperationalKeystore persistentOpKeyStore;
    constexpr FabricIndex kFabricIndex = 111;

    // Failure before Init of MoveOpKeysFromPersistentStorageToITS
    NL_TEST_ASSERT(inSuite,
                   psaOpKeyStore.MigrateOpKeypairForFabric(kFabricIndex, persistentOpKeyStore) == CHIP_ERROR_INCORRECT_STATE);

    // Initialize both operational key stores
    NL_TEST_ASSERT(inSuite, persistentOpKeyStore.Init(&storage) == CHIP_NO_ERROR);

    // Failure on invalid Fabric indexes
    NL_TEST_ASSERT(inSuite,
                   psaOpKeyStore.MigrateOpKeypairForFabric(kUndefinedFabricIndex, persistentOpKeyStore) ==
                       CHIP_ERROR_INVALID_FABRIC_INDEX);
    NL_TEST_ASSERT(inSuite,
                   psaOpKeyStore.MigrateOpKeypairForFabric(kMaxValidFabricIndex + 1, persistentOpKeyStore) ==
                       CHIP_ERROR_INVALID_FABRIC_INDEX);

    // Failure on the key migration, while the key does not exist in the any keystore.
    NL_TEST_ASSERT(inSuite, storage.GetNumKeys() == 0);
    NL_TEST_ASSERT(inSuite, storage.HasKey(DefaultStorageKeyAllocator::FabricOpKey(kFabricIndex).KeyName()) == false);
    NL_TEST_ASSERT(inSuite, psaOpKeyStore.HasOpKeypairForFabric(kFabricIndex) == false);
    NL_TEST_ASSERT(inSuite,
                   psaOpKeyStore.MigrateOpKeypairForFabric(kFabricIndex, persistentOpKeyStore) ==
                       CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    auto generateAndStore = [&](FabricIndex index, MutableByteSpan & buf, P256PublicKey & pubKey) {
        NL_TEST_ASSERT(inSuite, persistentOpKeyStore.HasPendingOpKeypair() == false);
        NL_TEST_ASSERT(inSuite, persistentOpKeyStore.NewOpKeypairForFabric(kFabricIndex, buf) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, VerifyCertificateSigningRequest(buf.data(), buf.size(), pubKey) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, persistentOpKeyStore.HasPendingOpKeypair() == true);
        NL_TEST_ASSERT(inSuite, persistentOpKeyStore.ActivateOpKeypairForFabric(kFabricIndex, pubKey) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, storage.GetNumKeys() == 0);
        NL_TEST_ASSERT(inSuite, persistentOpKeyStore.CommitOpKeypairForFabric(kFabricIndex) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, persistentOpKeyStore.HasPendingOpKeypair() == false);
        NL_TEST_ASSERT(inSuite, storage.GetNumKeys() == 1);
        NL_TEST_ASSERT(inSuite, storage.HasKey(DefaultStorageKeyAllocator::FabricOpKey(kFabricIndex).KeyName()) == true);
    };

    // Save a key to the old persistent storage
    uint8_t csrBuf[kMIN_CSR_Buffer_Size];
    MutableByteSpan csrSpan{ csrBuf };
    P256PublicKey csrPublicKey1;
    generateAndStore(kFabricIndex, csrSpan, csrPublicKey1);

    // Migrate key to PSA ITS
    NL_TEST_ASSERT(inSuite, psaOpKeyStore.MigrateOpKeypairForFabric(kFabricIndex, persistentOpKeyStore) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, psaOpKeyStore.HasOpKeypairForFabric(kFabricIndex) == true);

    // Verify the migrated keys
    P256ECDSASignature sig1;
    uint8_t message1[] = { 10, 11, 12, 13 };
    NL_TEST_ASSERT(inSuite, psaOpKeyStore.SignWithOpKeypair(kFabricIndex, ByteSpan{ message1 }, sig1) == CHIP_NO_ERROR);

    // To verify use the public key generated by the old persistent storage
    NL_TEST_ASSERT(inSuite, csrPublicKey1.ECDSA_validate_msg_signature(message1, sizeof(message1), sig1) == CHIP_NO_ERROR);

    // After migration there should be no old keys anymore
    NL_TEST_ASSERT(inSuite, storage.GetNumKeys() == 0);
    NL_TEST_ASSERT(inSuite, storage.HasKey(DefaultStorageKeyAllocator::FabricOpKey(kFabricIndex).KeyName()) == false);

    // Verify that migration method returns success when there is no OpKey stored in the old keystore, but already exists in PSA
    // ITS.
    NL_TEST_ASSERT(inSuite, psaOpKeyStore.MigrateOpKeypairForFabric(kFabricIndex, persistentOpKeyStore) == CHIP_NO_ERROR);

    // The key already exists in ITS, but there is an another attempt to migrate the new key.
    // The key should not be overwritten, but the key from the previous persistent keystore should be removed.
    MutableByteSpan csrSpan2{ csrBuf };
    generateAndStore(kFabricIndex, csrSpan2, csrPublicKey1);
    NL_TEST_ASSERT(inSuite, psaOpKeyStore.MigrateOpKeypairForFabric(kFabricIndex, persistentOpKeyStore) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, storage.GetNumKeys() == 0);
    NL_TEST_ASSERT(inSuite, storage.HasKey(DefaultStorageKeyAllocator::FabricOpKey(kFabricIndex).KeyName()) == false);

    // Finalize
    persistentOpKeyStore.Finish();
}

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = {
    NL_TEST_DEF("Test Basic Lifecycle of PersistentStorageOperationalKeystore", TestBasicLifeCycle),
    NL_TEST_DEF("Test ephemeral key management", TestEphemeralKeys),
    NL_TEST_DEF("Test keys migration to ITS", TestMigrationKeys),
    NL_TEST_SENTINEL(),
};

/**
 *  Set up the test suite.
 */
int Test_Setup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    VerifyOrReturnError(error == CHIP_NO_ERROR, FAILURE);

#if CHIP_CRYPTO_PSA
    psa_crypto_init();
#endif

    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
int Test_Teardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

} // namespace

/**
 *  Main
 */
int TestPSAOperationalKeystore()
{
    nlTestSuite theSuite = { "PSAOperationalKeystore tests", &sTests[0], Test_Setup, Test_Teardown };

    // Run test suite againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestPSAOperationalKeystore)
