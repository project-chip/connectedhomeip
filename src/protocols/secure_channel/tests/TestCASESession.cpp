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

#include <credentials/CHIPCert.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/UnitTestRegistration.h>
#include <messaging/tests/MessagingContext.h>
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
FabricTable gDeviceFabrics;
FabricIndex gDeviceFabricIndex;

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
    FabricInfo commissionerFabric;

    P256SerializedKeypair opKeysSerialized;
    memcpy((uint8_t *) (opKeysSerialized), sTestCert_Node01_01_PublicKey, sTestCert_Node01_01_PublicKey_Len);
    memcpy((uint8_t *) (opKeysSerialized) + sTestCert_Node01_01_PublicKey_Len, sTestCert_Node01_01_PrivateKey,
           sTestCert_Node01_01_PrivateKey_Len);

    ReturnErrorOnFailure(opKeysSerialized.SetLength(sTestCert_Node01_01_PublicKey_Len + sTestCert_Node01_01_PrivateKey_Len));

    P256Keypair opKey;
    ReturnErrorOnFailure(opKey.Deserialize(opKeysSerialized));
    ReturnErrorOnFailure(commissionerFabric.SetOperationalKeypair(&opKey));

    ReturnErrorOnFailure(commissionerFabric.SetRootCert(ByteSpan(sTestCert_Root01_Chip, sTestCert_Root01_Chip_Len)));
    ReturnErrorOnFailure(commissionerFabric.SetICACert(ByteSpan(sTestCert_ICA01_Chip, sTestCert_ICA01_Chip_Len)));
    ReturnErrorOnFailure(commissionerFabric.SetNOCCert(ByteSpan(sTestCert_Node01_01_Chip, sTestCert_Node01_01_Chip_Len)));

    ReturnErrorOnFailure(gCommissionerFabrics.AddNewFabric(commissionerFabric, &gCommissionerFabricIndex));

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

    return CHIP_NO_ERROR;
}

void CASE_SecurePairingWaitTest(nlTestSuite * inSuite, void * inContext)
{
    // Test all combinations of invalid parameters
    TestCASESecurePairingDelegate delegate;
    TestCASESessionIPK pairing;
    FabricTable fabrics;

    NL_TEST_ASSERT(inSuite, pairing.GetSecureSessionType() == SecureSession::Type::kCASE);
    CATValues peerCATs;
    peerCATs = pairing.GetPeerCATs();
    NL_TEST_ASSERT(inSuite, memcmp(&peerCATs, &kUndefinedCATs, sizeof(CATValues)) == 0);

    NL_TEST_ASSERT(inSuite, pairing.ListenForSessionEstablishment(0, nullptr, nullptr) == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite, pairing.ListenForSessionEstablishment(0, nullptr, &delegate) == CHIP_ERROR_INVALID_ARGUMENT);
    NL_TEST_ASSERT(inSuite, pairing.ListenForSessionEstablishment(0, &fabrics, &delegate) == CHIP_NO_ERROR);
}

