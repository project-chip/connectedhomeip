/*
 *   Copyright (c) 2023 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "BdxOTASender.h"

#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>
#include <platform/LockTracker.h>
#include <protocols/interaction_model/Constants.h>

using namespace chip;
using namespace chip::app;
using namespace chip::bdx;
using Protocols::InteractionModel::Status;

constexpr uint32_t kMaxBdxBlockSize = 1024;

// Allow enough time after QueryImageResponse for the requestor to open a BDX session and send ReceiveInit
// without letting an abandoned reservation hold a slot for the full 5-minute BDX transfer timeout.
constexpr System::Clock::Timeout kBdxInitReceivedTimeout = System::Clock::Seconds16(2 * 60);

constexpr System::Clock::Timeout kBdxTimeout        = System::Clock::Seconds16(5 * 60); // OTA Spec mandates >= 5 minutes
constexpr System::Clock::Timeout kBdxPollIntervalMs = System::Clock::Milliseconds32(50);
constexpr bdx::TransferRole kBdxRole                = bdx::TransferRole::kSender;

// Each session holds one exchange and one poll timer. Requestors beyond this limit get a Busy QueryImageResponse with a
// delayedActionTime.
constexpr size_t kMaxConcurrentBdxSessions = 8;

// ================= BdxOTASession Implementation =================

BdxOTASession::BdxOTASession(BdxOTASender * owner, jobject otaDelegate, System::Layer * systemLayer, FabricIndex fabricIndex,
                             NodeId nodeId, uint64_t creationOrder) :
    mOwner(owner),
    mOtaDelegate(otaDelegate), mTimerLayer(systemLayer), mFabricIndex(fabricIndex), mNodeId(nodeId), mCreationOrder(creationOrder)
{}

BdxOTASession::~BdxOTASession()
{
    ResetState();
}

void BdxOTASession::HandleBdxInitReceivedTimeoutExpired(System::Layer * systemLayer, void * state)
{
    VerifyOrReturn(state != nullptr);
    BdxOTASession * session = static_cast<BdxOTASession *>(state);
    ChipLogError(BDX, "Timed out waiting for BDX Init from node 0x" ChipLogFormatX64, ChipLogValueX64(session->mNodeId));
    session->EndSession(CHIP_ERROR_TIMEOUT);
}

CHIP_ERROR BdxOTASession::PrepareForTransfer()
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturnError(mTimerLayer != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(!mInitialized, CHIP_ERROR_INCORRECT_STATE);

    CHIP_ERROR err = mTimerLayer->StartTimer(kBdxInitReceivedTimeout, HandleBdxInitReceivedTimeoutExpired, this);
    LogErrorOnFailure(err);

    BitFlags<bdx::TransferControlFlags> flags(bdx::TransferControlFlags::kReceiverDrive);
    err = Responder::PrepareForTransfer(mTimerLayer, kBdxRole, flags, kMaxBdxBlockSize, kBdxTimeout, kBdxPollIntervalMs);
    if (err == CHIP_NO_ERROR)
    {
        mInitialized = true;
    }
    return err;
}

void BdxOTASession::ResetState()
{
    assertChipStackLockedByCurrentThread();
    if (mTimerLayer)
    {
        mTimerLayer->CancelTimer(HandleBdxInitReceivedTimeoutExpired, this);
    }
    if (!mInitialized)
    {
        return;
    }
    mInitialized = false;
    // Cancels the poll timer, so TransferFacilitator::PollForOutput will not re-arm it after returning.
    Responder::ResetTransfer();

    if (mExchangeCtx != nullptr)
    {
        mExchangeCtx->Close();
        mExchangeCtx = nullptr;
    }
}

void BdxOTASession::Finish(CHIP_ERROR error)
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturn(!mFinished);
    mFinished = true;

    const bool shouldNotify = mBeginNotified || (mInitialized && error != CHIP_NO_ERROR);
    mBeginNotified          = false;
    ResetState();
    if (shouldNotify)
    {
        NotifyTransferSessionEnd(error);
    }
}

void BdxOTASession::CancelForReplacement()
{
    assertChipStackLockedByCurrentThread();
    mFinished      = true;
    mBeginNotified = false;
    ResetState();
}

void BdxOTASession::EndSession(CHIP_ERROR error)
{
    Finish(error);
    // The owner only detaches us here; we are destroyed on a later event loop turn because callers
    // (PollForOutput, timer callbacks) may still touch members after we return.
    if (mOwner != nullptr)
    {
        mOwner->ScheduleSessionRemoval(this);
    }
}

void BdxOTASession::AbortTransfer(bdx::StatusCode reason, CHIP_ERROR error)
{
    mAbortError    = (error == CHIP_NO_ERROR) ? CHIP_ERROR_INTERNAL : error;
    CHIP_ERROR err = mTransfer.AbortTransfer(reason);
    if (err != CHIP_NO_ERROR)
    {
        // No StatusReport will be emitted, so end the session now rather than waiting for the BDX timeout.
        LogErrorOnFailure(err);
        EndSession(mAbortError);
    }
}

void BdxOTASession::NotifyTransferSessionEnd(CHIP_ERROR error)
{
    VerifyOrReturn(mOtaDelegate != nullptr);

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    jmethodID handleBDXTransferSessionEndMethod;
    CHIP_ERROR err = JniReferences::GetInstance().FindMethod(env, mOtaDelegate, "handleBDXTransferSessionEnd", "(JJ)V",
                                                             &handleBDXTransferSessionEndMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find handleBDXTransferSessionEnd method"));

    env->CallVoidMethod(mOtaDelegate, handleBDXTransferSessionEndMethod, static_cast<jlong>(error.AsInteger()),
                        static_cast<jlong>(mNodeId));
    if (env->ExceptionCheck())
    {
        ChipLogError(Support, "Exception in call java method");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

CHIP_ERROR BdxOTASession::OnMessageToSend(TransferSession::OutputEvent & event)
{
    assertChipStackLockedByCurrentThread();

    if (mExchangeCtx == nullptr)
    {
        EndSession(mAbortError == CHIP_NO_ERROR ? CHIP_ERROR_INCORRECT_STATE : mAbortError);
        return CHIP_ERROR_INCORRECT_STATE;
    }
    VerifyOrReturnError(mOtaDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);

    Messaging::SendFlags sendFlags;

    // All messages sent from the Sender expect a response, except for a StatusReport which would indicate an error and
    // the end of the transfer.
    if (!event.msgTypeData.HasMessageType(Protocols::SecureChannel::MsgType::StatusReport))
    {
        sendFlags.Set(Messaging::SendMessageFlags::kExpectResponse);
    }

    auto & msgTypeData = event.msgTypeData;
    CHIP_ERROR err =
        mExchangeCtx->SendMessage(msgTypeData.ProtocolId, msgTypeData.MessageType, std::move(event.MsgData), sendFlags);
    if (err != CHIP_NO_ERROR)
    {
        mExchangeCtx->Close();
        mExchangeCtx = nullptr;
        EndSession(err);
    }
    else if (event.msgTypeData.HasMessageType(Protocols::SecureChannel::MsgType::StatusReport))
    {
        mExchangeCtx = nullptr;
        EndSession(mAbortError == CHIP_NO_ERROR ? CHIP_ERROR_INTERNAL : mAbortError);
    }
    return err;
}

CHIP_ERROR BdxOTASession::OnTransferSessionBegin(TransferSession::OutputEvent & event)
{
    assertChipStackLockedByCurrentThread();
    if (mTimerLayer)
    {
        mTimerLayer->CancelTimer(HandleBdxInitReceivedTimeoutExpired, this);
    }

    VerifyOrReturnError(mFabricIndex != kUndefinedFabricIndex, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mNodeId != kUndefinedNodeId, CHIP_ERROR_INCORRECT_STATE);
    uint16_t fdl = 0;

    const uint8_t * fd = mTransfer.GetFileDesignator(fdl);
    VerifyOrReturnError(fdl <= bdx::kMaxFileDesignatorLen, CHIP_ERROR_INVALID_ARGUMENT);
    CharSpan fileDesignatorSpan(Uint8::to_const_char(fd), fdl);

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    JniLocalReferenceScope scope(env);
    UtfString fileDesignator(env, fileDesignatorSpan);

    uint64_t offset = mTransfer.GetStartOffset();

    jmethodID handleBDXTransferSessionBeginMethod;
    CHIP_ERROR err = JniReferences::GetInstance().FindMethod(env, mOtaDelegate, "handleBDXTransferSessionBegin",
                                                             "(JLjava/lang/String;J)V", &handleBDXTransferSessionBeginMethod);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err, ChipLogError(Controller, "Could not find handleBDXTransferSessionBegin method"));

    // From here on Java may have allocated per-transfer resources, so it must get exactly one handleBDXTransferSessionEnd.
    mBeginNotified = true;
    env->CallVoidMethod(mOtaDelegate, handleBDXTransferSessionBeginMethod, static_cast<jlong>(mNodeId), fileDesignator.jniValue(),
                        static_cast<jlong>(offset));
    if (env->ExceptionCheck())
    {
        ChipLogError(Support, "Exception in call java method");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return CHIP_JNI_ERROR_EXCEPTION_THROWN;
    }

    TransferSession::TransferAcceptData acceptData;
    acceptData.ControlMode  = bdx::TransferControlFlags::kReceiverDrive;
    acceptData.MaxBlockSize = mTransfer.GetTransferBlockSize();
    acceptData.StartOffset  = mTransfer.GetStartOffset();
    acceptData.Length       = mTransfer.GetTransferLength();

    return mTransfer.AcceptTransfer(acceptData);
}

CHIP_ERROR BdxOTASession::OnTransferSessionEnd(TransferSession::OutputEvent & event)
{
    assertChipStackLockedByCurrentThread();

    CHIP_ERROR error = CHIP_NO_ERROR;
    if (event.EventType == TransferSession::OutputEventType::kTransferTimeout)
    {
        error = CHIP_ERROR_TIMEOUT;
    }
    else if (event.EventType != TransferSession::OutputEventType::kAckEOFReceived)
    {
        error = CHIP_ERROR_INTERNAL;
    }

    EndSession(error);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BdxOTASession::OnBlockQuery(TransferSession::OutputEvent & event)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mFabricIndex != kUndefinedFabricIndex, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mNodeId != kUndefinedNodeId, CHIP_ERROR_INCORRECT_STATE);

    uint16_t blockSize  = mTransfer.GetTransferBlockSize();
    uint32_t blockIndex = mTransfer.GetNextBlockNum();

    uint64_t bytesToSkip = 0;
    if (event.EventType == TransferSession::OutputEventType::kQueryWithSkipReceived)
    {
        bytesToSkip = event.bytesToSkip.BytesToSkip;
    }

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    JniLocalReferenceScope scope(env);
    jmethodID handleBDXQueryMethod;
    CHIP_ERROR err = JniReferences::GetInstance().FindMethod(
        env, mOtaDelegate, "handleBDXQuery", "(JIJJ)Lchip/devicecontroller/OTAProviderDelegate$BDXData;", &handleBDXQueryMethod);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err, ChipLogError(Controller, "Could not find handleBDXQuery method"));

    jobject bdxData =
        env->CallObjectMethod(mOtaDelegate, handleBDXQueryMethod, static_cast<jlong>(mNodeId), static_cast<jint>(blockSize),
                              static_cast<jlong>(blockIndex), static_cast<jlong>(bytesToSkip));
    if (env->ExceptionCheck())
    {
        ChipLogError(Support, "Exception in call java method");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return CHIP_JNI_ERROR_EXCEPTION_THROWN;
    }

    if (bdxData == nullptr)
    {
        ChipLogError(Controller, "Delegate returned no BDX data for node 0x" ChipLogFormatX64, ChipLogValueX64(mNodeId));
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    jmethodID getDataMethod;
    err = JniReferences::GetInstance().FindMethod(env, bdxData, "getData", "()[B", &getDataMethod);
    if (env->ExceptionCheck())
    {
        ChipLogError(Support, "Exception in call java method");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return CHIP_JNI_ERROR_EXCEPTION_THROWN;
    }

    jmethodID isEOFMethod;
    err = JniReferences::GetInstance().FindMethod(env, bdxData, "isEOF", "()Z", &isEOFMethod);
    if (env->ExceptionCheck())
    {
        ChipLogError(Support, "Exception in call java method");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return CHIP_JNI_ERROR_EXCEPTION_THROWN;
    }
    jbyteArray jData = (jbyteArray) env->CallObjectMethod(bdxData, getDataMethod);
    jboolean jIsEOF  = env->CallBooleanMethod(bdxData, isEOFMethod);

    JniByteArray data(env, jData);

    TransferSession::BlockData blockData;
    blockData.Data   = static_cast<const uint8_t *>(data.byteSpan().data());
    blockData.Length = static_cast<size_t>(data.byteSpan().size());
    blockData.IsEof  = jIsEOF == JNI_TRUE;

    return mTransfer.PrepareBlock(blockData);
}

void BdxOTASession::HandleTransferSessionOutput(TransferSession::OutputEvent & event)
{
    VerifyOrReturn(mOtaDelegate != nullptr);

    CHIP_ERROR err = CHIP_NO_ERROR;
    switch (event.EventType)
    {
    case TransferSession::OutputEventType::kInitReceived:
        err = OnTransferSessionBegin(event);
        if (err != CHIP_NO_ERROR)
        {
            AbortTransfer(GetBdxStatusCodeFromChipError(err), err);
        }
        break;
    case TransferSession::OutputEventType::kStatusReceived:
        ChipLogError(BDX, "Got StatusReport %x", static_cast<uint16_t>(event.statusData.statusCode));
        FALLTHROUGH;
    case TransferSession::OutputEventType::kAckEOFReceived:
    case TransferSession::OutputEventType::kInternalError:
    case TransferSession::OutputEventType::kTransferTimeout:
        err = OnTransferSessionEnd(event);
        break;
    case TransferSession::OutputEventType::kQueryWithSkipReceived:
    case TransferSession::OutputEventType::kQueryReceived:
        err = OnBlockQuery(event);
        if (err != CHIP_NO_ERROR)
        {
            AbortTransfer(bdx::StatusCode::kUnknown, err);
        }
        break;
    case TransferSession::OutputEventType::kMsgToSend:
        err = OnMessageToSend(event);
        break;
    case TransferSession::OutputEventType::kNone:
    case TransferSession::OutputEventType::kAckReceived:
        break;
    case TransferSession::OutputEventType::kAcceptReceived:
    case TransferSession::OutputEventType::kBlockReceived:
    default:
        chipDie();
        break;
    }
    LogErrorOnFailure(err);
}

void BdxOTASession::OnResponseTimeout(Messaging::ExchangeContext * ec)
{
    ChipLogError(BDX, "Response timeout from node 0x" ChipLogFormatX64, ChipLogValueX64(mNodeId));
    // The exchange is being torn down by its owner (ExchangeContext::NotifyResponseTimeout/OnSessionReleased), so it
    // must not be Close()d again.
    mExchangeCtx = nullptr;
    EndSession(CHIP_ERROR_TIMEOUT);
}

// ================= BdxOTASender Implementation =================

CHIP_ERROR BdxOTASender::Init(System::Layer * systemLayer, Messaging::ExchangeManager * exchangeMgr)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mSystemLayer == nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mExchangeMgr == nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(systemLayer != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(exchangeMgr != nullptr, CHIP_ERROR_INCORRECT_STATE);

    TEMPORARY_RETURN_IGNORED exchangeMgr->RegisterUnsolicitedMessageHandlerForProtocol(Protocols::BDX::Id, this);

    mSystemLayer = systemLayer;
    mExchangeMgr = exchangeMgr;

    return CHIP_NO_ERROR;
}

CHIP_ERROR BdxOTASender::Shutdown()
{
    assertChipStackLockedByCurrentThread();
    if (mExchangeMgr != nullptr)
    {
        TEMPORARY_RETURN_IGNORED mExchangeMgr->UnregisterUnsolicitedMessageHandlerForProtocol(Protocols::BDX::Id);
        mExchangeMgr = nullptr;
    }
    ResetState();
    if (mSystemLayer != nullptr)
    {
        mSystemLayer->CancelTimer(HandlePendingRemovals, this);
    }
    mPendingRemovals.clear();
    mSystemLayer = nullptr;
    return CHIP_NO_ERROR;
}

void BdxOTASender::ResetState()
{
    assertChipStackLockedByCurrentThread();
    // Move the sessions out first so Java callbacks issued from Finish() cannot observe or mutate a map being iterated.
    SessionMap sessions;
    sessions.swap(mSessions);
    for (auto & entry : sessions)
    {
        entry.second->Finish(CHIP_ERROR_CANCELLED);
    }
}

void BdxOTASender::ResetSession(FabricIndex fabricIndex, NodeId nodeId, CHIP_ERROR error)
{
    assertChipStackLockedByCurrentThread();
    auto it = mSessions.find(std::make_pair(fabricIndex, nodeId));
    VerifyOrReturn(it != mSessions.end());
    it->second->Finish(error);
    DetachSession(it);
}

void BdxOTASender::ScheduleSessionRemoval(BdxOTASession * session)
{
    assertChipStackLockedByCurrentThread();
    auto it = mSessions.find(std::make_pair(session->GetFabricIndex(), session->GetNodeId()));
    // The key may already map to a newer session for the same node; only detach the one that finished.
    VerifyOrReturn(it != mSessions.end() && it->second.get() == session);
    DetachSession(it);
}

void BdxOTASender::DetachSession(SessionMap::iterator it)
{
    mPendingRemovals.push_back(std::move(it->second));
    mSessions.erase(it);
    if (mSystemLayer != nullptr)
    {
        LogErrorOnFailure(mSystemLayer->StartTimer(System::Clock::kZero, HandlePendingRemovals, this));
    }
}

void BdxOTASender::HandlePendingRemovals(System::Layer * systemLayer, void * state)
{
    VerifyOrReturn(state != nullptr);
    // Runs from the event loop, so no session is on the call stack here.
    static_cast<BdxOTASender *>(state)->mPendingRemovals.clear();
}

CHIP_ERROR BdxOTASender::PrepareForTransfer(FabricIndex fabricIndex, NodeId nodeId)
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturnError(mExchangeMgr != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mSystemLayer != nullptr, CHIP_ERROR_INCORRECT_STATE);

    auto key        = std::make_pair(fabricIndex, nodeId);
    auto existingIt = mSessions.find(key);
    if (existingIt != mSessions.end())
    {
        // A subsequent QueryImage from the same node supersedes its previous session after Java accepted the query.
        existingIt->second->CancelForReplacement();
        DetachSession(existingIt);
    }

    if (mSessions.size() >= kMaxConcurrentBdxSessions)
    {
        // Evict the oldest reservation that has not yet started a BDX transfer so abandoned QueryImage reservations
        // cannot starve nodes that are ready to transfer.
        auto oldestPreInitIt = mSessions.end();
        for (auto it = mSessions.begin(); it != mSessions.end(); ++it)
        {
            if (!it->second->HasTransferStarted() &&
                (oldestPreInitIt == mSessions.end() ||
                 it->second->GetCreationOrder() < oldestPreInitIt->second->GetCreationOrder()))
            {
                oldestPreInitIt = it;
            }
        }
        if (oldestPreInitIt != mSessions.end())
        {
            ChipLogProgress(Controller, "Evicting unstarted BDX reservation for node 0x" ChipLogFormatX64,
                            ChipLogValueX64(oldestPreInitIt->second->GetNodeId()));
            oldestPreInitIt->second->Finish(CHIP_ERROR_TIMEOUT);
            DetachSession(oldestPreInitIt);
        }
    }

    // Only live sessions count; detached ones awaiting HandlePendingRemovals have already released their exchange.
    VerifyOrReturnError(mSessions.size() < kMaxConcurrentBdxSessions, CHIP_ERROR_BUSY,
                        ChipLogError(Controller, "Rejecting BDX transfer for node 0x" ChipLogFormatX64 ": %u transfers in progress",
                                     ChipLogValueX64(nodeId), static_cast<unsigned>(mSessions.size())));

    auto session   = std::make_unique<BdxOTASession>(this, mOtaDelegate, mSystemLayer, fabricIndex, nodeId, ++mNextCreationOrder);
    CHIP_ERROR err = session->PrepareForTransfer();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to prepare BDX transfer for node 0x" ChipLogFormatX64 ": %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(nodeId), err.Format());
        return err;
    }

    mSessions[key] = std::move(session);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BdxOTASender::OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, const SessionHandle & session,
                                                      Messaging::ExchangeDelegate *& newDelegate)
{
    ScopedNodeId peer       = session->GetPeer();
    FabricIndex fabricIndex = peer.GetFabricIndex();
    NodeId peerNodeId       = peer.GetNodeId();

    auto key = std::make_pair(fabricIndex, peerNodeId);
    auto it  = mSessions.find(key);
    if (it != mSessions.end())
    {
        newDelegate = it->second.get();
        return CHIP_NO_ERROR;
    }

    ChipLogError(Controller, "No BDX session registered for node 0x" ChipLogFormatX64 ", fabric %u", ChipLogValueX64(peerNodeId),
                 fabricIndex);
    return CHIP_ERROR_NOT_FOUND;
}
