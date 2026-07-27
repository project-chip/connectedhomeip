/*
 *    Copyright (c) 2021-2025 Project CHIP Authors
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

#include <app/data-model/EncodableToTLV.h>
#include <clusters/NetworkCommissioning/Enums.h>
#include <platform/NetworkCommissioning.h>

namespace chip {
namespace app {
namespace Clusters {
namespace NetworkCommissioning {

/// Handles encoding a WifiScanResponseIterator into a TLV response structure
class WifiScanResponseToTLV : public chip::app::DataModel::EncodableToTLV
{
public:
    /// Object will iterate over `networks` during `EncodeTo`
    /// However it does NOT take ownership (expects caller to manage release)
    WifiScanResponseToTLV(NetworkCommissioningStatusEnum status, CharSpan debugText,
                          DeviceLayer::NetworkCommissioning::WiFiScanResponseIterator * networks) :
        mStatus(status),
        mDebugText(debugText), mNetworks(networks)
    {}

    CHIP_ERROR EncodeTo(TLV::TLVWriter & writer, TLV::Tag tag) const override;

private:
    NetworkCommissioningStatusEnum mStatus;
    CharSpan mDebugText;

    // User of class will release mNetworks, this class only uses it for
    // iterating
    DeviceLayer::NetworkCommissioning::WiFiScanResponseIterator * mNetworks;
};

} // namespace NetworkCommissioning
} // namespace Clusters
} // namespace app
} // namespace chip
