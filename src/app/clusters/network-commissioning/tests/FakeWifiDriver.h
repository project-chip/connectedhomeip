/*
 *    Copyright (c) 2025 Project CHIP Authors
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
#include "clusters/NetworkCommissioning/Enums.h"
#include "lib/core/CHIPError.h"
#include <platform/NetworkCommissioning.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace Testing {

class FakeWiFiDriver : public DeviceLayer::NetworkCommissioning::WiFiDriver
{
public:
    using NetworkCommissioningStatusEnum = app::Clusters::NetworkCommissioning::NetworkCommissioningStatusEnum;

    // WiFiDriver
    NetworkCommissioningStatusEnum AddOrUpdateNetwork(ByteSpan ssid, ByteSpan credentials, MutableCharSpan & outDebugText,
                                                      uint8_t & outNetworkIndex) override
    {
        return mAddOrUpdateStatus;
    }
    void ScanNetworks(ByteSpan ssid, ScanCallback * callback) override {}

    // Internal::WirelessDriver
    CHIP_ERROR CommitConfiguration() override { return CHIP_NO_ERROR; }
    CHIP_ERROR RevertConfiguration() override { return CHIP_NO_ERROR; }

    uint8_t GetScanNetworkTimeoutSeconds() override { return 2; }
    uint8_t GetConnectNetworkTimeoutSeconds() override { return 2; }

    void SetAddOrUpdateNetworkReturn(NetworkCommissioningStatusEnum value) { mAddOrUpdateStatus = value; }

    NetworkCommissioningStatusEnum RemoveNetwork(ByteSpan networkId, MutableCharSpan & outDebugText,
                                                 uint8_t & outNetworkIndex) override
    {
        return NetworkCommissioningStatusEnum::kSuccess;
    }
    NetworkCommissioningStatusEnum ReorderNetwork(ByteSpan networkId, uint8_t index, MutableCharSpan & outDebugText) override
    {
        return NetworkCommissioningStatusEnum::kSuccess;
    }

    void ConnectNetwork(ByteSpan networkId, ConnectCallback * callback) override
    {
        callback->OnResult(NetworkCommissioningStatusEnum::kSuccess, ""_span, 0);
    }

    // BaseDriver
    uint8_t GetMaxNetworks() override { return 0; }
    DeviceLayer::NetworkCommissioning::NetworkIterator * GetNetworks() override { return nullptr; }

private:
    NetworkCommissioningStatusEnum mAddOrUpdateStatus = NetworkCommissioningStatusEnum::kUnknownError;
};

} // namespace Testing
} // namespace chip
