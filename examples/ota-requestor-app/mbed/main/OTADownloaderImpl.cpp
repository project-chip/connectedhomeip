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

#include <OTADownloaderImpl.h>
#include <app/OperationalDeviceProxy.h>
#include <app/server/Server.h>
#include <platform/CHIPDeviceLayer.h>

using namespace ::chip;
using namespace chip::bdx;

OTADownloaderImpl OTADownloaderImpl::sInstance;

OTADownloaderImpl::OTADownloaderImpl() {}

void OTADownloaderImpl::BeginDownload()
{
    ChipLogProgress(SoftwareUpdate, "Begin download");

    OperationalDeviceProxy * deviceProxy = Server::GetInstance().GetOperationalDeviceProxy();
    if (deviceProxy == nullptr)
    {
        ChipLogError(SoftwareUpdate, "Provider connection not established");
        return;
    }

    Messaging::ExchangeManager * exchangeMgr = deviceProxy->GetExchangeManager();
    Optional<SessionHandle> session          = deviceProxy->GetSecureSession();
    Messaging::ExchangeContext * exchangeCtx = nullptr;

    if (exchangeMgr != nullptr && session.HasValue())
    {
        exchangeCtx = exchangeMgr->NewContext(session.Value(), this);
    }

    if (exchangeCtx == nullptr)
    {
        ChipLogError(SoftwareUpdate, "Failed to allocate exchange");
        return;
    }

    mExchangeCtx = exchangeCtx;

    TransferSession::TransferInitData initOptions;
    initOptions.TransferCtlFlags = TransferControlFlags::kReceiverDrive;
    initOptions.MaxBlockSize     = mBlockSize;
    initOptions.FileDesLength    = mImageInfo.imageName.size();
    initOptions.FileDesignator   = reinterpret_cast<const uint8_t *>(mImageInfo.imageName.data());

    CHIP_ERROR error =
        InitiateTransfer(&DeviceLayer::SystemLayer(), TransferRole::kReceiver, initOptions, System::Clock::Seconds16(20));

    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Failed to initiate BDX transfer: %" CHIP_ERROR_FORMAT, error.Format());
    }
}

void OTADownloaderImpl::OnPreparedForDownload()
{
    ChipLogProgress(SoftwareUpdate, "On Prepared For Download");
}

void OTADownloaderImpl::OnBlockProcessed(BlockActionType action)
{
    ChipLogProgress(SoftwareUpdate, "On Block Processed");
}

