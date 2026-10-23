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

#include <lib/support/tests/ExtraPwTestMacros.h>
#include <pw_unit_test/framework.h>

#include <app/clusters/network-identity-management-server/NetworkAdministratorSecret.h>
#include <app/clusters/network-identity-management-server/RawKeyNetworkIdentityKeystore.h>
#include <app/clusters/network-identity-management-server/tests/NASS_test_vectors.h>
#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Base64.h>
#include <lib/support/CHIPMem.h>

using namespace chip;
using namespace chip::Crypto;
using namespace chip::Credentials;

struct TestRawKeyNetworkIdentityKeystore : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_SUCCESS(chip::Platform::MemoryInit()); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

// DeriveECDSANetworkIdentity requires deterministic ECDSA
static bool DeriveECDSANetworkIdentitySupported()
{
    static bool supported = []() {
        P256Keypair keypair;
        P256ECDSASignature sig;
        const uint8_t probe = 0;
        RETURN_SAFELY_IGNORED keypair.Initialize(ECPKeyTarget::ECDSA);
        return keypair.ECDSA_sign_msg_det(&probe, 1, sig) != CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }();
    return supported;
}

// Pre-defined raw secrets for tests. Two distinct secrets are sufficient for all current tests.
static const NetworkAdministratorRawSecret kRawSecret1((const uint8_t[]){
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
});
static const NetworkAdministratorRawSecret kRawSecret2((const uint8_t[]){
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
});

// For the raw keystore, Import/Export are just buffer copies, so we can test
// the round-trip without any handle lifecycle management.
TEST_F(TestRawKeyNetworkIdentityKeystore, ImportExportRoundTrip)
{
    RawKeyNetworkIdentityKeystore keystore;
    HkdfKeyHandle handle;
    ASSERT_SUCCESS(keystore.ImportNetworkAdministratorSecret(kRawSecret1, handle));

    NetworkAdministratorRawSecret exported;
    ASSERT_SUCCESS(keystore.ExportNetworkAdministratorSecret(handle, exported));
    EXPECT_TRUE(kRawSecret1.Span().data_equal(exported.Span()));
}

TEST_F(TestRawKeyNetworkIdentityKeystore, DeriveProducesValidIdentity)
{
    if (!DeriveECDSANetworkIdentitySupported())
    {
        GTEST_SKIP() << "Skipping: DeriveECDSANetworkIdentity not supported on this platform";
    }

    RawKeyNetworkIdentityKeystore keystore;
    HkdfKeyHandle nassHandle;
    ASSERT_SUCCESS(keystore.ImportNetworkAdministratorSecret(kRawSecret1, nassHandle));

    P256KeypairHandle keypairHandle;
    uint8_t identityBuf[kMaxCHIPCompactNetworkIdentityLength];
    MutableByteSpan identity(identityBuf);
    ASSERT_SUCCESS(keystore.DeriveECDSANetworkIdentity(nassHandle, keypairHandle, identity));

    // The derived identity must be a valid Network Identity certificate
    EXPECT_SUCCESS(ValidateChipNetworkIdentity(identity));

    // The key identifier must be extractable from the identity
    CertificateKeyIdStorage keyIdStorage;
    MutableCertificateKeyId keyId(keyIdStorage);
    EXPECT_SUCCESS(ExtractIdentifierFromChipNetworkIdentity(identity, keyId));

    // For the raw keystore, P256KeypairHandle IS P256SerializedKeypair: re-derive
    // the identity from the deserialized keypair and verify it matches. This confirms
    // the handle holds the correct key material.
    // Note: The Deserialize() call is a type error on platforms with real key handles.
#if CHIP_CONFIG_P256_KEYPAIR_HANDLE_SIZE == 0
    P256Keypair keypair;
    EXPECT_SUCCESS(keypair.Deserialize(keypairHandle));
    uint8_t rederiveBuf[kMaxCHIPCompactNetworkIdentityLength];
    MutableByteSpan rederived(rederiveBuf);
    EXPECT_SUCCESS(DeriveChipNetworkIdentity(keypair, rederived));
    EXPECT_TRUE(identity.data_equal(rederived));
#endif
}

