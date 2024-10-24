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

namespace {

using namespace chip;
using namespace std;

using namespace chip::Crypto;
using namespace fuzztest;
using namespace chip::Transport;
using namespace chip::Messaging;
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
        InRange(00000000, 99999998),
        Arbitrary<vector<uint8_t>>().WithMinSize(kSpake2p_Min_PBKDF_Salt_Length).WithMaxSize(kSpake2p_Max_PBKDF_Salt_Length),
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
    .WithDomains(Arbitrary<uint32_t>(), Arbitrary<vector<uint8_t>>(), Arbitrary<uint32_t>());

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
    .WithDomains(Arbitrary<std::vector<uint8_t>>().WithMaxSize(kP256_FE_Length),
                 Arbitrary<std::vector<uint8_t>>().WithMaxSize(kP256_Point_Length), Arbitrary<vector<uint8_t>>(),
                 Arbitrary<uint32_t>(), Arbitrary<uint32_t>());

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

} // namespace
