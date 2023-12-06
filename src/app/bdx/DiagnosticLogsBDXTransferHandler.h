/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <lib/core/CHIPConfig.h>

#if CHIP_CONFIG_ENABLE_BDX_LOG_TRANSFER

#pragma once

#include <app/clusters/diagnostic-logs-server/diagnostic-logs-provider-delegate.h>

#include <protocols/bdx/TransferFacilitator.h>
#include <transport/Session.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DiagnosticLogs {

/**
 * The BDX transfer handler that initiates a BDX transfer session as a Sender using the synchronous Sender Drive
 * transfer mode. It gets the chunks of the log and sends the block across to the receiver until
 * all the blocks have been transferred and the delegate reports that end of file is reached.
 */
class DiagnosticLogsBDXTransferHandler : public bdx::Initiator
{
public:
    DiagnosticLogsBDXTransferHandler(){};
    ~DiagnosticLogsBDXTransferHandler(){};

    /**
     * Intializes the BDX transfer session by creating a new exchange context for the transfer session.
     * It starts the BDX transfer session by calling InitiateTransfer which sends the SendInit BDX message
     * to the log requestor.
     *
     * @param[in] exchangeMgr  The exchange manager from the command handler object for the RetrieveLogsRequest command
     * @param[in] sessionHandle The session handle from the command handler object for the RetrieveLogsRequest command
     * @param[in] fabricIndex The fabric index of the requestor
     * @param[in] peerNodeId The node id of the requestor
     * @param[in] delegate The log provider delegate that will provide the log chunks
     * @param[in] intent The log type requested
     * @param[in] fileDesignator The file designator to use for the BDX transfer
     */
    CHIP_ERROR InitializeTransfer(Messaging::ExchangeManager * exchangeMgr, const SessionHandle sessionHandle,
                                  FabricIndex fabricIndex, NodeId peerNodeId, LogProviderDelegate * delegate, IntentEnum intent,
                                  CharSpan fileDesignator);

    /**
     * This method handles BDX messages and other TransferSession events.
     *
     * @param[in] event An OutputEvent that contains output from the TransferSession object.
     */
    void HandleTransferSessionOutput(bdx::TransferSession::OutputEvent & event) override;

private:
    /**
     * This method is called to reset state. It resets the transfer, cleans up the
     * exchange and ends log collection.
     */
    void Reset();

    /**
     * This method handles any error that might occur during the BDX session.
     * If the error occurs before SendAccept is received, it sends a command response
     * with appropriate status to the requestor. After that, it resets its state and
     * schedules an asynchronous deletion of the DiagnosticLogsBDXTransferHandler object.
     *
     * @param[in] error The error that occured during the BDX session
     */
    void HandleBDXError(CHIP_ERROR error);

    /**
     * This method calls Reset to clean up state and asynchronously deletes the
     * DiagnosticLogsBDXTransferHandler object by scheduling the delete operation.
     * This should be called only once on a DiagnosticLogsBDXTransferHandler object
     * since whenever this is called the DiagnosticLogsBDXTransferHandler object will not
     * exist and can't be used anymore.
     *
     */
    void ScheduleCleanUp();

    Optional<FabricIndex> mFabricIndex;
    Optional<NodeId> mPeerNodeId;

    Messaging::ExchangeContext * mBDXTransferExchangeCtx;

    bool mInitialized = false;

    uint64_t mNumBytesSent;

    LogSessionHandle mLogSessionHandle = kInvalidLogSessionHandle;

    LogProviderDelegate * mDelegate;

    IntentEnum mIntent;

    bool mIsAcceptReceived = false;
};

} // namespace DiagnosticLogs
} // namespace Clusters
} // namespace app
} // namespace chip
#endif
