#include <cstddef>
#include <cstdint>
#include <stdarg.h>
#include <string.h>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include <app/icd/server/ICDServerConfig.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestUtils.h>
#include <messaging/tests/MessagingContext.h>
#include <protocols/secure_channel/PASESession.h>
#include <system/TLVPacketBufferBackingStore.h>

namespace chip {
namespace Testing {

using namespace std;

using namespace Crypto;
using namespace fuzztest;
using namespace Transport;
using namespace Messaging;
using namespace System::Clock::Literals;

// TODO: #35369 Refactor the classes below to Fixtures once Errors related to FuzzTest Fixtures are resolved
class FuzzLoopbackMessagingContext : public chip::Test::MessagingContext
{
public:
    ~FuzzLoopbackMessagingContext() {}

    // These functions wrap spLoopbackTransportManager methods
    static auto & GetSystemLayer() { return spLoopbackTransportManager->GetSystemLayer(); }
    static auto & GetLoopback() { return spLoopbackTransportManager->GetLoopback(); }
    static auto & GetTransportMgr() { return spLoopbackTransportManager->GetTransportMgr(); }
    static auto & GetIOContext() { return spLoopbackTransportManager->GetIOContext(); }

    template <typename... Ts>
    static void DrainAndServiceIO(Ts... args)
    {
        return spLoopbackTransportManager->DrainAndServiceIO(args...);
    }

    // Performs shared setup for all tests in the test suite
    static void SetUpTestSuite()
    {
        // Initialize memory.
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
        // Instantiate the LoopbackTransportManager.
        ASSERT_EQ(spLoopbackTransportManager, nullptr);
        spLoopbackTransportManager = new chip::Test::LoopbackTransportManager();
        ASSERT_NE(spLoopbackTransportManager, nullptr);
        // Initialize the LoopbackTransportManager.
        ASSERT_EQ(spLoopbackTransportManager->Init(), CHIP_NO_ERROR);
    }

    // Performs shared teardown for all tests in the test suite
    static void TearDownTestSuite()
    {
        // Shutdown the LoopbackTransportManager.
        spLoopbackTransportManager->Shutdown();
        // Destroy the LoopbackTransportManager.
        if (spLoopbackTransportManager != nullptr)
        {
            delete spLoopbackTransportManager;
            spLoopbackTransportManager = nullptr;
        }
        // Shutdown memory.
        chip::Platform::MemoryShutdown();
    }

    // Performs setup for each individual test in the test suite
    void SetUp() { ASSERT_EQ(MessagingContext::Init(&GetTransportMgr(), &GetIOContext()), CHIP_NO_ERROR); }

    // Performs teardown for each individual test in the test suite
    void TearDown() { MessagingContext::Shutdown(); }

    static chip::Test::LoopbackTransportManager * spLoopbackTransportManager;
};
chip::Test::LoopbackTransportManager * FuzzLoopbackMessagingContext::spLoopbackTransportManager = nullptr;

class TestSecurePairingDelegate : public SessionEstablishmentDelegate
{
public:
    void OnSessionEstablishmentError(CHIP_ERROR error) override { mNumPairingErrors++; }

    void OnSessionEstablished(const SessionHandle & session) override { mNumPairingComplete++; }

    uint32_t mNumPairingErrors   = 0;
    uint32_t mNumPairingComplete = 0;
};

class TestPASESession : public FuzzLoopbackMessagingContext
{
public:
    TestPASESession()
    {
        ConfigInitializeNodes(false);
        FuzzLoopbackMessagingContext::SetUpTestSuite();
        FuzzLoopbackMessagingContext::SetUp();
    }
    ~TestPASESession()
    {
        FuzzLoopbackMessagingContext::TearDown();
        FuzzLoopbackMessagingContext::TearDownTestSuite();
    }

    void SecurePairingHandshake(SessionManager & sessionManager, PASESession & pairingCommissioner,
                                TestSecurePairingDelegate & delegateCommissioner, TestSecurePairingDelegate & delegateAccessory,
                                const Spake2pVerifier & verifier, uint32_t pbkdf2IterCount, const ByteSpan & salt,
                                uint32_t SetUpPINCode);

    void FuzzHandlePBKDFParamRequest(vector<uint8_t> fuzzPBKDFLocalRandomData, uint32_t fuzzInitiatorSessionId,
                                     bool fuzzHavePBKDFParameters);

    void FuzzHandlePBKDFParamResponse(vector<uint8_t> fuzzPBKDFLocalRandomDataInitiator,
                                      vector<uint8_t> fuzzPBKDFLocalRandomDataResponder, uint32_t fuzzResponderSessionId,
                                      const vector<uint8_t> & fuzzedSalt, uint32_t fuzzedPBKDF2Iter, bool fuzzHavePBKDFParameters);

    void FuzzHandlePake1(const uint32_t fuzzedSetupPasscode, const vector<uint8_t> & fuzzedSalt, uint32_t fuzzedPBKDF2Iter,
                         const vector<uint8_t> & pA);

    void FuzzHandlePake2(const uint32_t fuzzedSetupPasscode, const vector<uint8_t> & fuzzedSalt, uint32_t fuzzedPBKDF2Iter,
                         const vector<uint8_t> & pB, const vector<uint8_t> & cB);

    void FuzzHandlePake3(const uint32_t fuzzedSetupPasscode, const vector<uint8_t> & fuzzedSalt, uint32_t fuzzedPBKDF2Iter,
                         const vector<uint8_t> & cA);

    CHIP_ERROR CallAllocateSecureSession(SessionManager & sessionManager, PASESession & pairing)
    {
        return pairing.AllocateSecureSession(sessionManager);
    }
};

class TemporarySessionManager
{
public:
    TemporarySessionManager(TestPASESession & ctx) : mCtx(ctx)
    {
        EXPECT_EQ(CHIP_NO_ERROR,
                  mSessionManager.Init(&ctx.GetSystemLayer(), &ctx.GetTransportMgr(), &ctx.GetMessageCounterManager(), &mStorage,
                                       &ctx.GetFabricTable(), ctx.GetSessionKeystore()));
        // The setup here is really weird: we are using one session manager for
        // the actual messages we send (the PASE handshake, so the
        // unauthenticated sessions) and a different one for allocating the PASE
        // sessions.  Since our Init() set us up as the thing to handle messages
        // on the transport manager, undo that.
        mCtx.GetTransportMgr().SetSessionManager(&mCtx.GetSecureSessionManager());
    }

