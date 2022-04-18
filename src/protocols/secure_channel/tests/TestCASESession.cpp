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

#include <credentials/CHIPCert.h>
#include <credentials/GroupDataProviderImpl.h>
#include <errno.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/UnitTestRegistration.h>
#include <messaging/tests/MessagingContext.h>
#include <nlunit-test.h>
#include <protocols/secure_channel/CASEServer.h>
#include <protocols/secure_channel/CASESession.h>
#include <stdarg.h>
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

using TestContext = Test::LoopbackMessagingContext<>;

namespace {
TestContext sContext;

auto & gLoopback = sContext.GetLoopback();

FabricTable gCommissionerFabrics;
FabricIndex gCommissionerFabricIndex;
GroupDataProviderImpl gCommissionerGroupDataProvider;
TestPersistentStorageDelegate gCommissionerStorageDelegate;

FabricTable gDeviceFabrics;
FabricIndex gDeviceFabricIndex;
GroupDataProviderImpl gDeviceGroupDataProvider;
TestPersistentStorageDelegate gDeviceStorageDelegate;

NodeId Node01_01 = 0xDEDEDEDE00010001;

class TestCASESecurePairingDelegate : public SessionEstablishmentDelegate
{
public:
    void OnSessionEstablishmentError(CHIP_ERROR error) override { mNumPairingErrors++; }

    void OnSessionEstablished() override { mNumPairingComplete++; }

