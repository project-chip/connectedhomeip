/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/commissioning-proxy-server/CommissioningProxyCluster.h>
#include <app/clusters/commissioning-proxy-server/tests/CommissioningProxyMockDelegate.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

using chip::ByteSpan;
using chip::Span;
using chip::BitMask;
using chip::app::DataModel::List;
using chip::app::DataModel::Nullable;
namespace CP = chip::app::Clusters::CommissioningProxy;
using ScanResult = CP::Structs::ScanResultStruct::Type;

CommissioningProxyMockDelegate::CommissioningProxyMockDelegate() = default;
CommissioningProxyMockDelegate::~CommissioningProxyMockDelegate() = default;

Protocols::InteractionModel::Status CommissioningProxyMockDelegate::ProxyConnectRequest(
                        DataModel::Nullable<chip::ByteSpan> address,
                        chip::app::Clusters::CommissioningProxy::CapabilitiesBitmap transport,
                        uint16_t discriminator,
                        chip::VendorId vendorid,
                        uint16_t productid,
                        uint16_t timeout,
                        chip::app::Clusters::CommissioningProxy::WiFiBandBitmap wiFiBand,
                        app::CommandHandler * commandObj,
                        const DataModel::InvokeRequest & request)
{
    // Send the ProxyConnectResponse synchronously (mock simulates immediate success).
    CP::Commands::ProxyConnectResponse::Type response;
    response.sessionId = 1;
    commandObj->AddResponse(request.path, response);

    // Transition cluster state to connected, as OnPafConnectSuccess would do in production.
    if (mServer != nullptr)
    {
        CHIP_ERROR err = mServer->SetCPState(CommissioningProxyCluster::kState_CPConnected);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Test, "MockDelegate: SetCPState(Connected) failed: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }

    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status CommissioningProxyMockDelegate::ProxyScanRequest(
    CapabilitiesBitmap transport, WiFiBandBitmap wiFiBands, app::CommandHandler * commandObj,
    const DataModel::InvokeRequest & request)
{
    // Bytes must live long enough
    static const uint8_t kAddr1[] = { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x01 };
    static const uint8_t kAddr2[] = { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x02 };
    static const uint8_t kExt1[]  = { 0x10, 0x20, 0x30 };
    static std::array<ScanResult, 2> results;

    results[0] = ScanResult{};
    results[0].address       = Nullable<ByteSpan>(ByteSpan(kAddr1, sizeof(kAddr1)));
    results[0].discriminator = 3840;
    results[0].vendorId      = chip::VendorId(0x1234);
    results[0].productId     = 0x0001;
    results[0].extendedData  = Nullable<ByteSpan>(ByteSpan(kExt1, sizeof(kExt1)));
    results[0].transport.Set(CP::CapabilitiesBitmap::kWiFiPAF);
    results[0].wiFiBand.SetValue(BitMask<WiFiBandBitmap>(CP::WiFiBandBitmap::k2g4));

    results[1] = ScanResult{};
    results[1].address       = Nullable<ByteSpan>(ByteSpan(kAddr2, sizeof(kAddr2)));
    results[1].discriminator = 1234;
    results[1].vendorId      = chip::VendorId(0x1234);
    results[1].productId     = 0x0002;
    results[1].extendedData = Nullable<ByteSpan>();
    results[1].transport.Set(CP::CapabilitiesBitmap::kWiFiPAF);
    results[1].wiFiBand.ClearValue();

    // Build response
    CP::Commands::ProxyScanResponse::Type response;
    response.proxyScanResult =
        List<const ScanResult>(Span<const ScanResult>(results.data(), results.size()));
    response.numberOfResults = static_cast<uint8_t>(results.size());
    commandObj->AddResponse(request.path, response);

    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status CommissioningProxyMockDelegate::ProxyMessageRequest(
    uint16_t sessionId, chip::Optional<chip::ByteSpan> message, uint8_t responseTimeout,
    app::CommandHandler * commandObj, const DataModel::InvokeRequest & request)
{
    // Mock: echo back with a null message response.
    CP::Commands::ProxyMessageResponse::Type response;
    response.sessionId = sessionId;
    response.message.SetNull();
    commandObj->AddResponse(request.path, response);

    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status CommissioningProxyMockDelegate::ProxyDisconnectRequest(uint16_t sessionId)
{
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status CommissioningProxyMockDelegate::ProxyBackgroundScanStartRequest(
    CP::CapabilitiesBitmap transport, uint16_t timeout, CP::WiFiBandBitmap wiFiBands,
    chip::FabricIndex fabricIndex, chip::NodeId nodeId,
    app::CommandHandler * commandObj, const DataModel::InvokeRequest & request)
{
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status CommissioningProxyMockDelegate::ProxyBackgroundScanStopRequest(
    CP::CapabilitiesBitmap transport, CP::WiFiBandBitmap wiFiBands,
    chip::FabricIndex fabricIndex, chip::NodeId nodeId)
{
    return Protocols::InteractionModel::Status::Success;
}

uint8_t CommissioningProxyMockDelegate::GetScanMaxTime()
{
    return mScanMaxTime;
}

void CommissioningProxyMockDelegate::SetScanMaxTime(uint8_t seconds)
{
    mScanMaxTime = seconds;
}

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
