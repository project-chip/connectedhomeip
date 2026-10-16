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
#pragma once

#include <jni.h>
#include <map>
#include <memory>
#include <utility>
#include <vector>

#include <messaging/ExchangeMgr.h>
#include <protocols/bdx/BdxUri.h>
#include <protocols/bdx/TransferFacilitator.h>

constexpr uint32_t kMaxBDXURILen = 256;

class BdxOTASender;

/**
 * Per-node BDX responder session that drives a single OTA firmware transfer and reports lifecycle
 * events to the Java OTAProviderDelegate.
 */
class BdxOTASession : public chip::bdx::Responder
{
public:
    BdxOTASession(BdxOTASender * owner, jobject otaDelegate, chip::System::Layer * systemLayer, chip::FabricIndex fabricIndex,
                  chip::NodeId nodeId, uint64_t creationOrder = 0);

    ~BdxOTASession() override;

    CHIP_ERROR PrepareForTransfer();

    /**
     * Terminates the session: notifies the Java delegate via handleBDXTransferSessionEnd (at most once, when a transfer
     * has begun or when an initialized reservation ends with an error) and releases transport state. Idempotent. Does
     * not touch the owner, so the caller is responsible for detaching the session from the owner.
     */
    void Finish(CHIP_ERROR error);

    /**
     * Releases transport state and marks the session finished without notifying the Java delegate. Used when a new
     * QueryImage from the same peer supersedes an existing session after the Java delegate has already accepted the new
     * query.
     */
    void CancelForReplacement();

    bool HasTransferStarted() const { return mBeginNotified; }

    uint64_t GetCreationOrder() const { return mCreationOrder; }

    chip::FabricIndex GetFabricIndex() const { return mFabricIndex; }

    chip::NodeId GetNodeId() const { return mNodeId; }

private:
    static void HandleBdxInitReceivedTimeoutExpired(chip::System::Layer * systemLayer, void * state);

    void ResetState();

    void EndSession(CHIP_ERROR error);

    void AbortTransfer(chip::bdx::StatusCode reason, CHIP_ERROR error);

    void NotifyTransferSessionEnd(CHIP_ERROR error);

    CHIP_ERROR OnMessageToSend(chip::bdx::TransferSession::OutputEvent & event);

    CHIP_ERROR OnTransferSessionBegin(chip::bdx::TransferSession::OutputEvent & event);

    CHIP_ERROR OnTransferSessionEnd(chip::bdx::TransferSession::OutputEvent & event);

    CHIP_ERROR OnBlockQuery(chip::bdx::TransferSession::OutputEvent & event);

    void HandleTransferSessionOutput(chip::bdx::TransferSession::OutputEvent & event) override;

    void OnResponseTimeout(chip::Messaging::ExchangeContext * ec) override;

    BdxOTASender * mOwner = nullptr;
    jobject mOtaDelegate  = nullptr;
    // Our own copy: it intentionally outlives Responder::ResetTransfer(), which nulls TransferFacilitator::mSystemLayer.
    chip::System::Layer * mTimerLayer = nullptr;
    bool mInitialized                 = false;
    bool mBeginNotified               = false;
    bool mFinished                    = false;
    CHIP_ERROR mAbortError            = CHIP_NO_ERROR;
    chip::FabricIndex mFabricIndex    = chip::kUndefinedFabricIndex;
    chip::NodeId mNodeId              = chip::kUndefinedNodeId;
    uint64_t mCreationOrder           = 0;
};

/**
 * Multiplexes unsolicited BDX messages across concurrent per-node BdxOTASession instances.
 */
class BdxOTASender : public chip::Messaging::UnsolicitedMessageHandler
{
public:
    BdxOTASender(jobject otaDelegate) : mOtaDelegate(otaDelegate) {}

    ~BdxOTASender() override { static_cast<void>(Shutdown()); }

    CHIP_ERROR PrepareForTransfer(chip::FabricIndex fabricIndex, chip::NodeId nodeId);

    CHIP_ERROR Init(chip::System::Layer * systemLayer, chip::Messaging::ExchangeManager * exchangeMgr);

    CHIP_ERROR Shutdown();

    /** Terminates all active BDX sessions. Must not be called from within a session callback. */
    void ResetState();

    /** Terminates the session for a single node, if any, leaving other nodes' transfers untouched. */
    void ResetSession(chip::FabricIndex fabricIndex, chip::NodeId nodeId, CHIP_ERROR error);

    /** Detaches a finished session immediately and schedules its destruction on the next event loop turn. */
    void ScheduleSessionRemoval(BdxOTASession * session);

    CHIP_ERROR OnUnsolicitedMessageReceived(const chip::PayloadHeader & payloadHeader, const chip::SessionHandle & session,
                                            chip::Messaging::ExchangeDelegate *& newDelegate) override;

private:
    using SessionKey = std::pair<chip::FabricIndex, chip::NodeId>;
    using SessionMap = std::map<SessionKey, std::unique_ptr<BdxOTASession>>;

    static void HandlePendingRemovals(chip::System::Layer * systemLayer, void * state);

    void DetachSession(SessionMap::iterator it);

    jobject mOtaDelegate                            = nullptr;
    chip::System::Layer * mSystemLayer              = nullptr;
    chip::Messaging::ExchangeManager * mExchangeMgr = nullptr;
    uint64_t mNextCreationOrder                     = 0;

    SessionMap mSessions;
    // Finished sessions that may still be on the call stack; freed from HandlePendingRemovals.
    std::vector<std::unique_ptr<BdxOTASession>> mPendingRemovals;
};