    ~TemporarySessionManager()
    {
        mSessionManager.Shutdown();
        // Reset the session manager on the transport again, just in case
        // shutdown messed with it.
        mCtx.GetTransportMgr().SetSessionManager(&mCtx.GetSecureSessionManager());
    }

    operator SessionManager &() { return mSessionManager; }

private:
    TestPASESession & mCtx;
    TestPersistentStorageDelegate mStorage;
    SessionManager mSessionManager;
};

class PASETestLoopbackTransportDelegate : public Test::LoopbackTransportDelegate
{
public:
    void OnMessageDropped() override { mMessageDropped = true; }
    bool mMessageDropped = false;
};

void TestPASESession::SecurePairingHandshake(SessionManager & sessionManager, PASESession & pairingCommissioner,
                                             TestSecurePairingDelegate & delegateCommissioner,
                                             TestSecurePairingDelegate & delegateAccessory, const Spake2pVerifier & verifier,
                                             uint32_t pbkdf2IterCount, const ByteSpan & salt, uint32_t SetUpPINCode)
{

    PASESession pairingAccessory;

    PASETestLoopbackTransportDelegate delegate;
    auto & loopback = GetLoopback();
    loopback.SetLoopbackTransportDelegate(&delegate);
    loopback.mSentMessageCount = 0;

    ExchangeContext * contextCommissioner = NewUnauthenticatedExchangeToBob(&pairingCommissioner);

    EXPECT_EQ(GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::PBKDFParamRequest,
                                                                            &pairingAccessory),
              CHIP_NO_ERROR);

    pairingAccessory.WaitForPairing(sessionManager, verifier, pbkdf2IterCount, salt,
                                    Optional<ReliableMessageProtocolConfig>::Missing(), &delegateAccessory);
    DrainAndServiceIO();

    pairingCommissioner.Pair(sessionManager, SetUpPINCode, Optional<ReliableMessageProtocolConfig>::Missing(), contextCommissioner,
                             &delegateCommissioner);

