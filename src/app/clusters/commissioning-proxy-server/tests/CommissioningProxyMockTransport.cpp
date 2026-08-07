/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
#include <app/clusters/commissioning-proxy-server/tests/CommissioningProxyMockTransport.h>

#include <clusters/CommissioningProxy/Commands.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

using Status     = Protocols::InteractionModel::Status;
using ScanResult = Structs::ScanResultStruct::Type;

Status CommissioningProxyMockTransport::Connect(app::CommandHandler * commandObj, const DataModel::InvokeRequest & request,
                                                uint16_t discriminator, uint16_t timeout)
{
    // A forced non-success result models a transport that could not connect: no
    // session is registered and no response is sent (the cluster surfaces the status).
    if (mConnectStatus != Status::Success)
    {
        return mConnectStatus;
    }

    uint16_t sessionId = mHost->Sessions().AllocSessionId();
    mHost->Sessions().RegisterSession(sessionId, mType, request.subjectDescriptor.fabricIndex);
    mLastSessionId = sessionId;

    Commands::ProxyConnectResponse::Type response;
    response.sessionID = sessionId;
    commandObj->AddResponse(request.path, response);

    // Mirror the real transports' success path: the link is up, so the cluster is
    // connected. SetCPState only ever returns CHIP_NO_ERROR here.
    LogErrorOnFailure(mHost->SetCPState(CommissioningProxyCluster::kState_CPConnected));
    return Status::Success;
}

CHIP_ERROR CommissioningProxyMockTransport::SendMessage(uint16_t sessionId, System::PacketBufferHandle && buf)
{
    if (mSendMessageError != CHIP_NO_ERROR)
    {
        return mSendMessageError;
    }

    // Simulate the commissionee never replying: the response timeout fires and the
    // pending ProxyMessageRequest resolves with TIMEOUT.
    if (mSendMessageTimeout)
    {
        mHost->Sessions().DispatchMessageFailure(sessionId, Status::Timeout);
        return CHIP_NO_ERROR;
    }

    // Simulate an immediate commissionee reply (null message) so a pending
    // ProxyMessageRequest completes synchronously.
    if (mAutoRespond)
    {
        mHost->Sessions().DispatchMessageResponse(sessionId, nullptr, 0);
    }

    return CHIP_NO_ERROR;
}

Status CommissioningProxyMockTransport::Scan(uint8_t scanMaxTime)
{
    if (mScanStatus != Status::Success)
    {
        return mScanStatus;
    }

    static const uint8_t kAddr1[] = { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x01 };
    static const uint8_t kAddr2[] = { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x02 };
    static const uint8_t kExt1[]  = { 0x10, 0x20, 0x30 };

    std::array<ScanResult, 2> results{};
    results[0].address       = DataModel::Nullable<ByteSpan>(ByteSpan(kAddr1, sizeof(kAddr1)));
    results[0].discriminator = 3840;
    results[0].vendorID      = static_cast<VendorId>(0x1234);
    results[0].productID     = 0x0001;
    results[0].extendedData  = DataModel::Nullable<ByteSpan>(ByteSpan(kExt1, sizeof(kExt1)));
    results[0].transport.Set(mType);

    results[1].address       = DataModel::Nullable<ByteSpan>(ByteSpan(kAddr2, sizeof(kAddr2)));
    results[1].discriminator = 1234;
    results[1].vendorID      = static_cast<VendorId>(0x1234);
    results[1].productID     = 0x0002;
    results[1].extendedData  = DataModel::Nullable<ByteSpan>();
    results[1].transport.Set(mType);

    // The aggregator deep-copies, so the local backing storage need not outlive this.
    // When mAutoContribute is false the scan is left in-flight (no Contribute), so a
    // second ProxyScanRequest sees the aggregator busy.
    if (mAutoContribute)
    {
        mHost->ScanAggregator().Contribute(Span<const ScanResult>(results.data(), results.size()));
    }
    return Status::Success;
}

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
