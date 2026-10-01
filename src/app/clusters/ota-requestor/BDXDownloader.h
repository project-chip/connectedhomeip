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
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/core/ScopedNodeId.h>
#include <protocols/bdx/BdxTransferSession.h>
#include <system/SystemPacketBuffer.h>
#include <transport/raw/MessageHeader.h>

#include <array>
#include <optional>

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
        // Called after the downloader went idle without an error because the provider could not continue the partial image: it
        // rejected the requested start offset, or it serves the image from its start. The partial image is gone, so starting the
        // download of the same image again, now from offset 0, within the current update attempt avoids waiting for the next
        // query.
        virtual void OnDownloadRestartRequested() {}
        virtual ~StateDelegate() = default;
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

    // Identifies the image the next SetBDXParams() downloads, along with the file designator in its TransferInitData. A partial
    // image is resumed only when the version, the provider and the file designator all match the ones it was downloaded with;
    // otherwise it is discarded and the download starts at offset 0. Without an identity, a download always starts at offset 0.
    //
    // The identity cannot cover the content of the image: QueryImageResponse carries no size or digest. A provider that replaces
    // an image under the same version and file designator therefore gets a resumed download that joins bytes of the old and the
    // new image. The image processor is expected to catch this by checking the finished image against its header before
    // Apply(), which costs the download but never applies a wrong image.
    //
    // A suspended partial image stays in the image processor's storage until the next download attempt (which resumes or
    // discards it), a cancelled update, a removed fabric, or three resumed attempts in a row without progress. There is no
    // time limit: a requestor that is never offered the update again keeps the partial image, so a platform with little storage
    // should have its processor discard it by itself.
    //
    // Resume state, by event:
    //
    //   event                              mResumeIdentity     partial image         next start offset
    //   ---------------------------------  ------------------  --------------------  ------------------------------
    //   SetBDXParams(), same identity      kept                kept                  processor's GetResumeOffset()
    //   SetBDXParams(), other identity     replaced            discarded             0
    //   timeout                            kept                suspended             (as above)
    //   timeout after the EOF block        cleared             discarded             0
    //   third resume without progress      kept                discarded             0
    //   resumed data starts with a header  kept                discarded             0, restarted in the same attempt
    //   StatusReport START_OFFSET_N/S      kept                discarded             0, restarted in the same attempt
    //   other failure, local abort         kept                discarded             0
    //   BlockAckEOF sent                   cleared             complete image        0
    //   EndDownload() with none running    cleared             discarded             0
    //   EndDownload() while preparing      cleared             discarded             0, the queued prepare is stale
    void SetImageIdentity(uint32_t softwareVersion, const ScopedNodeId & provider);

    System::Clock::Timeout GetTimeout();
    // If True, there's been a timeout in the transfer as measured by no download progress after 'mTimeout' seconds.
    // If False, there's been progress in the transfer.
    bool HasTransferTimedOut();

private:
    using ImageIdentity = std::array<uint8_t, Crypto::kSHA256_Hash_Length>;

    void PollTransferSession();
    void CleanupOnError(app::Clusters::OtaSoftwareUpdateRequestor::OTAChangeReasonEnum reason);
    CHIP_ERROR HandleBdxEvent(const chip::bdx::TransferSession::OutputEvent & outEvent);
    void SetState(State state, app::Clusters::OtaSoftwareUpdateRequestor::OTAChangeReasonEnum reason);
    void Reset();
    uint64_t ChooseStartOffset();
    void DiscardPartialImage();
    std::optional<ImageIdentity> ComputeImageIdentity(ByteSpan fileDesignator) const;
    static bool StartsWithImageHeader(ByteSpan data);

    chip::bdx::TransferSession mBdxTransfer;
    MessagingDelegate * mMsgDelegate = nullptr;
    StateDelegate * mStateDelegate   = nullptr;
    // Timeout value in seconds to abort the download if there's no progress in the transfer session.
    System::Clock::Timeout mTimeout = System::Clock::kZero;
    // Tracks the last block counter used during the transfer session as of the previous check.
    uint32_t mPrevBlockCounter = 0;
    std::optional<uint32_t> mImageVersion;
    ScopedNodeId mImageProvider;
    // SHA-256 over the version, provider and file designator of the image the processor holds data of; empty if unknown.
    std::optional<ImageIdentity> mResumeIdentity;
    uint64_t mRequestedOffset        = 0;
    uint8_t mAttemptsWithoutProgress = 0;
    bool mDiscardPending             = false;
    // Set when the current failure should be followed by an immediate restart from offset 0.
    bool mRestartFromStart = false;
    // Set while the first block of a transfer from a non-zero offset has not arrived yet.
    bool mCheckFirstBlock = false;
    // Set once the processor was asked to finalize the image, after the EOF block.
    bool mFinalized = false;
};

} // namespace chip
