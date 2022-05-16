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
    // TODO: override SkipData

    System::Clock::Timeout GetTimeout();
    // If True, there's been a timeout in the transfer as measured by no download progress after 'mTimeout' seconds.
    // If False, there's been progress in the transfer.
    bool HasTransferTimedOut();

private:
    void PollTransferSession();
    void CleanupOnError(app::Clusters::OtaSoftwareUpdateRequestor::OTAChangeReasonEnum reason);
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
};

} // namespace chip
