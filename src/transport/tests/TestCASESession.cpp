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
#include <stdarg.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/UnitTestRegistration.h>
#include <transport/CASESession.h>

#include "credentials/tests/CHIPCert_test_vectors.h"

using namespace chip;
using namespace Credentials;
using namespace TestCerts;

OperationalCredentialSet commissionerDevOpCred;
OperationalCredentialSet accessoryDevOpCred;

ChipCertificateSet commissionerCertificateSet;
ChipCertificateSet accessoryCertificateSet;

P256SerializedKeypair commissionerOpKeysSerialized;
P256SerializedKeypair accessoryOpKeysSerialized;

P256Keypair commissionerOpKeys;
P256Keypair accessoryOpKeys;

enum
{
    kStandardCertsCount = 4,
    kTestCertBufSize    = 1024, // Size of buffer needed to hold any of the test certificates
                                // (in either CHIP or DER form), or to decode the certificates.
};

class TestCASESecurePairingDelegate : public SessionEstablishmentDelegate
{
public:
    CHIP_ERROR SendSessionEstablishmentMessage(const PacketHeader & header, const Transport::PeerAddress & peerAddress,
                                               System::PacketBufferHandle msgBuf) override
    {
        mNumMessageSend++;
        return (peer != nullptr) ? peer->HandlePeerMessage(header, peerAddress, std::move(msgBuf)) : mMessageSendError;
    }

    void OnSessionEstablishmentError(CHIP_ERROR error) override { mNumPairingErrors++; }

    void OnSessionEstablished() override { mNumPairingComplete++; }

    uint32_t mNumMessageSend     = 0;
    uint32_t mNumPairingErrors   = 0;
    uint32_t mNumPairingComplete = 0;
    CHIP_ERROR mMessageSendError = CHIP_NO_ERROR;

    CASESession * peer = nullptr;
};

void CASE_SecurePairingWaitTest(nlTestSuite * inSuite, void * inContext)
{
    // Test all combinations of invalid parameters
    TestCASESecurePairingDelegate delegate;
    CASESession pairing;

    NL_TEST_ASSERT(inSuite,
                   pairing.WaitForSessionEstablishment(&accessoryDevOpCred, Optional<NodeId>::Value(1), 0, nullptr) ==
                       CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite,
                   pairing.WaitForSessionEstablishment(&accessoryDevOpCred, Optional<NodeId>::Value(1), 0, &delegate) ==
                       CHIP_NO_ERROR);
}

void CASE_SecurePairingStartTest(nlTestSuite * inSuite, void * inContext)
{
    // Test all combinations of invalid parameters
    TestCASESecurePairingDelegate delegate;
    CASESession pairing;

    NL_TEST_ASSERT(inSuite,
                   pairing.EstablishSession(Transport::PeerAddress(Transport::Type::kBle), &commissionerDevOpCred,
                                            Optional<NodeId>::Value(1), 2, 0, nullptr) != CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite,
                   pairing.EstablishSession(Transport::PeerAddress(Transport::Type::kBle), &commissionerDevOpCred,
                                            Optional<NodeId>::Value(1), 2, 0, &delegate) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, delegate.mNumMessageSend == 1);

    delegate.mMessageSendError = CHIP_ERROR_BAD_REQUEST;

    CASESession pairing1;

    NL_TEST_ASSERT(inSuite,
                   pairing1.EstablishSession(Transport::PeerAddress(Transport::Type::kBle), &commissionerDevOpCred,
                                             Optional<NodeId>::Value(1), 2, 0, &delegate) == CHIP_ERROR_BAD_REQUEST);
}

void CASE_SecurePairingHandshakeTestCommon(nlTestSuite * inSuite, void * inContext, CASESession & pairingCommissioner,
                                           TestCASESecurePairingDelegate & delegateCommissioner)
{
    // Test all combinations of invalid parameters
    TestCASESecurePairingDelegate delegateAccessory;
    CASESession pairingAccessory;
    CASESessionSerializable serializableCommissioner;
    CASESessionSerializable serializableAccessory;

    delegateCommissioner.peer = &pairingAccessory;
    delegateAccessory.peer    = &pairingCommissioner;

    NL_TEST_ASSERT(inSuite,
                   pairingAccessory.WaitForSessionEstablishment(&accessoryDevOpCred, Optional<NodeId>::Value(1), 0,
                                                                &delegateAccessory) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite,
                   pairingCommissioner.EstablishSession(Transport::PeerAddress(Transport::Type::kBle), &commissionerDevOpCred,
                                                        Optional<NodeId>::Value(2), 1, 0, &delegateCommissioner) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, delegateAccessory.mNumMessageSend == 1);
    NL_TEST_ASSERT(inSuite, delegateAccessory.mNumPairingComplete == 1);

    NL_TEST_ASSERT(inSuite, delegateCommissioner.mNumMessageSend == 2);
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
                       testPairingSession1->DeriveSecureSession(Uint8::from_const_char("abc"), 3, session1) == CHIP_NO_ERROR);

        NL_TEST_ASSERT(inSuite, session1.Encrypt(plain_text, sizeof(plain_text), encrypted, header, mac) == CHIP_NO_ERROR);
    }

    {
        SecureSession session2;
        NL_TEST_ASSERT(inSuite,
                       testPairingSession2->DeriveSecureSession(Uint8::from_const_char("abc"), 3, session2) == CHIP_NO_ERROR);

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
//
/**
 *  Set up the test suite.
 */
int CASE_TestSecurePairing_Setup(void * inContext)
{
    CHIP_ERROR error;
    CertificateKeyId trustedRootId = { .mId = sTestCert_Root01_SubjectKeyId, .mLen = sTestCert_Root01_SubjectKeyId_Len };

    error = chip::Platform::MemoryInit();
    SuccessOrExit(error);

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
    commissionerDevOpCred.Release();
    accessoryDevOpCred.Release();
    commissionerCertificateSet.Release();
    accessoryCertificateSet.Release();
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

// clang-format off
static nlTestSuite sSuite =
{
    "Test-CHIP-SecurePairing",
    &sTests[0],
    CASE_TestSecurePairing_Setup,
    CASE_TestSecurePairing_Teardown,
};
// clang-format on

/**
 *  Main
 */
int TestCASESession()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, nullptr);

    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestCASESession)