    DrainAndServiceIO();
}

//----------------------------------------**********Fuzz Tests*********------------------------------------------------

// This Fuzz Test should always result in Successful PASE Pairing, since all fuzzed inputs are within the valid bounds
void PASESession_Bounded(const uint32_t fuzzedSetupPasscode, const vector<uint8_t> & fuzzedSalt, uint32_t fuzzedPBKDF2Iter)
{

    Spake2pVerifier fuzzedSpake2pVerifier;
    ByteSpan fuzzedSaltSpan{ fuzzedSalt.data(), fuzzedSalt.size() };

    // Generating the Spake2+ verifier from the fuzzed inputs
    EXPECT_EQ(fuzzedSpake2pVerifier.Generate(fuzzedPBKDF2Iter, fuzzedSaltSpan, fuzzedSetupPasscode), CHIP_NO_ERROR);

    // TODO: #35369 Move this to a Fixture once Errors related to FuzzTest Fixtures are resolved
    TestPASESession PASELoopBack;
    TemporarySessionManager sessionManager(PASELoopBack);

    PASESession pairingCommissioner;

    TestSecurePairingDelegate delegateCommissioner;
    TestSecurePairingDelegate delegateCommissionee;

    PASELoopBack.SecurePairingHandshake(sessionManager, pairingCommissioner, delegateCommissioner, delegateCommissionee,
                                        fuzzedSpake2pVerifier, fuzzedPBKDF2Iter, fuzzedSaltSpan, fuzzedSetupPasscode);

    // Given that the inputs to this Fuzz Test are within the expected boundaries, the Pairing should Always be successful.
    EXPECT_EQ(delegateCommissionee.mNumPairingComplete, 1u);
    EXPECT_EQ(delegateCommissioner.mNumPairingComplete, 1u);

    EXPECT_EQ(delegateCommissionee.mNumPairingErrors, 0u);
    EXPECT_EQ(delegateCommissioner.mNumPairingErrors, 0u);
}

FUZZ_TEST(FuzzPASE_PW, PASESession_Bounded)
    .WithDomains(
        // fuzzedSetupPasscode: Tests the full 27-bit range allowed by the stack (0 to 0x7FFFFFF)
        InRange(00000000, 0x7FFFFFF),
        // fuzzedSalt
        Arbitrary<vector<uint8_t>>().WithMinSize(kSpake2p_Min_PBKDF_Salt_Length).WithMaxSize(kSpake2p_Max_PBKDF_Salt_Length),
        // fuzzedPBKDF2Iter
        InRange(kSpake2p_Min_PBKDF_Iterations, kSpake2p_Max_PBKDF_Iterations));

/* -------------------------------------------------------------------------------------------*/
// This Fuzz Test is the equivalent of the previous one, but with the fuzzed inputs not being within the valid bounds.
void PASESession_Unbounded(const uint32_t fuzzedSetupPasscode, const vector<uint8_t> & fuzzedSalt, uint32_t fuzzedPBKDF2Iter)
{

    Spake2pVerifier fuzzedSpake2pVerifier;
    ByteSpan fuzzedSaltSpan{ fuzzedSalt.data(), fuzzedSalt.size() };

    // Generating the Spake2+ verifier from fuzzed inputs
    fuzzedSpake2pVerifier.Generate(fuzzedPBKDF2Iter, fuzzedSaltSpan, fuzzedSetupPasscode);

    TestPASESession PASELoopBack;
    TemporarySessionManager sessionManager(PASELoopBack);

    PASESession pairingCommissioner;

    TestSecurePairingDelegate delegateCommissioner;
    TestSecurePairingDelegate delegateCommissionee;

    PASELoopBack.SecurePairingHandshake(sessionManager, pairingCommissioner, delegateCommissioner, delegateCommissionee,
                                        fuzzedSpake2pVerifier, fuzzedPBKDF2Iter, fuzzedSaltSpan, fuzzedSetupPasscode);
}

FUZZ_TEST(FuzzPASE_PW, PASESession_Unbounded)
    .WithDomains(
        // fuzzedSetupPasscode
        Arbitrary<uint32_t>(),
        // fuzzedSalt
        Arbitrary<vector<uint8_t>>(),
        // fuzzedPBKDF2Iter
        Arbitrary<uint32_t>());

/* -------------------------------------------------------------------------------------------*/
// In This FuzzTest, the Spake2pVerifier is fuzzed.
void FuzzSpake2pVerifier(const vector<uint8_t> & aW0, const vector<uint8_t> & aL, const vector<uint8_t> & aSalt,
                         const uint32_t fuzzedPBKDF2Iter, const uint32_t fuzzedSetupPasscode)
{
    Spake2pVerifier fuzzedSpake2pVerifier;

    copy_n(aW0.data(), aW0.size(), fuzzedSpake2pVerifier.mW0);
    copy_n(aL.data(), aL.size(), fuzzedSpake2pVerifier.mL);

    ByteSpan fuzzedSaltSpan(aSalt.data(), aSalt.size());

    TestPASESession PASELoopBack;
    TemporarySessionManager sessionManager(PASELoopBack);

    PASESession pairingCommissioner;

    TestSecurePairingDelegate delegateCommissioner;
    TestSecurePairingDelegate delegateCommissionee;

    PASELoopBack.SecurePairingHandshake(sessionManager, pairingCommissioner, delegateCommissioner, delegateCommissionee,
                                        fuzzedSpake2pVerifier, fuzzedPBKDF2Iter, fuzzedSaltSpan, fuzzedSetupPasscode);
}
FUZZ_TEST(FuzzPASE_PW, FuzzSpake2pVerifier)
    .WithDomains(
        // aW0
        Arbitrary<std::vector<uint8_t>>().WithMaxSize(kP256_FE_Length),
        // aL
        Arbitrary<std::vector<uint8_t>>().WithMaxSize(kP256_Point_Length),
        // aSalt
        Arbitrary<vector<uint8_t>>(),
        // fuzzedPBKDF2Iter
        Arbitrary<uint32_t>(),
        // fuzzedSetupPasscode
        Arbitrary<uint32_t>());

/* -------------------------------------------------------------------------------------------*/
// In This FuzzTest, Fuzzed Serialized Verifier is deserialized and Serialized Again, comparing the original with RoundTrip result.
void Spake2pVerifier_Serialize_RoundTrip(const vector<uint8_t> & FuzzedSerializedVerifier)
{

    Spake2pVerifierSerialized FuzzedSerializedVerifierArray;

    copy_n(FuzzedSerializedVerifier.data(), FuzzedSerializedVerifier.size(), FuzzedSerializedVerifierArray);

    // Deserialize the fuzzed SPAKE2+ Verifier
    Spake2pVerifier verifier;
    EXPECT_EQ(verifier.Deserialize(ByteSpan(FuzzedSerializedVerifierArray)), CHIP_NO_ERROR);

    // Serialize the fuzzed SPAKE2+ Verifier again
    Spake2pVerifierSerialized reserializedVerifier;
    MutableByteSpan reserializedVerifierSpan(reserializedVerifier);
    EXPECT_EQ(verifier.Serialize(reserializedVerifierSpan), CHIP_NO_ERROR);
    EXPECT_EQ(reserializedVerifierSpan.size(), kSpake2p_VerifierSerialized_Length);

    // The original fuzzed SPAKE2+ verifier should be the same as the deserialized and re-serialized verifier (RoundTrip).
    EXPECT_EQ(memcmp(reserializedVerifier, FuzzedSerializedVerifierArray, kSpake2p_VerifierSerialized_Length), 0);
}

FUZZ_TEST(FuzzPASE_PW, Spake2pVerifier_Serialize_RoundTrip)
    .WithDomains(Arbitrary<vector<uint8_t>>().WithSize(kSpake2p_VerifierSerialized_Length));

/* -------------------------------------------------------------------------------------------*/
// In This Test a Fuzzed PBKDFParamRequest Message is fuzzed, which is then injected into a PASE Session to test the behaviour of
// PASESession::HandlePBKDFParamRequest(); which will be called by the Accessory/Commissionee.
void TestPASESession::FuzzHandlePBKDFParamRequest(vector<uint8_t> fuzzPBKDFLocalRandomData, uint32_t fuzzInitiatorSessionId,
                                                  bool fuzzHavePBKDFParameters)
{
    TemporarySessionManager sessionManager(*this);

    // Commissioner: The Sender of PBKDFParamRequest
    PASESession pairingCommissioner;
    // Accessory: The Receiver of PBKDFParamRequest
    PASESession pairingAccessory;

    /*************************** Preparing Commissioner ***************************/

    CallAllocateSecureSession(sessionManager, pairingCommissioner);

    ReliableMessageProtocolConfig LocalMRPConfig(System::Clock::Milliseconds32(100), System::Clock::Milliseconds32(200),
                                                 System::Clock::Milliseconds16(4000));

    // After the Accessory Handles PBKDFParamRequest, it will send a PBKDFParamResponse back to pairingCommissioner, which should be
    // expecting it.
    pairingCommissioner.mNextExpectedMsg.SetValue(Protocols::SecureChannel::MsgType::PBKDFParamResponse);

    /**************************Constructing PBKDFParamRequest Message***************************************** */

    const size_t max_msg_len = TLV::EstimateStructOverhead(fuzzPBKDFLocalRandomData.size(),     // initiatorRandom,
                                                           sizeof(fuzzInitiatorSessionId),      // initiatorSessionId
                                                           sizeof(PasscodeId),                  // passcodeId,
                                                           sizeof(bool),                        // hasPBKDFParameters
                                                           SessionParameters::kEstimatedTLVSize // Session Parameters
    );

    System::PacketBufferHandle req = System::PacketBufferHandle::New(max_msg_len);
    EXPECT_FALSE(req.IsNull());

    System::PacketBufferTLVWriter tlvWriter;
    tlvWriter.Init(std::move(req));

    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;
    EXPECT_EQ(CHIP_NO_ERROR, tlvWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType));
    EXPECT_EQ(CHIP_NO_ERROR,
              tlvWriter.PutBytes(TLV::ContextTag(1), fuzzPBKDFLocalRandomData.data(), fuzzPBKDFLocalRandomData.size()));

    EXPECT_EQ(CHIP_NO_ERROR, tlvWriter.Put(TLV::ContextTag(2), fuzzInitiatorSessionId));
    EXPECT_EQ(CHIP_NO_ERROR, tlvWriter.Put(TLV::ContextTag(3), kDefaultCommissioningPasscodeId));
    EXPECT_EQ(CHIP_NO_ERROR, tlvWriter.PutBoolean(TLV::ContextTag(4), fuzzHavePBKDFParameters));

    EXPECT_EQ(CHIP_NO_ERROR, PASESession::EncodeSessionParameters(TLV::ContextTag(5), LocalMRPConfig, tlvWriter));
    EXPECT_EQ(CHIP_NO_ERROR, tlvWriter.EndContainer(outerContainerType));
    EXPECT_EQ(CHIP_NO_ERROR, tlvWriter.Finalize(&req));

