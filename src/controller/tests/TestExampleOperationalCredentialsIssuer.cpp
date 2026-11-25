/*
 *
 *    Copyright (c) 2020-2025 Project CHIP Authors
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

#include <pw_unit_test/framework.h>

#include <controller/ExampleOperationalCredentialsIssuer.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/PersistentStorageMacros.h>
#include <lib/support/TestGroupData.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/tests/ExtraPwTestMacros.h>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

using namespace chip;
using namespace chip::Controller;

namespace {

static constexpr size_t kNodeId   = 999;
static constexpr size_t kFabricId = 1000;

constexpr char kOperationalCredentialsIssuerKeypairStorage[]           = "ExampleOpCredsCAKey";
constexpr char kOperationalCredentialsRootCertificateStorage[]         = "ExampleCARootCert";
constexpr char kOperationalCredentialsIntermediateCertificateStorage[] = "ExampleCAIntermediateCert";
constexpr char kOperationalCredentialsIpkEpochKeyStorage[]             = "ExampleOpCredsIPK";

bool IsDefaultIpk(Crypto::IdentityProtectionKeySpan ipk)
{
    return ipk.data_equal(GroupTesting::DefaultIpkValue::GetDefaultIpk());
}

bool IsAllZero(Crypto::IdentityProtectionKeySpan ipk)
{
    return std::all_of(ipk.begin(), ipk.end(), [](uint8_t b) { return b == 0; });
}

class ExampleOperationalCredentialsIssuerTest : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

protected:
    NodeId mNodeId{ ::kNodeId };

    ExampleOperationalCredentialsIssuer mCredsIssuer{ ::kFabricId };

    TestPersistentStorageDelegate mPersistentStorageDelegate{};
};

TEST_F(ExampleOperationalCredentialsIssuerTest, SuccessfulyGeneratesDistinctRandomNodeIds)
{
    NodeId node_id_1{};
    NodeId node_id_2{};

    ASSERT_EQ(node_id_1, node_id_2); // should be both 0 since type of NodeId is uint64_t

    EXPECT_SUCCESS(ExampleOperationalCredentialsIssuer::GetRandomOperationalNodeId(&node_id_1));
    EXPECT_SUCCESS(ExampleOperationalCredentialsIssuer::GetRandomOperationalNodeId(&node_id_2));

    ASSERT_NE(node_id_1, node_id_2);
}

TEST_F(ExampleOperationalCredentialsIssuerTest, SuccessfulyGeneratesRandomNodeId)
{
    auto prev_node_id = mNodeId;
    auto r            = ExampleOperationalCredentialsIssuer::GetRandomOperationalNodeId(&mNodeId);

    ASSERT_EQ(prev_node_id, kNodeId);
    ASSERT_EQ(r, CHIP_NO_ERROR);
    ASSERT_NE(mNodeId, kNodeId);
}

TEST_F(ExampleOperationalCredentialsIssuerTest, SuccessfulyGeneratesNOCChainAfterValidation)
{
    Crypto::P256Keypair ephemeralKey;
    auto ephemeral_r = ephemeralKey.Initialize(Crypto::ECPKeyTarget::ECDSA);

    Platform::ScopedMemoryBuffer<uint8_t> noc;
    noc.Calloc(Controller::kMaxCHIPDERCertLength);
    ASSERT_TRUE(noc.Get());

    Platform::ScopedMemoryBuffer<uint8_t> icac;
    icac.Calloc(Controller::kMaxCHIPDERCertLength);
    ASSERT_TRUE(icac.Get());

    Platform::ScopedMemoryBuffer<uint8_t> rcac;
    rcac.Calloc(Controller::kMaxCHIPDERCertLength);
    ASSERT_TRUE(rcac.Get());

    ASSERT_EQ(ephemeral_r, CHIP_NO_ERROR);

    MutableByteSpan nocSpan(noc.Get(), Controller::kMaxCHIPDERCertLength);
    MutableByteSpan icacSpan(icac.Get(), Controller::kMaxCHIPDERCertLength);
    MutableByteSpan rcacSpan(rcac.Get(), Controller::kMaxCHIPDERCertLength);

    EXPECT_SUCCESS(mCredsIssuer.Initialize(mPersistentStorageDelegate));

    auto r = mCredsIssuer.GenerateNOCChainAfterValidation(mNodeId, ::kFabricId, chip::kUndefinedCATs, ephemeralKey.Pubkey(),
                                                          rcacSpan, icacSpan, nocSpan);

    ASSERT_EQ(r, CHIP_NO_ERROR);
}

TEST_F(ExampleOperationalCredentialsIssuerTest, SuccessfulyGeneratesNOCChainAfterValidationWithDataInStorage)
{
    Crypto::P256Keypair ephemeralKey;
    auto ephemeral_r = ephemeralKey.Initialize(Crypto::ECPKeyTarget::ECDSA);

    ASSERT_EQ(ephemeral_r, CHIP_NO_ERROR);

    Platform::ScopedMemoryBuffer<uint8_t> noc;
    noc.Calloc(Controller::kMaxCHIPDERCertLength);
    ASSERT_TRUE(noc.Get());

    MutableByteSpan nocSpan(noc.Get(), Controller::kMaxCHIPDERCertLength);

    std::array<uint8_t, Controller::kMaxCHIPDERCertLength> icacArray{};
    MutableByteSpan icacSpan(icacArray);

    std::array<uint8_t, Controller::kMaxCHIPDERCertLength> rcacArray{};
    MutableByteSpan rcacSpan(rcacArray);
    uint16_t rcacBufLen = static_cast<uint16_t>(std::min(rcacSpan.size(), static_cast<size_t>(UINT16_MAX)));

    CHIP_ERROR err = CHIP_NO_ERROR;
    PERSISTENT_KEY_OP(uint64_t{ 0 }, kOperationalCredentialsRootCertificateStorage, key,
                      err = mPersistentStorageDelegate.SyncSetKeyValue(key, rcacSpan.data(), rcacBufLen));

    ASSERT_EQ(CHIP_NO_ERROR, err);

    PERSISTENT_KEY_OP(uint64_t{ 0 }, kOperationalCredentialsIntermediateCertificateStorage, key,
                      err =
                          mPersistentStorageDelegate.SyncSetKeyValue(key, icacSpan.data(), static_cast<uint16_t>(icacSpan.size())));

    ASSERT_EQ(CHIP_NO_ERROR, err);

    EXPECT_SUCCESS(mCredsIssuer.Initialize(mPersistentStorageDelegate));

    mCredsIssuer.SetMaximallyLargeCertsUsed(true);

    auto r = mCredsIssuer.GenerateNOCChainAfterValidation(mNodeId, ::kFabricId, chip::kUndefinedCATs, ephemeralKey.Pubkey(),
                                                          rcacSpan, icacSpan, nocSpan);

    ASSERT_EQ(r, CHIP_NO_ERROR);
}

TEST_F(ExampleOperationalCredentialsIssuerTest, FreshStorageGetsRandomPersistedIpkEpochKey)
{
    EXPECT_SUCCESS(mCredsIssuer.Initialize(mPersistentStorageDelegate));
    Crypto::IdentityProtectionKeySpan ipk = mCredsIssuer.GetIpkEpochKey();
    EXPECT_FALSE(IsDefaultIpk(ipk));
    EXPECT_FALSE(IsAllZero(ipk));

    ExampleOperationalCredentialsIssuer reloaded{ ::kFabricId };
    EXPECT_SUCCESS(reloaded.Initialize(mPersistentStorageDelegate));
    EXPECT_TRUE(reloaded.GetIpkEpochKey().data_equal(ipk));
}

TEST_F(ExampleOperationalCredentialsIssuerTest, SeparateStoragesGetDistinctIpkEpochKeys)
{
    TestPersistentStorageDelegate otherStorage;
    ExampleOperationalCredentialsIssuer otherIssuer{ ::kFabricId };

    EXPECT_SUCCESS(mCredsIssuer.Initialize(mPersistentStorageDelegate));
    EXPECT_SUCCESS(otherIssuer.Initialize(otherStorage));
    EXPECT_FALSE(mCredsIssuer.GetIpkEpochKey().data_equal(otherIssuer.GetIpkEpochKey()));
}

TEST_F(ExampleOperationalCredentialsIssuerTest, SeparateIndexesGetDistinctIpkEpochKeys)
{
    ExampleOperationalCredentialsIssuer otherIndexIssuer{ ::kFabricId + 1 };

    EXPECT_SUCCESS(mCredsIssuer.Initialize(mPersistentStorageDelegate));
    EXPECT_SUCCESS(otherIndexIssuer.Initialize(mPersistentStorageDelegate));
    EXPECT_FALSE(mCredsIssuer.GetIpkEpochKey().data_equal(otherIndexIssuer.GetIpkEpochKey()));
}

// Storage written before the IPK epoch key was persisted holds a root keypair and no IPK entry.
// Nodes commissioned from it hold the default IPK, so the issuer must keep using it.
TEST_F(ExampleOperationalCredentialsIssuerTest, StorageWithRootKeypairButNoIpkKeepsDefaultIpkEpochKey)
{
    EXPECT_SUCCESS(mCredsIssuer.Initialize(mPersistentStorageDelegate));

    CHIP_ERROR err = CHIP_NO_ERROR;
    PERSISTENT_KEY_OP(uint64_t{ ::kFabricId }, kOperationalCredentialsIpkEpochKeyStorage, key,
                      err = mPersistentStorageDelegate.SyncDeleteKeyValue(key));
    ASSERT_SUCCESS(err);

    ExampleOperationalCredentialsIssuer legacy{ ::kFabricId };
    EXPECT_SUCCESS(legacy.Initialize(mPersistentStorageDelegate));
    EXPECT_TRUE(IsDefaultIpk(legacy.GetIpkEpochKey()));

    ExampleOperationalCredentialsIssuer reloaded{ ::kFabricId };
    EXPECT_SUCCESS(reloaded.Initialize(mPersistentStorageDelegate));
    EXPECT_TRUE(IsDefaultIpk(reloaded.GetIpkEpochKey()));
}

TEST_F(ExampleOperationalCredentialsIssuerTest, WrongSizeStoredIpkEpochKeyFailsInitialize)
{
    const uint8_t shortKey[4] = { 1, 2, 3, 4 };
    CHIP_ERROR err            = CHIP_NO_ERROR;
    PERSISTENT_KEY_OP(uint64_t{ ::kFabricId }, kOperationalCredentialsIpkEpochKeyStorage, key,
                      err = mPersistentStorageDelegate.SyncSetKeyValue(key, shortKey, sizeof(shortKey)));
    ASSERT_SUCCESS(err);

    EXPECT_NE(mCredsIssuer.Initialize(mPersistentStorageDelegate), CHIP_NO_ERROR);
}

// A first Initialize() that fails to save the IPK epoch key must not leave storage that later reads as
// pre-IPK storage, which would select the default IPK.
TEST_F(ExampleOperationalCredentialsIssuerTest, FailedIpkEpochKeySaveDoesNotSelectDefaultIpk)
{
    char ipkStorageKey[sizeof(kOperationalCredentialsIpkEpochKeyStorage) + 2 * sizeof(NodeId) + 1];
    PERSISTENT_KEY_OP(uint64_t{ ::kFabricId }, kOperationalCredentialsIpkEpochKeyStorage, key,
                      Platform::CopyString(ipkStorageKey, key));

    constexpr int kAlwaysSucceed = -1;
    constexpr int kAlwaysFail    = 0;
    mPersistentStorageDelegate.AddPoisonKey(ipkStorageKey, kAlwaysSucceed, kAlwaysFail);
    EXPECT_NE(mCredsIssuer.Initialize(mPersistentStorageDelegate), CHIP_NO_ERROR);

    mPersistentStorageDelegate.ClearPoisonKeys();
    ExampleOperationalCredentialsIssuer retried{ ::kFabricId };
    EXPECT_SUCCESS(retried.Initialize(mPersistentStorageDelegate));
    EXPECT_FALSE(IsDefaultIpk(retried.GetIpkEpochKey()));
}

// A root keypair that exists but cannot be read must not be replaced, and must not get a new IPK epoch key.
TEST_F(ExampleOperationalCredentialsIssuerTest, RootKeypairReadErrorFailsInitialize)
{
    EXPECT_SUCCESS(mCredsIssuer.Initialize(mPersistentStorageDelegate));

    CHIP_ERROR err = CHIP_NO_ERROR;
    PERSISTENT_KEY_OP(uint64_t{ ::kFabricId }, kOperationalCredentialsIpkEpochKeyStorage, key,
                      err = mPersistentStorageDelegate.SyncDeleteKeyValue(key));
    ASSERT_SUCCESS(err);

    char rootKeypairStorageKey[sizeof(kOperationalCredentialsIssuerKeypairStorage) + 2 * sizeof(NodeId) + 1];
    PERSISTENT_KEY_OP(uint64_t{ ::kFabricId }, kOperationalCredentialsIssuerKeypairStorage, key,
                      Platform::CopyString(rootKeypairStorageKey, key));

    constexpr int kAlwaysSucceed = -1;
    constexpr int kAlwaysFail    = 0;
    mPersistentStorageDelegate.AddPoisonKey(rootKeypairStorageKey, kAlwaysFail, kAlwaysSucceed);
    ExampleOperationalCredentialsIssuer failing{ ::kFabricId };
    EXPECT_NE(failing.Initialize(mPersistentStorageDelegate), CHIP_NO_ERROR);

    mPersistentStorageDelegate.ClearPoisonKeys();
    ExampleOperationalCredentialsIssuer recovered{ ::kFabricId };
    EXPECT_SUCCESS(recovered.Initialize(mPersistentStorageDelegate));
    EXPECT_TRUE(IsDefaultIpk(recovered.GetIpkEpochKey()));
}

TEST_F(ExampleOperationalCredentialsIssuerTest, SetIpkEpochKeyOverridesWithoutPersisting)
{
    EXPECT_SUCCESS(mCredsIssuer.Initialize(mPersistentStorageDelegate));
    std::array<uint8_t, Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES> storedIpk;
    memcpy(storedIpk.data(), mCredsIssuer.GetIpkEpochKey().data(), storedIpk.size());

    mCredsIssuer.SetIpkEpochKey(Crypto::IdentityProtectionKeySpan(GroupTesting::DefaultIpkValue::GetDefaultIpk().data()));
    EXPECT_TRUE(IsDefaultIpk(mCredsIssuer.GetIpkEpochKey()));

    ExampleOperationalCredentialsIssuer reloaded{ ::kFabricId };
    EXPECT_SUCCESS(reloaded.Initialize(mPersistentStorageDelegate));
    EXPECT_TRUE(reloaded.GetIpkEpochKey().data_equal(Crypto::IdentityProtectionKeySpan(storedIpk.data())));
}

} // namespace