    // TODO: Rename mNumPairing* to mNumEstablishment*
    uint32_t mNumPairingErrors   = 0;
    uint32_t mNumPairingComplete = 0;
};

class CASEServerForTest : public CASEServer
{
public:
    CASESession & GetSession() override { return mCaseSession; }

private:
    CASESession mCaseSession;
};

CHIP_ERROR InitTestIpk(GroupDataProvider & groupDataProvider, const FabricInfo & fabricInfo, size_t numIpks)
{
    VerifyOrReturnError((numIpks > 0) && (numIpks <= 3), CHIP_ERROR_INVALID_ARGUMENT);
    using KeySet         = chip::Credentials::GroupDataProvider::KeySet;
    using SecurityPolicy = chip::Credentials::GroupDataProvider::SecurityPolicy;

    KeySet ipkKeySet(GroupDataProvider::kIdentityProtectionKeySetId, SecurityPolicy::kTrustFirst, static_cast<uint8_t>(numIpks));

    for (size_t ipkIndex = 0; ipkIndex < numIpks; ++ipkIndex)
    {
        // Set start time to 0, 1000, 2000, etc
        ipkKeySet.epoch_keys[ipkIndex].start_time = static_cast<uint64_t>(ipkIndex * 1000);
        // Set IPK Epoch key to 00.....00, 01....01, 02.....02, etc
        memset(&ipkKeySet.epoch_keys[ipkIndex].key, static_cast<int>(ipkIndex), sizeof(ipkKeySet.epoch_keys[ipkIndex].key));
    }

    uint8_t compressedId[sizeof(uint64_t)];
    MutableByteSpan compressedIdSpan(compressedId);
    ReturnErrorOnFailure(fabricInfo.GetCompressedId(compressedIdSpan));
    return groupDataProvider.SetKeySet(fabricInfo.GetFabricIndex(), compressedIdSpan, ipkKeySet);
}

CHIP_ERROR InitCredentialSets()
{
    gCommissionerStorageDelegate.ClearStorage();
    gCommissionerGroupDataProvider.SetStorageDelegate(&gCommissionerStorageDelegate);
    ReturnErrorOnFailure(gCommissionerGroupDataProvider.Init());

    FabricInfo commissionerFabric;

    P256SerializedKeypair opKeysSerialized;
    // TODO: Rename gCommissioner* to gInitiator*
    memcpy((uint8_t *) (opKeysSerialized), sTestCert_Node01_02_PublicKey, sTestCert_Node01_02_PublicKey_Len);
    memcpy((uint8_t *) (opKeysSerialized) + sTestCert_Node01_02_PublicKey_Len, sTestCert_Node01_02_PrivateKey,
           sTestCert_Node01_02_PrivateKey_Len);

    ReturnErrorOnFailure(opKeysSerialized.SetLength(sTestCert_Node01_02_PublicKey_Len + sTestCert_Node01_02_PrivateKey_Len));

    P256Keypair opKey;
    ReturnErrorOnFailure(opKey.Deserialize(opKeysSerialized));
    ReturnErrorOnFailure(commissionerFabric.SetOperationalKeypair(&opKey));

    ReturnErrorOnFailure(commissionerFabric.SetRootCert(ByteSpan(sTestCert_Root01_Chip, sTestCert_Root01_Chip_Len)));
    ReturnErrorOnFailure(commissionerFabric.SetICACert(ByteSpan(sTestCert_ICA01_Chip, sTestCert_ICA01_Chip_Len)));
    ReturnErrorOnFailure(commissionerFabric.SetNOCCert(ByteSpan(sTestCert_Node01_02_Chip, sTestCert_Node01_02_Chip_Len)));

    ReturnErrorOnFailure(gCommissionerFabrics.AddNewFabric(commissionerFabric, &gCommissionerFabricIndex));

    FabricInfo * newFabric = gCommissionerFabrics.FindFabricWithIndex(gCommissionerFabricIndex);
    VerifyOrReturnError(newFabric != nullptr, CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(InitTestIpk(gCommissionerGroupDataProvider, *newFabric, /* numIpks= */ 1));

    gDeviceStorageDelegate.ClearStorage();
    gDeviceGroupDataProvider.SetStorageDelegate(&gDeviceStorageDelegate);
    ReturnErrorOnFailure(gDeviceGroupDataProvider.Init());
    FabricInfo deviceFabric;

    memcpy((uint8_t *) (opKeysSerialized), sTestCert_Node01_01_PublicKey, sTestCert_Node01_01_PublicKey_Len);
    memcpy((uint8_t *) (opKeysSerialized) + sTestCert_Node01_01_PublicKey_Len, sTestCert_Node01_01_PrivateKey,
           sTestCert_Node01_01_PrivateKey_Len);

    ReturnErrorOnFailure(opKeysSerialized.SetLength(sTestCert_Node01_01_PublicKey_Len + sTestCert_Node01_01_PrivateKey_Len));

    ReturnErrorOnFailure(opKey.Deserialize(opKeysSerialized));
    ReturnErrorOnFailure(deviceFabric.SetOperationalKeypair(&opKey));

    ReturnErrorOnFailure(deviceFabric.SetRootCert(ByteSpan(sTestCert_Root01_Chip, sTestCert_Root01_Chip_Len)));
    ReturnErrorOnFailure(deviceFabric.SetICACert(ByteSpan(sTestCert_ICA01_Chip, sTestCert_ICA01_Chip_Len)));
    ReturnErrorOnFailure(deviceFabric.SetNOCCert(ByteSpan(sTestCert_Node01_01_Chip, sTestCert_Node01_01_Chip_Len)));

    ReturnErrorOnFailure(gDeviceFabrics.AddNewFabric(deviceFabric, &gDeviceFabricIndex));

    // TODO: Validate more cases of number of IPKs on both sides
    newFabric = gDeviceFabrics.FindFabricWithIndex(gDeviceFabricIndex);
    VerifyOrReturnError(newFabric != nullptr, CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(InitTestIpk(gDeviceGroupDataProvider, *newFabric, /* numIpks= */ 1));

    return CHIP_NO_ERROR;
}

} // namespace

void CASE_SecurePairingWaitTest(nlTestSuite * inSuite, void * inContext)
{
    // Test all combinations of invalid parameters
    TestCASESecurePairingDelegate delegate;
    CASESession pairing;
    FabricTable fabrics;
    SessionManager sessionManager;

    NL_TEST_ASSERT(inSuite, pairing.GetSecureSessionType() == SecureSession::Type::kCASE);
    CATValues peerCATs;
    peerCATs = pairing.GetPeerCATs();
    NL_TEST_ASSERT(inSuite, memcmp(&peerCATs, &kUndefinedCATs, sizeof(CATValues)) == 0);

    pairing.SetGroupDataProvider(&gDeviceGroupDataProvider);
    NL_TEST_ASSERT(inSuite,
                   pairing.ListenForSessionEstablishment(sessionManager, nullptr, nullptr, nullptr) == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(
        inSuite, pairing.ListenForSessionEstablishment(sessionManager, nullptr, nullptr, &delegate) == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite, pairing.ListenForSessionEstablishment(sessionManager, &fabrics, nullptr, &delegate) == CHIP_NO_ERROR);
}

void CASE_SecurePairingStartTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    // Test all combinations of invalid parameters
    TestCASESecurePairingDelegate delegate;
    CASESession pairing;
    pairing.SetGroupDataProvider(&gCommissionerGroupDataProvider);

    FabricInfo * fabric = gCommissionerFabrics.FindFabricWithIndex(gCommissionerFabricIndex);
    NL_TEST_ASSERT(inSuite, fabric != nullptr);
    SessionManager sessionManager;

    ExchangeContext * context = ctx.NewUnauthenticatedExchangeToBob(&pairing);

    NL_TEST_ASSERT(inSuite,
                   pairing.EstablishSession(sessionManager, Transport::PeerAddress(Transport::Type::kBle), nullptr, Node01_01,
                                            nullptr, nullptr, nullptr) != CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(inSuite,
                   pairing.EstablishSession(sessionManager, Transport::PeerAddress(Transport::Type::kBle), fabric, Node01_01,
                                            nullptr, nullptr, nullptr) != CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(inSuite,
                   pairing.EstablishSession(sessionManager, Transport::PeerAddress(Transport::Type::kBle), fabric, Node01_01,
                                            context, nullptr, &delegate) == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(inSuite, gLoopback.mSentMessageCount == 1);

    // Clear pending packet in CRMP
    ReliableMessageMgr * rm     = ctx.GetExchangeManager().GetReliableMessageMgr();
    ReliableMessageContext * rc = context->GetReliableMessageContext();
    rm->ClearRetransTable(rc);

    gLoopback.mMessageSendError = CHIP_ERROR_BAD_REQUEST;

    CASESession pairing1;
    pairing1.SetGroupDataProvider(&gCommissionerGroupDataProvider);

    gLoopback.mSentMessageCount = 0;
    gLoopback.mMessageSendError = CHIP_ERROR_BAD_REQUEST;
    ExchangeContext * context1  = ctx.NewUnauthenticatedExchangeToBob(&pairing1);

    NL_TEST_ASSERT(inSuite,
                   pairing1.EstablishSession(sessionManager, Transport::PeerAddress(Transport::Type::kBle), fabric, Node01_01,
                                             context1, nullptr, &delegate) == CHIP_ERROR_BAD_REQUEST);
    ctx.DrainAndServiceIO();

    gLoopback.mMessageSendError = CHIP_NO_ERROR;
}

void CASE_SecurePairingHandshakeTestCommon(nlTestSuite * inSuite, void * inContext, CASESession & pairingCommissioner,
                                           TestCASESecurePairingDelegate & delegateCommissioner)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    // Test all combinations of invalid parameters
    TestCASESecurePairingDelegate delegateAccessory;
    CASESession pairingAccessory;
    SessionManager sessionManager;

    gLoopback.mSentMessageCount = 0;

    NL_TEST_ASSERT(inSuite,
                   ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::CASE_Sigma1,
                                                                                     &pairingAccessory) == CHIP_NO_ERROR);

    ExchangeContext * contextCommissioner = ctx.NewUnauthenticatedExchangeToBob(&pairingCommissioner);

    FabricInfo * fabric = gCommissionerFabrics.FindFabricWithIndex(gCommissionerFabricIndex);
    NL_TEST_ASSERT(inSuite, fabric != nullptr);

    pairingAccessory.SetGroupDataProvider(&gDeviceGroupDataProvider);
    NL_TEST_ASSERT(inSuite,
                   pairingAccessory.ListenForSessionEstablishment(sessionManager, &gDeviceFabrics, nullptr, &delegateAccessory) ==
                       CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite,
                   pairingCommissioner.EstablishSession(sessionManager, Transport::PeerAddress(Transport::Type::kBle), fabric,
                                                        Node01_01, contextCommissioner, nullptr,
                                                        &delegateCommissioner) == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(inSuite, gLoopback.mSentMessageCount == 5);
    NL_TEST_ASSERT(inSuite, delegateAccessory.mNumPairingComplete == 1);
    NL_TEST_ASSERT(inSuite, delegateCommissioner.mNumPairingComplete == 1);
}

void CASE_SecurePairingHandshakeTest(nlTestSuite * inSuite, void * inContext)
{
    TestCASESecurePairingDelegate delegateCommissioner;
    CASESession pairingCommissioner;
    pairingCommissioner.SetGroupDataProvider(&gCommissionerGroupDataProvider);
    CASE_SecurePairingHandshakeTestCommon(inSuite, inContext, pairingCommissioner, delegateCommissioner);
}

CASEServerForTest gPairingServer;

void CASE_SecurePairingHandshakeServerTest(nlTestSuite * inSuite, void * inContext)
{
    // TODO: Add cases for mismatching IPK config between initiator/responder

    TestCASESecurePairingDelegate delegateCommissioner;

    auto * pairingCommissioner = chip::Platform::New<CASESession>();
    pairingCommissioner->SetGroupDataProvider(&gCommissionerGroupDataProvider);

    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    gLoopback.mSentMessageCount = 0;

    // Use the same session manager on both CASE client and server sides to validate that both
    // components may work simultaneously on a single device.
    NL_TEST_ASSERT(inSuite,
                   gPairingServer.ListenForSessionEstablishment(&ctx.GetExchangeManager(), &ctx.GetTransportMgr(),
                                                                &ctx.GetSecureSessionManager(), &gDeviceFabrics, nullptr,
                                                                &gDeviceGroupDataProvider) == CHIP_NO_ERROR);

    ExchangeContext * contextCommissioner = ctx.NewUnauthenticatedExchangeToBob(pairingCommissioner);

    FabricInfo * fabric = gCommissionerFabrics.FindFabricWithIndex(gCommissionerFabricIndex);
    NL_TEST_ASSERT(inSuite, fabric != nullptr);

    NL_TEST_ASSERT(inSuite,
                   pairingCommissioner->EstablishSession(ctx.GetSecureSessionManager(),
                                                         Transport::PeerAddress(Transport::Type::kBle), fabric, Node01_01,
                                                         contextCommissioner, nullptr, &delegateCommissioner) == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(inSuite, gLoopback.mSentMessageCount == 5);
    NL_TEST_ASSERT(inSuite, delegateCommissioner.mNumPairingComplete == 1);

    // Validate that secure session can be created after the pairing
    SessionHolder sessionHolder;
    NL_TEST_ASSERT(inSuite,
                   ctx.GetSecureSessionManager().NewPairing(sessionHolder, NullOptional, Node01_01, pairingCommissioner,
                                                            CryptoContext::SessionRole::kInitiator,
                                                            gCommissionerFabricIndex) == CHIP_NO_ERROR);

    auto * pairingCommissioner1 = chip::Platform::New<CASESession>();
    pairingCommissioner1->SetGroupDataProvider(&gCommissionerGroupDataProvider);
    ExchangeContext * contextCommissioner1 = ctx.NewUnauthenticatedExchangeToBob(pairingCommissioner1);

    NL_TEST_ASSERT(inSuite,
                   pairingCommissioner1->EstablishSession(ctx.GetSecureSessionManager(),
                                                          Transport::PeerAddress(Transport::Type::kBle), fabric, Node01_01,
                                                          contextCommissioner1, nullptr, &delegateCommissioner) == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    chip::Platform::Delete(pairingCommissioner);
    chip::Platform::Delete(pairingCommissioner1);
}

struct Sigma1Params
{
    // Purposefully not using constants like kSigmaParamRandomNumberSize that
    // the code uses, so we have a cross-check.
    static constexpr size_t initiatorRandomLen    = 32;
    static constexpr uint16_t initiatorSessionId  = 0;
    static constexpr size_t destinationIdLen      = 32;
    static constexpr size_t initiatorEphPubKeyLen = 65;
    static constexpr size_t resumptionIdLen       = 0; // Nonzero means include it.
    static constexpr size_t initiatorResumeMICLen = 0; // Nonzero means include it.

    static constexpr uint8_t initiatorRandomTag    = 1;
    static constexpr uint8_t initiatorSessionIdTag = 2;
    static constexpr uint8_t destinationIdTag      = 3;
    static constexpr uint8_t initiatorEphPubKeyTag = 4;
    static constexpr uint8_t resumptionIdTag       = 6;
    static constexpr uint8_t initiatorResumeMICTag = 7;
    static constexpr TLV::Tag NumToTag(uint8_t num) { return TLV::ContextTag(num); }

    static constexpr bool includeStructEnd = true;

    static constexpr bool expectSuccess = true;
};

void CASE_DestinationIdTest(nlTestSuite * inSuite, void * inContext)
{
    // Validate example test vector from CASE section of spec

    const uint8_t kRootPubKeyFromSpec[Crypto::CHIP_CRYPTO_PUBLIC_KEY_SIZE_BYTES] = {
        0x04, 0x4a, 0x9f, 0x42, 0xb1, 0xca, 0x48, 0x40, 0xd3, 0x72, 0x92, 0xbb, 0xc7, 0xf6, 0xa7, 0xe1, 0x1e,
        0x22, 0x20, 0x0c, 0x97, 0x6f, 0xc9, 0x00, 0xdb, 0xc9, 0x8a, 0x7a, 0x38, 0x3a, 0x64, 0x1c, 0xb8, 0x25,
        0x4a, 0x2e, 0x56, 0xd4, 0xe2, 0x95, 0xa8, 0x47, 0x94, 0x3b, 0x4e, 0x38, 0x97, 0xc4, 0xa7, 0x73, 0xe9,
        0x30, 0x27, 0x7b, 0x4d, 0x9f, 0xbe, 0xde, 0x8a, 0x05, 0x26, 0x86, 0xbf, 0xac, 0xfa
    };

    const uint8_t kIpkOperationalGroupKeyFromSpec[Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES] = {
        0x9b, 0xc6, 0x1c, 0xd9, 0xc6, 0x2a, 0x2d, 0xf6, 0xd6, 0x4d, 0xfc, 0xaa, 0x9d, 0xc4, 0x72, 0xd4
    };

    const uint8_t kInitiatorRandomFromSpec[Sigma1Params::initiatorRandomLen] = { 0x7e, 0x17, 0x12, 0x31, 0x56, 0x8d, 0xfa, 0x17,
                                                                                 0x20, 0x6b, 0x3a, 0xcc, 0xf8, 0xfa, 0xec, 0x2f,
                                                                                 0x4d, 0x21, 0xb5, 0x80, 0x11, 0x31, 0x96, 0xf4,
                                                                                 0x7c, 0x7c, 0x4d, 0xeb, 0x81, 0x0a, 0x73, 0xdc };

    const uint8_t kExpectedDestinationIdFromSpec[Crypto::kSHA256_Hash_Length] = { 0xdc, 0x35, 0xdd, 0x5f, 0xc9, 0x13, 0x4c, 0xc5,
                                                                                  0x54, 0x45, 0x38, 0xc9, 0xc3, 0xfc, 0x42, 0x97,
                                                                                  0xc1, 0xec, 0x33, 0x70, 0xc8, 0x39, 0x13, 0x6a,
                                                                                  0x80, 0xe1, 0x07, 0x96, 0x45, 0x1d, 0x4c, 0x53 };

    const FabricId kFabricIdFromSpec = 0x2906C908D115D362;
    const NodeId kNodeIdFromSpec     = 0xCD5544AA7B13EF14;

    uint8_t destinationIdBuf[Crypto::kSHA256_Hash_Length] = { 0 };
    MutableByteSpan destinationIdSpan(destinationIdBuf);

    // Test exact example
    CHIP_ERROR err =
        GenerateCaseDestinationId(ByteSpan(kIpkOperationalGroupKeyFromSpec), ByteSpan(kInitiatorRandomFromSpec),
                                  ByteSpan(kRootPubKeyFromSpec), kFabricIdFromSpec, kNodeIdFromSpec, destinationIdSpan);
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(inSuite, destinationIdSpan.size() == sizeof(destinationIdBuf));
    NL_TEST_ASSERT(inSuite, destinationIdSpan.data_equal(ByteSpan(kExpectedDestinationIdFromSpec)));

    memset(destinationIdSpan.data(), 0, destinationIdSpan.size());

    // Test changing input: should yield different
    err = GenerateCaseDestinationId(ByteSpan(kIpkOperationalGroupKeyFromSpec), ByteSpan(kInitiatorRandomFromSpec),
                                    ByteSpan(kRootPubKeyFromSpec), kFabricIdFromSpec,
                                    kNodeIdFromSpec + 1, // <--- Change node ID
                                    destinationIdSpan);
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(inSuite, destinationIdSpan.size() == sizeof(destinationIdBuf));
    NL_TEST_ASSERT(inSuite, !destinationIdSpan.data_equal(ByteSpan(kExpectedDestinationIdFromSpec)));
}

template <typename Params>
static CHIP_ERROR EncodeSigma1(MutableByteSpan & buf)
{
    using namespace TLV;

    TLVWriter writer;
    writer.Init(buf);

    TLVType containerType;
    ReturnErrorOnFailure(writer.StartContainer(AnonymousTag(), kTLVType_Structure, containerType));
    uint8_t initiatorRandom[Params::initiatorRandomLen] = { 1 };
    ReturnErrorOnFailure(writer.Put(Params::NumToTag(Params::initiatorRandomTag), ByteSpan(initiatorRandom)));

    ReturnErrorOnFailure(writer.Put(Params::NumToTag(Params::initiatorSessionIdTag), Params::initiatorSessionId));

    uint8_t destinationId[Params::destinationIdLen] = { 2 };
    ReturnErrorOnFailure(writer.Put(Params::NumToTag(Params::destinationIdTag), ByteSpan(destinationId)));

    uint8_t initiatorEphPubKey[Params::initiatorEphPubKeyLen] = { 3 };
    ReturnErrorOnFailure(writer.Put(Params::NumToTag(Params::initiatorEphPubKeyTag), ByteSpan(initiatorEphPubKey)));

    // I wish we had "if constexpr" support here, so the compiler would know
    // resumptionIdLen is nonzero inside the block....
    if (Params::resumptionIdLen != 0)
    {
        uint8_t resumptionId[Params::resumptionIdLen];

        // to fix _FORTIFY_SOURCE issue, _FORTIFY_SOURCE=2 by default on Android
        (&memset)(resumptionId, 4, Params::resumptionIdLen);
        ReturnErrorOnFailure(
            writer.Put(Params::NumToTag(Params::resumptionIdTag), ByteSpan(resumptionId, Params::resumptionIdLen)));
    }

    if (Params::initiatorResumeMICLen != 0)
    {
        uint8_t initiatorResumeMIC[Params::initiatorResumeMICLen];
        // to fix _FORTIFY_SOURCE issue, _FORTIFY_SOURCE=2 by default on Android
        (&memset)(initiatorResumeMIC, 5, Params::initiatorResumeMICLen);
        ReturnErrorOnFailure(writer.Put(Params::NumToTag(Params::initiatorResumeMICTag),
                                        ByteSpan(initiatorResumeMIC, Params::initiatorResumeMICLen)));
    }

    if (Params::includeStructEnd)
    {
        ReturnErrorOnFailure(writer.EndContainer(containerType));
    }

    buf.reduce_size(writer.GetLengthWritten());
    return CHIP_NO_ERROR;
}

// A macro, so we can tell which test failed based on line number.
#define TestSigma1Parsing(inSuite, mem, bufferSize, params)                                                                        \
    do                                                                                                                             \
    {                                                                                                                              \
        MutableByteSpan buf(mem.Get(), bufferSize);                                                                                \
        CHIP_ERROR err = EncodeSigma1<params>(buf);                                                                                \
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);                                                                             \
                                                                                                                                   \
        TLV::ContiguousBufferTLVReader reader;                                                                                     \
        reader.Init(buf);                                                                                                          \
                                                                                                                                   \
        ByteSpan initiatorRandom;                                                                                                  \
        uint16_t initiatorSessionId;                                                                                               \
        ByteSpan destinationId;                                                                                                    \
        ByteSpan initiatorEphPubKey;                                                                                               \
        bool resumptionRequested;                                                                                                  \
        ByteSpan resumptionId;                                                                                                     \
        ByteSpan initiatorResumeMIC;                                                                                               \
        CASESession session;                                                                                                       \
        err = session.ParseSigma1(reader, initiatorRandom, initiatorSessionId, destinationId, initiatorEphPubKey,                  \
                                  resumptionRequested, resumptionId, initiatorResumeMIC);                                          \
        NL_TEST_ASSERT(inSuite, (err == CHIP_NO_ERROR) == params::expectSuccess);                                                  \
        if (params::expectSuccess)                                                                                                 \
        {                                                                                                                          \
            NL_TEST_ASSERT(inSuite, resumptionRequested == (params::resumptionIdLen != 0 && params::initiatorResumeMICLen != 0));  \
            /* Add other verification tests here as desired */                                                                     \
        }                                                                                                                          \
    } while (0)

struct BadSigma1ParamsBase : public Sigma1Params
{
    static constexpr bool expectSuccess = false;
};

struct Sigma1NoStructEnd : public BadSigma1ParamsBase
{
    static constexpr bool includeStructEnd = false;
};

struct Sigma1WrongTags : public BadSigma1ParamsBase
{
    static constexpr TLV::Tag NumToTag(uint8_t num) { return TLV::ProfileTag(0, num); }
};

struct Sigma1TooLongRandom : public BadSigma1ParamsBase
{
    static constexpr size_t initiatorRandomLen = 33;
};

struct Sigma1TooShortRandom : public BadSigma1ParamsBase
{
    static constexpr size_t initiatorRandomLen = 31;
};

struct Sigma1TooLongDest : public BadSigma1ParamsBase
{
    static constexpr size_t destinationIdLen = 33;
};

struct Sigma1TooShortDest : public BadSigma1ParamsBase
{
    static constexpr size_t destinationIdLen = 31;
};

struct Sigma1TooLongPubkey : public BadSigma1ParamsBase
{
    static constexpr size_t initiatorEphPubKeyLen = 66;
};

struct Sigma1TooShortPubkey : public BadSigma1ParamsBase
{
    static constexpr size_t initiatorEphPubKeyLen = 64;
};

struct Sigma1WithResumption : public Sigma1Params
{
    static constexpr size_t resumptionIdLen       = 16;
    static constexpr size_t initiatorResumeMICLen = 16;
};

struct Sigma1TooLongResumptionId : public Sigma1WithResumption
{
    static constexpr size_t resumptionIdLen = 17;
    static constexpr bool expectSuccess     = false;
};

struct Sigma1TooShortResumptionId : public BadSigma1ParamsBase
{
    static constexpr size_t resumptionIdLen = 15;
    static constexpr bool expectSuccess     = false;
};

struct Sigma1TooLongResumeMIC : public Sigma1WithResumption
{
    static constexpr size_t resumptionIdLen = 17;
    static constexpr bool expectSuccess     = false;
};

struct Sigma1TooShortResumeMIC : public Sigma1WithResumption
{
    static constexpr size_t initiatorResumeMICLen = 15;
    static constexpr bool expectSuccess           = false;
};

struct Sigma1SessionIdMax : public Sigma1Params
{
    static constexpr uint32_t initiatorSessionId = UINT16_MAX;
};

struct Sigma1SessionIdTooBig : public BadSigma1ParamsBase
{
    static constexpr uint32_t initiatorSessionId = UINT16_MAX + 1;
};

static void CASE_Sigma1ParsingTest(nlTestSuite * inSuite, void * inContext)
{
    // 1280 bytes must be enough by definition.
    constexpr size_t bufferSize = 1280;
    chip::Platform::ScopedMemoryBuffer<uint8_t> mem;
    NL_TEST_ASSERT(inSuite, mem.Calloc(bufferSize));

    TestSigma1Parsing(inSuite, mem, bufferSize, Sigma1Params);

    TestSigma1Parsing(inSuite, mem, bufferSize, Sigma1NoStructEnd);
    TestSigma1Parsing(inSuite, mem, bufferSize, Sigma1WrongTags);
    TestSigma1Parsing(inSuite, mem, bufferSize, Sigma1TooLongRandom);
    TestSigma1Parsing(inSuite, mem, bufferSize, Sigma1TooShortRandom);
    TestSigma1Parsing(inSuite, mem, bufferSize, Sigma1TooLongDest);
    TestSigma1Parsing(inSuite, mem, bufferSize, Sigma1TooShortDest);
    TestSigma1Parsing(inSuite, mem, bufferSize, Sigma1TooLongPubkey);
    TestSigma1Parsing(inSuite, mem, bufferSize, Sigma1TooShortPubkey);
    TestSigma1Parsing(inSuite, mem, bufferSize, Sigma1WithResumption);
    TestSigma1Parsing(inSuite, mem, bufferSize, Sigma1TooLongResumptionId);
    TestSigma1Parsing(inSuite, mem, bufferSize, Sigma1TooShortResumptionId);
    TestSigma1Parsing(inSuite, mem, bufferSize, Sigma1TooLongResumeMIC);
    TestSigma1Parsing(inSuite, mem, bufferSize, Sigma1TooShortResumeMIC);
    TestSigma1Parsing(inSuite, mem, bufferSize, Sigma1SessionIdMax);
    TestSigma1Parsing(inSuite, mem, bufferSize, Sigma1SessionIdTooBig);
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
    NL_TEST_DEF("Sigma1Parsing", CASE_Sigma1ParsingTest),
    NL_TEST_DEF("DestinationId", CASE_DestinationIdTest),

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

namespace {
/*
 *  Set up the test suite.
 */
CHIP_ERROR CASETestSecurePairingSetup(void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    ctx.ConfigInitializeNodes(false);
    ReturnErrorOnFailure(ctx.Init());
    ctx.EnableAsyncDispatch();

    gCommissionerFabrics.Init(&gCommissionerStorageDelegate);
    gDeviceFabrics.Init(&gDeviceStorageDelegate);

    return InitCredentialSets();
}
} // anonymous namespace

/**
 *  Set up the test suite.
 */
int CASE_TestSecurePairing_Setup(void * inContext)
{
    CHIP_ERROR err = CASETestSecurePairingSetup(inContext);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Support, "Failed to init tests %" CHIP_ERROR_FORMAT, err.Format());
        return FAILURE;
    }
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
int CASE_TestSecurePairing_Teardown(void * inContext)
{
    gCommissionerStorageDelegate.ClearStorage();
    gDeviceStorageDelegate.ClearStorage();
    gCommissionerFabrics.DeleteAllFabrics();
    gDeviceFabrics.DeleteAllFabrics();
    static_cast<TestContext *>(inContext)->Shutdown();
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
