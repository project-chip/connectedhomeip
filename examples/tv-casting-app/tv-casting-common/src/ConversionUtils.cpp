/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "ConversionUtils.h"

CHIP_ERROR ConvertToDiscoveredNodeData(TargetVideoPlayerInfo * inPlayer, chip::Dnssd::DiscoveredNodeData & outDiscNodeData)
{
    if (inPlayer == nullptr)
        return CHIP_ERROR_INVALID_ARGUMENT;

    outDiscNodeData.Set<chip::Dnssd::CommissionNodeData>();
    auto & outNodeData = outDiscNodeData.Get<chip::Dnssd::CommissionNodeData>();

    outNodeData.vendorId   = inPlayer->GetVendorId();
    outNodeData.productId  = static_cast<uint16_t>(inPlayer->GetProductId());
    outNodeData.deviceType = inPlayer->GetDeviceType();
    outNodeData.numIPs     = inPlayer->GetNumIPs();

    const chip::Inet::IPAddress * ipAddresses = inPlayer->GetIpAddresses();
    if (ipAddresses != nullptr)
    {
        for (size_t i = 0; i < outNodeData.numIPs && i < chip::Dnssd::CommonResolutionData::kMaxIPAddresses; i++)
        {
            outNodeData.ipAddress[i] = ipAddresses[i];
        }
    }

    chip::Platform::CopyString(outNodeData.deviceName, chip::Dnssd::kMaxDeviceNameLen + 1, inPlayer->GetDeviceName());
    chip::Platform::CopyString(outNodeData.hostName, chip::Dnssd::kHostNameMaxLength + 1, inPlayer->GetHostName());

    return CHIP_NO_ERROR;
}
