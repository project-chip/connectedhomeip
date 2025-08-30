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
#include "ThreadScanResponse.h"

#include "constants.h"
#include <clusters/NetworkCommissioning/Commands.h>
#include <clusters/NetworkCommissioning/Structs.h>
#include <lib/support/SortUtils.h>

using chip::DeviceLayer::NetworkCommissioning::ThreadScanResponse;

namespace chip {
namespace app {
namespace Clusters {
namespace NetworkCommissioning {

namespace {

/// Fills up scanResponseArray with valid and de-duplicated thread responses from mNetworks.
/// Handles sorting and keeping only larger rssi
///
/// Returns the valid list of scan responses into `validResponses`, which is only valid
/// as long as scanResponseArray is valid.
CHIP_ERROR LoadResponses(DeviceLayer::NetworkCommissioning::ThreadScanResponseIterator * networks,
                         Platform::ScopedMemoryBuffer<ThreadScanResponse> & scanResponseArray,
                         Span<ThreadScanResponse> & validResponses)
{
    VerifyOrReturnError(scanResponseArray.Alloc(std::min(networks == nullptr ? 0 : networks->Count(), kMaxNetworksInScanResponse)),
                        CHIP_ERROR_NO_MEMORY);

    ThreadScanResponse scanResponse;
    size_t scanResponseArrayLength = 0;
    for (; networks != nullptr && networks->Next(scanResponse);)
    {
        if ((scanResponseArrayLength == kMaxNetworksInScanResponse) &&
            (scanResponseArray[scanResponseArrayLength - 1].rssi > scanResponse.rssi))
        {
            continue;
        }

        bool isDuplicated = false;

        for (size_t i = 0; i < scanResponseArrayLength; i++)
        {
            if ((scanResponseArray[i].panId == scanResponse.panId) &&
                (scanResponseArray[i].extendedPanId == scanResponse.extendedPanId))
            {
                if (scanResponseArray[i].rssi < scanResponse.rssi)
                {
                    scanResponseArray[i] = scanResponseArray[--scanResponseArrayLength];
                }
                else
                {
                    isDuplicated = true;
                }
                break;
            }
        }

        if (isDuplicated)
        {
            continue;
        }

        if (scanResponseArrayLength < kMaxNetworksInScanResponse)
        {
            scanResponseArrayLength++;
        }
        scanResponseArray[scanResponseArrayLength - 1] = scanResponse;

        // TODO: this is a sort (insertion sort even, so O(n^2)) in a O(n) loop.
        ///      There should be some better alternatives to not have some O(n^3) processing complexity.
        Sorting::InsertionSort(scanResponseArray.Get(), scanResponseArrayLength,
                               [](const ThreadScanResponse & a, const ThreadScanResponse & b) -> bool { return a.rssi > b.rssi; });
    }

    validResponses = Span<ThreadScanResponse>(scanResponseArray.Get(), scanResponseArrayLength);

    return CHIP_NO_ERROR;
}
} // namespace

CHIP_ERROR ThreadScanResponseToTLV::EncodeTo(TLV::TLVWriter & writer, TLV::Tag tag) const
{
    Platform::ScopedMemoryBuffer<ThreadScanResponse> responseArray;
    Span<ThreadScanResponse> responseSpan;

    ReturnErrorOnFailure(LoadResponses(mNetworks, responseArray, responseSpan));

    TLV::TLVType outerType;
    ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outerType));

    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(Commands::ScanNetworksResponse::Fields::kNetworkingStatus), mStatus));
    if (mDebugText.size() != 0)
    {
        ReturnErrorOnFailure(
            DataModel::Encode(writer, TLV::ContextTag(Commands::ScanNetworksResponse::Fields::kDebugText), mDebugText));
    }

    {
        TLV::TLVType listContainerType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::ContextTag(Commands::ScanNetworksResponse::Fields::kThreadScanResults),
                                                   TLV::kTLVType_Array, listContainerType));

        for (const ThreadScanResponse & response : responseSpan)
        {
            Structs::ThreadInterfaceScanResultStruct::Type result;
            uint8_t extendedAddressBuffer[Thread::kSizeExtendedPanId];

            Encoding::BigEndian::Put64(extendedAddressBuffer, response.extendedAddress);
            result.panId           = response.panId;
            result.extendedPanId   = response.extendedPanId;
            result.networkName     = CharSpan(response.networkName, response.networkNameLen);
            result.channel         = response.channel;
            result.version         = response.version;
            result.extendedAddress = ByteSpan(extendedAddressBuffer);
            result.rssi            = response.rssi;
            result.lqi             = response.lqi;

            ReturnErrorOnFailure(DataModel::Encode(writer, TLV::AnonymousTag(), result));
        }

        ReturnErrorOnFailure(writer.EndContainer(listContainerType));
    }

    return writer.EndContainer(outerType);
}

} // namespace NetworkCommissioning
} // namespace Clusters
} // namespace app
} // namespace chip
