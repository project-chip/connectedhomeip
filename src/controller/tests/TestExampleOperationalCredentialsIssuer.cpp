/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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
#include <lib/support/TestPersistentStorageDelegate.h>

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

using namespace chip;
using namespace chip::Controller;

namespace {

static constexpr size_t kNodeId   = 999;
static constexpr size_t kFabricId = 1000;

constexpr char kOperationalCredentialsRootCertificateStorage[]         = "ExampleCARootCert";
constexpr char kOperationalCredentialsIntermediateCertificateStorage[] = "ExampleCAIntermediateCert";

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

TEST_F(ExampleOperationalCredentialsIssuerTest, SuccessfulyGeneratesRandomNodeId)
{
    auto prev_node_id = mNodeId;
    auto r            = ExampleOperationalCredentialsIssuer::GetRandomOperationalNodeId(std::addressof(mNodeId));

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
    auto noc_boolean_result = noc.operator bool();

    Platform::ScopedMemoryBuffer<uint8_t> icac;
    icac.Calloc(Controller::kMaxCHIPDERCertLength);
    auto icac_boolean_result = icac.operator bool();

    Platform::ScopedMemoryBuffer<uint8_t> rcac;
    rcac.Calloc(Controller::kMaxCHIPDERCertLength);
    auto rcac_boolean_result = rcac.operator bool();

    ASSERT_EQ(ephemeral_r, CHIP_NO_ERROR);
    ASSERT_TRUE(noc_boolean_result);
    ASSERT_TRUE(rcac_boolean_result);
    ASSERT_TRUE(icac_boolean_result);

    MutableByteSpan nocSpan(noc.Get(), Controller::kMaxCHIPDERCertLength);
    MutableByteSpan icacSpan(icac.Get(), Controller::kMaxCHIPDERCertLength);
    MutableByteSpan rcacSpan(rcac.Get(), Controller::kMaxCHIPDERCertLength);

    mCredsIssuer.Initialize(mPersistentStorageDelegate);

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
    MutableByteSpan nocSpan(noc.Get(), Controller::kMaxCHIPDERCertLength);

    std::array<uint8_t, Controller::kMaxCHIPDERCertLength> icacArray = { 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
                                                                         0x02, 0x02, 0x02, 0x02, 0x02 };
    MutableByteSpan icacSpan(icacArray);

    std::array<uint8_t, Controller::kMaxCHIPDERCertLength> rcacArray = { 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
                                                                         0x03, 0x03, 0x03, 0x03, 0x03, 0x03 };
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

    mCredsIssuer.Initialize(mPersistentStorageDelegate);

    mCredsIssuer.SetMaximallyLargeCertsUsed(true);

    auto r = mCredsIssuer.GenerateNOCChainAfterValidation(mNodeId, ::kFabricId, chip::kUndefinedCATs, ephemeralKey.Pubkey(),
                                                          rcacSpan, icacSpan, nocSpan);

    ASSERT_EQ(r, CHIP_NO_ERROR);
}

} // namespace
