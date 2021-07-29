/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 *    @file
 *      This file implements unit tests for the CASESession implementation.
 */

#include <errno.h>
#include <nlunit-test.h>

#include <core/CHIPCore.h>
#include <core/CHIPSafeCasts.h>
#include <credentials/CHIPCert.h>
#include <credentials/CHIPOperationalCredentials.h>
#include <messaging/tests/MessagingContext.h>
#include <protocols/secure_channel/CASEServer.h>
#include <protocols/secure_channel/CASESession.h>
#include <stdarg.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/ScopedBuffer.h>
#include <support/UnitTestRegistration.h>
#include <transport/raw/tests/NetworkTestHelpers.h>

#include "credentials/tests/CHIPCert_test_vectors.h"

using namespace chip;
using namespace Credentials;
using namespace TestCerts;

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::Messaging;
using namespace chip::Protocols;

using TestContext = chip::Test::MessagingContext;

namespace {
TransportMgrBase gTransportMgr;
Test::LoopbackTransport gLoopback;

OperationalCredentialSet commissionerDevOpCred;
OperationalCredentialSet accessoryDevOpCred;

ChipCertificateSet commissionerCertificateSet;
ChipCertificateSet accessoryCertificateSet;

P256SerializedKeypair commissionerOpKeysSerialized;
P256SerializedKeypair accessoryOpKeysSerialized;

P256Keypair commissionerOpKeys;
P256Keypair accessoryOpKeys;

CertificateKeyId trustedRootId = CertificateKeyId(sTestCert_Root01_SubjectKeyId);
uint8_t commissionerCredentialsIndex;

NodeId Node01_01 = 0xDEDEDEDE00010001;
} // namespace

enum
{
    kStandardCertsCount = 3,
};

class TestCASESecurePairingDelegate : public SessionEstablishmentDelegate
{
public:
    void OnSessionEstablishmentError(CHIP_ERROR error) override { mNumPairingErrors++; }

    void OnSessionEstablished() override { mNumPairingComplete++; }

    uint32_t mNumPairingErrors   = 0;
    uint32_t mNumPairingComplete = 0;
};

class TestCASESessionDestinationId : public CASESession
{
public:
    CHIP_ERROR GenerateDestinationID(const ByteSpan & random, const Credentials::P256PublicKeySpan & rootPubkey, NodeId nodeId,
                                     FabricId fabricId, const ByteSpan & ipk, MutableByteSpan & destinationId)
    {
        return CASESession::GenerateDestinationID(random, rootPubkey, nodeId, fabricId, ipk, destinationId);
    }
};

class TestCASESessionIPK : public CASESession
{
protected:
    ByteSpan * GetIPKList() const override
    {
        // TODO: Remove this list. Replace it with an actual method to retrieve an IPK list (e.g. from a Crypto Store API)
        static uint8_t sIPKList[][kIPKSize] = {
            { 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D,
              0x1D }, /* Corresponds to the FabricID for the Node01_01 Test Vector */
        };
        static ByteSpan ipkListSpan[] = { ByteSpan(sIPKList[0]) };
        return ipkListSpan;
    }
    size_t GetIPKListEntries() const override { return 1; }
};

class TestCASEServerIPK : public CASEServer
{
public:
    TestCASESessionIPK & GetSession() override { return mPairingSession; }

private:
    TestCASESessionIPK mPairingSession;
};