    /*************************** Preparing Accessory for Receiving PBKDFParamRequest Message ***************************/

    PASETestLoopbackTransportDelegate delegate;
    auto & loopback = GetLoopback();
    loopback.SetLoopbackTransportDelegate(&delegate);
    loopback.mSentMessageCount = 0;

    TestSecurePairingDelegate delegateAccessory;

    PayloadHeader payloadHeaderAccessory;

    // One Limitation of using this is that contextAccessory will automatically be an Initiator, while in real-life it should be a
    // responder.
    ExchangeContext * contextAccessory = NewUnauthenticatedExchangeToBob(&pairingAccessory);

    EXPECT_EQ(GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::PBKDFParamRequest,
                                                                            &pairingAccessory),
              CHIP_NO_ERROR);

    // Adding PASESession::Init in order to AllocateSecureSession and have a localsessionID generated for pairingAccessory
    pairingAccessory.Init(sessionManager, 0, &delegateAccessory);

    // This was done to have an exchange context
    pairingAccessory.mExchangeCtxt.Emplace(*contextAccessory);

    pairingAccessory.mLocalMRPConfig = MakeOptional(LocalMRPConfig);

    payloadHeaderAccessory.SetMessageType(Protocols::SecureChannel::MsgType::PBKDFParamRequest);
    pairingAccessory.mNextExpectedMsg.SetValue(Protocols::SecureChannel::MsgType::PBKDFParamRequest);

    pairingAccessory.OnMessageReceived(&pairingAccessory.mExchangeCtxt.Value().Get(), payloadHeaderAccessory, std::move(req));

    DrainAndServiceIO();
}

void HandlePBKDFParamRequest(vector<uint8_t> fuzzPBKDFLocalRandomData, uint32_t fuzzInitiatorSessionId,
                             bool fuzzHavePBKDFParameters)
{
    // TODO: #35369 Move this to a Fixture once Errors related to FuzzTest Fixtures are resolved
    TestPASESession PASELoopBack;
    PASELoopBack.FuzzHandlePBKDFParamRequest(fuzzPBKDFLocalRandomData, fuzzInitiatorSessionId, fuzzHavePBKDFParameters);
}

FUZZ_TEST(FuzzPASE_PW, HandlePBKDFParamRequest)
    .WithDomains(
        // PBKDFLocalRandomData
        Arbitrary<vector<uint8_t>>(),
        // InitiatorSessionId
        Arbitrary<uint32_t>(),
        // mHavePBKDFParameters
        Arbitrary<bool>());

/* -------------------------------------------------------------------------------------------*/
// In This Test a Fuzzed PBKDFParamResponse Message is fuzzed, which is then injected into a PASE Session to test the behaviour of
// PASESession::HandlePBKDFParamResponse(); which will be called by the Commissioner.
void TestPASESession::FuzzHandlePBKDFParamResponse(vector<uint8_t> fuzzPBKDFLocalRandomDataInitiator,
                                                   vector<uint8_t> fuzzPBKDFLocalRandomDataResponder,
                                                   uint32_t fuzzResponderSessionId, const vector<uint8_t> & fuzzedSalt,
                                                   uint32_t fuzzedPBKDF2Iter, bool fuzzHavePBKDFParameters)
{
    TemporarySessionManager sessionManager(*this);

    // Commissioner: The Receiver of PBKDFParamResponse
    PASESession pairingCommissioner;
    // Accessory: The Sender of PBKDFParamResponse
    PASESession pairingAccessory;

    /*************************** Preparing Commissioner ***************************/

    CallAllocateSecureSession(sessionManager, pairingCommissioner);

    ExchangeContext * contextCommissioner = NewUnauthenticatedExchangeToBob(&pairingCommissioner);

    // This was done to have an exchange context
    pairingCommissioner.mExchangeCtxt.Emplace(*contextCommissioner);

    ReliableMessageProtocolConfig LocalMRPConfig(System::Clock::Milliseconds32(100), System::Clock::Milliseconds32(200),
                                                 System::Clock::Milliseconds16(4000));

    pairingCommissioner.mLocalMRPConfig = MakeOptional(LocalMRPConfig);

    // preparing CommissioningHash needed for SetupSpake2p()
    pairingCommissioner.mCommissioningHash.Begin();
    pairingCommissioner.mCommissioningHash.AddData(ByteSpan(fuzzedSalt.data(), fuzzedSalt.size()));

    // The Commissioner will check if the PBKDFLocalRandomData it received in TLV is same as the stored mPBKDFLocalRandomData. So we
    // inject it here to be able to pass that check
    memcpy(&pairingCommissioner.mPBKDFLocalRandomData[0], fuzzPBKDFLocalRandomDataInitiator.data(),
           fuzzPBKDFLocalRandomDataInitiator.size());

    // In order to cover the Code path where the Commissioner has PBKDF Parameters before Starting PASE, as such, the Accessory will
    // not send the PBKDF Parameters in the Response message
    bool initiatorHasPBKDFParams             = fuzzHavePBKDFParameters;
    pairingCommissioner.mHavePBKDFParameters = fuzzHavePBKDFParameters;

    /**************************Constructing PBKDFParamResponse Message***************************************** */

    const size_t max_msg_len =
        TLV::EstimateStructOverhead(fuzzPBKDFLocalRandomDataInitiator.size(),                                 // initiatorRandom
                                    fuzzPBKDFLocalRandomDataResponder.size(),                                 // responderRandom
                                    sizeof(fuzzResponderSessionId),                                           // responderSessionId
                                    TLV::EstimateStructOverhead(sizeof(fuzzedPBKDF2Iter), fuzzedSalt.size()), // pbkdf_parameters
                                    SessionParameters::kEstimatedTLVSize                                      // Session Parameters
        );

    System::PacketBufferHandle resp = System::PacketBufferHandle::New(max_msg_len);
    EXPECT_FALSE(resp.IsNull());

    System::PacketBufferTLVWriter tlvWriter;
    tlvWriter.Init(std::move(resp));

    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;
    EXPECT_EQ(CHIP_NO_ERROR, tlvWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType));
    EXPECT_EQ(
        CHIP_NO_ERROR,
        tlvWriter.PutBytes(TLV::ContextTag(1), fuzzPBKDFLocalRandomDataInitiator.data(), fuzzPBKDFLocalRandomDataInitiator.size()));
    EXPECT_EQ(
        CHIP_NO_ERROR,
        tlvWriter.PutBytes(TLV::ContextTag(2), fuzzPBKDFLocalRandomDataResponder.data(), fuzzPBKDFLocalRandomDataResponder.size()));

    EXPECT_EQ(CHIP_NO_ERROR, tlvWriter.Put(TLV::ContextTag(3), fuzzResponderSessionId));

    if (!initiatorHasPBKDFParams)
    {
        TLV::TLVType pbkdfParamContainer;
        EXPECT_EQ(CHIP_NO_ERROR, tlvWriter.StartContainer(TLV::ContextTag(4), TLV::kTLVType_Structure, pbkdfParamContainer));
        EXPECT_EQ(CHIP_NO_ERROR, tlvWriter.Put(TLV::ContextTag(1), fuzzedPBKDF2Iter));
        EXPECT_EQ(CHIP_NO_ERROR, tlvWriter.PutBytes(TLV::ContextTag(2), fuzzedSalt.data(), fuzzedSalt.size()));
        EXPECT_EQ(CHIP_NO_ERROR, tlvWriter.EndContainer(pbkdfParamContainer));
    }

    EXPECT_EQ(CHIP_NO_ERROR, PASESession::EncodeSessionParameters(TLV::ContextTag(5), LocalMRPConfig, tlvWriter));
    EXPECT_EQ(CHIP_NO_ERROR, tlvWriter.EndContainer(outerContainerType));
    EXPECT_EQ(CHIP_NO_ERROR, tlvWriter.Finalize(&resp));

    /*************************** Preparing Commissioner for Receiving PBKDFParamResponse Message ***************************/

    PayloadHeader payloadHeaderCommissioner;

    payloadHeaderCommissioner.SetMessageType(Protocols::SecureChannel::MsgType::PBKDFParamResponse);
    pairingCommissioner.mNextExpectedMsg.SetValue(Protocols::SecureChannel::MsgType::PBKDFParamResponse);

    pairingCommissioner.OnMessageReceived(&pairingCommissioner.mExchangeCtxt.Value().Get(), payloadHeaderCommissioner,
                                          std::move(resp));

    DrainAndServiceIO();
}

