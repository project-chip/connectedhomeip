/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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
 *      This file implements the the CHIP CASE Session object that provides
 *      APIs for constructing a secure session using a certificate from the device's
 *      operational credentials.
 *
 */
#include <protocols/secure_channel/CASESession.h>

#include <atomic>
#include <inttypes.h>
#include <memory>
#include <string.h>

#include <lib/core/CHIPEncoding.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/TypeTraits.h>
#include <messaging/SessionParameters.h>
#include <platform/PlatformManager.h>
#include <protocols/Protocols.h>
#include <protocols/secure_channel/CASEDestinationId.h>
#include <protocols/secure_channel/PairingSession.h>
#include <protocols/secure_channel/SessionResumptionStorage.h>
#include <protocols/secure_channel/StatusReport.h>
#include <system/SystemClock.h>
#include <tracing/macros.h>
#include <tracing/metric_event.h>
#include <transport/SessionManager.h>

namespace {
// TBEDataTags works for both sigma-2-tbedata and sigma-3-tbedata as they have the same tag numbers for the elements common between
// them.
enum class TBEDataTags : uint8_t
{
    kSenderNOC    = 1,
    kSenderICAC   = 2,
    kSignature    = 3,
    kResumptionID = 4,
};

// TBSDataTags works for both sigma-2-tbsdata and sigma-3-tbsdata as they have the same tag numbers for the elements common between
// them.
enum class TBSDataTags : uint8_t
{
    kSenderNOC      = 1,
    kSenderICAC     = 2,
    kSenderPubKey   = 3,
    kReceiverPubKey = 4,
};

enum class Sigma1Tags : uint8_t
{
    kInitiatorRandom        = 1,
    kInitiatorSessionId     = 2,
    kDestinationId          = 3,
    kInitiatorEphPubKey     = 4,
    kInitiatorSessionParams = 5,
    kResumptionID           = 6,
    kResume1MIC             = 7,
};

enum class Sigma2Tags : uint8_t
{
    kResponderRandom        = 1,
    kResponderSessionId     = 2,
    kResponderEphPubKey     = 3,
    kEncrypted2             = 4,
    kResponderSessionParams = 5,
};

enum class Sigma2ResumeTags : uint8_t
{
    kResumptionID           = 1,
    kSigma2ResumeMIC        = 2,
    kResponderSessionID     = 3,
    kResponderSessionParams = 4,
};

enum class Sigma3Tags : uint8_t
{
    kEncrypted3 = 1,
};

// Utility to extract the underlying value of TLV Tag enum classes, used in TLV encoding and parsing.
template <typename Enum>
constexpr chip::TLV::Tag AsTlvContextTag(Enum e)
{
    return chip::TLV::ContextTag(chip::to_underlying(e));
}

constexpr size_t kCaseOverheadForFutureTBEData = 128;

} // namespace

namespace chip {

using namespace Crypto;
using namespace Credentials;
using namespace Messaging;
using namespace Encoding;
using namespace Protocols::SecureChannel;
using namespace Tracing;
using namespace TLV;

constexpr uint8_t kKDFSR2Info[] = { 0x53, 0x69, 0x67, 0x6d, 0x61, 0x32 };
constexpr uint8_t kKDFSR3Info[] = { 0x53, 0x69, 0x67, 0x6d, 0x61, 0x33 };

constexpr uint8_t kKDFS1RKeyInfo[] = { 0x53, 0x69, 0x67, 0x6d, 0x61, 0x31, 0x5f, 0x52, 0x65, 0x73, 0x75, 0x6d, 0x65 };
constexpr uint8_t kKDFS2RKeyInfo[] = { 0x53, 0x69, 0x67, 0x6d, 0x61, 0x32, 0x5f, 0x52, 0x65, 0x73, 0x75, 0x6d, 0x65 };

constexpr uint8_t kResume1MIC_Nonce[] =
    /* "NCASE_SigmaS1" */ { 0x4e, 0x43, 0x41, 0x53, 0x45, 0x5f, 0x53, 0x69, 0x67, 0x6d, 0x61, 0x53, 0x31 };
constexpr uint8_t kResume2MIC_Nonce[] =
    /* "NCASE_SigmaS2" */ { 0x4e, 0x43, 0x41, 0x53, 0x45, 0x5f, 0x53, 0x69, 0x67, 0x6d, 0x61, 0x53, 0x32 };
constexpr uint8_t kTBEData2_Nonce[] =
    /* "NCASE_Sigma2N" */ { 0x4e, 0x43, 0x41, 0x53, 0x45, 0x5f, 0x53, 0x69, 0x67, 0x6d, 0x61, 0x32, 0x4e };
constexpr uint8_t kTBEData3_Nonce[] =
    /* "NCASE_Sigma3N" */ { 0x4e, 0x43, 0x41, 0x53, 0x45, 0x5f, 0x53, 0x69, 0x67, 0x6d, 0x61, 0x33, 0x4e };
constexpr size_t kTBEDataNonceLength = sizeof(kTBEData2_Nonce);
static_assert(sizeof(kTBEData2_Nonce) == sizeof(kTBEData3_Nonce), "TBEData2_Nonce and TBEData3_Nonce must be same size");

// Amounts of time to allow for server-side processing of messages.
//
// These timeout values only allow for the server-side processing and assume that any transport-specific
// latency will be added to them.
//
// The session establishment fails if the response is not received within the resulting timeout window,
// which accounts for both transport latency and the server-side latency.
static constexpr ExchangeContext::Timeout kExpectedLowProcessingTime    = System::Clock::Seconds16(2);
static constexpr ExchangeContext::Timeout kExpectedSigma1ProcessingTime = kExpectedLowProcessingTime;
static constexpr ExchangeContext::Timeout kExpectedHighProcessingTime   = System::Clock::Seconds16(30);

// Helper for managing a session's outstanding work.
// Holds work data which is provided to a scheduled work callback (standalone),
// then (if not canceled) to a scheduled after work callback (on the session).
template <class DATA>
class CASESession::WorkHelper
{
public:
    // Work callback, processed in the background via `PlatformManager::ScheduleBackgroundWork`.
    // This is a non-member function which does not use the associated session.
    // The return value is passed to the after work callback (called afterward).
    // Set `cancel` to true if calling the after work callback is not necessary.
    typedef CHIP_ERROR (*WorkCallback)(DATA & data, bool & cancel);

    // After work callback, processed in the main Matter task via `PlatformManager::ScheduleWork`.
    // This is a member function to be called on the associated session after the work callback.
    // The `status` value is the result of the work callback (called beforehand), or the status of
    // queueing the after work callback back to the Matter thread, if the work callback succeeds
    // but queueing fails.
    //
    // When this callback is called asynchronously (i.e. via ScheduleWork), the helper guarantees
    // that it will keep itself (and hence `data`) alive until the callback completes.
    typedef CHIP_ERROR (CASESession::*AfterWorkCallback)(DATA & data, CHIP_ERROR status);

public:
    // Create a work helper using the specified session, work callback, after work callback, and data (template arg).
    // Lifetime is managed by sharing between the caller (typically the session) and the helper itself (while work is scheduled).
    static Platform::SharedPtr<WorkHelper> Create(CASESession & session, WorkCallback workCallback,
                                                  AfterWorkCallback afterWorkCallback)
    {
        struct EnableShared : public WorkHelper
        {
            EnableShared(CASESession & session, WorkCallback workCallback, AfterWorkCallback afterWorkCallback) :
                WorkHelper(session, workCallback, afterWorkCallback)
            {}
        };
        auto ptr = Platform::MakeShared<EnableShared>(session, workCallback, afterWorkCallback);
        if (ptr)
        {
            ptr->mWeakPtr = ptr; // used by `ScheduleWork`
        }
        return ptr;
    }

    // Do the work immediately.
    // No scheduling, no outstanding work, no shared lifetime management.
    //
    // The caller must guarantee that it keeps the helper alive across this call, most likely by
    // holding a reference to it on the stack.
    CHIP_ERROR DoWork()
    {
        // Ensure that this function is being called from main Matter thread
        assertChipStackLockedByCurrentThread();

        VerifyOrReturnError(mSession && mWorkCallback && mAfterWorkCallback, CHIP_ERROR_INCORRECT_STATE);
        auto * helper   = this;
        bool cancel     = false;
        helper->mStatus = helper->mWorkCallback(helper->mData, cancel);
        if (!cancel)
        {
            helper->mStatus = (helper->mSession->*(helper->mAfterWorkCallback))(helper->mData, helper->mStatus);
        }
        return helper->mStatus;
    }

    // Schedule the work for later execution.
    // If lifetime is managed, the helper shares management while work is outstanding.
    CHIP_ERROR ScheduleWork()
    {
        VerifyOrReturnError(mSession && mWorkCallback && mAfterWorkCallback, CHIP_ERROR_INCORRECT_STATE);
        // Hold strong ptr while work is outstanding
        mStrongPtr  = mWeakPtr.lock(); // set in `Create`
        auto status = DeviceLayer::PlatformMgr().ScheduleBackgroundWork(WorkHandler, reinterpret_cast<intptr_t>(this));
        if (status != CHIP_NO_ERROR)
        {
            // Release strong ptr since scheduling failed.
            mStrongPtr.reset();
        }
        return status;
    }

    // Cancel the work, by clearing the associated session.
    void CancelWork() { mSession.store(nullptr); }

    bool IsCancelled() const { return mSession.load() == nullptr; }

    // This API returns true when background thread fails to schedule the AfterWorkCallback
    bool UnableToScheduleAfterWorkCallback() { return mScheduleAfterWorkFailed.load(); }

    // Do after work immediately.
    // No scheduling, no outstanding work, no shared lifetime management.
    void DoAfterWork()
    {
        VerifyOrDie(UnableToScheduleAfterWorkCallback());
        AfterWorkHandler(reinterpret_cast<intptr_t>(this));
    }

private:
    // Create a work helper using the specified session, work callback, after work callback, and data (template arg).
    // Lifetime is not managed, see `Create` for that option.
    WorkHelper(CASESession & session, WorkCallback workCallback, AfterWorkCallback afterWorkCallback) :
        mSession(&session), mWorkCallback(workCallback), mAfterWorkCallback(afterWorkCallback)
    {}

    // Handler for the work callback.
    static void WorkHandler(intptr_t arg)
    {
        auto * helper = reinterpret_cast<WorkHelper *>(arg);
        // Hold strong ptr while work is handled
        auto strongPtr(std::move(helper->mStrongPtr));
        VerifyOrReturn(!helper->IsCancelled());
        bool cancel = false;
        // Execute callback in background thread; data must be OK with this
        helper->mStatus = helper->mWorkCallback(helper->mData, cancel);
        VerifyOrReturn(!cancel && !helper->IsCancelled());
        // Hold strong ptr to ourselves while work is outstanding
        helper->mStrongPtr.swap(strongPtr);
        auto status = DeviceLayer::PlatformMgr().ScheduleWork(AfterWorkHandler, reinterpret_cast<intptr_t>(helper));
        if (status != CHIP_NO_ERROR)
        {
            ChipLogError(SecureChannel, "Failed to Schedule the AfterWorkCallback on foreground thread: %" CHIP_ERROR_FORMAT,
                         status.Format());

            // We failed to schedule after work callback, so setting mScheduleAfterWorkFailed flag to true
            // This can be checked from foreground thread and after work callback can be retried
            helper->mStatus = status;

            // Release strong ptr to self since scheduling failed, because nothing guarantees
            // that AfterWorkHandler will get called at this point to release the reference,
            // and we don't want to leak.  That said, we want to ensure that "helper" stays
            // alive through the end of this function (so we can set mScheduleAfterWorkFailed
            // on it), but also want to avoid racing on the single SharedPtr instance in
            // helper->mStrongPtr.  That means we need to not touch helper->mStrongPtr after
            // writing to mScheduleAfterWorkFailed.
            //
            // The simplest way to do this is to move the reference in helper->mStrongPtr to
            // our stack, where it outlives all our accesses to "helper".
            strongPtr.swap(helper->mStrongPtr);

            // helper and any of its state should not be touched after storing mScheduleAfterWorkFailed.
            helper->mScheduleAfterWorkFailed.store(true);
        }
    }

    // Handler for the after work callback.
    static void AfterWorkHandler(intptr_t arg)
    {
        // Ensure that this function is being called from main Matter thread
        assertChipStackLockedByCurrentThread();

        auto * helper = reinterpret_cast<WorkHelper *>(arg);
        // Hold strong ptr while work is handled, and ensure that helper->mStrongPtr does not keep
        // holding a reference.
        auto strongPtr(std::move(helper->mStrongPtr));
        if (!strongPtr)
        {
            // This can happen if scheduling AfterWorkHandler failed.  Just grab a strong ref
            // to handler directly, to fulfill our API contract of holding a strong reference
            // across the after-work callback.  At this point, we are guaranteed that the
            // background thread is not touching the helper anymore.
            strongPtr = helper->mWeakPtr.lock();
        }
        if (auto * session = helper->mSession.load())
        {
            // Execute callback in Matter thread; session should be OK with this
            (session->*(helper->mAfterWorkCallback))(helper->mData, helper->mStatus);
        }
    }

private:
    // Lifetime management: `ScheduleWork` sets `mStrongPtr` from `mWeakPtr`.
    Platform::WeakPtr<WorkHelper> mWeakPtr;

    // Lifetime management: `ScheduleWork` sets `mStrongPtr` from `mWeakPtr`.
    Platform::SharedPtr<WorkHelper> mStrongPtr;

    // Associated session, cleared by `CancelWork`.
    std::atomic<CASESession *> mSession;

    // Work callback, called by `WorkHandler`.
    WorkCallback mWorkCallback;

    // After work callback, called by `AfterWorkHandler`.
    AfterWorkCallback mAfterWorkCallback;

    // Return value of `mWorkCallback`, passed to `mAfterWorkCallback`.
    CHIP_ERROR mStatus;