void OTADownloaderImpl::HandleTransferSessionOutput(TransferSession::OutputEvent & event)
{
    using OutputEventType  = TransferSession::OutputEventType;
    using SendMessageFlags = Messaging::SendMessageFlags;

    CHIP_ERROR error = CHIP_NO_ERROR;

    if (event.EventType != OutputEventType::kNone)
    {
        ChipLogDetail(BDX, "OutputEvent type: %s", event.ToString(event.EventType));
    }

    switch (event.EventType)
    {
    case OutputEventType::kNone:
        if (mIsTransferComplete)
        {
            ChipLogProgress(BDX, "Image file transfer complete");
            mTransfer.Reset();
            mIsTransferComplete = false;

            error = mImageProcessorDelegate->Finalize();
            if (error != CHIP_NO_ERROR)
            {
                ChipLogError(BDX, "Image processing finalize failed: %" CHIP_ERROR_FORMAT, error.Format());
                mImageProcessorDelegate->Abort();
                return;
            }

            if (mDownloadCompletedCallback)
            {
                mDownloadCompletedCallback(&mImageInfo);
            }
        }
        break;
    case OutputEventType::kMsgToSend: {
        Messaging::SendFlags flags;
        flags.Set(SendMessageFlags::kFromInitiator, event.msgTypeData.MessageType == to_underlying(MessageType::ReceiveInit));
        flags.Set(SendMessageFlags::kExpectResponse, event.msgTypeData.MessageType != to_underlying(MessageType::BlockAckEOF));

        VerifyOrReturn(mExchangeCtx != nullptr, ChipLogError(BDX, "Exchange context is null"));
        error =
            mExchangeCtx->SendMessage(event.msgTypeData.ProtocolId, event.msgTypeData.MessageType, std::move(event.MsgData), flags);
        VerifyOrReturn(error == CHIP_NO_ERROR, ChipLogError(BDX, "SendMessage failed: %" CHIP_ERROR_FORMAT, error.Format()));
        break;
    }
    case TransferSession::OutputEventType::kAcceptReceived:
        ChipLogProgress(BDX, "Starting image file transfer size %lldB", static_cast<uint64_t>(event.transferAcceptData.Length));
        error = mImageProcessorDelegate->PrepareDownload();
        if (error != CHIP_NO_ERROR)
        {
            ChipLogError(BDX, "Image processing prepare failed: %" CHIP_ERROR_FORMAT, error.Format());
            mTransfer.Reset();
            mImageProcessorDelegate->Abort();
            return;
        }

        mImageInfo.imageSize = 0;

        error = mTransfer.PrepareBlockQuery();
        VerifyOrReturn(error == CHIP_NO_ERROR, ChipLogError(BDX, "PrepareBlockQuery failed: %" CHIP_ERROR_FORMAT, error.Format()));

        break;
    case TransferSession::OutputEventType::kBlockReceived: {
        ChipLogProgress(BDX, "Received %uB (total: %ukB)", static_cast<unsigned>(event.blockdata.Length),
                        static_cast<unsigned>(mTransfer.GetNumBytesProcessed()) / 1024u);

        ByteSpan data(event.blockdata.Data, event.blockdata.Length);
        mImageProcessorDelegate->ProcessBlock(data);
        if (error != CHIP_NO_ERROR)
        {
            ChipLogError(BDX, "Image processing process block failed: %" CHIP_ERROR_FORMAT, error.Format());
            mTransfer.Reset();
            ;
            mImageProcessorDelegate->Abort();
            return;
        }

        mImageInfo.imageSize += event.blockdata.Length;

        if (event.blockdata.IsEof)
        {
            error = mTransfer.PrepareBlockAck();
            VerifyOrReturn(error == CHIP_NO_ERROR,
                           ChipLogError(BDX, "PrepareBlockAck failed: %" CHIP_ERROR_FORMAT, error.Format()));
            mIsTransferComplete = true;
        }
        else
        {
            error = mTransfer.PrepareBlockQuery();
            VerifyOrReturn(error == CHIP_NO_ERROR,
                           ChipLogError(BDX, "PrepareBlockQuery failed: %" CHIP_ERROR_FORMAT, error.Format()));
        }
        break;
    }
    case TransferSession::OutputEventType::kStatusReceived:
        ChipLogError(BDX, "Received status %" PRIu16, to_underlying(event.statusData.statusCode));

        if (event.statusData.statusCode != bdx::StatusCode::kNone)
        {
            mTransfer.Reset();
            mImageProcessorDelegate->Abort();
        }
        break;
    case TransferSession::OutputEventType::kInternalError:
        ChipLogError(BDX, "Transfer stopped due to internal error");
        mTransfer.Reset();
        mImageProcessorDelegate->Abort();
        break;
    case TransferSession::OutputEventType::kTransferTimeout:
        ChipLogError(BDX, "Transfer timed out");
        mTransfer.Reset();
        mImageProcessorDelegate->Abort();
        break;
    case TransferSession::OutputEventType::kInitReceived:
    case TransferSession::OutputEventType::kAckReceived:
    case TransferSession::OutputEventType::kQueryReceived:
    case TransferSession::OutputEventType::kAckEOFReceived:
    default:
        ChipLogError(BDX, "Unexpected BDX event type: %" PRIu16, to_underlying(event.EventType));
    }
}
