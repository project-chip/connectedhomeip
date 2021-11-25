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

#pragma once

#include "ota-downloader.h"

#include <lib/core/CHIPError.h>
#include <protocols/bdx/TransferSession.h>

class BdxDownloader : public OTADownloader
{
public:
    class MessagingDelegate
    {
        virtual CHIP_ERROR SendMessage(const chip::bdx::TransferSession::OutputEvent & msgEvent) = 0;
    };

    BdxDownloader() : OTADownloader() {}

    void OnMessageReceived(const PayloadHeader & payloadHeader, System::PacketBufferHandle msg);
    void SetMessageDelegate(MessagingDelegate * delegate) { mMsgDelegate = delegate; }

    // Initialize a BDX transfer session but will not proceed until OnPreparedForDownload() is called.
    CHIP_ERROR SetBDXParams(const chip::bdx::TransferSession::TransferInitData & bdxInitData);

    // OTADownloader Overrides
    CHIP_ERROR BeginPrepareDownload() override;
    CHIP_ERROR OnPreparedForDownload() override;
    void OnDownloadTimeout() override;
    // BDX does not provide a mechanism for the driver of a transfer to gracefully end the exchange, so it will abort the transfer
    // instead.
    void EndDownload(CHIP_ERROR reason = CHIP_NO_ERROR) override;
    CHIP_ERROR FetchNextData() override;
    // TODO: override SkipData

private:
    void PollTransferSession();
    CHIP_ERROR HandleBdxEvent(const chip::bdx::TransferSession::OutputEvent & outEvent);

    chip::bdx::TransferSession mBdxTransfer;
    MessagingDelegate * mMsgDelegate;
};