void HandlePBKDFParamResponse(vector<uint8_t> fuzzPBKDFLocalRandomDataInitiator, vector<uint8_t> fuzzPBKDFLocalRandomDataResponder,
                              uint32_t fuzzResponderSessionId, const vector<uint8_t> & fuzzedSalt, uint32_t fuzzedPBKDF2Iter,
                              bool fuzzHavePBKDFParameters)
{
    // TODO: #35369 Move this to a Fixture once Errors related to FuzzTest Fixtures are resolved
    TestPASESession PASELoopBack;
    PASELoopBack.FuzzHandlePBKDFParamResponse(fuzzPBKDFLocalRandomDataInitiator, fuzzPBKDFLocalRandomDataResponder,
                                              fuzzResponderSessionId, fuzzedSalt, fuzzedPBKDF2Iter, fuzzHavePBKDFParameters);
}

FUZZ_TEST(FuzzPASE_PW, HandlePBKDFParamResponse)
    .WithDomains(
        // fuzzPBKDFLocalRandomDataInitiator
        Arbitrary<vector<uint8_t>>().WithMaxSize(32),
        // fuzzPBKDFLocalRandomDataResponder
        Arbitrary<vector<uint8_t>>(),
        // fuzzResponderSessionId
        Arbitrary<uint32_t>(),
        // fuzzedSalt
        Arbitrary<vector<uint8_t>>(),
        // fuzzedPBKDF2Iter
        Arbitrary<uint32_t>(),
        // mHavePBKDFParameters
        Arbitrary<bool>());

/*------------------------------------------------------------------------------------------------------------------------------------*/
// In This Test we start by constructing a Fuzzed Pake1 Message, by fuzzing the payload pA. the Fuzzed Pake1 Message is then
// injected into a PASE Session to test the behaviour of PASESession::HandleMsg1_and_SendMsg2(); which will be called by the
// Accessory/Commissionee.
void TestPASESession::FuzzHandlePake1(const uint32_t fuzzedSetupPasscode, const vector<uint8_t> & fuzzedSalt,
                                      uint32_t fuzzedPBKDF2Iter, const vector<uint8_t> & fuzzed_pA)
{

    ByteSpan fuzzedSaltSpan(fuzzedSalt.data(), fuzzedSalt.size());

    TemporarySessionManager sessionManager(*this);

    // Commissioner: The Sender of PAKE1
    PASESession pairingCommissioner;
    // Accessory: The Receiver of PAKE1
    PASESession pairingAccessory;

    /*************************** Preparing Commissioner ***************************/

    // preparing CommissioningHash needed for SetupSpake2p()
    pairingCommissioner.mCommissioningHash.Begin();
    pairingCommissioner.mCommissioningHash.AddData(fuzzedSaltSpan);
    EXPECT_EQ(CHIP_NO_ERROR, pairingCommissioner.SetupSpake2p());

    uint8_t serializedWS[kSpake2p_WS_Length * 2] = { 0 };

    // Compute serializedWS, to be used in BeginProver()

    Spake2pVerifier::ComputeWS(fuzzedPBKDF2Iter, fuzzedSaltSpan, fuzzedSetupPasscode, serializedWS, sizeof(serializedWS));

    EXPECT_EQ(CHIP_NO_ERROR,
              pairingCommissioner.mSpake2p.BeginProver(nullptr, 0, nullptr, 0, &serializedWS[0], kSpake2p_WS_Length,
                                                       &serializedWS[kSpake2p_WS_Length], kSpake2p_WS_Length));

    /*********************** Constructing Fuzzed Pake1 Message, to later inject it into PASE Session *********************/

    constexpr uint8_t kPake1_pA = 1;

    const size_t max_msg_len       = TLV::EstimateStructOverhead(fuzzed_pA.size());
    System::PacketBufferHandle msg = System::PacketBufferHandle::New(max_msg_len);
    EXPECT_FALSE(msg.IsNull());

    System::PacketBufferTLVWriter tlvWriter;
    tlvWriter.Init(std::move(msg));

    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;
    EXPECT_EQ(CHIP_NO_ERROR, tlvWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType));
    EXPECT_EQ(CHIP_NO_ERROR, tlvWriter.Put(TLV::ContextTag(kPake1_pA), ByteSpan(fuzzed_pA.data(), fuzzed_pA.size())));
    EXPECT_EQ(CHIP_NO_ERROR, tlvWriter.EndContainer(outerContainerType));
    EXPECT_EQ(CHIP_NO_ERROR, tlvWriter.Finalize(&msg));

    /*************************** Preparing Accessory for Receiving Pake1 Message ***************************/

    // One Limitation of using this is that contextAccessory will automatically be an Initiator, while in real-life it should be a
    // responder.
    ExchangeContext * contextAccessory = NewUnauthenticatedExchangeToBob(&pairingAccessory);

    pairingAccessory.mExchangeCtxt.Emplace(*contextAccessory);

    pairingAccessory.mLocalMRPConfig = MakeOptional(ReliableMessageProtocolConfig(
        System::Clock::Milliseconds32(100), System::Clock::Milliseconds32(200), System::Clock::Milliseconds16(4000)));

    // preparing CommissioningHash needed for SetupSpake2p()
    pairingAccessory.mCommissioningHash.Begin();
    pairingAccessory.mCommissioningHash.AddData(fuzzedSaltSpan);
    EXPECT_EQ(CHIP_NO_ERROR, pairingAccessory.SetupSpake2p());

    //  Compute mPASEVerifier (in order for mSpake2p.BeginVerifier() to use it, once it is called by the pairingAccessory through
    //  HandleMsg1_and_SendMsg2)
    pairingAccessory.mPASEVerifier.Generate(fuzzedPBKDF2Iter, fuzzedSaltSpan, fuzzedSetupPasscode);

    /************************Injecting Fuzzed Pake1 Message into PaseSession::OnMessageReceived*************************/

    PayloadHeader payloadHeaderAccessory;

    // Below two lines are needed to trigger PASESession::OnMessageReceived to call HandleMsg1_and_SendMsg2
    payloadHeaderAccessory.SetMessageType(Protocols::SecureChannel::MsgType::PASE_Pake1);
    pairingAccessory.mNextExpectedMsg.SetValue(Protocols::SecureChannel::MsgType::PASE_Pake1);

    pairingAccessory.OnMessageReceived(&pairingAccessory.mExchangeCtxt.Value().Get(), payloadHeaderAccessory, std::move(msg));

    DrainAndServiceIO();
}

