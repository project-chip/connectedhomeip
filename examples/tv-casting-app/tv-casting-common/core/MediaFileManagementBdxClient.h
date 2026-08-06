/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/OperationalSessionSetup.h>
#include <lib/core/CHIPCallback.h>
#include <lib/core/CHIPError.h>
#include <lib/core/ScopedNodeId.h>
#include <messaging/ExchangeMgr.h>
#include <protocols/bdx/BdxTransferSession.h>
#include <protocols/bdx/TransferFacilitator.h>
#include <transport/Session.h>

#include <fstream>
#include <string>

namespace matter {
namespace casting {
namespace core {

/**
 * BDX receiver for the casting client's GetSharedFile flow.
 *
 * After the client calls GetSharedFile and receives a FileDescription carrying a
 * `bdx://<tv-app-node-id>/<designator>` ImageUri, it pulls the bytes from the
 * tv-app: it opens a CASE session to the Media Device, initiates a
 * receiver-drive BDX transfer for the given designator, and writes each received
 * block to a target path on disk.
 *
 * Mirror of the tv-app's MediaFileManagementBdxRequestor. A single instance
 * handles one transfer at a time.
 */
class MediaFileManagementBdxClient : public chip::bdx::Initiator
{
public:
    MediaFileManagementBdxClient() :
        mOnConnected(OnDeviceConnected, this), mOnConnectionFailure(OnDeviceConnectionFailure, this)
    {}

    /**
     * Download the file identified by `designator` from `peer` into `targetPath`.
     * Establishes a CASE session if needed, then drives the BDX transfer. The
     * transfer runs asynchronously; the file at `targetPath` is complete once the
     * BDX AckEOF is exchanged.
     */
    CHIP_ERROR StartDownload(chip::ScopedNodeId peer, std::string designator, std::string targetPath, uint64_t expectedSize);

    bool IsBusy() const { return mInitialized; }

    void AbortTransfer();

private:
    // Inherited from bdx::TransferFacilitator.
    void HandleTransferSessionOutput(chip::bdx::TransferSession::OutputEvent & event) override;

    // CASE session establishment callbacks.
    static void OnDeviceConnected(void * context, chip::Messaging::ExchangeManager & exchangeMgr,
                                  const chip::SessionHandle & sessionHandle);
    static void OnDeviceConnectionFailure(void * context, const chip::ScopedNodeId & peer, CHIP_ERROR error);

    CHIP_ERROR BeginTransfer(chip::Messaging::ExchangeManager & exchangeMgr, const chip::SessionHandle & sessionHandle);
    void Reset();

    chip::Callback::Callback<chip::OnDeviceConnected> mOnConnected;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnConnectionFailure;

    chip::ScopedNodeId mPeer;
    std::string mDesignator;
    std::string mTargetPath;
    uint64_t mExpectedSize = 0;
    uint64_t mBytesWritten = 0;
    std::ofstream mOutFile;
    bool mInitialized = false;
};

} // namespace core
} // namespace casting
} // namespace matter