void CASE_SecurePairingStartTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    // Test all combinations of invalid parameters
    TestCASESecurePairingDelegate delegate;
    CASESession pairing;
    FabricInfo * fabric = gCommissionerFabrics.FindFabricWithIndex(gCommissionerFabricIndex);
    NL_TEST_ASSERT(inSuite, fabric != nullptr);

    ExchangeContext * context = ctx.NewUnauthenticatedExchangeToBob(&pairing);

    NL_TEST_ASSERT(inSuite,
                   pairing.EstablishSession(Transport::PeerAddress(Transport::Type::kBle), nullptr, Node01_01, 0, nullptr,
                                            nullptr) != CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(inSuite,
                   pairing.EstablishSession(Transport::PeerAddress(Transport::Type::kBle), fabric, Node01_01, 0, nullptr,
                                            nullptr) != CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(inSuite,
                   pairing.EstablishSession(Transport::PeerAddress(Transport::Type::kBle), fabric, Node01_01, 0, context,
                                            &delegate) == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(inSuite, gLoopback.mSentMessageCount == 1);

    // Clear pending packet in CRMP
    ReliableMessageMgr * rm     = ctx.GetExchangeManager().GetReliableMessageMgr();
    ReliableMessageContext * rc = context->GetReliableMessageContext();
    rm->ClearRetransTable(rc);

    gLoopback.mMessageSendError = CHIP_ERROR_BAD_REQUEST;

    CASESession pairing1;

    gLoopback.mSentMessageCount = 0;
    gLoopback.mMessageSendError = CHIP_ERROR_BAD_REQUEST;
    ExchangeContext * context1  = ctx.NewUnauthenticatedExchangeToBob(&pairing1);

    NL_TEST_ASSERT(inSuite,
                   pairing1.EstablishSession(Transport::PeerAddress(Transport::Type::kBle), fabric, Node01_01, 0, context1,
                                             &delegate) == CHIP_ERROR_BAD_REQUEST);
    ctx.DrainAndServiceIO();

    gLoopback.mMessageSendError = CHIP_NO_ERROR;
}

void CASE_SecurePairingHandshakeTestCommon(nlTestSuite * inSuite, void * inContext, CASESession & pairingCommissioner,
                                           TestCASESecurePairingDelegate & delegateCommissioner)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    // Test all combinations of invalid parameters
    TestCASESecurePairingDelegate delegateAccessory;
    TestCASESessionIPK pairingAccessory;
    CASESessionCachable serializableCommissioner;
    CASESessionCachable serializableAccessory;

    gLoopback.mSentMessageCount = 0;

    NL_TEST_ASSERT(inSuite,
                   ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::CASE_Sigma1,
                                                                                     &pairingAccessory) == CHIP_NO_ERROR);

    ExchangeContext * contextCommissioner = ctx.NewUnauthenticatedExchangeToBob(&pairingCommissioner);

    FabricInfo * fabric = gCommissionerFabrics.FindFabricWithIndex(gCommissionerFabricIndex);
    NL_TEST_ASSERT(inSuite, fabric != nullptr);

    NL_TEST_ASSERT(inSuite,
                   pairingAccessory.ListenForSessionEstablishment(0, &gDeviceFabrics, &delegateAccessory) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite,
                   pairingCommissioner.EstablishSession(Transport::PeerAddress(Transport::Type::kBle), fabric, Node01_01, 0,
                                                        contextCommissioner, &delegateCommissioner) == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(inSuite, gLoopback.mSentMessageCount == 5);
    NL_TEST_ASSERT(inSuite, delegateAccessory.mNumPairingComplete == 1);
    NL_TEST_ASSERT(inSuite, delegateCommissioner.mNumPairingComplete == 1);

    NL_TEST_ASSERT(inSuite, pairingCommissioner.ToCachable(serializableCommissioner) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, pairingAccessory.ToCachable(serializableAccessory) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite,
                   memcmp(serializableCommissioner.mSharedSecret, serializableAccessory.mSharedSecret,
                          serializableCommissioner.mSharedSecretLen) == 0);
}

void CASE_SecurePairingHandshakeTest(nlTestSuite * inSuite, void * inContext)
{
    TestCASESecurePairingDelegate delegateCommissioner;
    TestCASESessionIPK pairingCommissioner;
    CASE_SecurePairingHandshakeTestCommon(inSuite, inContext, pairingCommissioner, delegateCommissioner);
}

class TestCASESessionPersistentStorageDelegate : public PersistentStorageDelegate, public FabricStorage
{
public:
    TestCASESessionPersistentStorageDelegate()
    {
        memset(keys, 0, sizeof(keys));
        memset(keysize, 0, sizeof(keysize));
        memset(values, 0, sizeof(values));
        memset(valuesize, 0, sizeof(valuesize));
    }

    ~TestCASESessionPersistentStorageDelegate() { Cleanup(); }

    void Cleanup()
    {
        for (int i = 0; i < 16; i++)
        {
            if (keys[i] != nullptr)
            {
                chip::Platform::MemoryFree(keys[i]);
                keys[i] = nullptr;
            }
            if (values[i] != nullptr)
            {
                chip::Platform::MemoryFree(values[i]);
                values[i] = nullptr;
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

    CHIP_ERROR SyncStore(FabricIndex fabricIndex, const char * key, const void * buffer, uint16_t size) override
    {
        return SyncSetKeyValue(key, buffer, size);
    };

    CHIP_ERROR SyncLoad(FabricIndex fabricIndex, const char * key, void * buffer, uint16_t & size) override
    {
        return SyncGetKeyValue(key, buffer, size);
    };

    CHIP_ERROR SyncDelete(FabricIndex fabricIndex, const char * key) override { return SyncDeleteKeyValue(key); };

private:
    char * keys[16];
    void * values[16];
    uint16_t keysize[16];
    uint16_t valuesize[16];
};

TestCASESessionPersistentStorageDelegate gCommissionerStorageDelegate;
TestCASESessionPersistentStorageDelegate gDeviceStorageDelegate;

TestCASEServerIPK gPairingServer;

void CASE_SecurePairingHandshakeServerTest(nlTestSuite * inSuite, void * inContext)
{
    TestCASESecurePairingDelegate delegateCommissioner;

    auto * pairingCommissioner = chip::Platform::New<TestCASESessionIPK>();

    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    gLoopback.mSentMessageCount = 0;

    SessionIDAllocator idAllocator;

    NL_TEST_ASSERT(inSuite,
                   gPairingServer.ListenForSessionEstablishment(&ctx.GetExchangeManager(), &ctx.GetTransportMgr(), nullptr,
                                                                &ctx.GetSecureSessionManager(), &gDeviceFabrics,
                                                                &idAllocator) == CHIP_NO_ERROR);

    ExchangeContext * contextCommissioner = ctx.NewUnauthenticatedExchangeToBob(pairingCommissioner);

    FabricInfo * fabric = gCommissionerFabrics.FindFabricWithIndex(gCommissionerFabricIndex);
    NL_TEST_ASSERT(inSuite, fabric != nullptr);

    NL_TEST_ASSERT(inSuite,
                   pairingCommissioner->EstablishSession(Transport::PeerAddress(Transport::Type::kBle), fabric, Node01_01, 0,
                                                         contextCommissioner, &delegateCommissioner) == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(inSuite, gLoopback.mSentMessageCount == 5);
    NL_TEST_ASSERT(inSuite, delegateCommissioner.mNumPairingComplete == 1);

    auto * pairingCommissioner1            = chip::Platform::New<TestCASESessionIPK>();
    ExchangeContext * contextCommissioner1 = ctx.NewUnauthenticatedExchangeToBob(pairingCommissioner1);

    NL_TEST_ASSERT(inSuite,
                   pairingCommissioner1->EstablishSession(Transport::PeerAddress(Transport::Type::kBle), fabric, Node01_01, 0,
                                                          contextCommissioner1, &delegateCommissioner) == CHIP_NO_ERROR);
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

    ReturnErrorOnFailure(ctx.Init());
    ctx.EnableAsyncDispatch();

    ctx.SetBobNodeId(kPlaceholderNodeId);
    ctx.SetAliceNodeId(kPlaceholderNodeId);
    ctx.SetBobKeyId(0);
    ctx.SetAliceKeyId(0);
    ctx.SetFabricIndex(kUndefinedFabricIndex);

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
    return CASETestSecurePairingSetup(inContext) == CHIP_NO_ERROR ? SUCCESS : FAILURE;
}

/**
 *  Tear down the test suite.
 */
int CASE_TestSecurePairing_Teardown(void * inContext)
{
    gCommissionerStorageDelegate.Cleanup();
    gDeviceStorageDelegate.Cleanup();
    gCommissionerFabrics.Reset();
    gDeviceFabrics.Reset();
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
