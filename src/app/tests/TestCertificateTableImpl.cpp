/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/tls-certificate-management-server/CertificateTableImpl.h>
#include <app/util/mock/Constants.h>
#include <app/util/mock/MockNodeConfig.h>
#include <lib/support/TestPersistentStorageDelegate.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::Test;
using namespace chip::app::DataModel;
using namespace chip::app::Storage;
using namespace chip::app::Storage::Data;
using namespace chip::app::Clusters::Tls;
using namespace chip::app::Clusters::TlsCertificateManagement;
using namespace chip::app::Clusters::TlsCertificateManagement::Structs;
using chip::TLV::EstimateStructOverhead;
using namespace chip::Crypto;
using namespace chip::Credentials;

namespace TestCertificates {

static constexpr uint16_t kSpecMaxCertBytes = 3000;

struct InlineBufferedRootCert : CertificateTable::BufferedRootCert
{
    PersistentStore<CHIP_CONFIG_TLS_PERSISTED_ROOT_CERT_BYTES> buffer;
    InlineBufferedRootCert() : CertificateTable::BufferedRootCert(buffer) {}
};

struct InlineBufferedClientCert : CertificateTable::BufferedClientCert
{
    PersistentStore<CHIP_CONFIG_TLS_PERSISTED_CLIENT_CERT_BYTES> buffer;
    InlineBufferedClientCert() : CertificateTable::BufferedClientCert(buffer) {}
};

// Test constants
constexpr FabricIndex kFabric1 = 1;
constexpr FabricIndex kFabric2 = 2;

class TestCertificateTableImpl : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        mpTestStorage = new chip::TestPersistentStorageDelegate;
        ASSERT_NE(mpTestStorage, nullptr);
        ASSERT_EQ(sCertificateTable.Init(*mpTestStorage), CHIP_NO_ERROR);
        ASSERT_EQ(sCertificateTable.SetEndpoint(kMockEndpoint1), CHIP_NO_ERROR);
    }

    static void TearDownTestSuite()
    {
        sCertificateTable.Finish();
        delete mpTestStorage;
        mpTestStorage = nullptr;
    }

    static void ResetCertificateTable()
    {
        EXPECT_SUCCESS(sCertificateTable.RemoveFabric(kFabric1));
        EXPECT_SUCCESS(sCertificateTable.RemoveFabric(kFabric2));
    }

protected:
    static CertificateTableImpl sCertificateTable;
    static chip::TestPersistentStorageDelegate * mpTestStorage;
};

CertificateTableImpl TestCertificateTableImpl::sCertificateTable;
chip::TestPersistentStorageDelegate * TestCertificateTableImpl::mpTestStorage = nullptr;

TEST_F(TestCertificateTableImpl, TestRootCertificateOperations)
{
    ResetCertificateTable();

    const uint8_t byteBuf[] = { 0x1, 0x2, 0x3, 0x4, 0x8, 0xF, 0xFF };
    ByteSpan testCert(byteBuf);
    auto buffer = std::make_unique<InlineBufferedRootCert>();
    Optional<TLSCAID> id;

    // Add a root certificate
    EXPECT_EQ(sCertificateTable.UpsertRootCertificateEntry(kFabric1, id, buffer->buffer, testCert), CHIP_NO_ERROR);
    EXPECT_TRUE(id.HasValue());
    TLSCAID certId = id.Value();

    // Check if it exists
    EXPECT_EQ(sCertificateTable.HasRootCertificateEntry(kFabric1, certId), CHIP_NO_ERROR);

    // Get the certificate
    auto entry = std::make_unique<InlineBufferedRootCert>();
    EXPECT_EQ(sCertificateTable.GetRootCertificateEntry(kFabric1, certId, *entry), CHIP_NO_ERROR);
    EXPECT_TRUE(entry->mCert.certificate.Value().data_equal(testCert));

    // Remove the certificate
    EXPECT_EQ(sCertificateTable.RemoveRootCertificate(kFabric1, certId), CHIP_NO_ERROR);

    // Check that it's gone
    EXPECT_EQ(sCertificateTable.HasRootCertificateEntry(kFabric1, certId), CHIP_ERROR_NOT_FOUND);
}

