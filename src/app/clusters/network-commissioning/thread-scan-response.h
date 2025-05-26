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

/// Handles encoding a ThreadScanResponseIterator into a TLV response structure.
class ThreadScanResponseToTLV : public chip::app::DataModel::EncodableToTLV
{
public:
    ThreadScanResponseToTLV(NetworkCommissioningStatusEnum status, CharSpan debugText,
                            DeviceLayer::NetworkCommissioning::ThreadScanResponseIterator * networks) :
        mStatus(status),
        mDebugText(debugText), mNetworks(networks)
    {}

    CHIP_ERROR EncodeTo(TLV::TLVWriter & writer, TLV::Tag tag, FabricIndex aAccessingFabricIndex) const override
    {
        return EncodeTo(writer, tag);
    }

    CHIP_ERROR EncodeTo(TLV::TLVWriter & writer, TLV::Tag tag) const override;

private:
    using ThreadScanResponse = DeviceLayer::NetworkCommissioning::ThreadScanResponse;

    NetworkCommissioningStatusEnum mStatus;
    CharSpan mDebugText;
    DeviceLayer::NetworkCommissioning::ThreadScanResponseIterator * mNetworks;

    /// Fills up scanResponseArray with valid and de-duplicated thread responses from mNetworks.
    /// Handles sorting and keeping only larger rssi
    ///
    /// Returns the valid list of scan responses into `validResponses`, which is only valid
    /// as long as scanResponseArray is valid.
    CHIP_ERROR LoadResponses(Platform::ScopedMemoryBuffer<ThreadScanResponse> & scanResponseArray,
                             Span<ThreadScanResponse> & validResponses) const;
};

} // namespace NetworkCommissioning
} // namespace Clusters
} // namespace app
} // namespace chip