static CHIP_ERROR InitCredentialSets()
{
    commissionerDevOpCred.Release();
    accessoryDevOpCred.Release();
    commissionerCertificateSet.Release();
    accessoryCertificateSet.Release();

    ReturnErrorOnFailure(
        commissionerOpKeysSerialized.SetLength(sTestCert_Node01_01_PublicKey_Len + sTestCert_Node01_01_PrivateKey_Len));

    memcpy((uint8_t *) (commissionerOpKeysSerialized), sTestCert_Node01_01_PublicKey, sTestCert_Node01_01_PublicKey_Len);
    memcpy((uint8_t *) (commissionerOpKeysSerialized) + sTestCert_Node01_01_PublicKey_Len, sTestCert_Node01_01_PrivateKey,
           sTestCert_Node01_01_PrivateKey_Len);

    ReturnErrorOnFailure(commissionerOpKeys.Deserialize(commissionerOpKeysSerialized));

    ReturnErrorOnFailure(
        accessoryOpKeysSerialized.SetLength(sTestCert_Node01_01_PublicKey_Len + sTestCert_Node01_01_PrivateKey_Len));

    memcpy((uint8_t *) (accessoryOpKeysSerialized), sTestCert_Node01_01_PublicKey, sTestCert_Node01_01_PublicKey_Len);
    memcpy((uint8_t *) (accessoryOpKeysSerialized) + sTestCert_Node01_01_PublicKey_Len, sTestCert_Node01_01_PrivateKey,
           sTestCert_Node01_01_PrivateKey_Len);

    ReturnErrorOnFailure(accessoryOpKeys.Deserialize(accessoryOpKeysSerialized));

    ReturnErrorOnFailure(commissionerCertificateSet.Init(kStandardCertsCount));

    ReturnErrorOnFailure(accessoryCertificateSet.Init(kStandardCertsCount));

    // Add the trusted root certificate to the certificate set.
    ReturnErrorOnFailure(commissionerCertificateSet.LoadCert(sTestCert_Root01_Chip, sTestCert_Root01_Chip_Len,
                                                             BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)));

    ReturnErrorOnFailure(accessoryCertificateSet.LoadCert(sTestCert_Root01_Chip, sTestCert_Root01_Chip_Len,
                                                          BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)));

    ReturnErrorOnFailure(commissionerCertificateSet.LoadCert(sTestCert_ICA01_Chip, sTestCert_ICA01_Chip_Len,
                                                             BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)));

    ReturnErrorOnFailure(accessoryCertificateSet.LoadCert(sTestCert_ICA01_Chip, sTestCert_ICA01_Chip_Len,
                                                          BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor)));

    ReturnErrorOnFailure(commissionerDevOpCred.Init(&commissionerCertificateSet, 1));
    commissionerCredentialsIndex = static_cast<uint8_t>(commissionerDevOpCred.GetCertCount() - 1U);

    ReturnErrorOnFailure(commissionerDevOpCred.SetDevOpCred(trustedRootId, sTestCert_Node01_01_Chip,
                                                            static_cast<uint16_t>(sTestCert_Node01_01_Chip_Len)));

    ReturnErrorOnFailure(commissionerDevOpCred.SetDevOpCredKeypair(trustedRootId, &commissionerOpKeys));

    ReturnErrorOnFailure(accessoryDevOpCred.Init(&accessoryCertificateSet, 1));

    ReturnErrorOnFailure(accessoryDevOpCred.SetDevOpCred(trustedRootId, sTestCert_Node01_01_Chip,
                                                         static_cast<uint16_t>(sTestCert_Node01_01_Chip_Len)));

    ReturnErrorOnFailure(accessoryDevOpCred.SetDevOpCredKeypair(trustedRootId, &accessoryOpKeys));

    return CHIP_NO_ERROR;
}

void CASE_SecurePairingWaitTest(nlTestSuite * inSuite, void * inContext)
{
    // Test all combinations of invalid parameters
    TestCASESecurePairingDelegate delegate;
    TestCASESessionIPK pairing;

    NL_TEST_ASSERT(inSuite, pairing.ListenForSessionEstablishment(&accessoryDevOpCred, 0, nullptr) == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite, pairing.ListenForSessionEstablishment(&accessoryDevOpCred, 0, &delegate) == CHIP_NO_ERROR);
}

void CASE_SecurePairingStartTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    // Test all combinations of invalid parameters
    TestCASESecurePairingDelegate delegate;
    CASESession pairing;

    NL_TEST_ASSERT(inSuite, pairing.MessageDispatch().Init(&gTransportMgr) == CHIP_NO_ERROR);
    ExchangeContext * context = ctx.NewExchangeToLocal(&pairing);

    NL_TEST_ASSERT(inSuite,
                   pairing.EstablishSession(Transport::PeerAddress(Transport::Type::kBle), &commissionerDevOpCred,
                                            commissionerCredentialsIndex, Node01_01, 0, nullptr, nullptr) != CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite,
                   pairing.EstablishSession(Transport::PeerAddress(Transport::Type::kBle), &commissionerDevOpCred,
                                            commissionerCredentialsIndex, Node01_01, 0, context, &delegate) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, gLoopback.mSentMessageCount == 1);

    gLoopback.mMessageSendError = CHIP_ERROR_BAD_REQUEST;

    CASESession pairing1;
    NL_TEST_ASSERT(inSuite, pairing1.MessageDispatch().Init(&gTransportMgr) == CHIP_NO_ERROR);

    gLoopback.mSentMessageCount = 0;
    gLoopback.mMessageSendError = CHIP_ERROR_BAD_REQUEST;
    ExchangeContext * context1  = ctx.NewExchangeToLocal(&pairing1);

    NL_TEST_ASSERT(inSuite,
                   pairing1.EstablishSession(Transport::PeerAddress(Transport::Type::kBle), &commissionerDevOpCred,
                                             commissionerCredentialsIndex, Node01_01, 0, context1,
                                             &delegate) == CHIP_ERROR_BAD_REQUEST);
    gLoopback.mMessageSendError = CHIP_NO_ERROR;
}