void HandlePake1(const uint32_t fuzzedSetupPasscode, const vector<uint8_t> & fuzzedSalt, uint32_t fuzzedPBKDF2Iter,
                 const vector<uint8_t> & fuzzed_pA)
{
    // TODO: #35369 Move this to a Fixture once Errors related to FuzzTest Fixtures are resolved
    TestPASESession PASELoopBack;

    // pA is an uncompressed public key format, it should start with 0x04.
    // Appending 0x04 to the fuzzed input to increase the likelihood of having a valid point format during fuzzing.
    vector<uint8_t> pA_appended = { 0x04 };
    pA_appended.insert(pA_appended.end(), fuzzed_pA.begin(), fuzzed_pA.end());
    PASELoopBack.FuzzHandlePake1(fuzzedSetupPasscode, fuzzedSalt, fuzzedPBKDF2Iter, pA_appended);
}

// In This FuzzTest, we will construct a PAKE1 Message with a fuzzed TLV length, and send it through a PASESession
FUZZ_TEST(FuzzPASE_PW, HandlePake1)
    .WithDomains(
        // Setup Code Range  (covers the full 27-bit range, plus 16 additional values)
        InRange(00000000, 0x800000F),
        // Salt length range (extending min and max by 4 bytes)
        Arbitrary<vector<uint8_t>>()
            .WithMinSize(kSpake2p_Min_PBKDF_Salt_Length - 4)
            .WithMaxSize(kSpake2p_Max_PBKDF_Salt_Length + 4),
        // PBKDF2Iterations count range (extending min and max by 10 iterations)
        InRange(kSpake2p_Min_PBKDF_Iterations - 10, kSpake2p_Max_PBKDF_Iterations + 10),
        // Fuzzed pA (Original size = kMAX_Point_Length)
        Arbitrary<vector<uint8_t>>());

