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

// TODO Expose a method onto the delegate to make that configurable.
constexpr uint32_t kMaxBdxBlockSize = 1024;

// Since the BDX timeout is 5 minutes and we are starting this after query image is available and before the BDX init comes,
// we just double the timeout to give enough time for the BDX init to come in a reasonable amount of time.
constexpr System::Clock::Timeout kBdxInitReceivedTimeout = System::Clock::Seconds16(10 * 60);

constexpr System::Clock::Timeout kBdxTimeout        = System::Clock::Seconds16(5 * 60); // OTA Spec mandates >= 5 minutes
constexpr System::Clock::Timeout kBdxPollIntervalMs = System::Clock::Milliseconds32(50);
constexpr bdx::TransferRole kBdxRole                = bdx::TransferRole::kSender;

CHIP_ERROR BdxOTASender::PrepareForTransfer(FabricIndex fabricIndex, NodeId nodeId)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mExchangeMgr != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mSystemLayer != nullptr, CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(ConfigureState(fabricIndex, nodeId));

    BitFlags<bdx::TransferControlFlags> flags(bdx::TransferControlFlags::kReceiverDrive);
    return Responder::PrepareForTransfer(mSystemLayer, kBdxRole, flags, kMaxBdxBlockSize, kBdxTimeout, kBdxPollIntervalMs);
}

CHIP_ERROR BdxOTASender::Init(System::Layer * systemLayer, Messaging::ExchangeManager * exchangeMgr)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mSystemLayer == nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mExchangeMgr == nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(systemLayer != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(exchangeMgr != nullptr, CHIP_ERROR_INCORRECT_STATE);

    exchangeMgr->RegisterUnsolicitedMessageHandlerForProtocol(Protocols::BDX::Id, this);

    mSystemLayer = systemLayer;
    mExchangeMgr = exchangeMgr;

    return CHIP_NO_ERROR;
}

CHIP_ERROR BdxOTASender::Shutdown()
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mSystemLayer != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mExchangeMgr != nullptr, CHIP_ERROR_INCORRECT_STATE);

    mExchangeMgr->UnregisterUnsolicitedMessageHandlerForProtocol(Protocols::BDX::Id);
    ResetState();

    mExchangeMgr = nullptr;
    mSystemLayer = nullptr;

    return CHIP_NO_ERROR;
}

void BdxOTASender::ResetState()
{
    assertChipStackLockedByCurrentThread();
    if (mNodeId != kUndefinedNodeId && mFabricIndex != kUndefinedFabricIndex)
    {
        ChipLogProgress(Controller,
                        "Resetting state for OTA Provider; no longer providing an update for node id 0x" ChipLogFormatX64
                        ", fabric index %u",
                        ChipLogValueX64(mNodeId), mFabricIndex);
    }
    else
    {
        ChipLogProgress(Controller, "Resetting state for OTA Provider");
    }
    if (mSystemLayer)
    {
        mSystemLayer->CancelTimer(HandleBdxInitReceivedTimeoutExpired, this);
    }
    // TODO: Check if this can be removed. It seems like we can close the exchange context and reset transfer regardless.
    if (!mInitialized)
    {
        return;
    }
    Responder::ResetTransfer();
    ++mTransferGeneration;
    mFabricIndex = kUndefinedFabricIndex;
    mNodeId      = kUndefinedNodeId;

    if (mExchangeCtx != nullptr)
    {
        mExchangeCtx->Close();
        mExchangeCtx = nullptr;
    }

    mInitialized = false;
}

CHIP_ERROR BdxOTASender::OnMessageToSend(TransferSession::OutputEvent & event)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mExchangeCtx != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mOtaDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);

    Messaging::SendFlags sendFlags;

    // All messages sent from the Sender expect a response, except for a StatusReport which would indicate an error and
    // the end of the transfer.
    if (!event.msgTypeData.HasMessageType(Protocols::SecureChannel::MsgType::StatusReport))
    {
        sendFlags.Set(Messaging::SendMessageFlags::kExpectResponse);
    }

    auto & msgTypeData = event.msgTypeData;
    // If there's an error sending the message, close the exchange and call ResetState.
    // TODO: If we can remove the !mInitialized check in ResetState(), just calling ResetState() will suffice here.
    CHIP_ERROR err =
        mExchangeCtx->SendMessage(msgTypeData.ProtocolId, msgTypeData.MessageType, std::move(event.MsgData), sendFlags);
    if (err != CHIP_NO_ERROR)
    {
        mExchangeCtx->Close();
        mExchangeCtx = nullptr;
        ResetState();
    }
    else if (event.msgTypeData.HasMessageType(Protocols::SecureChannel::MsgType::StatusReport))
    {
        // If the send was successful for a status report, since we are not expecting a response the exchange context is
        // already closed. We need to null out the reference to avoid having a dangling pointer.
        mExchangeCtx = nullptr;
        ResetState();
    }
    return err;
}