void CASE_SecurePairingHandshakeTestCommon(nlTestSuite * inSuite, void * inContext, CASESession & pairingCommissioner,
                                           TestCASESecurePairingDelegate & delegateCommissioner)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    // Test all combinations of invalid parameters
    TestCASESecurePairingDelegate delegateAccessory;
    TestCASESessionIPK pairingAccessory;
    CASESessionSerializable serializableCommissioner;
    CASESessionSerializable serializableAccessory;

    gLoopback.mSentMessageCount = 0;
    NL_TEST_ASSERT(inSuite, pairingCommissioner.MessageDispatch().Init(&gTransportMgr) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, pairingAccessory.MessageDispatch().Init(&gTransportMgr) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite,
                   ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(
                       Protocols::SecureChannel::MsgType::CASE_SigmaR1, &pairingAccessory) == CHIP_NO_ERROR);

    ExchangeContext * contextCommissioner = ctx.NewExchangeToLocal(&pairingCommissioner);

    NL_TEST_ASSERT(inSuite,
                   pairingAccessory.ListenForSessionEstablishment(&accessoryDevOpCred, 0, &delegateAccessory) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite,
                   pairingCommissioner.EstablishSession(Transport::PeerAddress(Transport::Type::kBle), &commissionerDevOpCred,
                                                        commissionerCredentialsIndex, Node01_01, 0, contextCommissioner,
                                                        &delegateCommissioner) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, gLoopback.mSentMessageCount == 3);
    NL_TEST_ASSERT(inSuite, delegateAccessory.mNumPairingComplete == 1);
    NL_TEST_ASSERT(inSuite, delegateCommissioner.mNumPairingComplete == 1);

    NL_TEST_ASSERT(inSuite, pairingCommissioner.ToSerializable(serializableCommissioner) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, pairingAccessory.ToSerializable(serializableAccessory) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite,
                   memcmp(serializableCommissioner.mSharedSecret, serializableAccessory.mSharedSecret,
                          serializableCommissioner.mSharedSecretLen) == 0);
}

void CASE_SecurePairingHandshakeTest(nlTestSuite * inSuite, void * inContext)
{
    TestCASESecurePairingDelegate delegateCommissioner;
    CASESession pairingCommissioner;
    CASE_SecurePairingHandshakeTestCommon(inSuite, inContext, pairingCommissioner, delegateCommissioner);
}

class TestPersistentStorageDelegate : public PersistentStorageDelegate
{
public:
    TestPersistentStorageDelegate()
    {
        memset(keys, 0, sizeof(keys));
        memset(keysize, 0, sizeof(keysize));
        memset(values, 0, sizeof(values));
        memset(valuesize, 0, sizeof(valuesize));
    }

    ~TestPersistentStorageDelegate()
    {
        for (int i = 0; i < 16; i++)
        {
            if (keys[i] != nullptr)
            {
                chip::Platform::MemoryFree(keys[i]);
            }
            if (values[i] != nullptr)
            {
                chip::Platform::MemoryFree(values[i]);
            }
        }
    }

    CHIP_ERROR SyncGetKeyValue(const char * key, void * buffer, uint16_t & size) override
    {
        for (int i = 0; i < 16; i++)
        {
            if (keys[i] != nullptr && keysize[i] != 0 && size >= valuesize[i])
            {
                if (memcmp(key, keys[i], keysize[i]) == 0)
                {
                    memcpy(buffer, values[i], valuesize[i]);
                    size = valuesize[i];
                    return CHIP_NO_ERROR;
                }
            }
        }
        return CHIP_ERROR_INTERNAL;
    }