/*------------------------------------------------------------------------------------------------------------------------------------*/
// In this FuzzTest we start by constructing a Fuzzed PAKE2 Message, by fuzzing the payloads pB and cB. the fuzzed message is then
// injected into a PASE Session to test the behavior of PASESession::HandleMsg2_and_SendMsg3(), which will be called by Commissioner
void TestPASESession::FuzzHandlePake2(const uint32_t fuzzedSetupPasscode, const vector<uint8_t> & fuzzedSalt,
                                      uint32_t fuzzedPBKDF2Iter, const vector<uint8_t> & fuzzed_pB,
                                      const vector<uint8_t> & fuzzed_cB)
{

    ByteSpan fuzzedSaltSpan(fuzzedSalt.data(), fuzzedSalt.size());

    TemporarySessionManager sessionManager(*this);

    // Commissioner: The Receiver of PAKE2
    PASESession pairingCommissioner;
    // Accessory: The Sender of PAKE2
    PASESession pairingAccessory;

    /*************************** Preparing Commissioner ***************************/

    CallAllocateSecureSession(sessionManager, pairingCommissioner);

    ExchangeContext * contextCommissioner = NewUnauthenticatedExchangeToBob(&pairingCommissioner);

    // This was done to have an exchange context
    pairingCommissioner.mExchangeCtxt.Emplace(*contextCommissioner);

    pairingCommissioner.mLocalMRPConfig = MakeOptional(ReliableMessageProtocolConfig(
        System::Clock::Milliseconds32(100), System::Clock::Milliseconds32(200), System::Clock::Milliseconds16(4000)));

    // preparing CommissioningHash needed for SetupSpake2p()
    pairingCommissioner.mCommissioningHash.Begin();
    pairingCommissioner.mCommissioningHash.AddData(fuzzedSaltSpan);
    EXPECT_EQ(CHIP_NO_ERROR, pairingCommissioner.SetupSpake2p());

    uint8_t serializedWS[kSpake2p_WS_Length * 2] = { 0 };

    // Compute serializedWS, to be used in BeginProver()
    Spake2pVerifier::ComputeWS(fuzzedPBKDF2Iter, fuzzedSaltSpan, fuzzedSetupPasscode, serializedWS, sizeof(serializedWS));

    EXPECT_EQ(CHIP_NO_ERROR,
              pairingCommissioner.mSpake2p.BeginProver(nullptr, 0, nullptr, 0, &serializedWS[0], kSpake2p_WS_Length,
                                                       &serializedWS[kSpake2p_WS_Length], kSpake2p_WS_Length));

    // The Commissioner should have alread called ComputeRoundOne As part of the PASE Session Exchange (and consequently have
    // mSpake2p.state = CHIP_SPAKE2P_STATE::R1), the computed values are not used in the test.
    uint8_t X[kMAX_Point_Length];
    size_t X_len = sizeof(X);
    pairingCommissioner.mSpake2p.ComputeRoundOne(nullptr, 0, X, &X_len);

    /*************************** Preparing Accessory  ***************************/

    // Init mCommissioningHash; needed for SetupSpake2p()
    pairingAccessory.mCommissioningHash.Begin();
    pairingAccessory.mCommissioningHash.AddData(fuzzedSaltSpan);
    EXPECT_EQ(CHIP_NO_ERROR, pairingAccessory.SetupSpake2p());

    // Below Steps take place in HandleMsg1
    // Compute mPASEVerifier to be able to pass it to BeginVerifier()
    pairingAccessory.mPASEVerifier.Generate(fuzzedPBKDF2Iter, fuzzedSaltSpan, fuzzedSetupPasscode);

    pairingAccessory.mSpake2p.BeginVerifier(nullptr, 0, nullptr, 0, pairingAccessory.mPASEVerifier.mW0, kP256_FE_Length,
                                            pairingAccessory.mPASEVerifier.mL, kP256_Point_Length);

    /*********************** Constructing Fuzzed PAKE2 Message, to later inject it into PASE Session *********************/

    constexpr uint8_t kPake2_pB = 1;
    constexpr uint8_t kPake2_cB = 2;

    const size_t max_msg_len        = TLV::EstimateStructOverhead(fuzzed_pB.size(), fuzzed_cB.size());
    System::PacketBufferHandle msg2 = System::PacketBufferHandle::New(max_msg_len);
    EXPECT_FALSE(msg2.IsNull());

    System::PacketBufferTLVWriter tlvWriter;
    tlvWriter.Init(std::move(msg2));

    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;
    EXPECT_EQ(CHIP_NO_ERROR, tlvWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType));
    EXPECT_EQ(CHIP_NO_ERROR, tlvWriter.Put(TLV::ContextTag(kPake2_pB), ByteSpan(fuzzed_pB.data(), fuzzed_pB.size())));
    EXPECT_EQ(CHIP_NO_ERROR, tlvWriter.Put(TLV::ContextTag(kPake2_cB), ByteSpan(fuzzed_cB.data(), fuzzed_cB.size())));
    EXPECT_EQ(CHIP_NO_ERROR, tlvWriter.EndContainer(outerContainerType));
    EXPECT_EQ(CHIP_NO_ERROR, tlvWriter.Finalize(&msg2));

    // After PAKE2 is sent, the pairingAccessory will expect a a Pake3 Message
    pairingAccessory.mNextExpectedMsg.SetValue(Protocols::SecureChannel::MsgType::PASE_Pake3);

    /************************Injecting Fuzzed PAKE2 Message into PaseSession::OnMessageReceived*************************/

    PayloadHeader payloadHeaderCommissioner;

    // Below two lines are needed to trigger PASESession::OnMessageReceived to call HandleMsg2_and_SendMsg3
    payloadHeaderCommissioner.SetMessageType(Protocols::SecureChannel::MsgType::PASE_Pake2);
    pairingCommissioner.mNextExpectedMsg.SetValue(Protocols::SecureChannel::MsgType::PASE_Pake2);

    pairingCommissioner.OnMessageReceived(&pairingCommissioner.mExchangeCtxt.Value().Get(), payloadHeaderCommissioner,
                                          std::move(msg2));

    DrainAndServiceIO();
}

void HandlePake2(const uint32_t fuzzedSetupPasscode, const vector<uint8_t> & fuzzedSalt, uint32_t fuzzedPBKDF2Iter,
                 const vector<uint8_t> & pB, const vector<uint8_t> & cB)
{
    TestPASESession PASELoopBack;
    // pB is an uncompressed public key format, it should start with 0x04.
    // Appending 0x04 to the fuzzed input to increase the likelihood of having a valid point format during fuzzing.
    vector<uint8_t> pB_appended = { 0x04 };
    pB_appended.insert(pB_appended.end(), pB.begin(), pB.end());

    PASELoopBack.FuzzHandlePake2(fuzzedSetupPasscode, fuzzedSalt, fuzzedPBKDF2Iter, pB_appended, cB);
}

FUZZ_TEST(FuzzPASE_PW, HandlePake2)
    .WithDomains(
        // Setup Code Range (covers the full 27-bit range, plus 16 additional values)
        InRange(00000000, 0x800000F),
        // Salt length range (extending min and max by 4 bytes)
        Arbitrary<vector<uint8_t>>()
            .WithMinSize(kSpake2p_Min_PBKDF_Salt_Length - 4)
            .WithMaxSize(kSpake2p_Max_PBKDF_Salt_Length + 4),
        // PBKDF2Iterations count range (extending min and max by 10 iterations)
        InRange(kSpake2p_Min_PBKDF_Iterations - 10, kSpake2p_Max_PBKDF_Iterations + 10),
        //  Fuzzed pB (Original size = kMAX_Point_Length),
        Arbitrary<vector<uint8_t>>(),
        //  Fuzzed cB (Original size = kMAX_Hash_Length)
        Arbitrary<vector<uint8_t>>());