CHIP_ERROR BdxOTASender::OnTransferSessionBegin(TransferSession::OutputEvent & event)
{
    assertChipStackLockedByCurrentThread();
    // Once we receive the BDX init, cancel the BDX Init timeout and start the BDX session
    if (mSystemLayer)
    {
        mSystemLayer->CancelTimer(HandleBdxInitReceivedTimeoutExpired, this);
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

    LogErrorOnFailure(mTransfer.AcceptTransfer(acceptData));

    return CHIP_NO_ERROR;
}

CHIP_ERROR BdxOTASender::OnTransferSessionEnd(TransferSession::OutputEvent & event)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mFabricIndex != kUndefinedFabricIndex, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mNodeId != kUndefinedNodeId, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mOtaDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);

    CHIP_ERROR error = CHIP_NO_ERROR;
    if (event.EventType == TransferSession::OutputEventType::kTransferTimeout)
    {
        error = CHIP_ERROR_TIMEOUT;
    }
    else if (event.EventType != TransferSession::OutputEventType::kAckEOFReceived)
    {
        error = CHIP_ERROR_INTERNAL;
    }

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    jmethodID handleBDXTransferSessionEndMethod;
    CHIP_ERROR err = JniReferences::GetInstance().FindMethod(env, mOtaDelegate, "handleBDXTransferSessionEnd", "(JJ)V",
                                                             &handleBDXTransferSessionEndMethod);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err, ChipLogError(Controller, "Could not find handleBDXTransferSessionEnd method"));

    env->CallVoidMethod(mOtaDelegate, handleBDXTransferSessionEndMethod, static_cast<jlong>(error.AsInteger()),
                        static_cast<jlong>(mNodeId));
    if (env->ExceptionCheck())
    {
        ChipLogError(Support, "Exception in call java method");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return CHIP_JNI_ERROR_EXCEPTION_THROWN;
    }

    ResetState();
    return CHIP_NO_ERROR;
}

CHIP_ERROR BdxOTASender::OnBlockQuery(TransferSession::OutputEvent & event)
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

    // uint64_t transferGeneration = mTransferGeneration;

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
        LogErrorOnFailure(mTransfer.AbortTransfer(bdx::StatusCode::kUnknown));
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

    err = mTransfer.PrepareBlock(blockData);
    if (CHIP_NO_ERROR != err)
    {
        LogErrorOnFailure(err);
        LogErrorOnFailure(mTransfer.AbortTransfer(bdx::StatusCode::kUnknown));
    }

    return CHIP_NO_ERROR;
}

void BdxOTASender::HandleTransferSessionOutput(TransferSession::OutputEvent & event)
{
    VerifyOrReturn(mOtaDelegate != nullptr);

    CHIP_ERROR err = CHIP_NO_ERROR;
    switch (event.EventType)
    {
    case TransferSession::OutputEventType::kInitReceived:
        err = OnTransferSessionBegin(event);
        if (err != CHIP_NO_ERROR)
        {
            LogErrorOnFailure(mTransfer.AbortTransfer(GetBdxStatusCodeFromChipError(err)));
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
        break;
    case TransferSession::OutputEventType::kMsgToSend:
        err = OnMessageToSend(event);
        break;
    case TransferSession::OutputEventType::kNone:
    case TransferSession::OutputEventType::kAckReceived:
        // Nothing to do.
        break;
    case TransferSession::OutputEventType::kAcceptReceived:
    case TransferSession::OutputEventType::kBlockReceived:
    default:
        // Should never happens.
        chipDie();
        break;
    }
    LogErrorOnFailure(err);
}

CHIP_ERROR BdxOTASender::ConfigureState(chip::FabricIndex fabricIndex, chip::NodeId nodeId)
{
    assertChipStackLockedByCurrentThread();

    if (mInitialized)
    {
        // Prevent a new node connection since another is active.
        VerifyOrReturnError(mFabricIndex == fabricIndex && mNodeId == nodeId, CHIP_ERROR_BUSY);

        // Reset stale connection from the same Node if exists.
        ResetState();
    }

    // Start a timer to track whether we receive a BDX init after a successful query image in a reasonable amount of time
    CHIP_ERROR err = mSystemLayer->StartTimer(kBdxInitReceivedTimeout, HandleBdxInitReceivedTimeoutExpired, this);
    LogErrorOnFailure(err);

    mFabricIndex = fabricIndex;
    mNodeId      = nodeId;

    mInitialized = true;

    return CHIP_NO_ERROR;
}