    CHIP_ERROR SyncSetKeyValue(const char * key, const void * value, uint16_t size) override
    {
        for (int i = 0; i < 16; i++)
        {
            if (keys[i] == nullptr && keysize[i] == 0 && valuesize[i] == 0)
            {
                keysize[i] = static_cast<uint16_t>(strlen(key));
                keysize[i]++;
                keys[i] = reinterpret_cast<char *>(chip::Platform::MemoryAlloc(keysize[i]));
                strcpy(keys[i], key);
                values[i] = reinterpret_cast<char *>(chip::Platform::MemoryAlloc(size));
                memcpy(values[i], value, size);
                valuesize[i] = size;
                return CHIP_NO_ERROR;
            }
        }
        return CHIP_ERROR_INTERNAL;
    }

    CHIP_ERROR SyncDeleteKeyValue(const char * key) override { return CHIP_NO_ERROR; }

private:
    char * keys[16];
    void * values[16];
    uint16_t keysize[16];
    uint16_t valuesize[16];
};

TestCASEServerIPK gPairingServer;

void CASE_SecurePairingHandshakeServerTest(nlTestSuite * inSuite, void * inContext)
{
    TestCASESecurePairingDelegate delegateCommissioner;

    auto * pairingCommissioner = chip::Platform::New<CASESession>();

    FabricTable fabricTable;
    TestPersistentStorageDelegate storageDelegate;
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    gLoopback.mSentMessageCount = 0;
    NL_TEST_ASSERT(inSuite, pairingCommissioner->MessageDispatch().Init(&gTransportMgr) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, gPairingServer.GetSession().MessageDispatch().Init(&gTransportMgr) == CHIP_NO_ERROR);

    SessionIDAllocator idAllocator;

    fabricTable.Init(&storageDelegate);

    FabricInfo * fabric = fabricTable.AssignFabricIndex(0);

    NL_TEST_ASSERT(inSuite, fabric->SetOperationalKey(accessoryOpKeys) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, fabric->SetRootCert(ByteSpan(sTestCert_Root01_Chip, sTestCert_Root01_Chip_Len)) == CHIP_NO_ERROR);

    uint8_t chipCert[kMaxCHIPCertLength * 2];
    MutableByteSpan chipCertSpan(chipCert, sizeof(chipCert));
    NL_TEST_ASSERT(inSuite,
                   ConvertX509CertsToChipCertArray(ByteSpan(sTestCert_Node01_01_DER, sTestCert_Node01_01_DER_Len),
                                                   ByteSpan(sTestCert_ICA01_DER, sTestCert_ICA01_DER_Len),
                                                   chipCertSpan) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, fabric->SetOperationalCertsFromCertArray(chipCertSpan) == CHIP_NO_ERROR);

    fabricTable.Store(0);
    fabricTable.ReleaseFabricIndex(0);

    fabricTable.LoadFromStorage(0);
    fabric = fabricTable.FindFabricWithIndex(0);

    ChipCertificateSet certificates;
    OperationalCredentialSet credentials;
    CertificateKeyId rootKeyId;
    uint8_t credentialsIndex;
    NL_TEST_ASSERT(inSuite, fabric->GetCredentials(credentials, certificates, rootKeyId, credentialsIndex) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite,
                   gPairingServer.ListenForSessionEstablishment(&ctx.GetExchangeManager(), &gTransportMgr,
                                                                &ctx.GetSecureSessionManager(), &fabricTable,
                                                                &idAllocator) == CHIP_NO_ERROR);

    ExchangeContext * contextCommissioner = ctx.NewExchangeToLocal(pairingCommissioner);

    NL_TEST_ASSERT(inSuite,
                   pairingCommissioner->EstablishSession(Transport::PeerAddress(Transport::Type::kBle), &credentials,
                                                         credentialsIndex, Node01_01, 0, contextCommissioner,
                                                         &delegateCommissioner) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, gLoopback.mSentMessageCount == 3);
    NL_TEST_ASSERT(inSuite, delegateCommissioner.mNumPairingComplete == 1);

    auto * pairingCommissioner1 = chip::Platform::New<CASESession>();
    NL_TEST_ASSERT(inSuite, pairingCommissioner1->MessageDispatch().Init(&gTransportMgr) == CHIP_NO_ERROR);
    ExchangeContext * contextCommissioner1 = ctx.NewExchangeToLocal(pairingCommissioner1);

    NL_TEST_ASSERT(inSuite,
                   pairingCommissioner1->EstablishSession(Transport::PeerAddress(Transport::Type::kBle), &credentials,
                                                          credentialsIndex, Node01_01, 0, contextCommissioner1,
                                                          &delegateCommissioner) == CHIP_NO_ERROR);

    chip::Platform::Delete(pairingCommissioner);
    chip::Platform::Delete(pairingCommissioner1);
}

