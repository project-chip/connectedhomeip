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

#pragma once

#include <lib/core/CHIPConfig.h>

#if CHIP_CONFIG_ENABLE_BDX_LOG_TRANSFER

#include <app/CommandHandler.h>
#include <app/clusters/diagnostic-logs-server/DiagnosticLogsProviderDelegate.h>
#include <protocols/bdx/TransferFacilitator.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DiagnosticLogs {

class BDXDiagnosticLogsProvider : public bdx::Initiator
{
public:
    BDXDiagnosticLogsProvider(){};
    ~BDXDiagnosticLogsProvider(){};

    /**
     * Intializes the BDX transfer session by creating a new exchange context for the transfer session.
     * It starts the BDX transfer session by calling InitiateTransfer which sends the SendInit BDX message
     * to the log requestor.
     *
     * @param commandObj      The command handler object from the RetrieveLogsRequest command
     * @param path            The command path from the RetrieveLogsRequest command
     * @param delegate        The log provider delegate that will provide the log chunks
     * @param intent          The log type requested
     * @param fileDesignator  The file designator to use for the BDX transfer
     */
    CHIP_ERROR InitializeTransfer(CommandHandler * commandObj, const ConcreteCommandPath & path,
                                  DiagnosticLogsProviderDelegate * delegate, IntentEnum intent, CharSpan fileDesignator);

    /**
     * This method handles BDX messages and other TransferSession events.
     *
     * @param[in] event An OutputEvent that contains output from the TransferSession object.
     */
    void HandleTransferSessionOutput(bdx::TransferSession::OutputEvent & event) override;

    bool IsBusy() const { return mInitialized; }

    void OnExchangeClosing(Messaging::ExchangeContext * ec) override;

private:
    void OnMsgToSend(bdx::TransferSession::OutputEvent & event);
    void OnAcceptReceived();
    void OnAckReceived();
    void OnAckEOFReceived();
    void OnStatusReceived(bdx::TransferSession::OutputEvent & event);
    void OnInternalError();
    void OnTimeout();

    void SendCommandResponse(StatusEnum status);

    /**
     * This method is called to reset state. It resets the transfer, cleans up the
     * exchange and ends log collection.
     */
    void Reset();

    Messaging::ExchangeContext * mBDXTransferExchangeCtx;
    DiagnosticLogsProviderDelegate * mDelegate;
    Optional<FabricIndex> mFabricIndex;
    Optional<NodeId> mPeerNodeId;
    bool mIsAcceptReceived             = false;
    LogSessionHandle mLogSessionHandle = kInvalidLogSessionHandle;
    Optional<uint64_t> mTimeStamp;
    Optional<uint64_t> mTimeSinceBoot;
    CommandHandler::Handle mAsyncCommandHandle;
    ConcreteCommandPath mRequestPath = ConcreteCommandPath(kInvalidEndpointId, kInvalidClusterId, kInvalidCommandId);
    bool mInitialized                = false;
};

} // namespace DiagnosticLogs
} // namespace Clusters
} // namespace app
} // namespace chip
#endif