TEST_F(TestCertificateTableImpl, TestClientCertificateOperations)
{
    ResetCertificateTable();

    const uint8_t byteBuf[] = { 0x1, 0x2, 0x3, 0x4, 0x8, 0xF, 0xFF };
    ByteSpan nonce(byteBuf);
    auto certBuffer = std::make_unique<InlineBufferedClientCert>();
    Optional<TLSCCDID> id;

    auto csr_buf = std::make_unique<std::array<uint8_t, kSpecMaxCertBytes>>();
    MutableByteSpan csr(*csr_buf);
    uint8_t nonce_sig_buf[Crypto::P256ECDSASignature::Capacity()];
    MutableByteSpan nonceSignature(nonce_sig_buf);

    // Prepare a client certificate
    EXPECT_EQ(sCertificateTable.PrepareClientCertificate(kFabric1, nonce, certBuffer->buffer, id, csr, nonceSignature),
              CHIP_NO_ERROR);
    EXPECT_TRUE(id.HasValue());
    TLSCCDID certId = id.Value();

    // Check if it exists
    EXPECT_EQ(sCertificateTable.HasClientCertificateEntry(kFabric1, certId), CHIP_NO_ERROR);

    // Remove the certificate
    EXPECT_EQ(sCertificateTable.RemoveClientCertificate(kFabric1, certId), CHIP_NO_ERROR);

    // Check that it's gone
    EXPECT_EQ(sCertificateTable.HasClientCertificateEntry(kFabric1, certId), CHIP_ERROR_NOT_FOUND);
}

TEST_F(TestCertificateTableImpl, TestFabricRemoval)
{
    ResetCertificateTable();

    const uint8_t byteBuf[] = { 0x1, 0x2, 0x3, 0x4, 0x8, 0xF, 0xFF };
    ByteSpan rootCert(byteBuf);
    auto rootBuffer = std::make_unique<InlineBufferedRootCert>();
    Optional<TLSCAID> rootId;
    EXPECT_EQ(sCertificateTable.UpsertRootCertificateEntry(kFabric1, rootId, rootBuffer->buffer, rootCert), CHIP_NO_ERROR);

    const uint8_t nonceByteBuf[] = { 0x1, 0x2, 0x3, 0x4, 0x8, 0xF, 0xFF };
    ByteSpan nonce(nonceByteBuf);
    auto clientBuffer = std::make_unique<InlineBufferedClientCert>();
    Optional<TLSCCDID> clientId;
    auto csr_buf = std::make_unique<std::array<uint8_t, kSpecMaxCertBytes>>();
    MutableByteSpan csr(*csr_buf);
    uint8_t nonce_sig_buf[Crypto::P256ECDSASignature::Capacity()];
    MutableByteSpan nonceSignature(nonce_sig_buf);
    EXPECT_EQ(sCertificateTable.PrepareClientCertificate(kFabric1, nonce, clientBuffer->buffer, clientId, csr, nonceSignature),
              CHIP_NO_ERROR);

    uint8_t count;
    EXPECT_EQ(sCertificateTable.GetRootCertificateCount(kFabric1, count), CHIP_NO_ERROR);
    EXPECT_EQ(count, 1);
    EXPECT_EQ(sCertificateTable.GetClientCertificateCount(kFabric1, count), CHIP_NO_ERROR);
    EXPECT_EQ(count, 1);

    EXPECT_EQ(sCertificateTable.RemoveFabric(kFabric1), CHIP_NO_ERROR);

    EXPECT_EQ(sCertificateTable.GetRootCertificateCount(kFabric1, count), CHIP_NO_ERROR);
    EXPECT_EQ(count, 0);
    EXPECT_EQ(sCertificateTable.GetClientCertificateCount(kFabric1, count), CHIP_NO_ERROR);
    EXPECT_EQ(count, 0);
}

TEST_F(TestCertificateTableImpl, TestRootCertificateIteration)
{
    ResetCertificateTable();

    const uint8_t byteBuf1[] = { 0x1, 0x2, 0x3, 0x4, 0x8, 0xF, 0xFF, 0xA };
    const uint8_t byteBuf2[] = { 0x1, 0x2, 0x3, 0x4, 0x8, 0xF, 0xFF, 0xB };
    ByteSpan rootCert1(byteBuf1);
    ByteSpan rootCert2(byteBuf2);
    auto rootBuffer1 = std::make_unique<InlineBufferedRootCert>();
    auto rootBuffer2 = std::make_unique<InlineBufferedRootCert>();
    Optional<TLSCAID> rootId1;
    Optional<TLSCAID> rootId2;

    EXPECT_EQ(sCertificateTable.UpsertRootCertificateEntry(kFabric1, rootId1, rootBuffer1->buffer, rootCert1), CHIP_NO_ERROR);
    EXPECT_EQ(sCertificateTable.UpsertRootCertificateEntry(kFabric1, rootId2, rootBuffer2->buffer, rootCert2), CHIP_NO_ERROR);
    EXPECT_NE(rootId1, rootId2);

    uint8_t count   = 0;
    auto certBuffer = std::make_unique<InlineBufferedRootCert>();
    auto iterFn     = [&](auto & iterator) {
        while (iterator.Next(certBuffer->GetCert()))
        {
            count++;
        }
        return CHIP_NO_ERROR;
    };

    EXPECT_EQ(sCertificateTable.IterateRootCertificates(kFabric1, *certBuffer, iterFn), CHIP_NO_ERROR);
    EXPECT_EQ(count, 2);
}