    // If background thread fails to schedule AfterWorkCallback then this flag is set to true
    // and CASEServer then can check this one and run the AfterWorkCallback for us.
    //
    // When this happens, the write to this boolean _must_ be the last code that touches this
    // object on the background thread.  After that, the Matter thread owns the object.
    std::atomic<bool> mScheduleAfterWorkFailed{ false };

public:
    // Data passed to `mWorkCallback` and `mAfterWorkCallback`.
    DATA mData;
};

CASESession::~CASESession()
{
    // Let's clear out any security state stored in the object, before destroying it.
    Clear();
}

void CASESession::OnSessionReleased()
{
    // Call into our super-class before we clear our state.
    PairingSession::OnSessionReleased();
    Clear();
}

void CASESession::Clear()
{
    MATTER_TRACE_SCOPE("Clear", "CASESession");
    // Cancel any outstanding work.
    if (mSendSigma3Helper)
    {
        mSendSigma3Helper->CancelWork();
        mSendSigma3Helper.reset();
    }
    if (mHandleSigma3Helper)
    {
        mHandleSigma3Helper->CancelWork();
        mHandleSigma3Helper.reset();
    }

    // This function zeroes out and resets the memory used by the object.
    // It's done so that no security related information will be leaked.
    mCommissioningHash.Clear();
    PairingSession::Clear();

    mState = State::kInitialized;
    Crypto::ClearSecretData(mIPK);

    if (mFabricsTable != nullptr)
    {
        mFabricsTable->RemoveFabricDelegate(this);

        mFabricsTable->ReleaseEphemeralKeypair(mEphemeralKey);
        mEphemeralKey = nullptr;
    }

    mLocalNodeId  = kUndefinedNodeId;
    mPeerNodeId   = kUndefinedNodeId;
    mFabricsTable = nullptr;
    mFabricIndex  = kUndefinedFabricIndex;
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    // Clear the context object.
    mTCPConnCbCtxt.appContext     = nullptr;
    mTCPConnCbCtxt.connCompleteCb = nullptr;
    mTCPConnCbCtxt.connClosedCb   = nullptr;
    mTCPConnCbCtxt.connReceivedCb = nullptr;

    if (mPeerConnState)
    {
        // Set the app state callback object in the Connection state to null
        // to prevent any dangling pointer to memory(mTCPConnCbCtxt) owned
        // by the CASESession object, that is now getting cleared.
        mPeerConnState->mAppState = nullptr;

        if (mPeerConnState->mConnectionState != Transport::TCPState::kConnected)
        {
            // Abort the connection if the CASESession is being destroyed and the
            // connection is in the middle of being set up.
            mSessionManager->TCPDisconnect(mPeerConnState, /* shouldAbort = */ true);
            mPeerConnState = nullptr;
        }
    }
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT
}

void CASESession::InvalidateIfPendingEstablishmentOnFabric(FabricIndex fabricIndex)
{
    if (mFabricIndex != fabricIndex)
    {
        return;
    }
    if (!IsSessionEstablishmentInProgress())
    {
        return;
    }
    AbortPendingEstablish(CHIP_ERROR_CANCELLED);
}

CHIP_ERROR CASESession::Init(SessionManager & sessionManager, Credentials::CertificateValidityPolicy * policy,
                             SessionEstablishmentDelegate * delegate, const ScopedNodeId & sessionEvictionHint)
{
    MATTER_TRACE_SCOPE("Init", "CASESession");
    VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mGroupDataProvider != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(sessionManager.GetSessionKeystore() != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    Clear();

    ReturnErrorOnFailure(mCommissioningHash.Begin());

    mDelegate       = delegate;
    mSessionManager = &sessionManager;

    ReturnErrorOnFailure(AllocateSecureSession(sessionManager, sessionEvictionHint));

    mValidContext.Reset();
    mValidContext.mRequiredKeyUsages.Set(KeyUsageFlags::kDigitalSignature);
    mValidContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kServerAuth);
    mValidContext.mValidityPolicy = policy;

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    mTCPConnCbCtxt.appContext     = this;
    mTCPConnCbCtxt.connCompleteCb = HandleConnectionAttemptComplete;
    mTCPConnCbCtxt.connClosedCb   = HandleConnectionClosed;
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT
    return CHIP_NO_ERROR;
}

CHIP_ERROR
CASESession::PrepareForSessionEstablishment(SessionManager & sessionManager, FabricTable * fabricTable,
                                            SessionResumptionStorage * sessionResumptionStorage,
                                            Credentials::CertificateValidityPolicy * policy,
                                            SessionEstablishmentDelegate * delegate, const ScopedNodeId & previouslyEstablishedPeer,
                                            Optional<ReliableMessageProtocolConfig> mrpLocalConfig)
{
    MATTER_TRACE_SCOPE("PrepareForSessionEstablishment", "CASESession");
    // Below VerifyOrReturnError is not SuccessOrExit since we only want to goto `exit:` after
    // Init has been successfully called.
    VerifyOrReturnError(fabricTable != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(Init(sessionManager, policy, delegate, previouslyEstablishedPeer));

    CHIP_ERROR err = CHIP_NO_ERROR;

    SuccessOrExit(err = fabricTable->AddFabricDelegate(this));

    mFabricsTable             = fabricTable;
    mRole                     = CryptoContext::SessionRole::kResponder;
    mSessionResumptionStorage = sessionResumptionStorage;
    mLocalMRPConfig           = MakeOptional(mrpLocalConfig.ValueOr(GetDefaultMRPConfig()));

    ChipLogDetail(SecureChannel, "Allocated SecureSession (%p) - waiting for Sigma1 msg",
                  mSecureSessionHolder.Get().Value()->AsSecureSession());

exit:
    if (err != CHIP_NO_ERROR)
    {
        Clear();
    }
    return err;
}

CHIP_ERROR CASESession::EstablishSession(SessionManager & sessionManager, FabricTable * fabricTable, ScopedNodeId peerScopedNodeId,
                                         ExchangeContext * exchangeCtxt, SessionResumptionStorage * sessionResumptionStorage,
                                         Credentials::CertificateValidityPolicy * policy, SessionEstablishmentDelegate * delegate,
                                         Optional<ReliableMessageProtocolConfig> mrpLocalConfig)
{
    MATTER_TRACE_SCOPE("EstablishSession", "CASESession");
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Return early on error here, as we have not initialized any state yet
    VerifyOrReturnErrorWithMetric(kMetricDeviceCASESession, exchangeCtxt != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnErrorWithMetric(kMetricDeviceCASESession, fabricTable != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // Use FabricTable directly to avoid situation of dangling index from stale FabricInfo
    // until we factor-out any FabricInfo direct usage.
    VerifyOrReturnErrorWithMetric(kMetricDeviceCASESession, peerScopedNodeId.GetFabricIndex() != kUndefinedFabricIndex,
                                  CHIP_ERROR_INVALID_ARGUMENT);
    const auto * fabricInfo = fabricTable->FindFabricWithIndex(peerScopedNodeId.GetFabricIndex());
    VerifyOrReturnErrorWithMetric(kMetricDeviceCASESession, fabricInfo != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    err = Init(sessionManager, policy, delegate, peerScopedNodeId);

    mRole = CryptoContext::SessionRole::kInitiator;

    // We are setting the exchange context specifically before checking for error.
    // This is to make sure the exchange will get closed if Init() returned an error.
    mExchangeCtxt.Emplace(*exchangeCtxt);

    Transport::PeerAddress peerAddress = mExchangeCtxt.Value()->GetSessionHandle()->AsUnauthenticatedSession()->GetPeerAddress();

    // From here onwards, let's go to exit on error, as some state might have already
    // been initialized
    SuccessOrExitWithMetric(kMetricDeviceCASESession, err);

    SuccessOrExitWithMetric(kMetricDeviceCASESession, err = fabricTable->AddFabricDelegate(this));

    MATTER_LOG_METRIC_BEGIN(kMetricDeviceCASESession);

    // Set the PeerAddress in the secure session up front to indicate the
    // Transport Type of the session that is being set up.
    mSecureSessionHolder->AsSecureSession()->SetPeerAddress(peerAddress);

    mFabricsTable             = fabricTable;
    mFabricIndex              = fabricInfo->GetFabricIndex();
    mSessionResumptionStorage = sessionResumptionStorage;
    mLocalMRPConfig           = MakeOptional(mrpLocalConfig.ValueOr(GetDefaultMRPConfig()));

    mExchangeCtxt.Value()->UseSuggestedResponseTimeout(kExpectedSigma1ProcessingTime);
    mPeerNodeId  = peerScopedNodeId.GetNodeId();
    mLocalNodeId = fabricInfo->GetNodeId();

    ChipLogProgress(SecureChannel, "Initiating session on local FabricIndex %u from 0x" ChipLogFormatX64 " -> 0x" ChipLogFormatX64,
                    static_cast<unsigned>(mFabricIndex), ChipLogValueX64(mLocalNodeId), ChipLogValueX64(mPeerNodeId));

    if (peerAddress.GetTransportType() == Transport::Type::kTcp)
    {
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
        err = sessionManager.TCPConnect(peerAddress, &mTCPConnCbCtxt, &mPeerConnState);
        SuccessOrExit(err);
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT
    }
    else
    {
        MATTER_LOG_METRIC_BEGIN(kMetricDeviceCASESessionSigma1);
        err = SendSigma1();
        SuccessOrExit(err);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        MATTER_LOG_METRIC_END(kMetricDeviceCASESessionSigma1, err);
        MATTER_LOG_METRIC_END(kMetricDeviceCASESession, err);
        Clear();
    }
    return err;
}

void CASESession::OnResponseTimeout(ExchangeContext * ec)
{
    MATTER_TRACE_SCOPE("OnResponseTimeout", "CASESession");
    VerifyOrReturn(ec != nullptr, ChipLogError(SecureChannel, "CASESession::OnResponseTimeout was called by null exchange"));
    VerifyOrReturn(mExchangeCtxt.HasValue() && (&mExchangeCtxt.Value().Get() == ec),
                   ChipLogError(SecureChannel, "CASESession::OnResponseTimeout exchange doesn't match"));
    ChipLogError(SecureChannel,
                 "CASESession timed out while waiting for a response from peer " ChipLogFormatScopedNodeId ". Current state was %u",
                 ChipLogValueScopedNodeId(GetPeer()), to_underlying(mState));
    MATTER_TRACE_COUNTER("CASETimeout");
    // Discard the exchange so that Clear() doesn't try aborting it.  The
    // exchange will handle that.
    DiscardExchange();
    AbortPendingEstablish(CHIP_ERROR_TIMEOUT);
}

void CASESession::AbortPendingEstablish(CHIP_ERROR err)
{
    MATTER_LOG_METRIC_END(kMetricDeviceCASESession, err);
    MATTER_TRACE_SCOPE("AbortPendingEstablish", "CASESession");
    // This needs to come before Clear() which will reset mState.
    SessionEstablishmentStage state = MapCASEStateToSessionEstablishmentStage(mState);
    Clear();
    // Do this last in case the delegate frees us.
    NotifySessionEstablishmentError(err, state);
}

CHIP_ERROR CASESession::DeriveSecureSession(CryptoContext & session)
{
    switch (mState)
    {
    case State::kFinished: {
        std::array<uint8_t, sizeof(mIPK) + kSHA256_Hash_Length> msg_salt;

        {
            Encoding::LittleEndian::BufferWriter bbuf(msg_salt);
            bbuf.Put(mIPK, sizeof(mIPK));
            bbuf.Put(mMessageDigest, sizeof(mMessageDigest));

            VerifyOrReturnError(bbuf.Fit(), CHIP_ERROR_BUFFER_TOO_SMALL);
        }

        ReturnErrorOnFailure(session.InitFromSecret(*mSessionManager->GetSessionKeystore(), mSharedSecret.Span(),
                                                    ByteSpan(msg_salt), CryptoContext::SessionInfoType::kSessionEstablishment,
                                                    mRole));

        return CHIP_NO_ERROR;
    }
    case State::kFinishedViaResume: {
        std::array<uint8_t, sizeof(mInitiatorRandom) + decltype(mResumeResumptionId)().size()> msg_salt;

        {
            Encoding::LittleEndian::BufferWriter bbuf(msg_salt);
            bbuf.Put(mInitiatorRandom, sizeof(mInitiatorRandom));
            bbuf.Put(mResumeResumptionId.data(), mResumeResumptionId.size());

            VerifyOrReturnError(bbuf.Fit(), CHIP_ERROR_BUFFER_TOO_SMALL);
        }

        ReturnErrorOnFailure(session.InitFromSecret(*mSessionManager->GetSessionKeystore(), mSharedSecret.Span(),
                                                    ByteSpan(msg_salt), CryptoContext::SessionInfoType::kSessionResumption, mRole));

        return CHIP_NO_ERROR;
    }
    default:
        return CHIP_ERROR_INCORRECT_STATE;
    }
}

CHIP_ERROR CASESession::RecoverInitiatorIpk()
{
    Credentials::GroupDataProvider::KeySet ipkKeySet;

    CHIP_ERROR err = mGroupDataProvider->GetIpkKeySet(mFabricIndex, ipkKeySet);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SecureChannel, "Failed to obtain IPK for initiating: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }
    if ((ipkKeySet.num_keys_used == 0) || (ipkKeySet.num_keys_used > Credentials::GroupDataProvider::KeySet::kEpochKeysMax))
    {
        ChipLogError(SecureChannel, "Found invalid IPK keyset for initiator.");
        return CHIP_ERROR_INTERNAL;
    }

    // For the generation of the Destination Identifier,
    // the originator SHALL use the operational group key with the second oldest
    // EpochStartTime, if one exists, otherwise it SHALL use the single operational
    // group key available. The EpochStartTime are already ordered
    size_t ipkIndex = (ipkKeySet.num_keys_used > 1) ? ((ipkKeySet.num_keys_used - 1) - 1) : 0;
    memcpy(&mIPK[0], ipkKeySet.epoch_keys[ipkIndex].key, sizeof(mIPK));

    // Leaving this logging code for debug, but this cannot be enabled at runtime
    // since it leaks private security material.
#if 0
    ChipLogProgress(SecureChannel, "RecoverInitiatorIpk: GroupDataProvider %p, Got IPK for FabricIndex %u", mGroupDataProvider,
                    static_cast<unsigned>(mFabricIndex));
    ChipLogByteSpan(SecureChannel, ByteSpan(mIPK));
#endif

    return CHIP_NO_ERROR;
}

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
void CASESession::HandleConnectionAttemptComplete(Transport::ActiveTCPConnectionState * conn, CHIP_ERROR err)
{
    VerifyOrReturn(conn != nullptr);
    // conn->mAppState should not be NULL. SessionManager has already checked
    // before calling this callback.
    VerifyOrDie(conn->mAppState != nullptr);

    char peerAddrBuf[chip::Transport::PeerAddress::kMaxToStringSize];
    conn->mPeerAddr.ToString(peerAddrBuf);

    CASESession * caseSession = reinterpret_cast<CASESession *>(conn->mAppState->appContext);
    VerifyOrReturn(caseSession != nullptr);

    // Exit and disconnect if connection setup encountered an error.
    SuccessOrExit(err);

    ChipLogDetail(SecureChannel, "TCP Connection established with %s before session establishment", peerAddrBuf);

    // Associate the connection with the current unauthenticated session for the
    // CASE exchange.
    caseSession->mExchangeCtxt.Value()->GetSessionHandle()->AsUnauthenticatedSession()->SetTCPConnection(conn);

    // Associate the connection with the current secure session that is being
    // set up.
    caseSession->mSecureSessionHolder.Get().Value()->AsSecureSession()->SetTCPConnection(conn);

    // Send Sigma1 after connection is established for sessions over TCP
    err = caseSession->SendSigma1();
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SecureChannel, "Connection establishment failed with peer at %s: %" CHIP_ERROR_FORMAT, peerAddrBuf,
                     err.Format());

        // Close the underlying connection and ensure that the CASESession is
        // not holding on to a stale ActiveTCPConnectionState. We call
        // TCPDisconnect() here explicitly in order to abort the connection
        // even after it establishes successfully, but SendSigma1() fails for
        // some reason.
        caseSession->mSessionManager->TCPDisconnect(conn, /* shouldAbort = */ true);
        caseSession->mPeerConnState = nullptr;

        caseSession->Clear();
    }
}

void CASESession::HandleConnectionClosed(Transport::ActiveTCPConnectionState * conn, CHIP_ERROR conErr)
{
    VerifyOrReturn(conn != nullptr);
    // conn->mAppState should not be NULL. SessionManager has already checked
    // before calling this callback.
    VerifyOrDie(conn->mAppState != nullptr);

    CASESession * caseSession = reinterpret_cast<CASESession *>(conn->mAppState->appContext);
    VerifyOrReturn(caseSession != nullptr);

    // Drop our pointer to the now-invalid connection state.
    //
    // Since the connection is closed, message sends over the ExchangeContext
    // will just fail and be handled like normal send errors.
    //
    // Additionally, SessionManager notifies (via ExchangeMgr) all ExchangeContexts on the
    // connection closures for the attached sessions and the ExchangeContexts
    // can close proactively if that's appropriate.
    caseSession->mPeerConnState = nullptr;
    ChipLogDetail(SecureChannel, "TCP Connection for this session has closed");
}
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

CHIP_ERROR CASESession::SendSigma1()
{
    MATTER_TRACE_SCOPE("SendSigma1", "CASESession");

    uint8_t destinationIdentifier[kSHA256_Hash_Length] = { 0 };

    // Struct that will be used as input to EncodeSigma1() method
    EncodeSigma1Inputs encodeSigma1Inputs;

    // Lookup fabric info.
    const auto * fabricInfo = mFabricsTable->FindFabricWithIndex(mFabricIndex);
    VerifyOrReturnError(fabricInfo != nullptr, CHIP_ERROR_INCORRECT_STATE);

    // Validate that we have a session ID allocated.
    VerifyOrReturnError(GetLocalSessionId().HasValue(), CHIP_ERROR_INCORRECT_STATE);
    encodeSigma1Inputs.initiatorSessionId = GetLocalSessionId().Value();

    // Generate an ephemeral keypair
    mEphemeralKey = mFabricsTable->AllocateEphemeralKeypairForCASE();
    VerifyOrReturnError(mEphemeralKey != nullptr, CHIP_ERROR_NO_MEMORY);
    ReturnErrorOnFailure(mEphemeralKey->Initialize(ECPKeyTarget::ECDH));
    encodeSigma1Inputs.initiatorEphPubKey = &mEphemeralKey->Pubkey();

    // Fill in the random value
    ReturnErrorOnFailure(DRBG_get_bytes(mInitiatorRandom, sizeof(mInitiatorRandom)));
    encodeSigma1Inputs.initiatorRandom = ByteSpan(mInitiatorRandom);

    // Generate a Destination Identifier based on the node we are attempting to reach
    {
        // Obtain originator IPK matching the fabric where we are trying to open a session. mIPK
        // will be properly set thereafter.
        ReturnErrorOnFailure(RecoverInitiatorIpk());

        FabricId fabricId = fabricInfo->GetFabricId();
        Crypto::P256PublicKey rootPubKey;
        ReturnErrorOnFailure(mFabricsTable->FetchRootPubkey(mFabricIndex, rootPubKey));
        Credentials::P256PublicKeySpan rootPubKeySpan{ rootPubKey.ConstBytes() };

        MutableByteSpan destinationIdSpan(destinationIdentifier);
        ReturnErrorOnFailure(GenerateCaseDestinationId(ByteSpan(mIPK), encodeSigma1Inputs.initiatorRandom, rootPubKeySpan, fabricId,
                                                       mPeerNodeId, destinationIdSpan));
        encodeSigma1Inputs.destinationId = destinationIdSpan;
    }

    VerifyOrReturnError(mLocalMRPConfig.HasValue(), CHIP_ERROR_INCORRECT_STATE);
    encodeSigma1Inputs.initiatorMrpConfig = &mLocalMRPConfig.Value();

    // Try to find persistent session, and resume it.
    if (mSessionResumptionStorage != nullptr)
    {
        CHIP_ERROR err = mSessionResumptionStorage->FindByScopedNodeId(fabricInfo->GetScopedNodeIdForNode(mPeerNodeId),
                                                                       mResumeResumptionId, mSharedSecret, mPeerCATs);
        if (err == CHIP_NO_ERROR)
        {
            // Found valid resumption state, try to resume the session.
            encodeSigma1Inputs.resumptionId = mResumeResumptionId;
            MutableByteSpan resumeMICSpan(encodeSigma1Inputs.initiatorResume1MICBuffer);
            ReturnErrorOnFailure(GenerateSigmaResumeMIC(encodeSigma1Inputs.initiatorRandom, encodeSigma1Inputs.resumptionId,
                                                        ByteSpan(kKDFS1RKeyInfo), ByteSpan(kResume1MIC_Nonce), resumeMICSpan));

            encodeSigma1Inputs.initiatorResumeMIC         = resumeMICSpan;
            encodeSigma1Inputs.sessionResumptionRequested = true;
        }
    }

    System::PacketBufferHandle msgR1;

    // Encode Sigma1 in CHIP TLV Format
    ReturnErrorOnFailure(EncodeSigma1(msgR1, encodeSigma1Inputs));

    ReturnErrorOnFailure(mCommissioningHash.AddData(ByteSpan{ msgR1->Start(), msgR1->DataLength() }));

    // Call delegate to send the msg to peer
    ReturnErrorOnFailure(mExchangeCtxt.Value()->SendMessage(Protocols::SecureChannel::MsgType::CASE_Sigma1, std::move(msgR1),
                                                            SendFlags(SendMessageFlags::kExpectResponse)));

    if (encodeSigma1Inputs.sessionResumptionRequested)
    {
        mState = State::kSentSigma1Resume;

        // Flags that Resume is being attempted
        MATTER_LOG_METRIC(kMetricDeviceCASESessionSigma1Resume);
    }
    else
    {
        mState = State::kSentSigma1;
    }

#if CHIP_PROGRESS_LOGGING
    const auto localMRPConfig = mLocalMRPConfig.Value();
#endif // CHIP_PROGRESS_LOGGING
    ChipLogProgress(SecureChannel, "Sent Sigma1 msg to " ChipLogFormatScopedNodeId " [II:%" PRIu32 "ms AI:%" PRIu32 "ms AT:%ums]",
                    ChipLogValueScopedNodeId(GetPeer()), localMRPConfig.mIdleRetransTimeout.count(),
                    localMRPConfig.mActiveRetransTimeout.count(), localMRPConfig.mActiveThresholdTime.count());

    mDelegate->OnSessionEstablishmentStarted();

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::EncodeSigma1(System::PacketBufferHandle & msg, EncodeSigma1Inputs & input)
{
    MATTER_TRACE_SCOPE("EncodeSigma1", "CASESession");

    VerifyOrReturnError(input.initiatorEphPubKey != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    size_t dataLen = EstimateStructOverhead(kSigmaParamRandomNumberSize,                 // initiatorRandom
                                            sizeof(uint16_t),                            // initiatorSessionId,
                                            kSHA256_Hash_Length,                         // destinationId
                                            kP256_PublicKey_Length,                      // InitiatorEphPubKey,
                                            SessionParameters::kEstimatedTLVSize,        // initiatorSessionParams
                                            SessionResumptionStorage::kResumptionIdSize, // resumptionId
                                            CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES            // initiatorResumeMIC
    );

    msg = System::PacketBufferHandle::New(dataLen);
    VerifyOrReturnError(!msg.IsNull(), CHIP_ERROR_NO_MEMORY);

    System::PacketBufferTLVWriter tlvWriter;
    tlvWriter.Init(std::move(msg));

    TLVType outerContainerType = kTLVType_NotSpecified;
    ReturnErrorOnFailure(tlvWriter.StartContainer(AnonymousTag(), kTLVType_Structure, outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Put(AsTlvContextTag(Sigma1Tags::kInitiatorRandom), input.initiatorRandom));
    ReturnErrorOnFailure(tlvWriter.Put(AsTlvContextTag(Sigma1Tags::kInitiatorSessionId), input.initiatorSessionId));
    ReturnErrorOnFailure(tlvWriter.Put(AsTlvContextTag(Sigma1Tags::kDestinationId), input.destinationId));

    ReturnErrorOnFailure(tlvWriter.PutBytes(AsTlvContextTag(Sigma1Tags::kInitiatorEphPubKey), *input.initiatorEphPubKey,
                                            static_cast<uint32_t>(input.initiatorEphPubKey->Length())));

    VerifyOrReturnError(input.initiatorMrpConfig != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(
        EncodeSessionParameters(AsTlvContextTag(Sigma1Tags::kInitiatorSessionParams), *input.initiatorMrpConfig, tlvWriter));

    if (input.sessionResumptionRequested)
    {
        ReturnErrorOnFailure(tlvWriter.Put(AsTlvContextTag(Sigma1Tags::kResumptionID), input.resumptionId));
        ReturnErrorOnFailure(tlvWriter.Put(AsTlvContextTag(Sigma1Tags::kResume1MIC), input.initiatorResumeMIC));
    }

    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Finalize(&msg));

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::HandleSigma1_and_SendSigma2(System::PacketBufferHandle && msg)
{
    MATTER_TRACE_SCOPE("HandleSigma1_and_SendSigma2", "CASESession");

    CHIP_ERROR err = CHIP_NO_ERROR;

    // Parse and Validate Received Sigma1, and decide next step
    NextStep nextStep = HandleSigma1(std::move(msg));
    VerifyOrExit(nextStep.Is<Step>(), err = nextStep.Get<CHIP_ERROR>());

    switch (nextStep.Get<Step>())
    {
    case Step::kSendSigma2: {

        System::PacketBufferHandle msgR2;
        EncodeSigma2Inputs encodeSigma2;

        SuccessOrExit(err = PrepareSigma2(encodeSigma2));
        SuccessOrExit(err = EncodeSigma2(msgR2, encodeSigma2));

        MATTER_LOG_METRIC_BEGIN(kMetricDeviceCASESessionSigma2);
        SuccessOrExitAction(err = SendSigma2(std::move(msgR2)), MATTER_LOG_METRIC_END(kMetricDeviceCASESessionSigma2, err));

        mDelegate->OnSessionEstablishmentStarted();
        break;
    }
    case Step::kSendSigma2Resume: {

        System::PacketBufferHandle msgR2Resume;
        EncodeSigma2ResumeInputs encodeSigma2Resume;

        SuccessOrExit(err = PrepareSigma2Resume(encodeSigma2Resume));
        SuccessOrExit(err = EncodeSigma2Resume(msgR2Resume, encodeSigma2Resume));

        MATTER_LOG_METRIC_BEGIN(kMetricDeviceCASESessionSigma2Resume);
        SuccessOrExitAction(err = SendSigma2Resume(std::move(msgR2Resume)),
                            MATTER_LOG_METRIC_END(kMetricDeviceCASESessionSigma2Resume, err));

        mDelegate->OnSessionEstablishmentStarted();
        break;
    }
    default:
        break;
    }

exit:
    if (err == CHIP_ERROR_KEY_NOT_FOUND)
    {
        SendStatusReport(mExchangeCtxt, kProtocolCodeNoSharedRoot);
        mState = State::kInitialized;
    }
    else if (err != CHIP_NO_ERROR)
    {
        SendStatusReport(mExchangeCtxt, kProtocolCodeInvalidParam);
        mState = State::kInitialized;
    }
    return err;
}

CHIP_ERROR CASESession::FindLocalNodeFromDestinationId(const ByteSpan & destinationId, const ByteSpan & initiatorRandom)
{
    MATTER_TRACE_SCOPE("FindLocalNodeFromDestinationId", "CASESession");
    VerifyOrReturnError(mFabricsTable != nullptr, CHIP_ERROR_INCORRECT_STATE);

    bool found = false;
    for (const FabricInfo & fabricInfo : *mFabricsTable)
    {
        // Basic data for candidate fabric, used to compute candidate destination identifiers
        FabricId fabricId = fabricInfo.GetFabricId();
        NodeId nodeId     = fabricInfo.GetNodeId();
        Crypto::P256PublicKey rootPubKey;
        ReturnErrorOnFailure(mFabricsTable->FetchRootPubkey(fabricInfo.GetFabricIndex(), rootPubKey));
        Credentials::P256PublicKeySpan rootPubKeySpan{ rootPubKey.ConstBytes() };

        // Get IPK operational group key set for current candidate fabric
        GroupDataProvider::KeySet ipkKeySet;
        CHIP_ERROR err = mGroupDataProvider->GetIpkKeySet(fabricInfo.GetFabricIndex(), ipkKeySet);
        if ((err != CHIP_NO_ERROR) ||
            ((ipkKeySet.num_keys_used == 0) || (ipkKeySet.num_keys_used > Credentials::GroupDataProvider::KeySet::kEpochKeysMax)))
        {
            continue;
        }

        // Try every IPK candidate we have for a match
        for (size_t keyIdx = 0; keyIdx < ipkKeySet.num_keys_used; ++keyIdx)
        {
            uint8_t candidateDestinationId[kSHA256_Hash_Length];
            MutableByteSpan candidateDestinationIdSpan(candidateDestinationId);
            ByteSpan candidateIpkSpan(ipkKeySet.epoch_keys[keyIdx].key);

            err = GenerateCaseDestinationId(candidateIpkSpan, initiatorRandom, rootPubKeySpan, fabricId, nodeId,
                                            candidateDestinationIdSpan);
            if ((err == CHIP_NO_ERROR) && (candidateDestinationIdSpan.data_equal(destinationId)))
            {
                // Found a match, stop working, cache IPK, update local fabric context
                found = true;
                MutableByteSpan ipkSpan(mIPK);
                CopySpanToMutableSpan(candidateIpkSpan, ipkSpan);
                mFabricIndex = fabricInfo.GetFabricIndex();
                mLocalNodeId = nodeId;
                break;
            }
        }

        if (found)
        {
            break;
        }
    }

    return found ? CHIP_NO_ERROR : CHIP_ERROR_KEY_NOT_FOUND;
}

CHIP_ERROR CASESession::TryResumeSession(SessionResumptionStorage::ConstResumptionIdView resumptionId, ByteSpan resume1MIC,
                                         ByteSpan initiatorRandom)
{
    MATTER_TRACE_SCOPE("TryResumeSession", "CASESession");
    VerifyOrReturnError(mSessionResumptionStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mFabricsTable != nullptr, CHIP_ERROR_INCORRECT_STATE);

    SessionResumptionStorage::ConstResumptionIdView resumptionIdSpan(resumptionId);
    ScopedNodeId node;
    ReturnErrorOnFailure(mSessionResumptionStorage->FindByResumptionId(resumptionIdSpan, node, mSharedSecret, mPeerCATs));

    // Cross check resume1MIC with the shared secret
    ReturnErrorOnFailure(
        ValidateSigmaResumeMIC(resume1MIC, initiatorRandom, resumptionId, ByteSpan(kKDFS1RKeyInfo), ByteSpan(kResume1MIC_Nonce)));

    const auto * fabricInfo = mFabricsTable->FindFabricWithIndex(node.GetFabricIndex());
    VerifyOrReturnError(fabricInfo != nullptr, CHIP_ERROR_INCORRECT_STATE);

    mFabricIndex = node.GetFabricIndex();
    mPeerNodeId  = node.GetNodeId();
    mLocalNodeId = fabricInfo->GetNodeId();

    return CHIP_NO_ERROR;
}
CASESession::NextStep CASESession::HandleSigma1(System::PacketBufferHandle && msg)
{
    MATTER_TRACE_SCOPE("HandleSigma1", "CASESession");
    ChipLogProgress(SecureChannel, "Received Sigma1 msg");
    MATTER_TRACE_COUNTER("Sigma1");

    VerifyOrReturnError(mFabricsTable != nullptr, NextStep::Create<CHIP_ERROR>(CHIP_ERROR_INCORRECT_STATE));

    ReturnErrorVariantOnFailure(NextStep, mCommissioningHash.AddData(ByteSpan{ msg->Start(), msg->DataLength() }));

    System::PacketBufferTLVReader tlvReader;
    tlvReader.Init(std::move(msg));

    // Struct that will serve as output in ParseSigma1
    ParsedSigma1 parsedSigma1;

    ReturnErrorVariantOnFailure(NextStep, ParseSigma1(tlvReader, parsedSigma1));

    ChipLogDetail(SecureChannel, "Peer (Initiator) assigned session ID %d", parsedSigma1.initiatorSessionId);
    SetPeerSessionId(parsedSigma1.initiatorSessionId);

    // Set the Session parameters provided in the Sigma1 message
    if (parsedSigma1.initiatorSessionParamStructPresent)
    {
        SetRemoteSessionParameters(parsedSigma1.initiatorSessionParams);
        mExchangeCtxt.Value()->GetSessionHandle()->AsUnauthenticatedSession()->SetRemoteSessionParameters(
            GetRemoteSessionParameters());
    }

    if (parsedSigma1.sessionResumptionRequested &&
        parsedSigma1.resumptionId.size() == SessionResumptionStorage::kResumptionIdSize &&
        CHIP_NO_ERROR ==
            TryResumeSession(SessionResumptionStorage::ConstResumptionIdView(parsedSigma1.resumptionId.data()),
                             parsedSigma1.initiatorResumeMIC, parsedSigma1.initiatorRandom))
    {
        std::copy(parsedSigma1.initiatorRandom.begin(), parsedSigma1.initiatorRandom.end(), mInitiatorRandom);
        std::copy(parsedSigma1.resumptionId.begin(), parsedSigma1.resumptionId.end(), mResumeResumptionId.begin());

        //  Early returning here, since the next Step is known to be Sigma2Resume, and no further processing is needed for the
        //  Sigma1 message
        return NextStep::Create<Step>(Step::kSendSigma2Resume);
    }

    //  ParseSigma1 ensures that:
    //  mRemotePubKey.Length() == initiatorPubKey.size() == kP256_PublicKey_Length.
    memcpy(mRemotePubKey.Bytes(), parsedSigma1.initiatorEphPubKey.data(), mRemotePubKey.Length());

    CHIP_ERROR err = CHIP_NO_ERROR;

    // Attempt to match the initiator's desired destination based on local fabric table.
    err = FindLocalNodeFromDestinationId(parsedSigma1.destinationId, parsedSigma1.initiatorRandom);
    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(SecureChannel, "CASE matched destination ID: fabricIndex %u, NodeID 0x" ChipLogFormatX64,
                        static_cast<unsigned>(mFabricIndex), ChipLogValueX64(mLocalNodeId));

        // Side-effect of FindLocalNodeFromDestinationId success was that mFabricIndex/mLocalNodeId are now
        // set to the local fabric and associated NodeId that was targeted by the initiator.

        return NextStep::Create<Step>(Step::kSendSigma2);
    }

    ChipLogError(SecureChannel, "CASE failed to match destination ID with local fabrics");
    ChipLogByteSpan(SecureChannel, parsedSigma1.destinationId);

    // FindLocalNodeFromDestinationId returns CHIP_ERROR_KEY_NOT_FOUND if Sigma1's DestinationId does not match any
    // candidateDestinationId, this will trigger a status Report with ProtocolCode = NoSharedTrustRoots.

    // Returning a CHIP_ERROR variant that will trigger a corresponding Status Report.
    return NextStep::Create<CHIP_ERROR>(err);
}

CHIP_ERROR CASESession::PrepareSigma2Resume(EncodeSigma2ResumeInputs & outSigma2ResData)
{
    MATTER_TRACE_SCOPE("PrepareSigma2Resume", "CASESession");

    VerifyOrReturnError(mLocalMRPConfig.HasValue(), CHIP_ERROR_INCORRECT_STATE);

    // Validate that we have a session ID allocated.
    VerifyOrReturnError(GetLocalSessionId().HasValue(), CHIP_ERROR_INCORRECT_STATE);
    outSigma2ResData.responderSessionId = GetLocalSessionId().Value();

    // Generate a new resumption ID
    ReturnErrorOnFailure(DRBG_get_bytes(mNewResumptionId.data(), mNewResumptionId.size()));
    outSigma2ResData.resumptionId = mNewResumptionId;

    ReturnErrorOnFailure(GenerateSigmaResumeMIC(ByteSpan(mInitiatorRandom), mNewResumptionId, ByteSpan(kKDFS2RKeyInfo),
                                                ByteSpan(kResume2MIC_Nonce), outSigma2ResData.sigma2ResumeMIC));

    outSigma2ResData.responderMrpConfig = &mLocalMRPConfig.Value();

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::EncodeSigma2Resume(System::PacketBufferHandle & msgR2Resume, EncodeSigma2ResumeInputs & input)
{
    MATTER_TRACE_SCOPE("EncodeSigma2Resume", "CASESession");

    VerifyOrReturnError(input.responderMrpConfig != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    size_t maxDatalLen = EstimateStructOverhead(SessionResumptionStorage::kResumptionIdSize, // resumptionID
                                                CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES,           // sigma2ResumeMIC
                                                sizeof(uint16_t),                            // responderSessionID
                                                SessionParameters::kEstimatedTLVSize         // responderSessionParams
    );

    msgR2Resume = System::PacketBufferHandle::New(maxDatalLen);
    VerifyOrReturnError(!msgR2Resume.IsNull(), CHIP_ERROR_NO_MEMORY);

    System::PacketBufferTLVWriter tlvWriter;
    tlvWriter.Init(std::move(msgR2Resume));

    TLVType outerContainerType = kTLVType_NotSpecified;

    ReturnErrorOnFailure(tlvWriter.StartContainer(AnonymousTag(), kTLVType_Structure, outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Put(AsTlvContextTag(Sigma2ResumeTags::kResumptionID), input.resumptionId));
    ReturnErrorOnFailure(tlvWriter.Put(AsTlvContextTag(Sigma2ResumeTags::kSigma2ResumeMIC), input.sigma2ResumeMIC));
    ReturnErrorOnFailure(tlvWriter.Put(AsTlvContextTag(Sigma2ResumeTags::kResponderSessionID), input.responderSessionId));

    ReturnErrorOnFailure(
        EncodeSessionParameters(AsTlvContextTag(Sigma2ResumeTags::kResponderSessionParams), *input.responderMrpConfig, tlvWriter));

    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Finalize(&msgR2Resume));

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::SendSigma2Resume(System::PacketBufferHandle && msgR2Resume)
{

    // Call delegate to send the msg to peer
    ReturnErrorOnFailure(mExchangeCtxt.Value()->SendMessage(Protocols::SecureChannel::MsgType::CASE_Sigma2Resume,
                                                            std::move(msgR2Resume), SendFlags(SendMessageFlags::kExpectResponse)));

    mState = State::kSentSigma2Resume;

    ChipLogDetail(SecureChannel, "Sent Sigma2Resume msg");

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::PrepareSigma2(EncodeSigma2Inputs & outSigma2Data)
{

    MATTER_TRACE_SCOPE("PrepareSigma2", "CASESession");

    VerifyOrReturnError(mFabricsTable != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mLocalMRPConfig.HasValue(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(GetLocalSessionId().HasValue(), CHIP_ERROR_INCORRECT_STATE);
    outSigma2Data.responderSessionId = GetLocalSessionId().Value();

    chip::Platform::ScopedMemoryBuffer<uint8_t> icacBuf;
    VerifyOrReturnError(icacBuf.Alloc(kMaxCHIPCertLength), CHIP_ERROR_NO_MEMORY);

    chip::Platform::ScopedMemoryBuffer<uint8_t> nocBuf;
    VerifyOrReturnError(nocBuf.Alloc(kMaxCHIPCertLength), CHIP_ERROR_NO_MEMORY);

    MutableByteSpan icaCert{ icacBuf.Get(), kMaxCHIPCertLength };
    ReturnErrorOnFailure(mFabricsTable->FetchICACert(mFabricIndex, icaCert));

    MutableByteSpan nocCert{ nocBuf.Get(), kMaxCHIPCertLength };
    ReturnErrorOnFailure(mFabricsTable->FetchNOCCert(mFabricIndex, nocCert));

    // Fill in the random value
    ReturnErrorOnFailure(DRBG_get_bytes(&outSigma2Data.responderRandom[0], sizeof(outSigma2Data.responderRandom)));

    // Generate an ephemeral keypair
    mEphemeralKey = mFabricsTable->AllocateEphemeralKeypairForCASE();
    VerifyOrReturnError(mEphemeralKey != nullptr, CHIP_ERROR_NO_MEMORY);
    ReturnErrorOnFailure(mEphemeralKey->Initialize(ECPKeyTarget::ECDH));
    outSigma2Data.responderEphPubKey = &mEphemeralKey->Pubkey();

    // Generate a Shared Secret
    ReturnErrorOnFailure(mEphemeralKey->ECDH_derive_secret(mRemotePubKey, mSharedSecret));

    uint8_t msgSalt[kIPKSize + kSigmaParamRandomNumberSize + kP256_PublicKey_Length + kSHA256_Hash_Length];

    MutableByteSpan saltSpan(msgSalt);
    ReturnErrorOnFailure(
        ConstructSaltSigma2(ByteSpan(outSigma2Data.responderRandom), mEphemeralKey->Pubkey(), ByteSpan(mIPK), saltSpan));

    AutoReleaseSessionKey sr2k(*mSessionManager->GetSessionKeystore());
    ReturnErrorOnFailure(DeriveSigmaKey(saltSpan, ByteSpan(kKDFSR2Info), sr2k));

    // Construct Sigma2 TBS Data
    P256ECDSASignature tbsData2Signature;
    {
        size_t msgR2SignedLen = EstimateStructOverhead(kMaxCHIPCertLength,     // responderNoc
                                                       kMaxCHIPCertLength,     // responderICAC
                                                       kP256_PublicKey_Length, // responderEphPubKey
                                                       kP256_PublicKey_Length  // InitiatorEphPubKey
        );

        chip::Platform::ScopedMemoryBuffer<uint8_t> msgR2Signed;
        VerifyOrReturnError(msgR2Signed.Alloc(msgR2SignedLen), CHIP_ERROR_NO_MEMORY);
        MutableByteSpan msgR2SignedSpan{ msgR2Signed.Get(), msgR2SignedLen };

        ReturnErrorOnFailure(ConstructTBSData(nocCert, icaCert, ByteSpan(mEphemeralKey->Pubkey(), mEphemeralKey->Pubkey().Length()),
                                              ByteSpan(mRemotePubKey, mRemotePubKey.Length()), msgR2SignedSpan));

        // Generate a Signature
        ReturnErrorOnFailure(mFabricsTable->SignWithOpKeypair(mFabricIndex, msgR2SignedSpan, tbsData2Signature));
    }
    // Construct Sigma2 TBE Data
    size_t msgR2SignedEncLen = EstimateStructOverhead(nocCert.size(),                             // responderNoc
                                                      icaCert.size(),                             // responderICAC
                                                      tbsData2Signature.Length(),                 // signature
                                                      SessionResumptionStorage::kResumptionIdSize // resumptionID
    );

    VerifyOrReturnError(outSigma2Data.msgR2Encrypted.Alloc(msgR2SignedEncLen + CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES),
                        CHIP_ERROR_NO_MEMORY);

    TLVWriter tlvWriter;
    tlvWriter.Init(outSigma2Data.msgR2Encrypted.Get(), msgR2SignedEncLen);

    TLVType outerContainerType = kTLVType_NotSpecified;

    ReturnErrorOnFailure(tlvWriter.StartContainer(AnonymousTag(), kTLVType_Structure, outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Put(AsTlvContextTag(TBEDataTags::kSenderNOC), nocCert));
    if (!icaCert.empty())
    {
        ReturnErrorOnFailure(tlvWriter.Put(AsTlvContextTag(TBEDataTags::kSenderICAC), icaCert));
    }

    // We are now done with ICAC and NOC certs so we can release the memory.
    {
        icacBuf.Free();
        icaCert = MutableByteSpan{};

        nocBuf.Free();
        nocCert = MutableByteSpan{};
    }

    ReturnErrorOnFailure(tlvWriter.PutBytes(AsTlvContextTag(TBEDataTags::kSignature), tbsData2Signature.ConstBytes(),
                                            static_cast<uint32_t>(tbsData2Signature.Length())));

    // Generate a new resumption ID
    ReturnErrorOnFailure(DRBG_get_bytes(mNewResumptionId.data(), mNewResumptionId.size()));
    ReturnErrorOnFailure(tlvWriter.Put(AsTlvContextTag(TBEDataTags::kResumptionID), mNewResumptionId));

    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Finalize());
    msgR2SignedEncLen              = static_cast<size_t>(tlvWriter.GetLengthWritten());
    outSigma2Data.encrypted2Length = msgR2SignedEncLen + CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES;
    // Generate the encrypted data blob
    ReturnErrorOnFailure(AES_CCM_encrypt(outSigma2Data.msgR2Encrypted.Get(), msgR2SignedEncLen, nullptr, 0, sr2k.KeyHandle(),
                                         kTBEData2_Nonce, kTBEDataNonceLength, outSigma2Data.msgR2Encrypted.Get(),
                                         outSigma2Data.msgR2Encrypted.Get() + msgR2SignedEncLen,
                                         CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES));

    outSigma2Data.responderMrpConfig = &mLocalMRPConfig.Value();

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::EncodeSigma2(System::PacketBufferHandle & msgR2, EncodeSigma2Inputs & input)
{
    VerifyOrReturnError(input.responderEphPubKey != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(input.msgR2Encrypted, CHIP_ERROR_INCORRECT_STATE);
    // Check if length of msgR2Encrypted is set and is at least larger than the MIC length
    VerifyOrReturnError(input.encrypted2Length > CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(input.responderMrpConfig != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    size_t dataLen = EstimateStructOverhead(kSigmaParamRandomNumberSize,         // responderRandom
                                            sizeof(uint16_t),                    // responderSessionId
                                            kP256_PublicKey_Length,              // responderEphPubKey
                                            input.encrypted2Length,              // encrypted2
                                            SessionParameters::kEstimatedTLVSize // responderSessionParams
    );

    msgR2 = System::PacketBufferHandle::New(dataLen);
    VerifyOrReturnError(!msgR2.IsNull(), CHIP_ERROR_NO_MEMORY);

    System::PacketBufferTLVWriter tlvWriterMsg2;
    tlvWriterMsg2.Init(std::move(msgR2));

    TLVType outerContainerType = kTLVType_NotSpecified;

    ReturnErrorOnFailure(tlvWriterMsg2.StartContainer(AnonymousTag(), kTLVType_Structure, outerContainerType));

    ReturnErrorOnFailure(tlvWriterMsg2.PutBytes(AsTlvContextTag(Sigma2Tags::kResponderRandom), &input.responderRandom[0],
                                                sizeof(input.responderRandom)));
    ReturnErrorOnFailure(tlvWriterMsg2.Put(AsTlvContextTag(Sigma2Tags::kResponderSessionId), input.responderSessionId));

    ReturnErrorOnFailure(tlvWriterMsg2.PutBytes(AsTlvContextTag(Sigma2Tags::kResponderEphPubKey), *input.responderEphPubKey,
                                                static_cast<uint32_t>(input.responderEphPubKey->Length())));

    ReturnErrorOnFailure(tlvWriterMsg2.PutBytes(AsTlvContextTag(Sigma2Tags::kEncrypted2), input.msgR2Encrypted.Get(),
                                                static_cast<uint32_t>(input.encrypted2Length)));
    input.msgR2Encrypted.Free();

    ReturnErrorOnFailure(
        EncodeSessionParameters(AsTlvContextTag(Sigma2Tags::kResponderSessionParams), *input.responderMrpConfig, tlvWriterMsg2));

    ReturnErrorOnFailure(tlvWriterMsg2.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriterMsg2.Finalize(&msgR2));

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::SendSigma2(System::PacketBufferHandle && msgR2)
{
    MATTER_TRACE_SCOPE("SendSigma2", "CASESession");

    ReturnErrorOnFailure(mCommissioningHash.AddData(ByteSpan{ msgR2->Start(), msgR2->DataLength() }));

    // Call delegate to send the msg to peer
    ReturnErrorOnFailure(mExchangeCtxt.Value()->SendMessage(Protocols::SecureChannel::MsgType::CASE_Sigma2, std::move(msgR2),
                                                            SendFlags(SendMessageFlags::kExpectResponse)));

    mState = State::kSentSigma2;

    ChipLogProgress(SecureChannel, "Sent Sigma2 msg");
    MATTER_TRACE_COUNTER("Sigma2");

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::HandleSigma2Resume(System::PacketBufferHandle && msg)
{
    MATTER_TRACE_SCOPE("HandleSigma2Resume", "CASESession");
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogDetail(SecureChannel, "Received Sigma2Resume msg");
    MATTER_TRACE_COUNTER("Sigma2Resume");
    MATTER_LOG_METRIC_END(kMetricDeviceCASESessionSigma1, err);

    System::PacketBufferTLVReader tlvReader;
    tlvReader.Init(std::move(msg));
    ParsedSigma2Resume parsedSigma2Resume;
    SuccessOrExit(err = ParseSigma2Resume(tlvReader, parsedSigma2Resume));

    SuccessOrExit(err = ValidateSigmaResumeMIC(parsedSigma2Resume.sigma2ResumeMIC, ByteSpan(mInitiatorRandom),
                                               parsedSigma2Resume.resumptionId, ByteSpan(kKDFS2RKeyInfo),
                                               ByteSpan(kResume2MIC_Nonce)));

    if (parsedSigma2Resume.responderSessionParamStructPresent)
    {
        SetRemoteSessionParameters(parsedSigma2Resume.responderSessionParams);
        mExchangeCtxt.Value()->GetSessionHandle()->AsUnauthenticatedSession()->SetRemoteSessionParameters(
            GetRemoteSessionParameters());
    }

    ChipLogDetail(SecureChannel, "Peer " ChipLogFormatScopedNodeId " assigned session ID %d", ChipLogValueScopedNodeId(GetPeer()),
                  parsedSigma2Resume.responderSessionId);
    SetPeerSessionId(parsedSigma2Resume.responderSessionId);

    if (mSessionResumptionStorage != nullptr)
    {
        CHIP_ERROR err2 = mSessionResumptionStorage->Save(
            GetPeer(), SessionResumptionStorage::ConstResumptionIdView(parsedSigma2Resume.resumptionId.data()), mSharedSecret,
            mPeerCATs);
        if (err2 != CHIP_NO_ERROR)
            ChipLogError(SecureChannel, "Unable to save session resumption state: %" CHIP_ERROR_FORMAT, err2.Format());
    }

    MATTER_LOG_METRIC(kMetricDeviceCASESessionSigmaFinished);
    SendStatusReport(mExchangeCtxt, kProtocolCodeSuccess);

    mState = State::kFinishedViaResume;
    Finish();

exit:
    if (err != CHIP_NO_ERROR)
    {
        SendStatusReport(mExchangeCtxt, kProtocolCodeInvalidParam);
    }
    return err;
}

CHIP_ERROR CASESession::ParseSigma2Resume(ContiguousBufferTLVReader & tlvReader, ParsedSigma2Resume & outParsedSigma2Resume)
{
    TLVType containerType = kTLVType_Structure;

    ReturnErrorOnFailure(tlvReader.Next(containerType, AnonymousTag()));
    ReturnErrorOnFailure(tlvReader.EnterContainer(containerType));

    ReturnErrorOnFailure(tlvReader.Next(AsTlvContextTag(Sigma2ResumeTags::kResumptionID)));
    ReturnErrorOnFailure(tlvReader.GetByteView(outParsedSigma2Resume.resumptionId));
    VerifyOrReturnError(outParsedSigma2Resume.resumptionId.size() == SessionResumptionStorage::kResumptionIdSize,
                        CHIP_ERROR_INVALID_CASE_PARAMETER);

    ReturnErrorOnFailure(tlvReader.Next(AsTlvContextTag(Sigma2ResumeTags::kSigma2ResumeMIC)));
    ReturnErrorOnFailure(tlvReader.GetByteView(outParsedSigma2Resume.sigma2ResumeMIC));
    VerifyOrReturnError(outParsedSigma2Resume.sigma2ResumeMIC.size() == CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES,
                        CHIP_ERROR_INVALID_CASE_PARAMETER);

    ReturnErrorOnFailure(tlvReader.Next(AsTlvContextTag(Sigma2ResumeTags::kResponderSessionID)));
    ReturnErrorOnFailure(tlvReader.Get(outParsedSigma2Resume.responderSessionId));

    CHIP_ERROR err = tlvReader.Next();
    if (err == CHIP_NO_ERROR && tlvReader.GetTag() == AsTlvContextTag(Sigma2ResumeTags::kResponderSessionParams))
    {
        ReturnErrorOnFailure(DecodeSessionParametersIfPresent(AsTlvContextTag(Sigma2ResumeTags::kResponderSessionParams), tlvReader,
                                                              outParsedSigma2Resume.responderSessionParams));
        outParsedSigma2Resume.responderSessionParamStructPresent = true;

        err = tlvReader.Next();
    }

    // Future-proofing: CHIP_NO_ERROR will be returned by Next() if we have additional non-parsed TLV Elements, which could
    // happen in the future if additional elements are added to the specification.
    VerifyOrReturnError(err == CHIP_END_OF_TLV || err == CHIP_NO_ERROR, err);
    // Exit Container will fail (return CHIP_END_OF_TLV) if the received encoded message is not properly terminated with an
    // EndOfContainer TLV Element.
    ReturnErrorOnFailure(tlvReader.ExitContainer(containerType));

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::HandleSigma2_and_SendSigma3(System::PacketBufferHandle && msg)
{
    MATTER_TRACE_SCOPE("HandleSigma2_and_SendSigma3", "CASESession");
    CHIP_ERROR err = HandleSigma2(std::move(msg));
    MATTER_LOG_METRIC_END(kMetricDeviceCASESessionSigma1, err);
    SuccessOrExit(err);

    MATTER_LOG_METRIC_BEGIN(kMetricDeviceCASESessionSigma3);
    err = SendSigma3a();
    if (CHIP_NO_ERROR != err)
    {
        MATTER_LOG_METRIC_END(kMetricDeviceCASESessionSigma3, err);
    }

exit:
    if (CHIP_NO_ERROR != err)
    {
        SendStatusReport(mExchangeCtxt, kProtocolCodeInvalidParam);
        mState = State::kInitialized;
    }
    return err;
}

CHIP_ERROR CASESession::HandleSigma2(System::PacketBufferHandle && msg)
{
    MATTER_TRACE_SCOPE("HandleSigma2", "CASESession");
    ChipLogProgress(SecureChannel, "Received Sigma2 msg");

    VerifyOrReturnError(mEphemeralKey != nullptr, CHIP_ERROR_INTERNAL);

    const uint8_t * buf = msg->Start();
    size_t buflen       = msg->DataLength();
    VerifyOrReturnError(buf != nullptr, CHIP_ERROR_MESSAGE_INCOMPLETE);

    FabricId fabricId = kUndefinedFabricId;
    {
        VerifyOrReturnError(mFabricsTable != nullptr, CHIP_ERROR_INCORRECT_STATE);
        const auto * fabricInfo = mFabricsTable->FindFabricWithIndex(mFabricIndex);
        VerifyOrReturnError(fabricInfo != nullptr, CHIP_ERROR_INCORRECT_STATE);
        fabricId = fabricInfo->GetFabricId();
    }

    System::PacketBufferTLVReader tlvReader;
    tlvReader.Init(std::move(msg));
    ParsedSigma2 parsedSigma2;
    ReturnErrorOnFailure(ParseSigma2(tlvReader, parsedSigma2));

    //  ParseSigma2 ensures that:
    //  mRemotePubKey.Length() == responderEphPubKey.size() == kP256_PublicKey_Length.
    memcpy(mRemotePubKey.Bytes(), parsedSigma2.responderEphPubKey.data(), mRemotePubKey.Length());

    // Generate a Shared Secret
    ReturnErrorOnFailure(mEphemeralKey->ECDH_derive_secret(mRemotePubKey, mSharedSecret));

    // Generate the S2K key
    AutoReleaseSessionKey sr2k(*mSessionManager->GetSessionKeystore());
    {
        uint8_t msg_salt[kIPKSize + kSigmaParamRandomNumberSize + kP256_PublicKey_Length + kSHA256_Hash_Length];
        MutableByteSpan saltSpan(msg_salt);
        ReturnErrorOnFailure(ConstructSaltSigma2(parsedSigma2.responderRandom, mRemotePubKey, ByteSpan(mIPK), saltSpan));
        ReturnErrorOnFailure(DeriveSigmaKey(saltSpan, ByteSpan(kKDFSR2Info), sr2k));
    }
    // Msg2 should only be added to MessageDigest after we construct SaltSigma2 that is used to derive S2K,
    // Because constructing SaltSigma2 uses the MessageDigest at a point when it should only include Msg1.
    ReturnErrorOnFailure(mCommissioningHash.AddData(ByteSpan{ buf, buflen }));

    ReturnErrorOnFailure(AES_CCM_decrypt(parsedSigma2.msgR2EncryptedPayload.data(), parsedSigma2.msgR2EncryptedPayload.size(),
                                         nullptr, 0, parsedSigma2.msgR2MIC.data(), parsedSigma2.msgR2MIC.size(), sr2k.KeyHandle(),
                                         kTBEData2_Nonce, kTBEDataNonceLength, parsedSigma2.msgR2EncryptedPayload.data()));

    parsedSigma2.msgR2Decrypted = std::move(parsedSigma2.msgR2Encrypted);
    size_t msgR2DecryptedLength = parsedSigma2.msgR2EncryptedPayload.size();

    ContiguousBufferTLVReader decryptedDataTlvReader;
    decryptedDataTlvReader.Init(parsedSigma2.msgR2Decrypted.Get(), msgR2DecryptedLength);
    ParsedSigma2TBEData parsedSigma2TBEData;
    ReturnErrorOnFailure(ParseSigma2TBEData(decryptedDataTlvReader, parsedSigma2TBEData));

    // Validate responder identity located in msgR2Decrypted
    // Constructing responder identity
    P256PublicKey responderPublicKey;
    {
        NodeId responderNodeId;

        CompressedFabricId unused;
        FabricId responderFabricId;
        ReturnErrorOnFailure(SetEffectiveTime());
        ReturnErrorOnFailure(mFabricsTable->VerifyCredentials(mFabricIndex, parsedSigma2TBEData.responderNOC,
                                                              parsedSigma2TBEData.responderICAC, mValidContext, unused,
                                                              responderFabricId, responderNodeId, responderPublicKey));
        VerifyOrReturnError(fabricId == responderFabricId, CHIP_ERROR_INVALID_CASE_PARAMETER);
        // Verify that responderNodeId (from responderNOC) matches one that was included
        // in the computation of the Destination Identifier when generating Sigma1.
        VerifyOrReturnError(mPeerNodeId == responderNodeId, CHIP_ERROR_INVALID_CASE_PARAMETER);
    }

    // Construct msgR2Signed and validate the signature in msgR2Decrypted.
    size_t msgR2SignedLen = EstimateStructOverhead(parsedSigma2TBEData.responderNOC.size(),  // resonderNOC
                                                   parsedSigma2TBEData.responderICAC.size(), // responderICAC
                                                   kP256_PublicKey_Length,                   // responderEphPubKey
                                                   kP256_PublicKey_Length                    // initiatorEphPubKey
    );

    chip::Platform::ScopedMemoryBuffer<uint8_t> msgR2Signed;
    VerifyOrReturnError(msgR2Signed.Alloc(msgR2SignedLen), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan msgR2SignedSpan{ msgR2Signed.Get(), msgR2SignedLen };

    ReturnErrorOnFailure(ConstructTBSData(parsedSigma2TBEData.responderNOC, parsedSigma2TBEData.responderICAC,
                                          ByteSpan(mRemotePubKey, mRemotePubKey.Length()),
                                          ByteSpan(mEphemeralKey->Pubkey(), mEphemeralKey->Pubkey().Length()), msgR2SignedSpan));

    // Validate signature
    ReturnErrorOnFailure(responderPublicKey.ECDSA_validate_msg_signature(msgR2SignedSpan.data(), msgR2SignedSpan.size(),
                                                                         parsedSigma2TBEData.tbsData2Signature));

    ChipLogDetail(SecureChannel, "Peer " ChipLogFormatScopedNodeId " assigned session ID %d", ChipLogValueScopedNodeId(GetPeer()),
                  parsedSigma2.responderSessionId);
    SetPeerSessionId(parsedSigma2.responderSessionId);

    std::copy(parsedSigma2TBEData.resumptionId.begin(), parsedSigma2TBEData.resumptionId.end(), mNewResumptionId.begin());

    // Retrieve peer CASE Authenticated Tags (CATs) from peer's NOC.
    ReturnErrorOnFailure(ExtractCATsFromOpCert(parsedSigma2TBEData.responderNOC, mPeerCATs));

    if (parsedSigma2.responderSessionParamStructPresent)
    {
        SetRemoteSessionParameters(parsedSigma2.responderSessionParams);
        mExchangeCtxt.Value()->GetSessionHandle()->AsUnauthenticatedSession()->SetRemoteSessionParameters(
            GetRemoteSessionParameters());
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::ParseSigma2(ContiguousBufferTLVReader & tlvReader, ParsedSigma2 & outParsedSigma2)
{
    TLVType containerType = kTLVType_Structure;

    ReturnErrorOnFailure(tlvReader.Next(containerType, AnonymousTag()));
    ReturnErrorOnFailure(tlvReader.EnterContainer(containerType));

    // Retrieve Responder's Random value
    ReturnErrorOnFailure(tlvReader.Next(AsTlvContextTag(Sigma2Tags::kResponderRandom)));
    ReturnErrorOnFailure(tlvReader.GetByteView(outParsedSigma2.responderRandom));
    VerifyOrReturnError(outParsedSigma2.responderRandom.size() == kSigmaParamRandomNumberSize, CHIP_ERROR_INVALID_CASE_PARAMETER);

    // Assign Session ID
    ReturnErrorOnFailure(tlvReader.Next(AsTlvContextTag(Sigma2Tags::kResponderSessionId)));
    ReturnErrorOnFailure(tlvReader.Get(outParsedSigma2.responderSessionId));

    // Retrieve Responder's Ephemeral Pubkey
    ReturnErrorOnFailure(tlvReader.Next(AsTlvContextTag(Sigma2Tags::kResponderEphPubKey)));
    ReturnErrorOnFailure(tlvReader.GetByteView(outParsedSigma2.responderEphPubKey));
    VerifyOrReturnError(outParsedSigma2.responderEphPubKey.size() == kP256_PublicKey_Length, CHIP_ERROR_INVALID_CASE_PARAMETER);

    // Generate decrypted data
    ReturnErrorOnFailure(tlvReader.Next(AsTlvContextTag(Sigma2Tags::kEncrypted2)));

    size_t maxMsgR2SignedEncLen = EstimateStructOverhead(kMaxCHIPCertLength,                          // responderNOC
                                                         kMaxCHIPCertLength,                          // responderICAC
                                                         kMax_ECDSA_Signature_Length,                 // signature
                                                         SessionResumptionStorage::kResumptionIdSize, // resumptionID
                                                         kCaseOverheadForFutureTBEData // extra bytes for future-proofing
    );

    size_t msgR2EncryptedLenWithTag = tlvReader.GetLength();

    // Validate we did not receive a buffer larger than legal
    VerifyOrReturnError(msgR2EncryptedLenWithTag <= maxMsgR2SignedEncLen, CHIP_ERROR_INVALID_TLV_ELEMENT);
    VerifyOrReturnError(msgR2EncryptedLenWithTag > CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES, CHIP_ERROR_INVALID_TLV_ELEMENT);
    VerifyOrReturnError(outParsedSigma2.msgR2Encrypted.Alloc(msgR2EncryptedLenWithTag), CHIP_ERROR_NO_MEMORY);
    ReturnErrorOnFailure(tlvReader.GetBytes(outParsedSigma2.msgR2Encrypted.Get(), outParsedSigma2.msgR2Encrypted.AllocatedSize()));

    size_t msgR2EncryptedPayloadLen       = msgR2EncryptedLenWithTag - CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES;
    outParsedSigma2.msgR2EncryptedPayload = MutableByteSpan(outParsedSigma2.msgR2Encrypted.Get(), msgR2EncryptedPayloadLen);
    outParsedSigma2.msgR2MIC =
        ByteSpan(outParsedSigma2.msgR2Encrypted.Get() + msgR2EncryptedPayloadLen, CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES);

    // Retrieve responderSessionParams if present
    CHIP_ERROR err = tlvReader.Next();
    if (err == CHIP_NO_ERROR && tlvReader.GetTag() == AsTlvContextTag(Sigma2Tags::kResponderSessionParams))
    {
        ReturnErrorOnFailure(DecodeSessionParametersIfPresent(AsTlvContextTag(Sigma2Tags::kResponderSessionParams), tlvReader,
                                                              outParsedSigma2.responderSessionParams));
        outParsedSigma2.responderSessionParamStructPresent = true;

        err = tlvReader.Next();
    }

    // Future-proofing: CHIP_NO_ERROR will be returned by Next() if we have additional non-parsed TLV Elements, which could
    // happen in the future if additional elements are added to the specification.
    VerifyOrReturnError(err == CHIP_END_OF_TLV || err == CHIP_NO_ERROR, err);

    // Exit Container will fail (return CHIP_END_OF_TLV) if the received encoded message is not properly terminated with an
    // EndOfContainer TLV Element.
    ReturnErrorOnFailure(tlvReader.ExitContainer(containerType));

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::ParseSigma2TBEData(ContiguousBufferTLVReader & decryptedDataTlvReader,
                                           ParsedSigma2TBEData & outParsedSigma2TBE)
{
    TLVType containerType = kTLVType_Structure;

    ReturnErrorOnFailure(decryptedDataTlvReader.Next(containerType, AnonymousTag()));
    ReturnErrorOnFailure(decryptedDataTlvReader.EnterContainer(containerType));

    ReturnErrorOnFailure(decryptedDataTlvReader.Next(AsTlvContextTag(TBEDataTags::kSenderNOC)));
    ReturnErrorOnFailure(decryptedDataTlvReader.GetByteView(outParsedSigma2TBE.responderNOC));
    VerifyOrReturnError(outParsedSigma2TBE.responderNOC.size() <= kMaxCHIPCertLength, CHIP_ERROR_INVALID_CASE_PARAMETER);

    ReturnErrorOnFailure(decryptedDataTlvReader.Next());
    if (decryptedDataTlvReader.GetTag() == AsTlvContextTag(TBEDataTags::kSenderICAC))
    {
        ReturnErrorOnFailure(decryptedDataTlvReader.GetByteView(outParsedSigma2TBE.responderICAC));
        VerifyOrReturnError(outParsedSigma2TBE.responderICAC.size() <= kMaxCHIPCertLength, CHIP_ERROR_INVALID_CASE_PARAMETER);

        ReturnErrorOnFailure(decryptedDataTlvReader.Next(kTLVType_ByteString, AsTlvContextTag(TBEDataTags::kSignature)));
    }

    VerifyOrReturnError(decryptedDataTlvReader.GetTag() == AsTlvContextTag(TBEDataTags::kSignature), CHIP_ERROR_INVALID_TLV_TAG);
    // tbsData2Signature's length should equal kMax_ECDSA_Signature_Length as per the Specification
    size_t signatureLen = decryptedDataTlvReader.GetLength();
    VerifyOrReturnError(outParsedSigma2TBE.tbsData2Signature.Capacity() == signatureLen, CHIP_ERROR_INVALID_TLV_ELEMENT);
    outParsedSigma2TBE.tbsData2Signature.SetLength(signatureLen);
    ReturnErrorOnFailure(decryptedDataTlvReader.GetBytes(outParsedSigma2TBE.tbsData2Signature.Bytes(),
                                                         outParsedSigma2TBE.tbsData2Signature.Length()));

    // Retrieve session resumption ID
    ReturnErrorOnFailure(decryptedDataTlvReader.Next(AsTlvContextTag(TBEDataTags::kResumptionID)));
    ReturnErrorOnFailure(decryptedDataTlvReader.GetByteView(outParsedSigma2TBE.resumptionId));
    VerifyOrReturnError(outParsedSigma2TBE.resumptionId.size() == SessionResumptionStorage::kResumptionIdSize,
                        CHIP_ERROR_INVALID_CASE_PARAMETER);

    // Exit Container will fail (return CHIP_END_OF_TLV) if the received encoded message is not properly terminated with an
    // EndOfContainer TLV Element.
    ReturnErrorOnFailure(decryptedDataTlvReader.ExitContainer(containerType));

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::SendSigma3a()
{
    MATTER_TRACE_SCOPE("SendSigma3", "CASESession");

    ChipLogDetail(SecureChannel, "Sending Sigma3");

    auto helper = WorkHelper<SendSigma3Data>::Create(*this, &SendSigma3b, &CASESession::SendSigma3c);
    VerifyOrReturnError(helper, CHIP_ERROR_NO_MEMORY);
    {
        auto & data = helper->mData;

        VerifyOrReturnError(mFabricsTable != nullptr, CHIP_ERROR_INCORRECT_STATE);
        data.fabricIndex = mFabricIndex;
        data.fabricTable = nullptr;
        data.keystore    = nullptr;

        {
            const FabricInfo * fabricInfo = mFabricsTable->FindFabricWithIndex(mFabricIndex);
            VerifyOrReturnError(fabricInfo != nullptr, CHIP_ERROR_KEY_NOT_FOUND);
            auto * keystore = mFabricsTable->GetOperationalKeystore();
            if (!fabricInfo->HasOperationalKey() && keystore != nullptr && keystore->SupportsSignWithOpKeypairInBackground())
            {
                // NOTE: used to sign in background.
                data.keystore = keystore;
            }
            else
            {
                // NOTE: used to sign in foreground.
                data.fabricTable = mFabricsTable;
            }
        }

        VerifyOrReturnError(mEphemeralKey != nullptr, CHIP_ERROR_INTERNAL);

        VerifyOrReturnError(data.icacBuf.Alloc(kMaxCHIPCertLength), CHIP_ERROR_NO_MEMORY);
        data.icaCert = MutableByteSpan{ data.icacBuf.Get(), kMaxCHIPCertLength };

        VerifyOrReturnError(data.nocBuf.Alloc(kMaxCHIPCertLength), CHIP_ERROR_NO_MEMORY);
        data.nocCert = MutableByteSpan{ data.nocBuf.Get(), kMaxCHIPCertLength };

        ReturnErrorOnFailure(mFabricsTable->FetchICACert(mFabricIndex, data.icaCert));
        ReturnErrorOnFailure(mFabricsTable->FetchNOCCert(mFabricIndex, data.nocCert));

        // Prepare Sigma3 TBS Data Blob
        size_t msgR3SignedLen = EstimateStructOverhead(data.nocCert.size(),    // initiatorNOC
                                                       data.icaCert.size(),    // initiatorICAC
                                                       kP256_PublicKey_Length, // initiatorEphPubKey
                                                       kP256_PublicKey_Length  // responderEphPubKey
        );

        VerifyOrReturnError(data.msgR3Signed.Alloc(msgR3SignedLen), CHIP_ERROR_NO_MEMORY);
        data.msgR3SignedSpan = MutableByteSpan{ data.msgR3Signed.Get(), msgR3SignedLen };

        ReturnErrorOnFailure(ConstructTBSData(data.nocCert, data.icaCert,
                                              ByteSpan(mEphemeralKey->Pubkey(), mEphemeralKey->Pubkey().Length()),
                                              ByteSpan(mRemotePubKey, mRemotePubKey.Length()), data.msgR3SignedSpan));

        if (data.keystore != nullptr)
        {
            ReturnErrorOnFailure(helper->ScheduleWork());
            mSendSigma3Helper = helper;
            mExchangeCtxt.Value()->WillSendMessage();
            mState = State::kSendSigma3Pending;
        }
        else
        {
            ReturnErrorOnFailure(helper->DoWork());
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::SendSigma3b(SendSigma3Data & data, bool & cancel)
{
    // Generate a signature
    if (data.keystore != nullptr)
    {
        // Recommended case: delegate to operational keystore
        ReturnErrorOnFailure(data.keystore->SignWithOpKeypair(data.fabricIndex, data.msgR3SignedSpan, data.tbsData3Signature));
    }
    else
    {
        // Legacy case: delegate to fabric table fabric info
        ReturnErrorOnFailure(data.fabricTable->SignWithOpKeypair(data.fabricIndex, data.msgR3SignedSpan, data.tbsData3Signature));
    }

    // Prepare Sigma3 TBE Data Blob
    data.msg_r3_encrypted_len =
        TLV::EstimateStructOverhead(data.nocCert.size(), data.icaCert.size(), data.tbsData3Signature.Length());

    VerifyOrReturnError(data.msg_R3_Encrypted.Alloc(data.msg_r3_encrypted_len + CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES),
                        CHIP_ERROR_NO_MEMORY);

    {
        TLVWriter tlvWriter;
        TLVType outerContainerType = kTLVType_NotSpecified;

        tlvWriter.Init(data.msg_R3_Encrypted.Get(), data.msg_r3_encrypted_len);
        ReturnErrorOnFailure(tlvWriter.StartContainer(AnonymousTag(), kTLVType_Structure, outerContainerType));
        ReturnErrorOnFailure(tlvWriter.Put(AsTlvContextTag(TBEDataTags::kSenderNOC), data.nocCert));
        if (!data.icaCert.empty())
        {
            ReturnErrorOnFailure(tlvWriter.Put(AsTlvContextTag(TBEDataTags::kSenderICAC), data.icaCert));
        }

        // We are now done with ICAC and NOC certs so we can release the memory.
        {
            data.icacBuf.Free();
            data.icaCert = MutableByteSpan{};

            data.nocBuf.Free();
            data.nocCert = MutableByteSpan{};
        }

        ReturnErrorOnFailure(tlvWriter.PutBytes(AsTlvContextTag(TBEDataTags::kSignature), data.tbsData3Signature.ConstBytes(),
                                                static_cast<uint32_t>(data.tbsData3Signature.Length())));
        ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
        ReturnErrorOnFailure(tlvWriter.Finalize());
        data.msg_r3_encrypted_len = static_cast<size_t>(tlvWriter.GetLengthWritten());
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::SendSigma3c(SendSigma3Data & data, CHIP_ERROR status)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    System::PacketBufferHandle msg_R3;
    size_t data_len;

    uint8_t msg_salt[kIPKSize + kSHA256_Hash_Length];

    AutoReleaseSessionKey sr3k(*mSessionManager->GetSessionKeystore());

    VerifyOrDieWithMsg(data.keystore == nullptr || mState == State::kSendSigma3Pending, SecureChannel, "Bad internal state.");

    SuccessOrExit(err = status);

    // Generate S3K key
    {
        MutableByteSpan saltSpan(msg_salt);
        SuccessOrExit(err = ConstructSaltSigma3(ByteSpan(mIPK), saltSpan));
        SuccessOrExit(err = DeriveSigmaKey(saltSpan, ByteSpan(kKDFSR3Info), sr3k));
    }

    // Generated Encrypted data blob
    SuccessOrExit(err =
                      AES_CCM_encrypt(data.msg_R3_Encrypted.Get(), data.msg_r3_encrypted_len, nullptr, 0, sr3k.KeyHandle(),
                                      kTBEData3_Nonce, kTBEDataNonceLength, data.msg_R3_Encrypted.Get(),
                                      data.msg_R3_Encrypted.Get() + data.msg_r3_encrypted_len, CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES));

    // Generate Sigma3 Msg
    data_len = TLV::EstimateStructOverhead(CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES, data.msg_r3_encrypted_len);

    msg_R3 = System::PacketBufferHandle::New(data_len);
    VerifyOrExit(!msg_R3.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    {
        System::PacketBufferTLVWriter tlvWriter;
        TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

        tlvWriter.Init(std::move(msg_R3));
        err = tlvWriter.StartContainer(AnonymousTag(), kTLVType_Structure, outerContainerType);
        SuccessOrExit(err);
        err = tlvWriter.PutBytes(AsTlvContextTag(Sigma3Tags::kEncrypted3), data.msg_R3_Encrypted.Get(),
                                 static_cast<uint32_t>(data.msg_r3_encrypted_len + CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES));
        SuccessOrExit(err);
        err = tlvWriter.EndContainer(outerContainerType);
        SuccessOrExit(err);
        err = tlvWriter.Finalize(&msg_R3);
        SuccessOrExit(err);
    }

    err = mCommissioningHash.AddData(ByteSpan{ msg_R3->Start(), msg_R3->DataLength() });
    SuccessOrExit(err);

    // Call delegate to send the Msg3 to peer
    err = mExchangeCtxt.Value()->SendMessage(Protocols::SecureChannel::MsgType::CASE_Sigma3, std::move(msg_R3),
                                             SendFlags(SendMessageFlags::kExpectResponse));
    SuccessOrExit(err);

    ChipLogProgress(SecureChannel, "Sent Sigma3 msg");

    {
        MutableByteSpan messageDigestSpan(mMessageDigest);
        SuccessOrExit(err = mCommissioningHash.Finish(messageDigestSpan));
    }

    mState = State::kSentSigma3;

exit:
    mSendSigma3Helper.reset();

    // If data.keystore is set, processing occurred in the background, so if an error occurred,
    // need to send status report (normally occurs in SendSigma3a), and discard exchange and
    // abort pending establish (normally occurs in OnMessageReceived).
    if (data.keystore != nullptr && err != CHIP_NO_ERROR)
    {
        SendStatusReport(mExchangeCtxt, kProtocolCodeInvalidParam);
        DiscardExchange();
        AbortPendingEstablish(err);
    }

    return err;
}

CHIP_ERROR CASESession::HandleSigma3a(System::PacketBufferHandle && msg)
{
    MATTER_TRACE_SCOPE("HandleSigma3", "CASESession");
    CHIP_ERROR err = CHIP_NO_ERROR;
    ContiguousBufferTLVReader decryptedDataTlvReader;
    TLVType containerType = kTLVType_Structure;

    const uint8_t * buf = msg->Start();
    const size_t bufLen = msg->DataLength();

    AutoReleaseSessionKey sr3k(*mSessionManager->GetSessionKeystore());

    uint8_t msg_salt[kIPKSize + kSHA256_Hash_Length];

    ChipLogProgress(SecureChannel, "Received Sigma3 msg");
    MATTER_TRACE_COUNTER("Sigma3");
    MATTER_LOG_METRIC_END(kMetricDeviceCASESessionSigma2, err);

    auto helper = WorkHelper<HandleSigma3Data>::Create(*this, &HandleSigma3b, &CASESession::HandleSigma3c);
    VerifyOrExit(helper, err = CHIP_ERROR_NO_MEMORY);
    {
        auto & data = helper->mData;

        {
            VerifyOrExit(mFabricsTable != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
            const auto * fabricInfo = mFabricsTable->FindFabricWithIndex(mFabricIndex);
            VerifyOrExit(fabricInfo != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
            data.fabricId = fabricInfo->GetFabricId();
        }

        VerifyOrExit(mEphemeralKey != nullptr, err = CHIP_ERROR_INTERNAL);

        // Step 1
        // msgR3Encrypted will be allocated and initialised within ParseSigma3()
        Platform::ScopedMemoryBufferWithSize<uint8_t> msgR3Encrypted;
        // both msgR3EncryptedPayload and msgR3MIC will become backed by msgR3Encrypted in ParseSigma3()
        MutableByteSpan msgR3EncryptedPayload;
        ByteSpan msgR3MIC;
        {
            System::PacketBufferTLVReader tlvReader;
            tlvReader.Init(std::move(msg));
            SuccessOrExit(err = ParseSigma3(tlvReader, msgR3Encrypted, msgR3EncryptedPayload, msgR3MIC));

            // Generate the S3K key
            MutableByteSpan saltSpan(msg_salt);
            SuccessOrExit(err = ConstructSaltSigma3(ByteSpan(mIPK), saltSpan));
            SuccessOrExit(err = DeriveSigmaKey(saltSpan, ByteSpan(kKDFSR3Info), sr3k));

            // Add Sigma3 to the TranscriptHash which will be used to generate the Session Encryption Keys
            SuccessOrExit(err = mCommissioningHash.AddData(ByteSpan{ buf, bufLen }));
        }
        // Step 2 - Decrypt data blob
        SuccessOrExit(err = AES_CCM_decrypt(msgR3EncryptedPayload.data(), msgR3EncryptedPayload.size(), nullptr, 0, msgR3MIC.data(),
                                            msgR3MIC.size(), sr3k.KeyHandle(), kTBEData3_Nonce, kTBEDataNonceLength,
                                            msgR3EncryptedPayload.data()));

        decryptedDataTlvReader.Init(msgR3EncryptedPayload.data(), msgR3EncryptedPayload.size());
        SuccessOrExit(err = ParseSigma3TBEData(decryptedDataTlvReader, data));

        // Step 3 - Construct Sigma3 TBS Data
        size_t msgR3SignedLen = TLV::EstimateStructOverhead(data.initiatorNOC.size(),  // initiatorNOC
                                                            data.initiatorICAC.size(), // initiatorICAC
                                                            kP256_PublicKey_Length,    // initiatorEphPubKey
                                                            kP256_PublicKey_Length     // responderEphPubKey
        );

        VerifyOrExit(data.msgR3Signed.Alloc(msgR3SignedLen), err = CHIP_ERROR_NO_MEMORY);
        data.msgR3SignedSpan = MutableByteSpan{ data.msgR3Signed.Get(), msgR3SignedLen };

        SuccessOrExit(err = ConstructTBSData(data.initiatorNOC, data.initiatorICAC, ByteSpan(mRemotePubKey, mRemotePubKey.Length()),
                                             ByteSpan(mEphemeralKey->Pubkey(), mEphemeralKey->Pubkey().Length()),
                                             data.msgR3SignedSpan));

        // Prepare for Step 4/5
        {
            MutableByteSpan fabricRCAC{ data.rootCertBuf };
            SuccessOrExit(err = mFabricsTable->FetchRootCert(mFabricIndex, fabricRCAC));
            data.fabricRCAC = fabricRCAC;
            // TODO probably should make SetEffectiveTime static and call closer to VerifyCredentials
            SuccessOrExit(err = SetEffectiveTime());
        }

        // Copy remaining needed data into work structure
        {
            data.validContext = mValidContext;

            // initiatorNOC and initiatorICAC are spans into msgR3Encrypted
            // which is going away, so to save memory, redirect them to their
            // copies in msgR3Signed, which is staying around
            TLV::ContiguousBufferTLVReader signedDataTlvReader;
            signedDataTlvReader.Init(data.msgR3SignedSpan);
            SuccessOrExit(err = signedDataTlvReader.Next(containerType, AnonymousTag()));
            SuccessOrExit(err = signedDataTlvReader.EnterContainer(containerType));

            SuccessOrExit(err = signedDataTlvReader.Next(AsTlvContextTag(TBSDataTags::kSenderNOC)));
            SuccessOrExit(err = signedDataTlvReader.GetByteView(data.initiatorNOC));

            if (!data.initiatorICAC.empty())
            {
                SuccessOrExit(err = signedDataTlvReader.Next(AsTlvContextTag(TBSDataTags::kSenderICAC)));
                SuccessOrExit(err = signedDataTlvReader.GetByteView(data.initiatorICAC));
            }

            SuccessOrExit(err = signedDataTlvReader.ExitContainer(containerType));
        }

        SuccessOrExit(err = helper->ScheduleWork());
        mHandleSigma3Helper = helper;
        mExchangeCtxt.Value()->WillSendMessage();
        mState = State::kHandleSigma3Pending;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        SendStatusReport(mExchangeCtxt, kProtocolCodeInvalidParam);
    }

    return err;
}

CHIP_ERROR CASESession::ParseSigma3(ContiguousBufferTLVReader & tlvReader,
                                    Platform::ScopedMemoryBufferWithSize<uint8_t> & outMsgR3Encrypted,
                                    MutableByteSpan & outMsgR3EncryptedPayload, ByteSpan & outMsgR3MIC)
{
    TLVType containerType = kTLVType_Structure;

    ReturnErrorOnFailure(tlvReader.Next(containerType, AnonymousTag()));
    ReturnErrorOnFailure(tlvReader.EnterContainer(containerType));

    // Fetch encrypted data
    ReturnErrorOnFailure(tlvReader.Next(AsTlvContextTag(Sigma3Tags::kEncrypted3)));

    size_t maxMsgR3SignedEncLen = EstimateStructOverhead(kMaxCHIPCertLength,           // initiatorNOC
                                                         kMaxCHIPCertLength,           // initiatorICAC
                                                         kMax_ECDSA_Signature_Length,  // signature
                                                         kCaseOverheadForFutureTBEData // extra bytes for future-proofing
    );

    size_t msgR3EncryptedLenWithTag = tlvReader.GetLength();

    // Validate we did not receive a buffer larger than legal
    VerifyOrReturnError(msgR3EncryptedLenWithTag <= maxMsgR3SignedEncLen, CHIP_ERROR_INVALID_TLV_ELEMENT);
    VerifyOrReturnError(msgR3EncryptedLenWithTag > CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES, CHIP_ERROR_INVALID_TLV_ELEMENT);
    VerifyOrReturnError(outMsgR3Encrypted.Alloc(msgR3EncryptedLenWithTag), CHIP_ERROR_NO_MEMORY);
    ReturnErrorOnFailure(tlvReader.GetBytes(outMsgR3Encrypted.Get(), outMsgR3Encrypted.AllocatedSize()));

    size_t msgR3EncryptedPayloadLen = msgR3EncryptedLenWithTag - CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES;
    outMsgR3EncryptedPayload        = MutableByteSpan(outMsgR3Encrypted.Get(), msgR3EncryptedPayloadLen);
    outMsgR3MIC = ByteSpan(outMsgR3Encrypted.Get() + msgR3EncryptedPayloadLen, CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES);

    ReturnErrorOnFailure(tlvReader.ExitContainer(containerType));

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::ParseSigma3TBEData(ContiguousBufferTLVReader & decryptedDataTlvReader,
                                           HandleSigma3Data & outHandleSigma3TBEData)
{

    TLVType containerType = kTLVType_Structure;
    ReturnErrorOnFailure(decryptedDataTlvReader.Next(containerType, TLV::AnonymousTag()));
    ReturnErrorOnFailure(decryptedDataTlvReader.EnterContainer(containerType));

    ReturnErrorOnFailure(decryptedDataTlvReader.Next(AsTlvContextTag(TBEDataTags::kSenderNOC)));
    ReturnErrorOnFailure(decryptedDataTlvReader.GetByteView(outHandleSigma3TBEData.initiatorNOC));
    VerifyOrReturnError(outHandleSigma3TBEData.initiatorNOC.size() <= kMaxCHIPCertLength, CHIP_ERROR_INVALID_CASE_PARAMETER);

    ReturnErrorOnFailure(decryptedDataTlvReader.Next());
    if (decryptedDataTlvReader.GetTag() == AsTlvContextTag(TBEDataTags::kSenderICAC))
    {
        ReturnErrorOnFailure(decryptedDataTlvReader.GetByteView(outHandleSigma3TBEData.initiatorICAC));
        VerifyOrReturnError(outHandleSigma3TBEData.initiatorICAC.size() <= kMaxCHIPCertLength, CHIP_ERROR_INVALID_CASE_PARAMETER);
        ReturnErrorOnFailure(decryptedDataTlvReader.Next(TLV::kTLVType_ByteString, AsTlvContextTag(TBEDataTags::kSignature)));
    }

    VerifyOrReturnError(decryptedDataTlvReader.GetTag() == AsTlvContextTag(TBEDataTags::kSignature), CHIP_ERROR_INVALID_TLV_TAG);
    size_t signatureLen = decryptedDataTlvReader.GetLength();
    VerifyOrReturnError(outHandleSigma3TBEData.tbsData3Signature.Capacity() == signatureLen, CHIP_ERROR_INVALID_TLV_ELEMENT);
    outHandleSigma3TBEData.tbsData3Signature.SetLength(signatureLen);
    ReturnErrorOnFailure(decryptedDataTlvReader.GetBytes(outHandleSigma3TBEData.tbsData3Signature.Bytes(),
                                                         outHandleSigma3TBEData.tbsData3Signature.Length()));

    ReturnErrorOnFailure(decryptedDataTlvReader.ExitContainer(containerType));

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::HandleSigma3b(HandleSigma3Data & data, bool & cancel)
{
    // Step 5/6
    // Validate initiator identity located in msg->Start()
    // Constructing responder identity
    CompressedFabricId unused;
    FabricId initiatorFabricId;
    P256PublicKey initiatorPublicKey;
    ReturnErrorOnFailure(FabricTable::VerifyCredentials(data.initiatorNOC, data.initiatorICAC, data.fabricRCAC, data.validContext,
                                                        unused, initiatorFabricId, data.initiatorNodeId, initiatorPublicKey));
    VerifyOrReturnError(data.fabricId == initiatorFabricId, CHIP_ERROR_INVALID_CASE_PARAMETER);

    // Step 7 - Validate Signature
    ReturnErrorOnFailure(initiatorPublicKey.ECDSA_validate_msg_signature(data.msgR3SignedSpan.data(), data.msgR3SignedSpan.size(),
                                                                         data.tbsData3Signature));

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::HandleSigma3c(HandleSigma3Data & data, CHIP_ERROR status)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mState == State::kHandleSigma3Pending, err = CHIP_ERROR_INCORRECT_STATE);

    SuccessOrExit(err = status);

    mPeerNodeId = data.initiatorNodeId;

    {
        MutableByteSpan messageDigestSpan(mMessageDigest);
        SuccessOrExit(err = mCommissioningHash.Finish(messageDigestSpan));
    }

    // Retrieve peer CASE Authenticated Tags (CATs) from peer's NOC.
    {
        SuccessOrExit(err = ExtractCATsFromOpCert(data.initiatorNOC, mPeerCATs));
    }

    if (mSessionResumptionStorage != nullptr)
    {
        CHIP_ERROR err2 = mSessionResumptionStorage->Save(GetPeer(), mNewResumptionId, mSharedSecret, mPeerCATs);
        if (err2 != CHIP_NO_ERROR)
        {
            ChipLogError(SecureChannel, "Unable to save session resumption state: %" CHIP_ERROR_FORMAT, err2.Format());
        }
    }

    MATTER_LOG_METRIC(kMetricDeviceCASESessionSigmaFinished);
    SendStatusReport(mExchangeCtxt, kProtocolCodeSuccess);

    mState = State::kFinished;
    Finish();

exit:
    mHandleSigma3Helper.reset();

    if (err != CHIP_NO_ERROR)
    {
        SendStatusReport(mExchangeCtxt, kProtocolCodeInvalidParam);
        // Abort the pending establish, which is normally done by CASESession::OnMessageReceived,
        // but in the background processing case must be done here.
        DiscardExchange();
        AbortPendingEstablish(err);
    }

    return err;
}

CHIP_ERROR CASESession::DeriveSigmaKey(const ByteSpan & salt, const ByteSpan & info, AutoReleaseSessionKey & key) const
{
    return mSessionManager->GetSessionKeystore()->DeriveKey(mSharedSecret, salt, info, key.KeyHandle());
}

CHIP_ERROR CASESession::ConstructSaltSigma2(const ByteSpan & rand, const Crypto::P256PublicKey & pubkey, const ByteSpan & ipk,
                                            MutableByteSpan & salt)
{
    uint8_t md[kSHA256_Hash_Length];
    memset(salt.data(), 0, salt.size());
    Encoding::LittleEndian::BufferWriter bbuf(salt.data(), salt.size());

    bbuf.Put(ipk.data(), ipk.size());
    bbuf.Put(rand.data(), kSigmaParamRandomNumberSize);
    bbuf.Put(pubkey, pubkey.Length());
    MutableByteSpan messageDigestSpan(md);
    ReturnErrorOnFailure(mCommissioningHash.GetDigest(messageDigestSpan));
    bbuf.Put(messageDigestSpan.data(), messageDigestSpan.size());

    size_t saltWritten = 0;
    VerifyOrReturnError(bbuf.Fit(saltWritten), CHIP_ERROR_BUFFER_TOO_SMALL);
    salt = salt.SubSpan(0, saltWritten);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::ConstructSaltSigma3(const ByteSpan & ipk, MutableByteSpan & salt)
{
    uint8_t md[kSHA256_Hash_Length];
    memset(salt.data(), 0, salt.size());
    Encoding::LittleEndian::BufferWriter bbuf(salt.data(), salt.size());

    bbuf.Put(ipk.data(), ipk.size());
    MutableByteSpan messageDigestSpan(md);
    ReturnErrorOnFailure(mCommissioningHash.GetDigest(messageDigestSpan));
    bbuf.Put(messageDigestSpan.data(), messageDigestSpan.size());

    size_t saltWritten = 0;
    VerifyOrReturnError(bbuf.Fit(saltWritten), CHIP_ERROR_BUFFER_TOO_SMALL);
    salt = salt.SubSpan(0, saltWritten);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::ConstructSigmaResumeKey(const ByteSpan & initiatorRandom, const ByteSpan & resumptionID,
                                                const ByteSpan & skInfo, const ByteSpan & nonce, AutoReleaseSessionKey & resumeKey)
{
    constexpr size_t saltSize = kSigmaParamRandomNumberSize + SessionResumptionStorage::kResumptionIdSize;
    uint8_t salt[saltSize];

    memset(salt, 0, saltSize);
    Encoding::LittleEndian::BufferWriter bbuf(salt, saltSize);

    bbuf.Put(initiatorRandom.data(), initiatorRandom.size());
    bbuf.Put(resumptionID.data(), resumptionID.size());

    size_t saltWritten = 0;
    VerifyOrReturnError(bbuf.Fit(saltWritten), CHIP_ERROR_BUFFER_TOO_SMALL);

    return DeriveSigmaKey(ByteSpan(salt, saltWritten), skInfo, resumeKey);
}

CHIP_ERROR CASESession::GenerateSigmaResumeMIC(const ByteSpan & initiatorRandom, const ByteSpan & resumptionID,
                                               const ByteSpan & skInfo, const ByteSpan & nonce, MutableByteSpan & resumeMIC)
{
    VerifyOrReturnError(resumeMIC.size() >= CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES, CHIP_ERROR_BUFFER_TOO_SMALL);

    AutoReleaseSessionKey srk(*mSessionManager->GetSessionKeystore());
    ReturnErrorOnFailure(ConstructSigmaResumeKey(initiatorRandom, resumptionID, skInfo, nonce, srk));
    ReturnErrorOnFailure(AES_CCM_encrypt(nullptr, 0, nullptr, 0, srk.KeyHandle(), nonce.data(), nonce.size(), nullptr,
                                         resumeMIC.data(), CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES));
    resumeMIC.reduce_size(CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::ValidateSigmaResumeMIC(const ByteSpan & resumeMIC, const ByteSpan & initiatorRandom,
                                               const ByteSpan & resumptionID, const ByteSpan & skInfo, const ByteSpan & nonce)
{
    VerifyOrReturnError(resumeMIC.size() == CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES, CHIP_ERROR_BUFFER_TOO_SMALL);

    AutoReleaseSessionKey srk(*mSessionManager->GetSessionKeystore());
    ReturnErrorOnFailure(ConstructSigmaResumeKey(initiatorRandom, resumptionID, skInfo, nonce, srk));
    ReturnErrorOnFailure(AES_CCM_decrypt(nullptr, 0, nullptr, 0, resumeMIC.data(), resumeMIC.size(), srk.KeyHandle(), nonce.data(),
                                         nonce.size(), nullptr));

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::ConstructTBSData(const ByteSpan & senderNOC, const ByteSpan & senderICAC, const ByteSpan & senderPubKey,
                                         const ByteSpan & receiverPubKey, MutableByteSpan & outTbsData)
{
    TLVWriter tlvWriter;
    TLVType outerContainerType = kTLVType_NotSpecified;

    tlvWriter.Init(outTbsData);
    ReturnErrorOnFailure(tlvWriter.StartContainer(AnonymousTag(), kTLVType_Structure, outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Put(AsTlvContextTag(TBSDataTags::kSenderNOC), senderNOC));
    if (!senderICAC.empty())
    {
        ReturnErrorOnFailure(tlvWriter.Put(AsTlvContextTag(TBSDataTags::kSenderICAC), senderICAC));
    }
    ReturnErrorOnFailure(tlvWriter.Put(AsTlvContextTag(TBSDataTags::kSenderPubKey), senderPubKey));
    ReturnErrorOnFailure(tlvWriter.Put(AsTlvContextTag(TBSDataTags::kReceiverPubKey), receiverPubKey));
    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Finalize());
    outTbsData.reduce_size(static_cast<size_t>(tlvWriter.GetLengthWritten()));

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::SetEffectiveTime()
{
    System::Clock::Milliseconds64 currentUnixTimeMS;
    CHIP_ERROR err = System::SystemClock().GetClock_RealTimeMS(currentUnixTimeMS);

    if (err == CHIP_NO_ERROR)
    {
        // If the system has given us a wall clock time, we must use it or
        // fail.  Conversion failures here are therefore always an error.
        System::Clock::Seconds32 currentUnixTime = std::chrono::duration_cast<System::Clock::Seconds32>(currentUnixTimeMS);
        ReturnErrorOnFailure(mValidContext.SetEffectiveTimeFromUnixTime<CurrentChipEpochTime>(currentUnixTime));
    }
    else
    {
        // If we don't have wall clock time, the spec dictates that we should
        // fall back to Last Known Good Time.  Ultimately, the calling application's
        // validity policy will determine whether this is permissible.
        System::Clock::Seconds32 lastKnownGoodChipEpochTime;
        ChipLogError(SecureChannel,
                     "The device does not support GetClock_RealTimeMS() API: %" CHIP_ERROR_FORMAT
                     ".  Falling back to Last Known Good UTC Time",
                     err.Format());
        VerifyOrReturnError(mFabricsTable != nullptr, CHIP_ERROR_INCORRECT_STATE);
        err = mFabricsTable->GetLastKnownGoodChipEpochTime(lastKnownGoodChipEpochTime);
        if (err != CHIP_NO_ERROR)
        {
            // If we have no time available, the Validity Policy will
            // determine what to do.
            ChipLogError(SecureChannel, "Failed to retrieve Last Known Good UTC Time");
        }
        else
        {
            mValidContext.SetEffectiveTime<LastKnownGoodChipEpochTime>(lastKnownGoodChipEpochTime);
        }
    }
    return CHIP_NO_ERROR;
}

void CASESession::OnSuccessStatusReport()
{
    ChipLogProgress(SecureChannel, "Success status report received. Session was established");

    if (mSessionResumptionStorage != nullptr)
    {
        CHIP_ERROR err2 = mSessionResumptionStorage->Save(GetPeer(), mNewResumptionId, mSharedSecret, mPeerCATs);
        if (err2 != CHIP_NO_ERROR)
            ChipLogError(SecureChannel, "Unable to save session resumption state: %" CHIP_ERROR_FORMAT, err2.Format());
    }

    switch (mState)
    {
    case State::kSentSigma3:
        mState = State::kFinished;
        break;
    case State::kSentSigma2Resume:
        mState = State::kFinishedViaResume;
        break;
    default:
        VerifyOrDie(false && "Reached invalid internal state keeping in CASE session");
        break;
    }

    Finish();
}

CHIP_ERROR CASESession::OnFailureStatusReport(Protocols::SecureChannel::GeneralStatusCode generalCode, uint16_t protocolCode,
                                              Optional<uintptr_t> protocolData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    switch (protocolCode)
    {
    case kProtocolCodeInvalidParam:
        err = CHIP_ERROR_INVALID_CASE_PARAMETER;
        break;

    case kProtocolCodeNoSharedRoot:
        err = CHIP_ERROR_NO_SHARED_TRUSTED_ROOT;
        break;

    case kProtocolCodeBusy:
        err = CHIP_ERROR_BUSY;
        if (protocolData.HasValue())
        {
            mDelegate->OnResponderBusy(System::Clock::Milliseconds16(static_cast<uint16_t>(protocolData.Value())));
        }
        break;

    default:
        err = CHIP_ERROR_INTERNAL;
        break;
    };
    mState = State::kInitialized;
    ChipLogError(SecureChannel, "Received error (protocol code %d) during pairing process: %" CHIP_ERROR_FORMAT, protocolCode,
                 err.Format());
    return err;
}

CHIP_ERROR CASESession::ParseSigma1(TLV::ContiguousBufferTLVReader & tlvReader, ParsedSigma1 & outParsedSigma1)
{

    TLVType containerType = kTLVType_Structure;
    ReturnErrorOnFailure(tlvReader.Next(containerType, AnonymousTag()));
    ReturnErrorOnFailure(tlvReader.EnterContainer(containerType));

    ReturnErrorOnFailure(tlvReader.Next(AsTlvContextTag(Sigma1Tags::kInitiatorRandom)));
    ReturnErrorOnFailure(tlvReader.GetByteView(outParsedSigma1.initiatorRandom));
    VerifyOrReturnError(outParsedSigma1.initiatorRandom.size() == kSigmaParamRandomNumberSize, CHIP_ERROR_INVALID_CASE_PARAMETER);

    ReturnErrorOnFailure(tlvReader.Next(AsTlvContextTag(Sigma1Tags::kInitiatorSessionId)));
    ReturnErrorOnFailure(tlvReader.Get(outParsedSigma1.initiatorSessionId));

    ReturnErrorOnFailure(tlvReader.Next(AsTlvContextTag(Sigma1Tags::kDestinationId)));
    ReturnErrorOnFailure(tlvReader.GetByteView(outParsedSigma1.destinationId));
    VerifyOrReturnError(outParsedSigma1.destinationId.size() == kSHA256_Hash_Length, CHIP_ERROR_INVALID_CASE_PARAMETER);

    ReturnErrorOnFailure(tlvReader.Next(AsTlvContextTag(Sigma1Tags::kInitiatorEphPubKey)));
    ReturnErrorOnFailure(tlvReader.GetByteView(outParsedSigma1.initiatorEphPubKey));
    VerifyOrReturnError(outParsedSigma1.initiatorEphPubKey.size() == kP256_PublicKey_Length, CHIP_ERROR_INVALID_CASE_PARAMETER);

    // Optional members start here.
    CHIP_ERROR err = tlvReader.Next();
    if (err == CHIP_NO_ERROR && tlvReader.GetTag() == AsTlvContextTag(Sigma1Tags::kInitiatorSessionParams))
    {
        ReturnErrorOnFailure(DecodeSessionParametersIfPresent(AsTlvContextTag(Sigma1Tags::kInitiatorSessionParams), tlvReader,
                                                              outParsedSigma1.initiatorSessionParams));
        outParsedSigma1.initiatorSessionParamStructPresent = true;

        err = tlvReader.Next();
    }

    bool resumptionIDTagFound = false;
    bool resume1MICTagFound   = false;

    if (err == CHIP_NO_ERROR && tlvReader.GetTag() == AsTlvContextTag(Sigma1Tags::kResumptionID))
    {
        resumptionIDTagFound = true;
        ReturnErrorOnFailure(tlvReader.GetByteView(outParsedSigma1.resumptionId));
        VerifyOrReturnError(outParsedSigma1.resumptionId.size() == SessionResumptionStorage::kResumptionIdSize,
                            CHIP_ERROR_INVALID_CASE_PARAMETER);
        err = tlvReader.Next();
    }

    if (err == CHIP_NO_ERROR && tlvReader.GetTag() == AsTlvContextTag(Sigma1Tags::kResume1MIC))
    {
        resume1MICTagFound = true;
        ReturnErrorOnFailure(tlvReader.GetByteView(outParsedSigma1.initiatorResumeMIC));
        VerifyOrReturnError(outParsedSigma1.initiatorResumeMIC.size() == CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES,
                            CHIP_ERROR_INVALID_CASE_PARAMETER);
        err = tlvReader.Next();
    }

    if (err == CHIP_END_OF_TLV)
    {
        // We ran out of struct members, but that's OK, because they were optional.
        err = CHIP_NO_ERROR;
    }

    ReturnErrorOnFailure(err);
    ReturnErrorOnFailure(tlvReader.ExitContainer(containerType));

    if (resumptionIDTagFound && resume1MICTagFound)
    {
        outParsedSigma1.sessionResumptionRequested = true;
    }
    else if (!resumptionIDTagFound && !resume1MICTagFound)
    {
        outParsedSigma1.sessionResumptionRequested = false;
    }
    else
    {
        return CHIP_ERROR_UNEXPECTED_TLV_ELEMENT;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::ValidateReceivedMessage(ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                                const System::PacketBufferHandle & msg)
{
    VerifyOrReturnError(ec != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // mExchangeCtxt can be nullptr if this is the first message (CASE_Sigma1) received by CASESession
    // via UnsolicitedMessageHandler. The exchange context is allocated by exchange manager and provided
    // to the handler (CASESession object).
    if (mExchangeCtxt.HasValue())
    {
        if (&mExchangeCtxt.Value().Get() != ec)
        {
            ReturnErrorOnFailure(CHIP_ERROR_INVALID_ARGUMENT);
        }
    }
    else
    {
        mExchangeCtxt.Emplace(*ec);
    }
    mExchangeCtxt.Value()->UseSuggestedResponseTimeout(kExpectedHighProcessingTime);

    VerifyOrReturnError(!msg.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESession::OnMessageReceived(ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                          System::PacketBufferHandle && msg)
{
    MATTER_TRACE_SCOPE("OnMessageReceived", "CASESession");
    CHIP_ERROR err                            = ValidateReceivedMessage(ec, payloadHeader, msg);
    Protocols::SecureChannel::MsgType msgType = static_cast<Protocols::SecureChannel::MsgType>(payloadHeader.GetMessageType());
    SuccessOrExit(err);

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    if (mStopHandshakeAtState.HasValue() && mState == mStopHandshakeAtState.Value())
    {
        mStopHandshakeAtState = Optional<State>::Missing();
        // For testing purposes we are trying to stop a successful CASESession from happening by dropping part of the
        // handshake in the middle. We are trying to keep both sides of the CASESession establishment in an active
        // pending state. In order to keep this side open we have to tell the exchange context that we will send an
        // async message.
        //
        // Should you need to resume the CASESession, you could theoretically pass along the msg to a callback that gets
        // registered when setting mStopHandshakeAtState.
        mExchangeCtxt.Value()->WillSendMessage();
        return CHIP_NO_ERROR;
    }
#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST

#if CHIP_CONFIG_SLOW_CRYPTO
    if ((msgType == Protocols::SecureChannel::MsgType::CASE_Sigma1 || msgType == Protocols::SecureChannel::MsgType::CASE_Sigma2 ||
         msgType == Protocols::SecureChannel::MsgType::CASE_Sigma2Resume ||
         msgType == Protocols::SecureChannel::MsgType::CASE_Sigma3) &&
        mExchangeCtxt.Value()->GetSessionHandle()->AsUnauthenticatedSession()->GetPeerAddress().GetTransportType() !=
            Transport::Type::kTcp)
    {
        // TODO: Rename FlushAcks() to something more semantically correct and
        // call unconditionally for TCP or MRP from here. Inside, the
        // PeerAddress type could be consulted to selectively flush MRP Acks
        // when transport is not TCP. Issue #33183
        SuccessOrExit(err = mExchangeCtxt.Value()->FlushAcks());
    }
#endif // CHIP_CONFIG_SLOW_CRYPTO

    // By default, CHIP_ERROR_INVALID_MESSAGE_TYPE is returned if in the current state
    // a message handler is not defined for the received message type.
    err = CHIP_ERROR_INVALID_MESSAGE_TYPE;

    switch (mState)
    {
    case State::kInitialized:
        if (msgType == Protocols::SecureChannel::MsgType::CASE_Sigma1)
        {
            err = HandleSigma1_and_SendSigma2(std::move(msg));
        }
        break;
    case State::kSentSigma1:
        switch (static_cast<Protocols::SecureChannel::MsgType>(payloadHeader.GetMessageType()))
        {
        case Protocols::SecureChannel::MsgType::CASE_Sigma2:
            err = HandleSigma2_and_SendSigma3(std::move(msg));
            break;

        case MsgType::StatusReport:
            err = HandleStatusReport(std::move(msg), /* successExpected*/ false);
            MATTER_LOG_METRIC_END(kMetricDeviceCASESessionSigma1, err);
            break;

        default:
            // Return the default error that was set above
            break;
        };
        break;
    case State::kSentSigma1Resume:
        switch (static_cast<Protocols::SecureChannel::MsgType>(payloadHeader.GetMessageType()))
        {
        case Protocols::SecureChannel::MsgType::CASE_Sigma2:
            err = HandleSigma2_and_SendSigma3(std::move(msg));
            break;

        case Protocols::SecureChannel::MsgType::CASE_Sigma2Resume:
            err = HandleSigma2Resume(std::move(msg));
            break;

        case MsgType::StatusReport:
            err = HandleStatusReport(std::move(msg), /* successExpected*/ false);
            MATTER_LOG_METRIC_END(kMetricDeviceCASESessionSigma1, err);
            break;

        default:
            // Return the default error that was set above
            break;
        };
        break;
    case State::kSentSigma2:
        switch (static_cast<Protocols::SecureChannel::MsgType>(payloadHeader.GetMessageType()))
        {
        case Protocols::SecureChannel::MsgType::CASE_Sigma3:
            err = HandleSigma3a(std::move(msg));
            break;

        case MsgType::StatusReport:
            err = HandleStatusReport(std::move(msg), /* successExpected*/ false);
            MATTER_LOG_METRIC_END(kMetricDeviceCASESessionSigma2, err);
            break;

        default:
            // Return the default error that was set above
            break;
        };
        break;
    case State::kSentSigma3:
    case State::kSentSigma2Resume:
        if (msgType == Protocols::SecureChannel::MsgType::StatusReport)
        {
            // Need to capture before invoking status report since 'this' might be deallocated on successful completion of
            // sigma3
            MetricKey key = (mState == State::kSentSigma3) ? kMetricDeviceCASESessionSigma3 : kMetricDeviceCASESessionSigma2Resume;
            err           = HandleStatusReport(std::move(msg), /* successExpected*/ true);
            MATTER_LOG_METRIC_END(key, err);
            IgnoreUnusedVariable(key);
        }
        break;
    default:
        // Return the default error that was set above
        break;
    };

exit:

    if (err == CHIP_ERROR_INVALID_MESSAGE_TYPE)
    {
        ChipLogError(SecureChannel, "Received message (type %d) cannot be handled in %d state.", to_underlying(msgType),
                     to_underlying(mState));
    }

    // Call delegate to indicate session establishment failure.
    if (err != CHIP_NO_ERROR)
    {
        // Discard the exchange so that Clear() doesn't try aborting it.  The
        // exchange will handle that.
        DiscardExchange();
        AbortPendingEstablish(err);
    }
    return err;
}

namespace {
System::Clock::Timeout ComputeRoundTripTimeout(ExchangeContext::Timeout serverProcessingTime,
                                               const ReliableMessageProtocolConfig & remoteMrpConfig)
{
    // TODO: This is duplicating logic from Session::ComputeRoundTripTimeout.  Unfortunately, it's called by
    // consumers who do not have a session.
    const auto & maybeLocalMRPConfig = GetLocalMRPConfig();
    const auto & defaultMRRPConfig   = GetDefaultMRPConfig();
    const auto & localMRPConfig      = maybeLocalMRPConfig.ValueOr(defaultMRRPConfig);
    return GetRetransmissionTimeout(remoteMrpConfig.mActiveRetransTimeout, remoteMrpConfig.mIdleRetransTimeout,
                                    // Assume peer is idle, as a worst-case assumption (probably true for
                                    // Sigma1, since that will be our initial message on the session, but less
                                    // so for Sigma2).
                                    System::Clock::kZero, remoteMrpConfig.mActiveThresholdTime) +
        serverProcessingTime +
        GetRetransmissionTimeout(localMRPConfig.mActiveRetransTimeout, localMRPConfig.mIdleRetransTimeout,
                                 // Peer will assume we are active, since it's
                                 // responding to our message.
                                 System::SystemClock().GetMonotonicTimestamp(), localMRPConfig.mActiveThresholdTime);
}
} // anonymous namespace

System::Clock::Timeout CASESession::ComputeSigma1ResponseTimeout(const ReliableMessageProtocolConfig & remoteMrpConfig)
{
    return ComputeRoundTripTimeout(kExpectedSigma1ProcessingTime, remoteMrpConfig);
}

System::Clock::Timeout CASESession::ComputeSigma2ResponseTimeout(const ReliableMessageProtocolConfig & remoteMrpConfig)
{
    return ComputeRoundTripTimeout(kExpectedHighProcessingTime, remoteMrpConfig);
}

bool CASESession::InvokeBackgroundWorkWatchdog()
{
    bool watchdogFired = false;

    if (mSendSigma3Helper && mSendSigma3Helper->UnableToScheduleAfterWorkCallback())
    {
        ChipLogError(SecureChannel, "SendSigma3Helper was unable to schedule the AfterWorkCallback");
        mSendSigma3Helper->DoAfterWork();
        watchdogFired = true;
    }

    if (mHandleSigma3Helper && mHandleSigma3Helper->UnableToScheduleAfterWorkCallback())
    {
        ChipLogError(SecureChannel, "HandleSigma3Helper was unable to schedule the AfterWorkCallback");
        mHandleSigma3Helper->DoAfterWork();
        watchdogFired = true;
    }

    return watchdogFired;
}

// Helper function to map CASESession::State to SessionEstablishmentStage
SessionEstablishmentStage CASESession::MapCASEStateToSessionEstablishmentStage(State caseState)
{
    switch (caseState)
    {
    case State::kInitialized:
        return SessionEstablishmentStage::kNotInKeyExchange;
    case State::kSentSigma1:
    case State::kSentSigma1Resume:
        return SessionEstablishmentStage::kSentSigma1;
    case State::kSentSigma2:
    case State::kSentSigma2Resume:
        return SessionEstablishmentStage::kSentSigma2;
    case State::kSendSigma3Pending:
        return SessionEstablishmentStage::kReceivedSigma2;
    case State::kSentSigma3:
        return SessionEstablishmentStage::kSentSigma3;
    case State::kHandleSigma3Pending:
        return SessionEstablishmentStage::kReceivedSigma3;
    // Add more mappings here for other states
    default:
        return SessionEstablishmentStage::kUnknown; // Default mapping
    }
}

} // namespace chip
