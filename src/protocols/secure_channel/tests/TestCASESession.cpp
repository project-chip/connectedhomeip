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
#include <protocols/secure_channel/CASESession.h>
#include <stdarg.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/UnitTestRegistration.h>

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

class LoopbackTransport : public Transport::Base
{
public:
    CHIP_ERROR SendMessage(const PeerAddress & address, System::PacketBufferHandle msgBuf) override
    {
        ReturnErrorOnFailure(mMessageSendError);
        mSentMessageCount++;
        HandleMessageReceived(address, std::move(msgBuf));

        return CHIP_NO_ERROR;
    }

    bool CanSendToPeer(const PeerAddress & address) override { return true; }

    uint32_t mSentMessageCount   = 0;
    CHIP_ERROR mMessageSendError = CHIP_NO_ERROR;
};

namespace {
TransportMgrBase gTransportMgr;
LoopbackTransport gLoopback;

OperationalCredentialSet commissionerDevOpCred;
OperationalCredentialSet accessoryDevOpCred;

ChipCertificateSet commissionerCertificateSet;
ChipCertificateSet accessoryCertificateSet;

P256SerializedKeypair commissionerOpKeysSerialized;
P256SerializedKeypair accessoryOpKeysSerialized;

P256Keypair commissionerOpKeys;
P256Keypair accessoryOpKeys;
} // namespace

enum
{
    kStandardCertsCount = 4,
    kTestCertBufSize    = 1024, // Size of buffer needed to hold any of the test certificates
                                // (in either CHIP or DER form), or to decode the certificates.
};

class TestCASESecurePairingDelegate : public SessionEstablishmentDelegate
{
public:
    void OnSessionEstablishmentError(CHIP_ERROR error) override { mNumPairingErrors++; }

    void OnSessionEstablished() override { mNumPairingComplete++; }

    uint32_t mNumPairingErrors   = 0;
    uint32_t mNumPairingComplete = 0;
};

void CASE_SecurePairingWaitTest(nlTestSuite * inSuite, void * inContext)
{
    // Test all combinations of invalid parameters
    TestCASESecurePairingDelegate delegate;
    CASESession pairing;

    NL_TEST_ASSERT(inSuite, pairing.WaitForSessionEstablishment(&accessoryDevOpCred, 0, nullptr) == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite, pairing.WaitForSessionEstablishment(&accessoryDevOpCred, 0, &delegate) == CHIP_NO_ERROR);
}

void CASE_SecurePairingStartTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    // Test all combinations of invalid parameters
    TestCASESecurePairingDelegate delegate;
    CASESession pairing;

    ExchangeContext * context = ctx.NewUnsecureExchange(&pairing);

    NL_TEST_ASSERT(inSuite,
                   pairing.EstablishSession(Transport::PeerAddress(Transport::Type::kBle), &commissionerDevOpCred, 2, 0, nullptr,
                                            nullptr) != CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite,
                   pairing.EstablishSession(Transport::PeerAddress(Transport::Type::kBle), &commissionerDevOpCred, 2, 0, context,
                                            &delegate) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, gLoopback.mSentMessageCount == 1);

    gLoopback.mMessageSendError = CHIP_ERROR_BAD_REQUEST;

    CASESession pairing1;

    gLoopback.mSentMessageCount = 0;
    gLoopback.mMessageSendError = CHIP_ERROR_BAD_REQUEST;
    ExchangeContext * context1  = ctx.NewUnsecureExchange(&pairing1);

    NL_TEST_ASSERT(inSuite,
                   pairing1.EstablishSession(Transport::PeerAddress(Transport::Type::kBle), &commissionerDevOpCred, 2, 0, context1,
                                             &delegate) == CHIP_ERROR_BAD_REQUEST);
    gLoopback.mMessageSendError = CHIP_NO_ERROR;
}

void CASE_SecurePairingHandshakeTestCommon(nlTestSuite * inSuite, void * inContext, CASESession & pairingCommissioner,
                                           TestCASESecurePairingDelegate & delegateCommissioner)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    // Test all combinations of invalid parameters
    TestCASESecurePairingDelegate delegateAccessory;
    CASESession pairingAccessory;
    CASESessionSerializable serializableCommissioner;
    CASESessionSerializable serializableAccessory;

    gLoopback.mSentMessageCount = 0;

    NL_TEST_ASSERT(inSuite,
                   ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(
                       Protocols::SecureChannel::MsgType::CASE_SigmaR1, &pairingAccessory) == CHIP_NO_ERROR);

    ExchangeContext * contextCommissioner = ctx.NewUnsecureExchange(&pairingCommissioner);

    NL_TEST_ASSERT(inSuite,
                   pairingAccessory.WaitForSessionEstablishment(&accessoryDevOpCred, 0, &delegateAccessory) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite,
                   pairingCommissioner.EstablishSession(Transport::PeerAddress(Transport::Type::kBle), &commissionerDevOpCred, 1, 0,
                                                        contextCommissioner, &delegateCommissioner) == CHIP_NO_ERROR);

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
    NL_TEST_DEF("Serialize",   CASE_SecurePairingSerializeTest),

    NL_TEST_SENTINEL()
};
// clang-format on

