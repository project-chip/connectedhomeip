/**
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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
#import <Foundation/Foundation.h>

#import "DiscoveredNodeDataConverter.hpp"

@implementation DiscoveredNodeDataConverter

+ (DiscoveredNodeData *)convertToObjC:(const chip::Dnssd::DiscoveredNodeData *)chipDiscoveredNodeData
{
    DiscoveredNodeData * objCDiscoveredNodeData = [DiscoveredNodeData new];

    // from CommissionNodeData
    objCDiscoveredNodeData.deviceType = chipDiscoveredNodeData->commissionData.deviceType;
    objCDiscoveredNodeData.vendorId = chipDiscoveredNodeData->commissionData.vendorId;
    objCDiscoveredNodeData.productId = chipDiscoveredNodeData->commissionData.productId;
    objCDiscoveredNodeData.longDiscriminator = chipDiscoveredNodeData->commissionData.longDiscriminator;
    objCDiscoveredNodeData.commissioningMode = chipDiscoveredNodeData->commissionData.commissioningMode;
    objCDiscoveredNodeData.pairingHint = chipDiscoveredNodeData->commissionData.pairingHint;
    objCDiscoveredNodeData.deviceName = [NSString stringWithCString:chipDiscoveredNodeData->commissionData.deviceName
                                                           encoding:NSASCIIStringEncoding];
    objCDiscoveredNodeData.rotatingIdLen = chipDiscoveredNodeData->commissionData.rotatingIdLen;
    objCDiscoveredNodeData.rotatingId = chipDiscoveredNodeData->commissionData.rotatingId;
    objCDiscoveredNodeData.instanceName = [NSString stringWithCString:chipDiscoveredNodeData->commissionData.instanceName
                                                             encoding:NSASCIIStringEncoding];

    // from CommonResolutionData
    objCDiscoveredNodeData.port = chipDiscoveredNodeData->resolutionData.port;
    objCDiscoveredNodeData.hostName = [NSString stringWithCString:chipDiscoveredNodeData->resolutionData.hostName
                                                         encoding:NSASCIIStringEncoding];
    objCDiscoveredNodeData.platformInterface = chipDiscoveredNodeData->resolutionData.interfaceId.GetPlatformInterface();
    objCDiscoveredNodeData.numIPs = chipDiscoveredNodeData->resolutionData.numIPs;
    if (chipDiscoveredNodeData->resolutionData.numIPs > 0) {
        objCDiscoveredNodeData.ipAddresses = [NSMutableArray new];
    }
    for (int i = 0; i < chipDiscoveredNodeData->resolutionData.numIPs; i++) {
        char addrCString[chip::Inet::IPAddress::kMaxStringLength];
        chipDiscoveredNodeData->resolutionData.ipAddress->ToString(addrCString, chip::Inet::IPAddress::kMaxStringLength);
        objCDiscoveredNodeData.ipAddresses[i] = [NSString stringWithCString:addrCString encoding:NSASCIIStringEncoding];
    }
    return objCDiscoveredNodeData;
}

@end