void CASE_SecurePairingDeserialize(nlTestSuite * inSuite, void * inContext, CASESession & pairingCommissioner,
                                   CASESession & deserialized)
{
    CASESessionSerialized serialized;
    NL_TEST_ASSERT(inSuite, pairingCommissioner.Serialize(serialized) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, deserialized.Deserialize(serialized) == CHIP_NO_ERROR);

    // Serialize from the deserialized session, and check we get the same string back
    CASESessionSerialized serialized2;
    NL_TEST_ASSERT(inSuite, deserialized.Serialize(serialized2) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, strncmp(Uint8::to_char(serialized.inner), Uint8::to_char(serialized2.inner), sizeof(serialized)) == 0);
}

void CASE_SecurePairingSerializeTest(nlTestSuite * inSuite, void * inContext)
{
    TestCASESecurePairingDelegate delegateCommissioner;

    // Allocate on the heap to avoid stack overflow in some restricted test scenarios (e.g. QEMU)
    auto * testPairingSession1 = chip::Platform::New<CASESession>();
    auto * testPairingSession2 = chip::Platform::New<CASESession>();

    CASE_SecurePairingHandshakeTestCommon(inSuite, inContext, *testPairingSession1, delegateCommissioner);
    CASE_SecurePairingDeserialize(inSuite, inContext, *testPairingSession1, *testPairingSession2);

    const uint8_t plain_text[] = { 0x86, 0x74, 0x64, 0xe5, 0x0b, 0xd4, 0x0d, 0x90, 0xe1, 0x17, 0xa3, 0x2d, 0x4b, 0xd4, 0xe1, 0xe6 };
    uint8_t encrypted[64];
    PacketHeader header;
    MessageAuthenticationCode mac;

    // Let's try encrypting using original session, and decrypting using deserialized
    {
        SecureSession session1;

        NL_TEST_ASSERT(inSuite,
                       testPairingSession1->DeriveSecureSession(session1, SecureSession::SessionRole::kInitiator) == CHIP_NO_ERROR);

        NL_TEST_ASSERT(inSuite, session1.Encrypt(plain_text, sizeof(plain_text), encrypted, header, mac) == CHIP_NO_ERROR);
    }

    {
        SecureSession session2;
        NL_TEST_ASSERT(inSuite,
                       testPairingSession2->DeriveSecureSession(session2, SecureSession::SessionRole::kResponder) == CHIP_NO_ERROR);

        uint8_t decrypted[64];
        NL_TEST_ASSERT(inSuite, session2.Decrypt(encrypted, sizeof(plain_text), decrypted, header, mac) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, memcmp(plain_text, decrypted, sizeof(plain_text)) == 0);
    }

    chip::Platform::Delete(testPairingSession1);
    chip::Platform::Delete(testPairingSession2);
}