TEST_F(TestRawKeyNetworkIdentityKeystore, DeriveIsDeterministic)
{
    if (!DeriveECDSANetworkIdentitySupported())
    {
        GTEST_SKIP() << "Skipping: DeriveECDSANetworkIdentity not supported on this platform";
    }

    RawKeyNetworkIdentityKeystore keystore;
    HkdfKeyHandle nassHandle;
    ASSERT_SUCCESS(keystore.ImportNetworkAdministratorSecret(kRawSecret2, nassHandle));

    P256KeypairHandle keypairHandle1;
    uint8_t identityBuf1[kMaxCHIPCompactNetworkIdentityLength];
    MutableByteSpan identity1(identityBuf1);
    ASSERT_SUCCESS(keystore.DeriveECDSANetworkIdentity(nassHandle, keypairHandle1, identity1));

    P256KeypairHandle keypairHandle2;
    uint8_t identityBuf2[kMaxCHIPCompactNetworkIdentityLength];
    MutableByteSpan identity2(identityBuf2);
    ASSERT_SUCCESS(keystore.DeriveECDSANetworkIdentity(nassHandle, keypairHandle2, identity2));

    EXPECT_TRUE(identity1.data_equal(identity2));
}

TEST_F(TestRawKeyNetworkIdentityKeystore, DifferentNASSProducesDifferentIdentity)
{
    if (!DeriveECDSANetworkIdentitySupported())
    {
        GTEST_SKIP() << "Skipping: DeriveECDSANetworkIdentity not supported on this platform";
    }

    RawKeyNetworkIdentityKeystore keystore;
    HkdfKeyHandle nassHandle1, nassHandle2;
    ASSERT_SUCCESS(keystore.ImportNetworkAdministratorSecret(kRawSecret1, nassHandle1));
    ASSERT_SUCCESS(keystore.ImportNetworkAdministratorSecret(kRawSecret2, nassHandle2));

    P256KeypairHandle keypairHandle1, keypairHandle2;
    uint8_t identityBuf1[kMaxCHIPCompactNetworkIdentityLength];
    uint8_t identityBuf2[kMaxCHIPCompactNetworkIdentityLength];
    MutableByteSpan identity1(identityBuf1);
    MutableByteSpan identity2(identityBuf2);

    ASSERT_SUCCESS(keystore.DeriveECDSANetworkIdentity(nassHandle1, keypairHandle1, identity1));
    ASSERT_SUCCESS(keystore.DeriveECDSANetworkIdentity(nassHandle2, keypairHandle2, identity2));

    EXPECT_FALSE(identity1.data_equal(identity2));
}

TEST_F(TestRawKeyNetworkIdentityKeystore, DeriveNASSSpecVector)
{
    if (!DeriveECDSANetworkIdentitySupported())
    {
        GTEST_SKIP() << "Skipping: DeriveECDSANetworkIdentity not supported on this platform";
    }

    // Decode the PEM test vector to extract the raw secret.
    uint8_t tlvBuf[kNetworkAdministratorSecretDataMaxEncodedLength];
    uint16_t tlvLen = Base64Decode(kNASSTestVector1_PEM, static_cast<uint16_t>(strlen(kNASSTestVector1_PEM)), tlvBuf);
    ASSERT_NE(tlvLen, UINT16_MAX);
    NetworkAdministratorSecretData secretData;
    ASSERT_SUCCESS(DecodeNetworkAdministratorSecret(ByteSpan(tlvBuf, tlvLen), secretData));

    RawKeyNetworkIdentityKeystore keystore;
    HkdfKeyHandle nassHandle;
    ASSERT_SUCCESS(keystore.ImportNetworkAdministratorSecret(secretData.rawSecret, nassHandle));

    P256KeypairHandle keypairHandle;
    uint8_t identityBuf[kMaxCHIPCompactNetworkIdentityLength];
    MutableByteSpan identity(identityBuf);
    ASSERT_SUCCESS(keystore.DeriveECDSANetworkIdentity(nassHandle, keypairHandle, identity));

    EXPECT_TRUE(identity.data_equal(ByteSpan(kNASSTestVector1_CompactIdentity)));
    EXPECT_SUCCESS(ValidateChipNetworkIdentity(identity));

    CertificateKeyIdStorage keyIdStorage;
    MutableCertificateKeyId keyId(keyIdStorage);
    EXPECT_SUCCESS(ExtractIdentifierFromChipNetworkIdentity(identity, keyId));
    EXPECT_TRUE(keyId.data_equal(ByteSpan(kNASSTestVector1_KeyIdentifier)));
}