int CASE_TestSecurePairing_Setup(void * inContext);
int CASE_TestSecurePairing_Teardown(void * inContext);

// clang-format off
static nlTestSuite sSuite =
{
    "Test-CHIP-SecurePairing",
    &sTests[0],
    CASE_TestSecurePairing_Setup,
    CASE_TestSecurePairing_Teardown,
};
// clang-format on

static TestContext sContext;

/**
 *  Set up the test suite.
 */
int CASE_TestSecurePairing_Setup(void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    CHIP_ERROR error;
    CertificateKeyId trustedRootId = { .mId = sTestCert_Root01_SubjectKeyId, .mLen = sTestCert_Root01_SubjectKeyId_Len };

    error = chip::Platform::MemoryInit();
    SuccessOrExit(error);

    gTransportMgr.Init(&gLoopback);

    error = ctx.Init(&sSuite, &gTransportMgr);
    SuccessOrExit(error);

    ctx.SetSourceNodeId(kAnyNodeId);
    ctx.SetDestinationNodeId(kAnyNodeId);
    ctx.SetLocalKeyId(0);
    ctx.SetPeerKeyId(0);
    ctx.SetAdminId(kUndefinedAdminId);

    gTransportMgr.SetSecureSessionMgr(&ctx.GetSecureSessionManager());

    error = commissionerOpKeysSerialized.SetLength(sTestCert_Node01_01_PublicKey_Len + sTestCert_Node01_01_PrivateKey_Len);
    SuccessOrExit(error);

    memcpy((uint8_t *) (commissionerOpKeysSerialized), sTestCert_Node01_01_PublicKey, sTestCert_Node01_01_PublicKey_Len);
    memcpy((uint8_t *) (commissionerOpKeysSerialized) + sTestCert_Node01_01_PublicKey_Len, sTestCert_Node01_01_PrivateKey,
           sTestCert_Node01_01_PrivateKey_Len);

    error = commissionerOpKeys.Deserialize(commissionerOpKeysSerialized);
    SuccessOrExit(error);

    error = accessoryOpKeysSerialized.SetLength(sTestCert_Node01_01_PublicKey_Len + sTestCert_Node01_01_PrivateKey_Len);
    SuccessOrExit(error);

    memcpy((uint8_t *) (accessoryOpKeysSerialized), sTestCert_Node01_01_PublicKey, sTestCert_Node01_01_PublicKey_Len);
    memcpy((uint8_t *) (accessoryOpKeysSerialized) + sTestCert_Node01_01_PublicKey_Len, sTestCert_Node01_01_PrivateKey,
           sTestCert_Node01_01_PrivateKey_Len);

    error = accessoryOpKeys.Deserialize(accessoryOpKeysSerialized);
    SuccessOrExit(error);

    error = commissionerCertificateSet.Init(kStandardCertsCount, kTestCertBufSize);
    SuccessOrExit(error);

    error = accessoryCertificateSet.Init(kStandardCertsCount, kTestCertBufSize);
    SuccessOrExit(error);

    // Add the trusted root certificate to the certificate set.
    error = commissionerCertificateSet.LoadCert(sTestCert_Root01_Chip, sTestCert_Root01_Chip_Len,
                                                BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor));
    SuccessOrExit(error);

    error = accessoryCertificateSet.LoadCert(sTestCert_Root01_Chip, sTestCert_Root01_Chip_Len,
                                             BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor));
    SuccessOrExit(error);

    error = commissionerCertificateSet.LoadCert(sTestCert_ICA01_Chip, sTestCert_ICA01_Chip_Len,
                                                BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor));
    SuccessOrExit(error);

    error = accessoryCertificateSet.LoadCert(sTestCert_ICA01_Chip, sTestCert_ICA01_Chip_Len,
                                             BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor));
    SuccessOrExit(error);

    error = commissionerDevOpCred.Init(&commissionerCertificateSet, 1);
    SuccessOrExit(error);

    error = commissionerDevOpCred.SetDevOpCred(trustedRootId, sTestCert_Node01_01_Chip,
                                               static_cast<uint16_t>(sTestCert_Node01_01_Chip_Len));
    SuccessOrExit(error);

    error = commissionerDevOpCred.SetDevOpCredKeypair(trustedRootId, &commissionerOpKeys);
    SuccessOrExit(error);

    error = accessoryDevOpCred.Init(&accessoryCertificateSet, 1);
    SuccessOrExit(error);

    error = accessoryDevOpCred.SetDevOpCred(trustedRootId, sTestCert_Node01_01_Chip,
                                            static_cast<uint16_t>(sTestCert_Node01_01_Chip_Len));
    SuccessOrExit(error);

    error = accessoryDevOpCred.SetDevOpCredKeypair(trustedRootId, &accessoryOpKeys);
    SuccessOrExit(error);

exit:
    return error;
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
