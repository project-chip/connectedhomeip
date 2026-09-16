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

/* This file contains a definition for a class that implements OTADownloader and downloads CHIP OTA images using the BDX protocol.
 * It should not execute any logic that is application specific.
 */

// TODO: unit tests

#pragma once

#include "OTADownloader.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <lib/core/CHIPError.h>
#include <protocols/bdx/BdxTransferSession.h>
#include <system/SystemPacketBuffer.h>
#include <transport/raw/MessageHeader.h>

namespace chip {

class BDXDownloader : public chip::OTADownloader
{
public:
    // A delegate for passing messages to/from BDXDownloader and some messaging layer. This is mainly to make BDXDownloader more
    // easily unit-testable.
    class MessagingDelegate
    {
    public:
        virtual CHIP_ERROR SendMessage(const chip::bdx::TransferSession::OutputEvent & msgEvent) = 0;
        virtual ~MessagingDelegate() {}
    };

    class StateDelegate
    {
    public:
        // Handle download state change
        virtual void OnDownloadStateChanged(State state, app::Clusters::OtaSoftwareUpdateRequestor::OTAChangeReasonEnum reason) = 0;
        // Handle update progress change
        virtual void OnUpdateProgressChanged(app::DataModel::Nullable<uint8_t> percent) = 0;
        virtual ~StateDelegate()                                                        = default;
    };

    // To be called when there is an incoming message to handle (of any protocol type)
    void OnMessageReceived(const chip::PayloadHeader & payloadHeader, chip::System::PacketBufferHandle msg);

    void SetMessageDelegate(MessagingDelegate * delegate) { mMsgDelegate = delegate; }
    void SetStateDelegate(StateDelegate * delegate) { mStateDelegate = delegate; }

    // Initialize a BDX transfer session but will not proceed until OnPreparedForDownload() is called.
    CHIP_ERROR SetBDXParams(const chip::bdx::TransferSession::TransferInitData & bdxInitData, System::Clock::Timeout timeout);

    // OTADownloader Overrides
    CHIP_ERROR BeginPrepareDownload() override;
    CHIP_ERROR OnPreparedForDownload(CHIP_ERROR status) override;
    void OnDownloadTimeout() override;
    // BDX does not provide a mechanism for the driver of a transfer to gracefully end the exchange, so it will abort the transfer
    // instead.
    void EndDownload(CHIP_ERROR reason = CHIP_NO_ERROR) override;
    CHIP_ERROR FetchNextData() override;
    CHIP_ERROR SkipData(uint32_t numBytes) override;

    System::Clock::Timeout GetTimeout();
    // If True, there's been a timeout in the transfer as measured by no download progress after 'mTimeout' seconds.
    // If False, there's been progress in the transfer.
    bool HasTransferTimedOut();

    // Most recent download failure cause, so the requestor can route only a true peer abort
    // (kPeerStatusReport) through the short-retry path rather than the generic kFailure reason that
    // the downloader funnels every failure mode through.
    enum class LastFailureCause
    {
        kNone,
        kPeerStatusReport,   // Peer sent a BDX StatusReport mid-transfer (TransferSession::kStatusReceived)
        kLocalInternalError, // TransferSession::kInternalError (locally-detected protocol/state error)
        kLocalPrepareFailed, // OnPreparedForDownload(non-CHIP_NO_ERROR) — image processor prepare failed locally
        kTimeout,            // TransferSession::kTransferTimeout
    };
    LastFailureCause GetLastFailureCause() const { return mLastFailureCause; }

    // Human-readable name for the LastFailureCause enum, for logging.
    static const char * LastFailureCauseToString(LastFailureCause cause)
    {
        switch (cause)
        {
        case LastFailureCause::kNone:
            return "kNone";
        case LastFailureCause::kPeerStatusReport:
            return "kPeerStatusReport";
        case LastFailureCause::kLocalInternalError:
            return "kLocalInternalError";
        case LastFailureCause::kLocalPrepareFailed:
            return "kLocalPrepareFailed";
        case LastFailureCause::kTimeout:
            return "kTimeout";
        }
        return "kUnknown";
    }

private:
    void PollTransferSession();
    // Tear down BDX state and transition to kIdle. Reset() clears mLastFailureCause, so the cause
    // must be passed here (assigned after the resets, before SetState() fires the StateDelegate)
    // rather than set directly beforehand.
    void CleanupOnError(app::Clusters::OtaSoftwareUpdateRequestor::OTAChangeReasonEnum reason,
                        LastFailureCause cause = LastFailureCause::kNone);
    CHIP_ERROR HandleBdxEvent(const chip::bdx::TransferSession::OutputEvent & outEvent);
    void SetState(State state, app::Clusters::OtaSoftwareUpdateRequestor::OTAChangeReasonEnum reason);
    void Reset();

    chip::bdx::TransferSession mBdxTransfer;
    MessagingDelegate * mMsgDelegate = nullptr;
    StateDelegate * mStateDelegate   = nullptr;
    // Timeout value in seconds to abort the download if there's no progress in the transfer session.
    System::Clock::Timeout mTimeout = System::Clock::kZero;
    // Tracks the last block counter used during the transfer session as of the previous check.
    uint32_t mPrevBlockCounter = 0;
    // Records the most recent failure cause so OnDownloadStateChanged consumers can disambiguate
    // peer-driven aborts from locally-caused failures. Cleared on successful state transitions.
    LastFailureCause mLastFailureCause = LastFailureCause::kNone;
};

} // namespace chip