/*------------------------------------------------------------------------------------------------------------------------------------*/
// In This Test we start by constructing a Fuzzed Pake3 Message, by fuzzing the payload cA. The Fuzzed message is then injected into
// a PASE Session to test the behavior of PASESession::HandleMsg3(); which will be called by the Accessory/Commissionee.
void TestPASESession::FuzzHandlePake3(const uint32_t fuzzedSetupPasscode, const vector<uint8_t> & fuzzedSalt,
                                      uint32_t fuzzedPBKDF2Iter, const vector<uint8_t> & fuzzed_cA)
{

    ByteSpan fuzzedSaltSpan(fuzzedSalt.data(), fuzzedSalt.size());

    TemporarySessionManager sessionManager(*this);

    // Commissioner: The Sender of PAKE3
    PASESession pairingCommissioner;
    // Accessory: The Receiver of PAKE3
    PASESession pairingAccessory;

    /*************************** Preparing Commissioner  ***************************/

    // preparing CommissioningHash needed for SetupSpake2p()
    pairingCommissioner.mCommissioningHash.Begin();
    pairingCommissioner.mCommissioningHash.AddData(fuzzedSaltSpan);
    EXPECT_EQ(CHIP_NO_ERROR, pairingCommissioner.SetupSpake2p());

    uint8_t serializedWS[kSpake2p_WS_Length * 2] = { 0 };

    // Compute serializedWS, to be used in BeginProver()

    Spake2pVerifier::ComputeWS(fuzzedPBKDF2Iter, fuzzedSaltSpan, fuzzedSetupPasscode, serializedWS, sizeof(serializedWS));

    EXPECT_EQ(CHIP_NO_ERROR,
              pairingCommissioner.mSpake2p.BeginProver(nullptr, 0, nullptr, 0, &serializedWS[0], kSpake2p_WS_Length,
                                                       &serializedWS[kSpake2p_WS_Length], kSpake2p_WS_Length));

    // The Commissioner should have already called ComputeRoundOne As part of the Exchange  (and consequently have state =
    // CHIP_SPAKE2P_STATE::R1), the computed values are not used.
    uint8_t X[kMAX_Point_Length];
    size_t X_len = sizeof(X);
    pairingCommissioner.mSpake2p.ComputeRoundOne(nullptr, 0, X, &X_len);

    /*************************** Preparing Accessory ***************************/

    CallAllocateSecureSession(sessionManager, pairingAccessory);

    // One Limitation of using this is that contextAccessory will automatically be an Initiator, while in real-life it should be a
    // responder.
    ExchangeContext * contextAccessory = NewUnauthenticatedExchangeToBob(&pairingAccessory);

    // This was done to have an exchange context
    pairingAccessory.mExchangeCtxt.Emplace(*contextAccessory);

    pairingAccessory.mLocalMRPConfig = MakeOptional(ReliableMessageProtocolConfig(
        System::Clock::Milliseconds32(100), System::Clock::Milliseconds32(200), System::Clock::Milliseconds16(4000)));

    // preparing CommissioningHash needed for SetupSpake2p()
    pairingAccessory.mCommissioningHash.Begin();
    pairingAccessory.mCommissioningHash.AddData(fuzzedSaltSpan);
    EXPECT_EQ(CHIP_NO_ERROR, pairingAccessory.SetupSpake2p());

    // Below Steps take place in HandleMsg1
    //  compute mPASEVerifier to be able to pass it to BeginVerifier()
    pairingAccessory.mPASEVerifier.Generate(fuzzedPBKDF2Iter, fuzzedSaltSpan, fuzzedSetupPasscode);

    pairingAccessory.mSpake2p.BeginVerifier(nullptr, 0, nullptr, 0, pairingAccessory.mPASEVerifier.mW0, kP256_FE_Length,
                                            pairingAccessory.mPASEVerifier.mL, kP256_Point_Length);

    /*************************** Preparing Accessory for Receiving PAKE3 Message ***************************/

    uint8_t Y[kMAX_Point_Length];
    size_t Y_len = sizeof(Y);

    uint8_t verifier[kMAX_Hash_Length];
    size_t verifier_len = kMAX_Hash_Length;

    // These calls are usually done on the Accessory before sending Pake2. They are needed to make sure that the Accessory is at the
    // correct Spake2p state (CHIP_SPAKE2P_STATE::R2) to handle Pake3 Messages.
    pairingAccessory.mSpake2p.ComputeRoundOne(X, X_len, Y, &Y_len);
    pairingAccessory.mSpake2p.ComputeRoundTwo(X, X_len, verifier, &verifier_len);

    /*********************** Constructing Fuzzed PAKE3 Message, to later inject it into PASE Session *********************/

    constexpr uint8_t kPake3_cA = 1;

    const size_t max_msg_len        = TLV::EstimateStructOverhead(fuzzed_cA.size());
    System::PacketBufferHandle msg3 = System::PacketBufferHandle::New(max_msg_len);
    EXPECT_FALSE(msg3.IsNull());

    System::PacketBufferTLVWriter tlvWriter;
    tlvWriter.Init(std::move(msg3));

    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;
    EXPECT_EQ(CHIP_NO_ERROR, tlvWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType));
    EXPECT_EQ(CHIP_NO_ERROR, tlvWriter.Put(TLV::ContextTag(kPake3_cA), ByteSpan(fuzzed_cA.data(), fuzzed_cA.size())));
    EXPECT_EQ(CHIP_NO_ERROR, tlvWriter.EndContainer(outerContainerType));
    EXPECT_EQ(CHIP_NO_ERROR, tlvWriter.Finalize(&msg3));

    // After PAKE3 is sent, the pairingCommissioner will expect a StatusReport
    pairingCommissioner.mNextExpectedMsg.SetValue(Protocols::SecureChannel::MsgType::StatusReport);

    /************************Injecting Fuzzed PAKE3 Message into PaseSession::OnMessageReceived*************************/

    PayloadHeader payloadHeaderAccessory;

    // Below two lines are needed to trigger PASESession::OnMessageReceived to call HandleMsg3
    payloadHeaderAccessory.SetMessageType(Protocols::SecureChannel::MsgType::PASE_Pake3);
    pairingAccessory.mNextExpectedMsg.SetValue(Protocols::SecureChannel::MsgType::PASE_Pake3);

    pairingAccessory.OnMessageReceived(&pairingAccessory.mExchangeCtxt.Value().Get(), payloadHeaderAccessory, std::move(msg3));

    DrainAndServiceIO();
}

void HandlePake3(const uint32_t fuzzedSetupPasscode, const vector<uint8_t> & fuzzedSalt, uint32_t fuzzedPBKDF2Iter,
                 const vector<uint8_t> & fuzzedcA)
{
    TestPASESession PASELoopBack;
    PASELoopBack.FuzzHandlePake3(fuzzedSetupPasscode, fuzzedSalt, fuzzedPBKDF2Iter, fuzzedcA);
}

FUZZ_TEST(FuzzPASE_PW, HandlePake3)
    .WithDomains(
        // Setup Code Range (covers the full 27-bit range, plus 16 additional values)
        InRange(00000000, 0x800000F),
        // Salt length range (extending min and max by 4 bytes)
        Arbitrary<vector<uint8_t>>()
            .WithMinSize(kSpake2p_Min_PBKDF_Salt_Length - 4)
            .WithMaxSize(kSpake2p_Max_PBKDF_Salt_Length + 4),
        // PBKDF2Iterations count range (extending min and max by 10 iterations)
        InRange(kSpake2p_Min_PBKDF_Iterations - 10, kSpake2p_Max_PBKDF_Iterations + 10),
        // Fuzzed cA (Original size = kMAX_Hash_Length)
        Arbitrary<vector<uint8_t>>());

} // namespace Testing
} // namespace chip
