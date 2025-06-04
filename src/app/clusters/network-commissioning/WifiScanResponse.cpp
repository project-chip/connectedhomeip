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
#include "WifiScanResponse.h"

#include "constants.h"
#include <clusters/NetworkCommissioning/Commands.h>
#include <clusters/NetworkCommissioning/Structs.h>

using chip::DeviceLayer::NetworkCommissioning::WiFiScanResponse;

namespace chip {
namespace app {
namespace Clusters {
namespace NetworkCommissioning {

CHIP_ERROR WifiScanResponseToTLV::EncodeTo(TLV::TLVWriter & writer, TLV::Tag tag) const
{
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
        ReturnErrorOnFailure(writer.StartContainer(TLV::ContextTag(Commands::ScanNetworksResponse::Fields::kWiFiScanResults),
                                                   TLV::kTLVType_Array, listContainerType));

        if ((mStatus == NetworkCommissioningStatusEnum::kSuccess) && (mNetworks != nullptr))
        {
            WiFiScanResponse scanResponse;
            size_t networksEncoded = 0;
            while (mNetworks->Next(scanResponse))
            {
                Structs::WiFiInterfaceScanResultStruct::Type result;
                result.security = scanResponse.security;
                result.ssid     = ByteSpan(scanResponse.ssid, scanResponse.ssidLen);
                result.bssid    = ByteSpan(scanResponse.bssid, sizeof(scanResponse.bssid));
                result.channel  = scanResponse.channel;
                result.wiFiBand = scanResponse.wiFiBand;
                result.rssi     = scanResponse.rssi;
                ReturnErrorOnFailure(DataModel::Encode(writer, TLV::AnonymousTag(), result));

                ++networksEncoded;
                if (networksEncoded >= kMaxNetworksInScanResponse)
                {
                    break;
                }
            }
        }

        ReturnErrorOnFailure(writer.EndContainer(listContainerType));
    }

    return writer.EndContainer(outerType);
}

} // namespace NetworkCommissioning
} // namespace Clusters
} // namespace app
} // namespace chip