void CASE_DestinationIDGenerationTest(nlTestSuite * inSuite, void * inContext)
{
    TestCASESessionDestinationId pairingCommissioner;

    uint8_t random[kSigmaParamRandomNumberSize]        = { 0x7e, 0x17, 0x12, 0x31, 0x56, 0x8d, 0xfa, 0x17, 0x20, 0x6b, 0x3a,
                                                    0xcc, 0xf8, 0xfa, 0xec, 0x2f, 0x4d, 0x21, 0xb5, 0x80, 0x11, 0x31,
                                                    0x96, 0xf4, 0x7c, 0x7c, 0x4d, 0xeb, 0x81, 0x0a, 0x73, 0xdc };
    uint8_t destinationIdentifier[kSHA256_Hash_Length] = { 0 };
    NodeId nodeId                                      = 0xCD5544AA7B13EF14;
    FabricId fabricId                                  = 0x2906C908D115D362;
    uint8_t rootPubkey[kP256_PublicKey_Length] = { 0x04, 0x4a, 0x9f, 0x42, 0xb1, 0xca, 0x48, 0x40, 0xd3, 0x72, 0x92, 0xbb, 0xc7,
                                                   0xf6, 0xa7, 0xe1, 0x1e, 0x22, 0x20, 0x0c, 0x97, 0x6f, 0xc9, 0x00, 0xdb, 0xc9,
                                                   0x8a, 0x7a, 0x38, 0x3a, 0x64, 0x1c, 0xb8, 0x25, 0x4a, 0x2e, 0x56, 0xd4, 0xe2,
                                                   0x95, 0xa8, 0x47, 0x94, 0x3b, 0x4e, 0x38, 0x97, 0xc4, 0xa7, 0x73, 0xe9, 0x30,
                                                   0x27, 0x7b, 0x4d, 0x9f, 0xbe, 0xde, 0x8a, 0x05, 0x26, 0x86, 0xbf, 0xac, 0xfa };
    P256PublicKeySpan rootPubkeySpan(rootPubkey);
    uint8_t destinationIdentifierTestVector[kSHA256_Hash_Length] = { 0xc8, 0xe1, 0x70, 0x0d, 0x12, 0x5a, 0xff, 0xbc,
                                                                     0xea, 0xda, 0x34, 0x2a, 0x0d, 0x00, 0xdb, 0x7c,
                                                                     0xa0, 0x65, 0x05, 0xae, 0x5d, 0x0b, 0x29, 0x87,
                                                                     0xf3, 0xaf, 0x4b, 0x77, 0xe3, 0x94, 0x05, 0x1d };

    uint8_t ipk[] = { 0x4a, 0x71, 0xcd, 0xd7, 0xb2, 0xa3, 0xca, 0x90, 0x24, 0xf9, 0x6f, 0x3c, 0x96, 0xa1, 0x9d, 0xee };

    {
        MutableByteSpan destinationIdSpan(destinationIdentifier, sizeof(destinationIdentifier));
        NL_TEST_ASSERT(inSuite,
                       pairingCommissioner.GenerateDestinationID(ByteSpan(random, sizeof(random)), rootPubkeySpan, nodeId, fabricId,
                                                                 ByteSpan(ipk, sizeof(ipk)), destinationIdSpan) == CHIP_NO_ERROR);
    }

    NL_TEST_ASSERT(inSuite, memcmp(destinationIdentifier, destinationIdentifierTestVector, sizeof(destinationIdentifier)) == 0);
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("WaitInit",    CASE_SecurePairingWaitTest),
    NL_TEST_DEF("Start",       CASE_SecurePairingStartTest),
    NL_TEST_DEF("Handshake",   CASE_SecurePairingHandshakeTest),
    NL_TEST_DEF("ServerHandshake", CASE_SecurePairingHandshakeServerTest),
    NL_TEST_DEF("Serialize",   CASE_SecurePairingSerializeTest),
    NL_TEST_DEF("DestinationID Generation", CASE_DestinationIDGenerationTest),

    NL_TEST_SENTINEL()
};
// clang-format on

int CASE_TestSecurePairing_Setup(void * inContext);
int CASE_TestSecurePairing_Teardown(void * inContext);

// clang-format off
static nlTestSuite sSuite =
{
    "Test-CHIP-SecurePairing-CASE",
    &sTests[0],
    CASE_TestSecurePairing_Setup,
    CASE_TestSecurePairing_Teardown,
};
// clang-format on

static TestContext sContext;

namespace {
/*
 *  Set up the test suite.
 */
CHIP_ERROR CASETestSecurePairingSetup(void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    ReturnErrorOnFailure(chip::Platform::MemoryInit());

    gTransportMgr.Init(&gLoopback);

    ReturnErrorOnFailure(ctx.Init(&sSuite, &gTransportMgr));

    ctx.SetSourceNodeId(kPlaceholderNodeId);
    ctx.SetDestinationNodeId(kPlaceholderNodeId);
    ctx.SetLocalKeyId(0);
    ctx.SetPeerKeyId(0);
    ctx.SetFabricIndex(kUndefinedFabricIndex);

    gTransportMgr.SetSecureSessionMgr(&ctx.GetSecureSessionManager());

    return InitCredentialSets();
}
} // anonymous namespace

/**
 *  Set up the test suite.
 */
int CASE_TestSecurePairing_Setup(void * inContext)
{
    return CASETestSecurePairingSetup(inContext) == CHIP_NO_ERROR ? SUCCESS : FAILURE;
}

/**
 *  Tear down the test suite.
 */
int CASE_TestSecurePairing_Teardown(void * inContext)
{
    reinterpret_cast<TestContext *>(inContext)->Shutdown();
    commissionerDevOpCred.Release();
    accessoryDevOpCred.Release();
    commissionerCertificateSet.Release();
    accessoryCertificateSet.Release();
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

/**
 *  Main
 */
int TestCASESession()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, &sContext);

    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestCASESession)