CHIP_ERROR GetTestCert(MutableByteSpan & signedCert, InlineBufferedClientCert & cert, const ByteSpan & csr)
{
    ChipDN ica_dn;
    ReturnErrorOnFailure(ica_dn.AddAttribute_MatterICACId(0xABCDABCDABCDABCD));
    ChipDN issuer_dn;
    ReturnErrorOnFailure(issuer_dn.AddAttribute_MatterRCACId(0x43215678FEDCABCD));
    X509CertRequestParams params = { 1234, 631161876, 729942000, ica_dn, issuer_dn };
    P256PublicKey pubkey;
    ReturnErrorOnFailure(VerifyCertificateSigningRequest(csr.data(), csr.size(), pubkey));
    Crypto::P256Keypair testPair;
    ReturnErrorOnFailure(testPair.Initialize(Crypto::ECPKeyTarget::ECDSA));
    ReturnErrorOnFailure(NewICAX509Cert(params, pubkey, testPair, signedCert));
    cert.GetCert().clientCertificate.SetValue(signedCert);
    return CHIP_NO_ERROR;
}

TEST_F(TestCertificateTableImpl, TestClientCertificateIteration)
{
    ResetCertificateTable();

    const uint8_t byteBuf[] = { 0x1, 0x2, 0x3, 0x4, 0x8, 0xF, 0xFF };
    ByteSpan nonce(byteBuf);
    auto certBuffer1 = std::make_unique<InlineBufferedClientCert>();
    auto certBuffer2 = std::make_unique<InlineBufferedClientCert>();
    Optional<TLSCAID> clientId1;
    Optional<TLSCAID> clientId2;

    auto csr_buf = std::make_unique<std::array<uint8_t, kSpecMaxCertBytes>>();
    MutableByteSpan csr(*csr_buf);
    uint8_t nonceSigBuff[Crypto::P256ECDSASignature::Capacity()];
    MutableByteSpan nonceSignature(nonceSigBuff);

    uint8_t signedCert[kMaxDERCertLength];
    MutableByteSpan signedCertSpan(signedCert);
    // Prepare client certificates
    EXPECT_EQ(sCertificateTable.PrepareClientCertificate(kFabric1, nonce, certBuffer1->buffer, clientId1, csr, nonceSignature),
              CHIP_NO_ERROR);
    EXPECT_EQ(GetTestCert(signedCertSpan, *certBuffer1, csr), CHIP_NO_ERROR);
    EXPECT_EQ(
        sCertificateTable.UpdateClientCertificateEntry(kFabric1, clientId1.Value(), certBuffer1->buffer, certBuffer1->GetCert()),
        CHIP_NO_ERROR);

    csr            = MutableByteSpan(*csr_buf);
    signedCertSpan = MutableByteSpan(signedCert);
    EXPECT_EQ(sCertificateTable.PrepareClientCertificate(kFabric1, nonce, certBuffer2->buffer, clientId2, csr, nonceSignature),
              CHIP_NO_ERROR);
    EXPECT_EQ(GetTestCert(signedCertSpan, *certBuffer2, csr), CHIP_NO_ERROR);
    EXPECT_EQ(
        sCertificateTable.UpdateClientCertificateEntry(kFabric1, clientId2.Value(), certBuffer2->buffer, certBuffer2->GetCert()),
        CHIP_NO_ERROR);
    EXPECT_NE(clientId1, clientId2);

    uint8_t count   = 0;
    auto certBuffer = std::make_unique<InlineBufferedClientCert>();
    auto iterFn     = [&](auto & iterator) {
        while (iterator.Next(certBuffer->mCertWithKey))
        {
            count++;
        }
        return CHIP_NO_ERROR;
    };

    EXPECT_EQ(sCertificateTable.IterateClientCertificates(kFabric1, *certBuffer, iterFn), CHIP_NO_ERROR);
    EXPECT_EQ(count, 2);
}

} // namespace TestCertificates
